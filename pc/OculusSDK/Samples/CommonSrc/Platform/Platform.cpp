/************************************************************************************

Filename    :   Platform.cpp
Content     :   Platform-independent app framework for Oculus samples
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



#include "Platform.h"
#include <Kernel/OVR_Std.h>
#include <Kernel/OVR_Timer.h>
#include "../Render/Render_Device.h"
#include "Gamepad.h"

namespace OVR { namespace Platform {


const SetupGraphicsDeviceSet* SetupGraphicsDeviceSet::PickSetupDevice(const char* typeArg) const
{
    // Search for graphics creation object that matches type arg.
    if (typeArg)
    {
        for (const SetupGraphicsDeviceSet* p = this; p != 0; p = p->pNext)
        {
            if (!OVR_stricmp(p->pTypeArg, typeArg))
                return p;
        }
    }
    return this;
}

//-------------------------------------------------------------------------------------

PlatformCore::PlatformCore(Application *app)
{
    pApp = app;
    pApp->SetPlatformCore(this);    
    StartupTicks = OVR::Timer::GetTicks();
}

double PlatformCore::GetAppTime() const
{
    return (OVR::Timer::GetTicks() - StartupTicks) * (1.0 / (double)OVR::Timer::MksPerSecond);
}

bool PlatformCore::SetFullscreen(const Render::RendererParams&, int fullscreen)
{
    if (pRender)
        return pRender->SetFullscreen((Render::DisplayMode)fullscreen);
    return 0;
}

Render::DisplayId PlatformCore::GetDisplay(int screen)
{
    OVR_UNUSED(screen); return Render::DisplayId();
}

}}
