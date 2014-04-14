/************************************************************************************

Filename    :   Render_GL_Win32 Device.h
Content     :   Win32 OpenGL Device implementation header
Created     :   September 10, 2012
Authors     :   Andrew Reisse, Michael Antonov

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

#ifndef OVR_Render_GL_Win32_Device_h
#define OVR_Render_GL_Win32_Device_h

#include "Render_GL_Device.h"

#ifdef WIN32
#include <Windows.h>
#endif


namespace OVR { namespace Render { namespace GL { namespace Win32 {

// ***** GL::Win32::RenderDevice

// Win32-Specific GL Render Device, used to create OpenGL under Windows.
class RenderDevice : public GL::RenderDevice
{
    HWND   Window;
    HGLRC  WglContext;
    HDC    GdiDc;

    int PreFullscreenX, PreFullscreenY, PreFullscreenW, PreFullscreenH;

public:
    RenderDevice(const Render::RendererParams& p, HWND win, HDC dc, HGLRC gl)
        : GL::RenderDevice(p), Window(win), WglContext(gl), GdiDc(dc) { OVR_UNUSED(p); }

    // Implement static initializer function to create this class.
    static Render::RenderDevice* CreateDevice(const RendererParams& rp, void* oswnd);

    virtual bool SetFullscreen(DisplayMode fullscreen);

    virtual void Shutdown();
    virtual void Present();
};


}}}} // OVR::Render::GL::Win32

#endif
