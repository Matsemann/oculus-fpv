/************************************************************************************

Filename    :   Win32_Gamepad.h
Content     :   Win32 implementation of Gamepad functionality.
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

#ifndef OVR_Win32_Gamepad_h
#define OVR_Win32_Gamepad_h

#include "Gamepad.h"

#include <windows.h>
#include <xinput.h>

namespace OVR { namespace Platform { namespace Win32 {

class GamepadManager : public Platform::GamepadManager
{
public:
    GamepadManager();
    ~GamepadManager();

    virtual UInt32  GetGamepadCount();
    virtual bool    GetGamepadState(UInt32 index, GamepadState* pState);

private:
    // Dynamically ink to XInput to simplify projects.
    HMODULE             hXInputModule;
    typedef DWORD (WINAPI *PFn_XInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
    PFn_XInputGetState  pXInputGetState;

    UInt32              LastPadPacketNo;
};

}}}

#endif // OVR_Win32_Gamepad_h
