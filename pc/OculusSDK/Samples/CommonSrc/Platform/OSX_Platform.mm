
/***********************************************************************
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
************************************************************************/

#import "../Platform/OSX_PlatformObjc.h"

using namespace OVR;
using namespace OVR::Platform;

@implementation OVRApp

- (void)dealloc
{
    [super dealloc];
}

- (void)run
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    _running = YES;
    OVR::Platform::Application* app;
    {
        using namespace OVR;
        using namespace OVR::Platform;
        
        // CreateApplication must be the first call since it does OVR::System::Initialize.
        app = Application::CreateApplication();
        OSX::PlatformCore* platform = new OSX::PlatformCore(app, self);
        // The platform attached to an app will be deleted by DestroyApplication.
        app->SetPlatformCore(platform);
        
        [self setApp:app];
        [self setPlatform:platform];
        
        const char* argv[] = {"OVRApp"};
        int exitCode = app->OnStartup(1, argv);
        if (exitCode)
        {
            Application::DestroyApplication(app);
            exit(exitCode);
        }
    }
    [self finishLaunching];
    [pool drain];

    while ([self isRunning])
    {
        pool = [[NSAutoreleasePool alloc] init];
        NSEvent* event = [self nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (event)
        {
            [self sendEvent:event];
        }
        _App->OnIdle();
        [pool drain];
    }
    OVR::Platform::Application::DestroyApplication(app);
}

@end

static int KeyMap[][2] =
{
    { NSDeleteFunctionKey,      OVR::Key_Delete },
    { '\t',       OVR::Key_Tab },
    { '\n',    OVR::Key_Return },
    { NSPauseFunctionKey,     OVR::Key_Pause },
    { 27,      OVR::Key_Escape },
    { 127,     OVR::Key_Backspace },
    { ' ',     OVR::Key_Space },
    { NSPageUpFunctionKey,     OVR::Key_PageUp },
    { NSPageDownFunctionKey,      OVR::Key_PageDown },
    { NSNextFunctionKey,      OVR::Key_PageDown },
    { NSEndFunctionKey,       OVR::Key_End },
    { NSHomeFunctionKey,      OVR::Key_Home },
    { NSLeftArrowFunctionKey,      OVR::Key_Left },
    { NSUpArrowFunctionKey,        OVR::Key_Up },
    { NSRightArrowFunctionKey,     OVR::Key_Right },
    { NSDownArrowFunctionKey,      OVR::Key_Down },
    { NSInsertFunctionKey,    OVR::Key_Insert },
    { NSDeleteFunctionKey,    OVR::Key_Delete },
    { NSHelpFunctionKey,      OVR::Key_Insert },
};


