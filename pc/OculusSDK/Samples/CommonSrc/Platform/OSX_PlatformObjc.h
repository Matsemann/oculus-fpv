
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
#import <Cocoa/Cocoa.h>
#import "OSX_Platform.h"
#import "OSX_Gamepad.h"

#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGDirectDisplay.h>

@interface OVRApp : NSApplication

@property (assign) IBOutlet NSWindow* win;
@property (assign) OVR::Platform::OSX::PlatformCore* Platform;
@property (assign) OVR::Platform::Application* App;

-(void) run;

@end

@interface OVRView : NSOpenGLView <NSWindowDelegate>

@property (assign) OVR::Platform::OSX::PlatformCore* Platform;
@property (assign) OVR::Platform::Application* App;
@property unsigned long Modifiers;

-(void)ProcessMouse:(NSEvent*)event;
-(void)warpMouseToCenter;

+(CGDirectDisplayID) displayFromScreen:(NSScreen*)s;

@end

