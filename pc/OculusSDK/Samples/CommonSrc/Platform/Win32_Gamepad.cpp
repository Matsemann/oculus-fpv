/************************************************************************************

Filename    :   Win32_Gamepad.cpp
Content     :   Win32 implementation of Platform app infrastructure
Created     :   May 6, 2013
Authors     :   Lee Cooper

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

#include "Win32_Gamepad.h"

namespace OVR { namespace Platform { namespace Win32 {

GamepadManager::GamepadManager()
{
    hXInputModule = ::LoadLibraryA("Xinput9_1_0.dll");
    if (hXInputModule)
    {
        pXInputGetState = (PFn_XInputGetState)
            ::GetProcAddress(hXInputModule, "XInputGetState");        
    }
}

GamepadManager::~GamepadManager()
{
    if (hXInputModule)
        ::FreeLibrary(hXInputModule);
}

static inline float GamepadStick(short in)
{
    float v;
    if (abs(in) < 9000)
        return 0;
    else if (in > 9000)
        v = (float) in - 9000;
    else
        v = (float) in + 9000;
    return v / (32767 - 9000);
}

static inline float GamepadTrigger(BYTE in)
{
    if (in < 30)
        return 0;
    else
        return float(in-30) / 225;
}

UInt32 GamepadManager::GetGamepadCount()
{
    return 1;
}

bool GamepadManager::GetGamepadState(UInt32 index, GamepadState* pState)
{
    // For now we just support one gamepad.
    OVR_UNUSED(index);

    if (pXInputGetState)
    {
        XINPUT_STATE xis;
                
        if (pXInputGetState(0, &xis))
            return false;

        if (xis.dwPacketNumber == LastPadPacketNo)
            return false;

        // State changed.
        pState->Buttons = xis.Gamepad.wButtons; // Currently matches Xinput
        pState->LT = GamepadTrigger(xis.Gamepad.bLeftTrigger);
        pState->RT = GamepadTrigger(xis.Gamepad.bRightTrigger);
        pState->LX = GamepadStick(xis.Gamepad.sThumbLX);
        pState->LY = GamepadStick(xis.Gamepad.sThumbLY);
        pState->RX = GamepadStick(xis.Gamepad.sThumbRX);
        pState->RY = GamepadStick(xis.Gamepad.sThumbRY);

        LastPadPacketNo = xis.dwPacketNumber;

        return true;
    }

    return false;
}

}}} // OVR::Platform::Win32