static KeyCode MapToKeyCode(wchar_t vk)
{
    unsigned key = Key_None;
    
    if ((vk >= 'a') && (vk <= 'z'))
    {
        key = vk - 'a' + Key_A;
    }
    else if ((vk >= ' ') && (vk <= '~'))
    {
        key = vk;
    }
    else if ((vk >= '0') && (vk <= '9'))
    {
        key = vk - '0' + Key_Num0;
    }
    else if ((vk >= NSF1FunctionKey) && (vk <= NSF15FunctionKey))
    {
        key = vk - NSF1FunctionKey + Key_F1;
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

static int MapModifiers(unsigned long xmod)
{
    int mod = 0;
    if (xmod & NSShiftKeyMask)
        mod |= OVR::Platform::Mod_Shift;
    if (xmod & NSCommandKeyMask)
        mod |= OVR::Platform::Mod_Control;
    if (xmod & NSAlternateKeyMask)
        mod |= OVR::Platform::Mod_Alt;
    if (xmod & NSControlKeyMask)
        mod |= OVR::Platform::Mod_Meta;
    return mod;
}

@implementation OVRView

-(BOOL) acceptsFirstResponder
{
    return YES;
}
-(BOOL) acceptsFirstMouse:(NSEvent *)ev
{
    return YES;
}

+(CGDirectDisplayID) displayFromScreen:(NSScreen *)s
{
    NSNumber* didref = (NSNumber*)[[s deviceDescription] objectForKey:@"NSScreenNumber"];
    CGDirectDisplayID disp = (CGDirectDisplayID)[didref longValue];
    return disp;
}

-(void) warpMouseToCenter
{
    NSPoint w;
    w.x = _Platform->Width/2.0f;
    w.y = _Platform->Height/2.0f;
    w = [[self window] convertBaseToScreen:w];
    CGDirectDisplayID disp = [OVRView displayFromScreen:[[self window] screen]];
    CGPoint p = {w.x, CGDisplayPixelsHigh(disp)-w.y};
    CGDisplayMoveCursorToPoint(disp, p);
}

static bool LookupKey(NSEvent* ev, wchar_t& ch, OVR::KeyCode& key, unsigned& mods)
{
    NSString* chars = [ev charactersIgnoringModifiers];
    if ([chars length] == 0)
		return false;
	ch = [chars characterAtIndex:0];
    mods = MapModifiers([ev modifierFlags]);

	// check for Cmd+Latin Letter
    NSString* modchars = [ev characters];
    if ([modchars length])
	{
        wchar_t modch = [modchars characterAtIndex:0];
		if (modch >= 'a' && modch <= 'z')
			ch = modch;
	}
	key = MapToKeyCode(ch);
    return true;
}

-(void) keyDown:(NSEvent*)ev
{
	OVR::KeyCode key;
	unsigned     mods;
	wchar_t      ch;
	if (!LookupKey(ev, ch, key, mods))
		return;
    if (key == Key_Escape && _Platform->MMode == Mouse_Relative)
    {
        [self warpMouseToCenter];
        CGAssociateMouseAndMouseCursorPosition(true);
        [NSCursor unhide];
        _Platform->MMode = Mouse_RelativeEscaped;
    }
    _App->OnKey(key, ch, true, mods);
}
-(void) keyUp:(NSEvent*)ev
{
	OVR::KeyCode key;
	unsigned     mods;
	wchar_t      ch;
	if (LookupKey(ev, ch, key, mods))
	    _App->OnKey(key, ch, false, mods);
}

static const OVR::KeyCode ModifierKeys[] = {OVR::Key_None, OVR::Key_Shift, OVR::Key_Control, OVR::Key_Alt, OVR::Key_Meta};

-(void)flagsChanged:(NSEvent *)ev
{
    unsigned long cmods = [ev modifierFlags];
    if ((cmods & 0xffff0000) != _Modifiers)
    {
        uint32_t mods = MapModifiers(cmods);
        for (int i = 1; i <= 4; i++)
        {
            unsigned long m = (1 << (16+i));
            if ((cmods & m) != (_Modifiers & m))
            {
                if (cmods & m)
                    _App->OnKey(ModifierKeys[i], 0, true, mods);
                else
                    _App->OnKey(ModifierKeys[i], 0, false, mods);
            }
        }
        _Modifiers = cmods & 0xffff0000;
    }
}

-(void)ProcessMouse:(NSEvent*)ev
{
    switch ([ev type])
    {
        case NSLeftMouseDragged:
        case NSRightMouseDragged:
        case NSOtherMouseDragged:
        case NSMouseMoved:
        {
            if (_Platform->MMode == OVR::Platform::Mouse_Relative)
            {
                int dx = [ev deltaX];
                int dy = [ev deltaY];
                
                if (dx != 0 || dy != 0)
                {
                    _App->OnMouseMove(dx, dy, Mod_MouseRelative|MapModifiers([ev modifierFlags]));
                    [self warpMouseToCenter];
                }
            }
            else
            {
                NSPoint p = [ev locationInWindow];
                _App->OnMouseMove(p.x, p.y, MapModifiers([ev modifierFlags]));
            }
        }
        break;
        case NSLeftMouseDown:
        case NSRightMouseDown:
        case NSOtherMouseDown:
            break;
    }
}

-(void) mouseMoved:(NSEvent*)ev
{
    [self ProcessMouse:ev];
}
-(void) mouseDragged:(NSEvent*)ev
{
    [self ProcessMouse:ev];
}
-(void) mouseDown:(NSEvent*)ev
{
    if (_Platform->MMode == Mouse_RelativeEscaped)
    {
        [self warpMouseToCenter];
        CGAssociateMouseAndMouseCursorPosition(false);
        [NSCursor hide];
        _Platform->MMode = Mouse_Relative;
    }
}

//-(void)

-(id) initWithFrame:(NSRect)frameRect
{
    NSOpenGLPixelFormatAttribute attr[] =
    {NSOpenGLPFAWindow, NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 24, nil};
        
    NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attr] autorelease];
    
    self = [super initWithFrame:frameRect pixelFormat:pf];
    GLint swap = 0;
    [[self openGLContext] setValues:&swap forParameter:NSOpenGLCPSwapInterval];
    //[self setWantsBestResolutionOpenGLSurface:YES];
    return self;
}

-(void) reshape
{
    NSRect bounds = [self bounds];
    _App->OnResize(bounds.size.width, bounds.size.height);
    
    _Platform->Width = bounds.size.width;
    _Platform->Height = bounds.size.height;
    
    if (_Platform->GetRenderer())
        _Platform->GetRenderer()->SetWindowSize(bounds.size.width, bounds.size.height);
}

-(BOOL)windowShouldClose:(id)sender
{
    if (_Platform)
        _Platform->Exit(0);
    else
        exit(0);
    return 1;
}

@end

