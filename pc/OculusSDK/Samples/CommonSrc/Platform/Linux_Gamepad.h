/************************************************************************************

Filename    :   Linux_Gamepad.h
Content     :   Linux implementation of Gamepad functionality.
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

#ifndef OVR_Linux_Gamepad_h
#define OVR_Linux_Gamepad_h

#include "Gamepad.h"

namespace OVR { namespace Platform { namespace Linux {

class Gamepad;  // forward declaration for readability

class GamepadManager : public Platform::GamepadManager
{
public:

    GamepadManager();
    ~GamepadManager();

    virtual UInt32      GetGamepadCount();
    virtual bool        GetGamepadState(UInt32 index, GamepadState *pState);

private:

    Gamepad *pDevice;
};

class Gamepad
{
public:

    Gamepad();
    virtual ~Gamepad();

    bool                 Open(const String& devicePathName);
    bool                 Close();
    bool                 IsSupportedType();
    const String&        GetIdentifier();
    void                 UpdateState();
    const GamepadState*  GetState();

private:

    void                 SetStateAxis(GamepadState *pState, UInt32 axis, SInt32 value);
    void                 SetStateButton(GamepadState *pState, UInt32 button, SInt32 value);

    enum GamepadType
    {
        UNDEFINED,
        XBOX360GAMEPADWIRELESS,
        XBOX360GAMEPADWIRED
    };

    UInt32               FileDescriptor;
    bool                 IsInitialized; 
    String               Name;
    GamepadType          Type;
    GamepadState         State;
};

}}}

#endif // OVR_Linux_Gamepad_h
