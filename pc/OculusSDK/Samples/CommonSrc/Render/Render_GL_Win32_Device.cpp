/************************************************************************************

Filename    :   Render_GL_Win32 Device.cpp
Content     :   Win32 OpenGL Device implementation
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

#include "Render_GL_Win32_Device.h"

namespace OVR { namespace Render { namespace GL { namespace Win32 {


// ***** GL::Win32::RenderDevice

// Implement static initializer function to create this class.
Render::RenderDevice* RenderDevice::CreateDevice(const RendererParams& rp, void* oswnd)
{
    HWND hwnd = (HWND)oswnd;

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));

    pfd.nSize       = sizeof(pfd);
    pfd.nVersion    = 1;
    pfd.iPixelType  = PFD_TYPE_RGBA;
    pfd.dwFlags     = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.cColorBits  = 32;
    pfd.cDepthBits  = 16;

    HDC dc = GetDC(hwnd);
    int pf = ChoosePixelFormat(dc, &pfd);
    if (!pf)
    {
        ReleaseDC(hwnd, dc);
        return NULL;
    }
    if (!SetPixelFormat(dc, pf, &pfd))
    {
        ReleaseDC(hwnd, dc);
        return NULL;
    }
    HGLRC context = wglCreateContext(dc);
    if (!wglMakeCurrent(dc, context))
    {
        wglDeleteContext(context);
        ReleaseDC(hwnd, dc);
        return NULL;
    }

    InitGLExtensions();

    return new RenderDevice(rp, hwnd, dc, context);
   // return 0;
}


bool RenderDevice::SetFullscreen(DisplayMode fullscreen)
{
    if (fullscreen == Params.Fullscreen)
    {
        return true;
    }

    if (Params.Fullscreen == Display_FakeFullscreen)
    {
        SetWindowLong(Window, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS);
        SetWindowPos(Window, NULL, PreFullscreenX, PreFullscreenY,
                     PreFullscreenW, PreFullscreenH, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    // TBD: Need to initialize based on MonitorEnumFunc.
    int FSDesktopX = 0;
    int FSDesktopY = 0;

    if (fullscreen == Display_FakeFullscreen)
    {
        // Get WINDOWPLACEMENT before changing style to get OVERLAPPED coordinates,
        // which we will restore.
        WINDOWPLACEMENT wp;
        wp.length = sizeof(wp);
        GetWindowPlacement(Window, &wp);
        PreFullscreenW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
        PreFullscreenH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        PreFullscreenX = wp.rcNormalPosition.left;
        PreFullscreenY = wp.rcNormalPosition.top;
        // Warning: SetWindowLong sends message computed based on old size (incorrect).
        // A proper work-around would be to mask that message out during window frame change in Platform.
        SetWindowLong(Window, GWL_STYLE, WS_OVERLAPPED | WS_VISIBLE | WS_CLIPSIBLINGS);
        SetWindowPos(Window, NULL, FSDesktopX, FSDesktopY, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        // Relocate cursor into the window to avoid losing focus on first click.
        POINT oldCursor;
        if (GetCursorPos(&oldCursor) &&
            ((oldCursor.x < FSDesktopX) || (oldCursor.x > (FSDesktopX + PreFullscreenW)) ||
            (oldCursor.y < FSDesktopY) || (oldCursor.x > (FSDesktopY + PreFullscreenH))))
        {
            // TBD: FullScreen window logic should really be in platform; it causes world rotation
            // in relative mouse mode.
            ::SetCursorPos(FSDesktopX, FSDesktopY);
        }
    }
    else
    {
        /*
        HRESULT hr = SwapChain->SetFullscreenState(fullscreen, fullscreen ? FullscreenOutput : NULL);
        if (FAILED(hr))
        {
            return false;
        } */
    }

    Params.Fullscreen = fullscreen;
    return true;
}

void RenderDevice::Present()
{
    SwapBuffers(GdiDc);
}

void RenderDevice::Shutdown()
{
    if (WglContext)
    {
        wglMakeCurrent(NULL,NULL);
        wglDeleteContext(WglContext);
        ReleaseDC(Window, GdiDc);
        WglContext = NULL;
        GdiDc = NULL;
        Window = NULL;
    }
}

}}}}

