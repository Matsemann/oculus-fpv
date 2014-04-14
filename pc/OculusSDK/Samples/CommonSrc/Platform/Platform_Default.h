/************************************************************************************

Filename    :   Platform_Default.h
Content     :   Default Platform class and RenderDevice selection file
Created     :   October 4, 2012
Authors     :   

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

*************************************************************************************/

#ifndef OVR_Platform_Default_h
#define OVR_Platform_Default_h

// This should select proper header file for the platform/compiler.
#include <Kernel/OVR_Types.h>

#if defined(OVR_OS_WIN32)
  #include "Win32_Platform.h"

  #include "../Render/Render_D3D11_Device.h"
  #undef OVR_D3D_VERSION  
  #include "../Render/Render_D3D10_Device.h"
//  #include "../Render/Render_GL_Win32_Device.h"

// Modify this list or pass a smaller set to select a specific render device,
// while avoiding linking extra classes.
  #define OVR_DEFAULT_RENDER_DEVICE_SET                                                    \
        SetupGraphicsDeviceSet("D3D11", &OVR::Render::D3D11::RenderDevice::CreateDevice,       \
        SetupGraphicsDeviceSet("D3D10", &OVR::Render::D3D10::RenderDevice::CreateDevice) )

#elif defined(OVR_OS_MAC) && !defined(OVR_MAC_X11)
  #include "OSX_Platform.h"

  #define OVR_DEFAULT_RENDER_DEVICE_SET                                         \
    SetupGraphicsDeviceSet("GL", &OVR::Render::GL::OSX::RenderDevice::CreateDevice)

#else

  #include "Linux_Platform.h"

  #define OVR_DEFAULT_RENDER_DEVICE_SET                                         \
    SetupGraphicsDeviceSet("GL", &OVR::Render::GL::Linux::RenderDevice::CreateDevice)

#endif

#endif // OVR_Platform_Default_h
