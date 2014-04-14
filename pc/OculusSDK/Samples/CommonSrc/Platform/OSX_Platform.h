/************************************************************************************


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

#include "../Platform/Platform.h"
#include "../Render/Render_GL_Device.h"

namespace OVR { namespace Platform { namespace OSX {

class PlatformCore : public Platform::PlatformCore
{
public:
    void*        Win;
    void*        View;
    void*        NsApp;
    bool         Quit;
    int          ExitCode;
    int          Width, Height;
    MouseMode    MMode;
    
    void RunIdle();

public:
    PlatformCore(Application* app, void* nsapp);
    ~PlatformCore();

    bool      SetupWindow(int w, int h);
    void      Exit(int exitcode);

    RenderDevice* SetupGraphics(const SetupGraphicsDeviceSet& setupGraphicsDesc,
                                const char* gtype, const Render::RendererParams& rp);

    void      SetMouseMode(MouseMode mm);
    void      GetWindowSize(int* w, int* h) const;

    void      SetWindowTitle(const char*title);

    void      ShowWindow(bool show);
    void      DestroyWindow();
    bool      SetFullscreen(const Render::RendererParams& rp, int fullscreen);
    int       GetDisplayCount();
    Render::DisplayId GetDisplay(int screen);

    String    GetContentDirectory() const;
};

}}
namespace Render { namespace GL { namespace OSX {
        
class RenderDevice : public Render::GL::RenderDevice
{
public:
    void* Context;

    RenderDevice(const Render::RendererParams& p, void* context)
    : GL::RenderDevice(p), Context(context) {}
            
    virtual void Shutdown();
    virtual void Present();

    virtual bool SetFullscreen(DisplayMode fullscreen);
    
    // oswnd = X11::PlatformCore*
    static Render::RenderDevice* CreateDevice(const RendererParams& rp, void* oswnd);
};
        
}}}}


// OVR_PLATFORM_APP_ARGS specifies the Application class to use for startup,
// providing it with startup arguments.
#define OVR_PLATFORM_APP_ARGS(AppClass, args)                                            \
OVR::Platform::Application* OVR::Platform::Application::CreateApplication()          \
{ OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));                \
return new AppClass args; }                                                        \
void OVR::Platform::Application::DestroyApplication(OVR::Platform::Application* app) \
{ OVR::Platform::PlatformCore* platform = app->pPlatform;                            \
delete app; delete platform; OVR::System::Destroy(); };

// OVR_PLATFORM_APP_ARGS specifies the Application startup class with no args.
#define OVR_PLATFORM_APP(AppClass) OVR_PLATFORM_APP_ARGS(AppClass, ())


