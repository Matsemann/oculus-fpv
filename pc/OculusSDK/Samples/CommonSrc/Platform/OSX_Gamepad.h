/************************************************************************************

Filename    :   OSX_Gamepad.h
Content     :   OSX implementation of Gamepad functionality.
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

#ifndef OVR_OSX_Gamepad_h
#define OVR_OSX_Gamepad_h

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>

#include "Gamepad.h"

namespace OVR { namespace Platform { namespace OSX {

    
class GamepadManager : public Platform::GamepadManager
{
public:
    GamepadManager();
    ~GamepadManager();

    virtual UInt32  GetGamepadCount();
    virtual bool    GetGamepadState(UInt32 index, GamepadState* pState);

private:
    static void staticOnDeviceMatched(void* context, IOReturn result, void* sender, IOHIDDeviceRef device);
    void        onDeviceMatched(IOHIDDeviceRef device);
    
    static void staticOnDeviceRemoved(void* context, IOReturn result, void* sender, IOHIDDeviceRef device);
    void        onDeviceRemoved(IOHIDDeviceRef device);

    static void staticOnDeviceValueChanged(void* context, IOReturn result, void* sender, IOHIDValueRef value);
    void        onDeviceValueChanged(IOHIDValueRef value);
    
    int         getIntDeviceProperty(IOHIDDeviceRef device, CFStringRef key);
    float       mapAnalogAxis(IOHIDValueRef value, IOHIDElementRef element);
    void        manipulateBitField(unsigned int& bitfield, unsigned int mask, bool val);
    bool        setStateIfDifferent(float& state, float newState);
    
    IOHIDManagerRef HidManager;
    GamepadState    State;
    bool            bStateChanged;
};

}}}

#endif // OVR_OSX_Gamepad_h
