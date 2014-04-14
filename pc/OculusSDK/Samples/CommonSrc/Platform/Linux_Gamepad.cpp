/************************************************************************************

Filename    :   Linux_Gamepad.cpp
Content     :   Linux implementation of Platform app infrastructure
Created     :   May 6, 2013
Authors     :   Lee Cooper, Simon Hallam

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <linux/joystick.h>
#include "Linux_Gamepad.h"


namespace OVR { namespace Platform { namespace Linux {

const char* pNameXbox360Wireless = "Xbox 360";
const char* pNameXbox360Wired    = "Microsoft X-Box 360";


GamepadManager::GamepadManager() :
    pDevice(NULL)
{
}

GamepadManager::~GamepadManager()
{
    // if we have an open device, close it
    if (pDevice)
    {
        pDevice->Close();
        pDevice = NULL;
    }
}

UInt32 GamepadManager::GetGamepadCount()
{
    return 1;
}

bool GamepadManager::GetGamepadState(UInt32 index, GamepadState *pState)
{
    if (!pDevice)
    {
        // get a list of paths to all the connected joystick devices
        glob_t joystickGlobBuffer;
        glob("/dev/input/js*", 0, NULL, &joystickGlobBuffer);

        // open each joystick device, until we find one that will work for our needs
        for (UInt32 i = 0; i < joystickGlobBuffer.gl_pathc; i++)
        {
            pDevice = new Gamepad();
            if (pDevice->Open(joystickGlobBuffer.gl_pathv[i]))
            {

                if (pDevice->IsSupportedType())
                {
                    break;
                }
            }

            // we don't know why the device was not useable, make sure it gets closed cleanly
            pDevice->Close();
            pDevice = NULL;
        }

    }

    if (pDevice)
    {
        // we have a device, so update it
        pDevice->UpdateState();

        // copy the device state into the struct param
        memcpy(pState, pDevice->GetState(), sizeof(GamepadState));

        // TODO: is the device still active/connected?  if not, we should close it 
        //       and clear pDevice, so that another device can take over

        return true;
    }
    else
    {
        return false;
    }
}

Gamepad::Gamepad() :
    IsInitialized(false),
    Name(String("Undefined")),
    Type(UNDEFINED)
{
}

Gamepad::~Gamepad()
{
    this->Close();
}

bool Gamepad::Open(const String& devicePathName)
{
    Name = "Undefined";
    Type = UNDEFINED;

    FileDescriptor = ::open(devicePathName.ToCStr(), O_RDONLY | O_NONBLOCK);
    if (FileDescriptor == -1)
    {
        return false;
    }

    // get the device name
    char name[128];
    if (ioctl(FileDescriptor, JSIOCGNAME(sizeof(name)), name) < 0)
    {
        return false;
    }

    Name = name;

    // see if device name matches one of our supported devices
    static const UInt32 Wireless360Len = String(pNameXbox360Wireless).GetLength();
    static const UInt32 Wired360Len = String(pNameXbox360Wired).GetLength();
    if (Name.Substring(0, Wireless360Len) == pNameXbox360Wireless)
    {
        Type = XBOX360GAMEPADWIRELESS;
        return true;
    }
    else if(Name.Substring(0, Wired360Len) == pNameXbox360Wired)
    {
        Type = XBOX360GAMEPADWIRED;
        return true;
    }

    return false;
}

bool Gamepad::Close()
{
    IsInitialized = false;
    Name = "Undefined";
    Type = UNDEFINED;
    return !::close(FileDescriptor);
}

void Gamepad::UpdateState()
{
    GamepadState *pState = &State;
    js_event gamepadEvent;

    // read the latest batch of events
    while (read(FileDescriptor, &gamepadEvent, sizeof(struct js_event)) != -1)
    {
        switch (gamepadEvent.type)
        {
        case JS_EVENT_BUTTON:
            IsInitialized = true;
            SetStateButton(pState, gamepadEvent.number, gamepadEvent.value);
            break;

        case JS_EVENT_AXIS:
            IsInitialized = true;
            SetStateAxis(pState, gamepadEvent.number, gamepadEvent.value);
            break;

        case JS_EVENT_BUTTON | JS_EVENT_INIT:
            if (IsInitialized) // skip the fake values during device event initialization
            {
                SetStateButton(pState, gamepadEvent.number, gamepadEvent.value);
            }
            break;

        case JS_EVENT_AXIS | JS_EVENT_INIT:
            if (IsInitialized) // skip the fake values during device event initialization
            {
                SetStateAxis(pState, gamepadEvent.number, gamepadEvent.value);
            }
            break;

        default:
            LogText("OVR::Linux::UpdateState unknown event type\n");
        }
    }
}

const GamepadState* Gamepad::GetState()
{
    return &State;
}


bool Gamepad::IsSupportedType()
{
    return Type != UNDEFINED;
}

const String& Gamepad::GetIdentifier()
{
    return Name;
}

static inline float NormalizeGamepadStickXbox360(SInt32 in)
{
    float v;
    if (abs(in) < 9000) return 0;
    else if (in > 9000) v = (float)in - 9000;
    else v = (float)in + 9000;
    return v / (32767 - 9000);
}

static inline float NormalizeGamepadTriggerXbox360(SInt32 in, 
                                                   SInt32 offset, 
                                                   SInt32 deadBand, 
                                                   float divisor)
{
    in += offset;

    if (in < deadBand) 
    {
        return 0;
    }
    else
    {
        return float(in - deadBand) / divisor;
    }
}

static inline void UpdateButtonMaskAndBitfield(GamepadState *pState, 
                                               SInt32 value, 
                                               UInt32 buttonBitfield)
{
    if (value)
    {
        pState->Buttons |= buttonBitfield;
    }
    else
    {
        pState->Buttons = pState->Buttons & (0xFFFFFFFF ^ buttonBitfield);
    }
}

void Gamepad::SetStateAxis(GamepadState *pState, UInt32 axis, SInt32 value)
{
    // some pads/sticks have lots in common with one another,
    // handle those shared cases first
    switch (Type)
    {
    case XBOX360GAMEPADWIRELESS:
    case XBOX360GAMEPADWIRED:
        switch (axis)
        {
        case 0:
            pState->LX = NormalizeGamepadStickXbox360(value);
            break;

        case 1:
            pState->LY = -NormalizeGamepadStickXbox360(value);
            break;

        case 3:
            pState->RX = NormalizeGamepadStickXbox360(value);
            break;

        case 4:
            pState->RY = -NormalizeGamepadStickXbox360(value);
            break;
        }
        break;

    case UNDEFINED:
    default:
        break;
    }

    // handle the special cases, or pads/sticks which are unique
    switch (Type)
    {
    case XBOX360GAMEPADWIRELESS:
        switch (axis)
        {
        case 2:
            pState->LT = NormalizeGamepadTriggerXbox360(value, 0, 500, 32267);
            break;

        case 5:
            pState->RT = NormalizeGamepadTriggerXbox360(value, 0, 500, 32267);
            break;
        }
        break;

    case XBOX360GAMEPADWIRED:
        switch (axis)
        {
        case 2:
            pState->LT = NormalizeGamepadTriggerXbox360(value, 32767, 1000, 64535);
            break;

        case 5:
            pState->RT = NormalizeGamepadTriggerXbox360(value, 32767, 1000, 64535);
            break;

        case 6:
            if (value == 0)
            {
                UpdateButtonMaskAndBitfield(pState, 0, Gamepad_Left);
                UpdateButtonMaskAndBitfield(pState, 0, Gamepad_Right);
            }
            else if (value < 0)
            {
                UpdateButtonMaskAndBitfield(pState, 1, Gamepad_Left);
            }
            else if (value > 0)
            {
                UpdateButtonMaskAndBitfield(pState, 1, Gamepad_Right);
            }
            break;

        case 7:
            if (value == 0)
            {
                UpdateButtonMaskAndBitfield(pState, 0, Gamepad_Up);
                UpdateButtonMaskAndBitfield(pState, 0, Gamepad_Down);
            }
            else if (value < 0)
            {
                UpdateButtonMaskAndBitfield(pState, 1, Gamepad_Up);
            }
            else if (value > 0)
            {
                UpdateButtonMaskAndBitfield(pState, 1, Gamepad_Down);
            }
            break;
        }
        break;

    case UNDEFINED:
    default:
        break;
    }
}

void Gamepad::SetStateButton(GamepadState *pState, UInt32 button, SInt32 value)
{
    // some pads/sticks have lots in common with one another,
    // handle those shared cases first
    switch (Type)
    {
    case XBOX360GAMEPADWIRELESS:
    case XBOX360GAMEPADWIRED:
        switch (button)
        {
        case 0:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_A);
            break;

        case 1:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_B);
            break;

        case 2:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_X);
            break;

        case 3:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Y);
            break;

        case 4:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_L1);
            break;

        case 5:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_R1);
            break;

        case 6:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Back);
            break;

        case 7:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Start);
            break;

        case 8:
            // we have no value defined for the Xbox/power button
            break;

        case 9:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_LStick);
            break;

        case 10:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_RStick);
            break;
        }
        break;

    case UNDEFINED:
    default:
        break;
    }

    // handle the special cases, or pads/sticks which are unique
    switch (Type)
    {
    case XBOX360GAMEPADWIRELESS:
        switch (button)
        {
        case 11:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Left);
            break;

        case 12:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Right);
            break;

        case 13:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Up);
            break;

        case 14:
            UpdateButtonMaskAndBitfield(pState, value, Gamepad_Down);
            break;
        }

    case XBOX360GAMEPADWIRED:
        break;

    case UNDEFINED:
    default:
        break;
    }
}

}}} // OVR::Platform::Linux

