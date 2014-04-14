/************************************************************************************

Filename    :   Win32_Platform.cpp
Content     :   Win32 implementation of Platform app infrastructure
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

#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_String.h"

#include "Win32_Platform.h"
#include "Win32_Gamepad.h"
#include "../Render/Render_Device.h"

namespace OVR { namespace Platform { namespace Win32 {


PlatformCore::PlatformCore(Application* app, HINSTANCE hinst)
  : Platform::PlatformCore(app), hWnd(NULL), hInstance(hinst), Quit(0), MMode(Mouse_Normal),
    Cursor(0), Modifiers(0), WindowTitle("App")
{
    pGamepadManager = *new Win32::GamepadManager();
}

PlatformCore::~PlatformCore()
{
}

bool PlatformCore::SetupWindow(int w, int h)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpszClassName = L"OVRAppWindow";
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = systemWindowProc;
    wc.cbWndExtra    = sizeof(PlatformCore*);

    RegisterClass(&wc);

    Width = w;
    Height = h;
    RECT winSize;
    winSize.left = winSize.top = 0;
    winSize.right = Width;
    winSize.bottom = Height;
    AdjustWindowRect(&winSize, WS_OVERLAPPEDWINDOW, false);
    hWnd = CreateWindowA("OVRAppWindow", WindowTitle.ToCStr(), WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                        //  1950, 10,
                          winSize.right-winSize.left, winSize.bottom-winSize.top,
                          NULL, NULL, hInstance, (LPVOID)this);
    Modifiers = 0;

    Cursor = LoadCursor(NULL, IDC_CROSS);    

    // Initialize Window center in screen coordinates
    POINT center = { Width / 2, Height / 2 };
    ::ClientToScreen(hWnd, &center);
    WindowCenter = center;
    
    if (MMode == Mouse_Relative)
    {
        ::SetCursorPos(WindowCenter.x, WindowCenter.y);
        ShowCursor(FALSE);
    }
    ::SetFocus(hWnd);

    return (hWnd != NULL);
}

void PlatformCore::DestroyWindow()
{
    // Release renderer.
    pRender.Clear();

    // Release gamepad.
    pGamepadManager.Clear();

    // Release window resources.
    ::DestroyWindow(hWnd);
    UnregisterClass(L"OVRAppWindow", hInstance);
    hWnd = 0;
    Width = Height = 0;

    //DestroyCursor(Cursor);
    Cursor = 0;    
}

void PlatformCore::ShowWindow(bool visible)
{
    ::ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
}

void PlatformCore::SetMouseMode(MouseMode mm)
{
    if (mm == MMode)
        return;

    if (hWnd)
    {
        if (mm == Mouse_Relative)
        {
            ShowCursor(FALSE);
            ::SetCursorPos(WindowCenter.x, WindowCenter.y);
        }
        else
        {
            if (MMode == Mouse_Relative)
                ShowCursor(TRUE);
        }
    }
    MMode = mm;
}

void PlatformCore::GetWindowSize(int* w, int* h) const
{
    *w = Width;
    *h = Height;
}


void PlatformCore::SetWindowTitle(const char* title)
{
    WindowTitle = title;
    if (hWnd)
    ::SetWindowTextA(hWnd, title);
}

static UByte KeyMap[][2] = 
{
    { VK_BACK,      Key_Backspace },
    { VK_TAB,       Key_Tab },
    { VK_CLEAR,     Key_Clear },
    { VK_RETURN,    Key_Return },
    { VK_SHIFT,     Key_Shift },
    { VK_CONTROL,   Key_Control },
    { VK_MENU,      Key_Alt },
    { VK_PAUSE,     Key_Pause },
    { VK_CAPITAL,   Key_CapsLock },
    { VK_ESCAPE,    Key_Escape },
    { VK_SPACE,     Key_Space },
    { VK_PRIOR,     Key_PageUp },
    { VK_NEXT,      Key_PageDown },
    { VK_END,       Key_End },
    { VK_HOME,      Key_Home },
    { VK_LEFT,      Key_Left },
    { VK_UP,        Key_Up },
    { VK_RIGHT,     Key_Right },
    { VK_DOWN,      Key_Down },
    { VK_INSERT,    Key_Insert },
    { VK_DELETE,    Key_Delete },
    { VK_HELP,      Key_Help },

    { VK_NUMLOCK,   Key_NumLock },
    { VK_SCROLL,    Key_ScrollLock },

    { VK_OEM_1,     Key_Semicolon },
    { VK_OEM_PLUS,  Key_Equal },
    { VK_OEM_COMMA, Key_Comma },
    { VK_OEM_MINUS, Key_Minus },
    { VK_OEM_PERIOD,Key_Period },
    { VK_OEM_2,     Key_Slash },
    { VK_OEM_3,     Key_Bar },
    { VK_OEM_4,     Key_BracketLeft },
    { VK_OEM_5,     Key_Backslash },
    { VK_OEM_6,     Key_BracketRight },
    { VK_OEM_7,     Key_Quote },

    { VK_OEM_AX,	Key_OEM_AX },   //  'AX' key on Japanese AX keyboard.
    { VK_OEM_102,   Key_OEM_102 },  //  "<>" or "\|" on RT 102-key keyboard.
    { VK_ICO_HELP,  Key_ICO_HELP },
    { VK_ICO_00,	Key_ICO_00 }
};


KeyCode MapVKToKeyCode(unsigned vk)
{
    unsigned key = Key_None;

    if ((vk >= '0') && (vk <= '9'))
    {
        key = vk - '0' + Key_Num0;
    }
    else if ((vk >= 'A') && (vk <= 'Z'))
    {
        key = vk - 'A' + Key_A;
    }
    else if ((vk >= VK_NUMPAD0) && (vk <= VK_DIVIDE))
    {
        key = vk - VK_NUMPAD0 + Key_KP_0;
    }
    else if ((vk >= VK_F1) && (vk <= VK_F15))
    {
        key = vk - VK_F1 + Key_F1;
    }
    else 
    {
        for (unsigned i = 0; i< (sizeof(KeyMap) / sizeof(KeyMap[1])); i++)
        {
            if (vk == KeyMap[i][0])
            {                
                key = KeyMap[i][1];
                break;
            }
        }
    }

    return (KeyCode)key;
}



LRESULT CALLBACK PlatformCore::systemWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    PlatformCore* self;  

    // WM_NCCREATE should be the first message to come it; use it to set class pointer.
    if (msg == WM_NCCREATE)
    {
        self = static_cast<PlatformCore*>(((LPCREATESTRUCT)lp)->lpCreateParams);

        if (self)
        {
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)self);
            self->hWnd = hwnd;
        }
    }
    else
    {
        self = (PlatformCore*)(UPInt)GetWindowLongPtr(hwnd, 0);
    }
        
    return self ? self->WindowProc(msg, wp, lp) :
                  DefWindowProc(hwnd, msg, wp, lp);
}


LRESULT PlatformCore::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
    KeyCode keyCode;

    switch (msg)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        return 0;

    case WM_SETCURSOR:
        ::SetCursor(Cursor);
        return 0;

    case WM_MOUSEMOVE:
        if (MMode == Mouse_Relative)
        {
            POINT newPos = { LOWORD(lp), HIWORD(lp) };
            ::ClientToScreen(hWnd, &newPos);
            if ((newPos.x == WindowCenter.x) && (newPos.y == WindowCenter.y))
                break;
            ::SetCursorPos(WindowCenter.x, WindowCenter.y);

            LONG dx = newPos.x - WindowCenter.x;
            LONG dy = newPos.y - WindowCenter.y;
           
            pApp->OnMouseMove(dx, dy, Mod_MouseRelative);
        }
        else
        {
            pApp->OnMouseMove(LOWORD(lp), HIWORD(lp), 0);
        }
        break;

    case WM_MOVE:
        {
            RECT r;
            GetClientRect(hWnd, &r);
            WindowCenter.x = r.right/2;
            WindowCenter.y = r.bottom/2;
            ::ClientToScreen(hWnd, &WindowCenter);
        }
        break;

    case WM_KEYDOWN:        
        switch (wp)
        {
        case VK_CONTROL:        Modifiers |= Mod_Control; break;
        case VK_MENU:           Modifiers |= Mod_Alt; break;
        case VK_SHIFT:          Modifiers |= Mod_Shift; break;
        case VK_LWIN:
        case VK_RWIN:           Modifiers |= Mod_Meta; break;
        }
        if ((keyCode = MapVKToKeyCode((unsigned)wp)) != Key_None)
            pApp->OnKey(keyCode, 0, true, Modifiers);

        if (keyCode == Key_Escape && MMode == Mouse_Relative)
        {
            MMode = Mouse_RelativeEscaped;
            ShowCursor(TRUE);
        }
        break;

    case WM_KEYUP:
        if ((keyCode = MapVKToKeyCode((unsigned)wp)) != Key_None)
            pApp->OnKey(keyCode, 0, false, Modifiers);
        switch (wp)
        {
        case VK_CONTROL:        Modifiers &= ~Mod_Control; break;
        case VK_MENU:           Modifiers &= ~Mod_Alt; break;
        case VK_SHIFT:          Modifiers &= ~Mod_Shift; break;
        case VK_LWIN:
        case VK_RWIN:           Modifiers &= ~Mod_Meta; break;
        }
        break;

    case WM_LBUTTONDOWN:
        //App->OnMouseButton(0, 

        ::SetCapture(hWnd);

        if (MMode == Mouse_RelativeEscaped)
        {            
            ::SetCursorPos(WindowCenter.x, WindowCenter.y);
            ::ShowCursor(FALSE);
            MMode = Mouse_Relative;
        }
        break;

    case WM_LBUTTONUP:
        ReleaseCapture();
        break;

    case WM_SETFOCUS:
        // Do NOT restore the Relative mode here, since calling SetCursorPos
        // would screw up titlebar window dragging.
        // Let users click in the center instead to resume.        
        break;

    case WM_KILLFOCUS:
        if (MMode == Mouse_Relative)
        {            
            MMode = Mouse_RelativeEscaped;
            ShowCursor(TRUE);
        }
        break;

    case WM_SIZE:
        // Change window size as long as we're not being minimized. 
        if (wp != SIZE_MINIMIZED)
        {
        Width = LOWORD(lp);
        Height = HIWORD(lp);
        if (pRender)
            pRender->SetWindowSize(Width, Height);
        pApp->OnResize(Width,Height);
        }
        break;

    case WM_STYLECHANGING:
        // Resize the window. This is needed because the size includes any present system controls, and
        // windows does not adjust it when changing to fullscreen.
        {
            STYLESTRUCT* pss = (STYLESTRUCT*)lp;
            RECT winSize;
            winSize.left = winSize.top = 0;
            winSize.right = Width;
            winSize.bottom = Height;
            int w = winSize.right-winSize.left;
            int h = winSize.bottom-winSize.top;
            AdjustWindowRect(&winSize, pss->styleNew, false);
            ::SetWindowPos(hWnd, NULL, 0, 0, w, h, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
        }
        break;

    case WM_QUIT:
    case WM_CLOSE:
        pApp->OnQuitRequest();
        return false;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}

int PlatformCore::Run()
{
    while (!Quit)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            pApp->OnIdle();

            // Keep sleeping when we're minimized.
            if (IsIconic(hWnd))
            {
                Sleep(10);
        }
    }
    }

    return ExitCode;
}


RenderDevice* PlatformCore::SetupGraphics(const SetupGraphicsDeviceSet& setupGraphicsDesc,
                                          const char* type, const Render::RendererParams& rp)
{
    const SetupGraphicsDeviceSet* setupDesc = setupGraphicsDesc.PickSetupDevice(type);
    OVR_ASSERT(setupDesc);

    pRender = *setupDesc->pCreateDevice(rp, (void*)hWnd);
    if (pRender)
        pRender->SetWindowSize(Width, Height);

    ::ShowWindow(hWnd, SW_RESTORE);
    return pRender.GetPtr();
}


void PlatformCore::PlayMusicFile(const char *fileName)
{
	PlaySoundA(fileName, NULL, SND_FILENAME | SND_LOOP | SND_ASYNC); 
}


//-----------------------------------------------------------------------------

// Used to capture all the active monitor handles
struct MonitorSet
{
    enum { MaxMonitors = 8 };
    HMONITOR Monitors[MaxMonitors];
    int      MonitorCount;
    int      PrimaryCount;
};


BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
    MonitorSet* monitorSet = (MonitorSet*)dwData;
    if (monitorSet->MonitorCount > MonitorSet::MaxMonitors)
        return FALSE;

    monitorSet->Monitors[monitorSet->MonitorCount] = hMonitor;
    monitorSet->MonitorCount++;
    return TRUE;
};


// Returns the number of active screens for extended displays and 1 for mirrored display
int PlatformCore::GetDisplayCount()
{
    // Get all the monitor handles
    MonitorSet monitors;
    monitors.MonitorCount = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    // Count the primary monitors
    int primary = 0;
    MONITORINFOEX info;
    for (int m=0; m < monitors.MonitorCount; m++)
    {
        info.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(monitors.Monitors[m], &info);
        
        if (info.dwFlags & MONITORINFOF_PRIMARY)
           primary++;
    }

    if (primary > 1)
        return 1;                      // Regard mirrored displays as a single screen
    else
        return monitors.MonitorCount;  // Return all extended displays 
}

//-----------------------------------------------------------------------------
// Returns the device name for the given screen index or empty string for invalid index
// The zero index will always return the primary screen name
Render::DisplayId PlatformCore::GetDisplay(int screen)
{
    // Get all the monitor handles
    MonitorSet monitors;
    monitors.MonitorCount = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    String screen_name;

    // Get the name of the suppled screen index with the requirement
    // that screen 0 is the primary monitor
    int non_primary_count = 0;
    MONITORINFOEX info;
    for (int m=0; m < monitors.MonitorCount; m++)
    {
        info.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(monitors.Monitors[m], &info);
        
        if (info.dwFlags & MONITORINFOF_PRIMARY)
        {
            if (screen == 0)
            {
                screen_name = info.szDevice;
                break;
            }
        }
        else
        {
            non_primary_count++;
            if (screen == non_primary_count)
            {
                screen_name = info.szDevice;
                break;
            }
        }
    }

    return screen_name;
}


}}}

OVR::Platform::Application*     g_app;

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE prevInst, LPSTR inArgs, int show)
{
    using namespace OVR;
    using namespace OVR::Platform;

    OVR_UNUSED2(prevInst, show);
    
    // CreateApplication must be the first call since it does OVR::System::Initialize.
    g_app = Application::CreateApplication();
    Win32::PlatformCore* platform = new Win32::PlatformCore(g_app, hinst);
    // The platform attached to an app will be deleted by DestroyApplication.
    g_app->SetPlatformCore(platform);

    int exitCode = 0;

    // Nested scope for container destructors to shutdown before DestroyApplication.
    {
        Array<String>      args;
        Array<const char*> argv;
        argv.PushBack("app");

        const char* p = inArgs;
        const char* pstart = inArgs;
        while (*p)
        {
            if (*p == ' ')
            {
                args.PushBack(String(pstart, p - pstart));
                while (*p == ' ')
                    p++;
                pstart = p;
            }
            else
            {
                p++;
            }
        }
        if (p != pstart)
            args.PushBack(String(pstart, p - pstart));
        for (UPInt i = 0; i < args.GetSize(); i++)
            argv.PushBack(args[i].ToCStr());

        exitCode = g_app->OnStartup((int)argv.GetSize(), &argv[0]);
        if (!exitCode)
            exitCode = platform->Run();
    }

    // No OVR functions involving memory are allowed after this.
    Application::DestroyApplication(g_app);
    g_app = 0;

    OVR_DEBUG_STATEMENT(_CrtDumpMemoryLeaks());
    return exitCode;
}