namespace OVR { namespace Platform { namespace OSX {

PlatformCore::PlatformCore(Application* app, void* nsapp)
    : Platform::PlatformCore(app), NsApp(nsapp), Win(NULL), View(NULL), Quit(0), MMode(Mouse_Normal)
{
    pGamepadManager = *new OSX::GamepadManager();
}
PlatformCore::~PlatformCore()
{
}
    
void PlatformCore::Exit(int exitcode)
{
    OVRApp* nsApp = (OVRApp*)NsApp;
    [nsApp stop:nil];
}
    
String PlatformCore::GetContentDirectory() const
{
    NSBundle* bundle = [NSBundle mainBundle];
    if (bundle)
        return String([[bundle bundlePath] UTF8String]) + "/Contents/Resources";
    else
        return ".";
}


void PlatformCore::SetMouseMode(MouseMode mm)
{
    if (mm == MMode)
        return;

    if (Win)
    {
        if (mm == Mouse_Relative)
        {
            [NSCursor hide];
            [(OVRView*)View warpMouseToCenter];
            CGAssociateMouseAndMouseCursorPosition(false);
        }
        else
        {
            if (MMode == Mouse_Relative)
            {
                CGAssociateMouseAndMouseCursorPosition(true);
                [NSCursor unhide];
                [(OVRView*)View warpMouseToCenter];
            }
        }
    }
    MMode = mm;
}


void PlatformCore::GetWindowSize(int* w, int* h) const
{
    *w = Width;
    *h = Height;
}
    
bool PlatformCore::SetupWindow(int w, int h)
{
    NSRect winrect;
    winrect.origin.x = 0;
    winrect.origin.y = 1000;
    winrect.size.width = w;
    winrect.size.height = h;
    NSWindow* win = [[NSWindow alloc] initWithContentRect:winrect styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];
    
    OVRView* view = [[OVRView alloc] initWithFrame:winrect];
    [view setPlatform:this];
    [win setContentView:view];
    [win setAcceptsMouseMovedEvents:YES];
    [win setDelegate:view];
    [view setApp:pApp];
    Win = win;
    View = view;
    return 1;
}
    
void PlatformCore::SetWindowTitle(const char* title)
{
    [((NSWindow*)Win) setTitle:[[NSString alloc] initWithBytes:title length:strlen(title) encoding:NSUTF8StringEncoding]];
}
    
void PlatformCore::ShowWindow(bool show)
{
    if (show)
        [((NSWindow*)Win) makeKeyAndOrderFront:nil];
    else
        [((NSWindow*)Win) orderOut:nil];
}

void PlatformCore::DestroyWindow()
{
    [((NSWindow*)Win) close];
    Win = NULL;
}

RenderDevice* PlatformCore::SetupGraphics(const SetupGraphicsDeviceSet& setupGraphicsDesc,
                                          const char* type, const Render::RendererParams& rp)
{
    const SetupGraphicsDeviceSet* setupDesc = setupGraphicsDesc.PickSetupDevice(type);
    OVR_ASSERT(setupDesc);
        
    pRender = *setupDesc->pCreateDevice(rp, this);
    if (pRender)
        pRender->SetWindowSize(Width, Height);

    return pRender.GetPtr();
}
    
int       PlatformCore::GetDisplayCount()
{
    return (int)[[NSScreen screens] count];
}

Render::DisplayId PlatformCore::GetDisplay(int i)
{
    NSScreen* s = (NSScreen*)[[NSScreen screens] objectAtIndex:i];
    return Render::DisplayId([OVRView displayFromScreen:s]);
}

bool PlatformCore::SetFullscreen(const Render::RendererParams& rp, int fullscreen)
{
    if (fullscreen == Render::Display_Window)
        [(OVRView*)View exitFullScreenModeWithOptions:nil];
    else
    {
        NSScreen* usescreen = [NSScreen mainScreen];
        NSArray* screens = [NSScreen screens];
        for (int i = 0; i < [screens count]; i++)
        {
            NSScreen* s = (NSScreen*)[screens objectAtIndex:i];
            CGDirectDisplayID disp = [OVRView displayFromScreen:s];

            if (disp == rp.Display.CgDisplayId)
                usescreen = s;
        }
        
        [(OVRView*)View enterFullScreenMode:usescreen withOptions:nil];
    }

    if (pRender)
        pRender->SetFullscreen((Render::DisplayMode)fullscreen);
    return 1;
}

}}
// GL
namespace Render { namespace GL { namespace OSX {

Render::RenderDevice* RenderDevice::CreateDevice(const RendererParams& rp, void* oswnd)
{
    Platform::OSX::PlatformCore* PC = (Platform::OSX::PlatformCore*)oswnd;

    OVRView* view = (OVRView*)PC->View;
    NSOpenGLContext *context = [view openGLContext];
    if (!context)
        return NULL;

    [context makeCurrentContext];
    [((NSWindow*)PC->Win) makeKeyAndOrderFront:nil];

    return new Render::GL::OSX::RenderDevice(rp, context);
}

void RenderDevice::Present()
{
    NSOpenGLContext *context = (NSOpenGLContext*)Context;
    [context flushBuffer];
}

void RenderDevice::Shutdown()
{
    Context = NULL;
}

bool RenderDevice::SetFullscreen(DisplayMode fullscreen)
{
    Params.Fullscreen = fullscreen;
    return 1;
}
    
}}}}


int main(int argc, char *argv[])
{
    NSApplication* nsapp = [OVRApp sharedApplication];
    [nsapp run];
    return 0;
}

