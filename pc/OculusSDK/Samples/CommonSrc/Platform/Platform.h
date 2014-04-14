/************************************************************************************

Filename    :   Platform.h
Content     :   Platform-independent app and rendering framework for Oculus samples
Created     :   September 6, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_Platform_h
#define OVR_Platform_h

#include "OVR.h"

#include "Kernel/OVR_KeyCodes.h"

namespace OVR { namespace Render {
    class RenderDevice;
    struct DisplayId;
    struct RendererParams;
}}

namespace OVR { namespace Platform {

using Render::RenderDevice;

class PlatformCore;
class Application;
class GamepadManager;

// MouseMode configures mouse input behavior of the app. Three states are
// currently supported:
//   Normal          - Reports absolute coordinates with cursor shown.
//   Relative        - Reports relative delta coordinates with cursor hidden
//                     until 'Esc' key is pressed or window loses focus.
//   RelativeEscaped - Relative input is desired, but has been escaped until
//                     mouse is clicked in the window, which will return the state
//                     to relative. Absolute coordinates are reported.

enum MouseMode
{
    Mouse_Normal,
    Mouse_Relative,        // Cursor hidden, mouse grab, OnMouseMove reports relative deltas.
    Mouse_RelativeEscaped, // Clicking in window will return to Relative state.
};


enum Modifiers
{
    Mod_Shift       = 0x001,
    Mod_Control     = 0x002,
    Mod_Meta        = 0x004,
    Mod_Alt         = 0x008,

    // Set for input Mouse_Relative mode, indicating that x,y are relative deltas.
    Mod_MouseRelative = 0x100,
};

//-------------------------------------------------------------------------------------
// ***** SetupGraphicsDeviceSet

typedef RenderDevice* (*RenderDeviceCreateFunc)(const Render::RendererParams&, void*);

// SetupGraphicsDeviceSet is a PlatformCore::SetupGraphics initialization helper class,
// used to build up a list of RenderDevices that can be used for rendering.
// Specifying a smaller set allows application to avoid linking unused graphics devices.
struct SetupGraphicsDeviceSet
{    
    SetupGraphicsDeviceSet(const char* typeArg, RenderDeviceCreateFunc createFunc)
        : pTypeArg(typeArg), pCreateDevice(createFunc), pNext(0) { }
    SetupGraphicsDeviceSet(const char* typeArg, RenderDeviceCreateFunc createFunc,
                           const SetupGraphicsDeviceSet& next)
        : pTypeArg(typeArg), pCreateDevice(createFunc), pNext(&next) { }

    // Selects graphics object based on type string; returns 'this' if not found.
    const SetupGraphicsDeviceSet* PickSetupDevice(const char* typeArg) const;

    const char*               pTypeArg;
    RenderDeviceCreateFunc    pCreateDevice;        

private:
    const SetupGraphicsDeviceSet*  pNext;
};

//-------------------------------------------------------------------------------------
// ***** PlatformCore

// PlatformCore defines abstract system window/viewport setup functionality and
// maintains a renderer. This class is separated from Application because it can have
// derived platform-specific implementations.
// Specific implementation classes are hidden within platform-specific versions
// such as Win32::PlatformCore.

class PlatformCore : public NewOverrideBase
{
protected:
    Application*        pApp;
    Ptr<RenderDevice>   pRender;
    Ptr<GamepadManager> pGamepadManager;
    UInt64              StartupTicks; 

public:
    PlatformCore(Application *app);
    virtual ~PlatformCore() { }
    Application*    GetApp() { return pApp; }
    RenderDevice*   GetRenderer() const { return pRender; }
    GamepadManager* GetGamepadManager() const { return pGamepadManager; }

    virtual bool    SetupWindow(int w, int h) = 0;
    // Destroys window and also releases renderer.
    virtual void    DestroyWindow() = 0;
    virtual void    Exit(int exitcode) = 0;

    virtual void    ShowWindow(bool visible) = 0;
    
    virtual bool    SetFullscreen(const Render::RendererParams& rp, int fullscreen);
   
    // Search for a matching graphics renderer based on type argument and initializes it.    
    virtual RenderDevice* SetupGraphics(const SetupGraphicsDeviceSet& setupGraphicsDesc,
                                        const char* gtype,
                                        const Render::RendererParams& rp) = 0;

    virtual void    SetMouseMode(MouseMode mm) { OVR_UNUSED(mm); }

    virtual void    GetWindowSize(int* w, int* h) const = 0;

    virtual void    SetWindowTitle(const char*title) = 0;
	virtual void	PlayMusicFile(const char *fileName) { OVR_UNUSED(fileName); }
    virtual int     GetDisplayCount() { return 0; }
    virtual Render::DisplayId GetDisplay(int screen);
    
    // Get time since start of application in seconds.
    double          GetAppTime() const; 
    
    virtual String  GetContentDirectory() const { return "."; }
};

//-------------------------------------------------------------------------------------
// PlatformApp is a base application class from which end-user application
// classes derive.

class Application : public NewOverrideBase
{
protected:
    class PlatformCore* pPlatform;

public:
    virtual ~Application() { }

    virtual int  OnStartup(int argc, const char** argv) = 0;
    virtual void OnQuitRequest() { pPlatform->Exit(0); }

    virtual void OnIdle() {}

    virtual void OnKey(KeyCode key, int chr, bool down, int modifiers)
    { OVR_UNUSED4(key, chr, down, modifiers); }
    virtual void OnMouseMove(int x, int y, int modifiers)
    { OVR_UNUSED3(x, y, modifiers); }

    virtual void OnResize(int width, int height)
    { OVR_UNUSED2(width, height); }

    void         SetPlatformCore(PlatformCore* p) { pPlatform = p; }
    PlatformCore* GetPlatformCore() const         { return pPlatform; }


    // Static functions defined by OVR_PLATFORM_APP and used to initialize and
    // shut down the application class.
    static Application* CreateApplication();
    static void         DestroyApplication(Application* app);
};


}}

#endif
