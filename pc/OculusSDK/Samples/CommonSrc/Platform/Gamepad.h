/************************************************************************************

Filename    :   Gamepad.h
Content     :   Cross platform Gamepad interface.
Created     :   May 6, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

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

#ifndef OVR_Gamepad_h
#define OVR_Gamepad_h

#include <OVR.h>

namespace OVR { namespace Platform {

// Buttons on a typical gamepad controller.
enum GamepadButtons
{
    Gamepad_A               = 0x1000,
    Gamepad_CROSS           = 0x1000,
    Gamepad_B               = 0x2000,
    Gamepad_CIRCLE          = 0x2000,
    Gamepad_X               = 0x4000,
    Gamepad_SQUARE          = 0x4000,
    Gamepad_Y               = 0x8000,
    Gamepad_TRIANGLE        = 0x8000,
    Gamepad_Up              = 0x0001,
    Gamepad_Down            = 0x0002,
    Gamepad_Left            = 0x0004,
    Gamepad_Right           = 0x0008,
    Gamepad_Start           = 0x0010,
    Gamepad_Back            = 0x0020,
    Gamepad_LStick          = 0x0040,
    Gamepad_RStick          = 0x0080,
    Gamepad_L1              = 0x0100,
    Gamepad_R1              = 0x0200,
};

//-------------------------------------------------------------------------------------
// ***** GamepadState

// Describes the state of the controller buttons and analog inputs.
struct GamepadState
{
    UInt32  Buttons;            // Bitfield representing button state.
    float   LX;                 // Left stick X axis [-1,1]
    float   LY;                 // Left stick Y axis [-1,1]
    float   RX;                 // Right stick X axis [-1,1]
    float   RY;                 // Right stick Y axis [-1,1]
    float   LT;                 // Left trigger [0,1]
    float   RT;                 // Right trigger [0,1]

    GamepadState() : Buttons(0), LX(0), LY(0), RX(0), RY(0), LT(0), RT(0) {}

    bool operator==(const GamepadState& b) const
    {
        return Buttons == b.Buttons && LX == b.LX && LY == b.LY && RX == b.RX && RY == b.RY && LT == b.LT && RT == b.RT;
    }
    bool operator!=(const GamepadState& b) const
    {
        return !(*this == b);
    }
	void Debug()
	{
		OVR_DEBUG_LOG(("Buttons:0x%4x LX:%.2f LY:%.2f RX:%.2f RY:%.2f LT:%.2f RT:%.2f", Buttons, LX, LY, RX, RY, LT, RT));
	}
};

//-------------------------------------------------------------------------------------
// ***** GamepadManager

// GamepadManager provides a cross platform interface for accessing gamepad controller
// state.
class GamepadManager : public RefCountBase<GamepadManager>
{
public:

    // Get the number of connected gamepads.
    virtual UInt32  GetGamepadCount() = 0;

    // Get the state of the gamepad with a given index.
    virtual bool    GetGamepadState(UInt32 index, GamepadState* pState) = 0;
};

}} // OVR::Platform

#endif // OVR_Gamepad_h
