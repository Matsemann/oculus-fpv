/************************************************************************************

Filename    :   OSX_Gamepad.cpp
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

#include "OSX_Gamepad.h"


static const UInt32 Logitech_F710_VendorID = 0x046D;
static const UInt32 Logitech_F710_ProductID = 0xC219;

static const UInt32 Sony_DualShock3_VendorID = 0x054C;
static const UInt32 Sony_DualShock3_ProductID = 0x0268;


namespace OVR { namespace Platform { namespace OSX {

    
GamepadManager::GamepadManager()
 :  bStateChanged(false)
{
    
    HidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    IOHIDManagerOpen(HidManager, kIOHIDOptionsTypeNone);
    IOHIDManagerScheduleWithRunLoop(HidManager,
                                    CFRunLoopGetCurrent(),
                                    kCFRunLoopDefaultMode);
    
    
    // Setup device matching.
    CFStringRef keys[] = {  CFSTR(kIOHIDDeviceUsagePageKey),
                            CFSTR(kIOHIDDeviceUsageKey)};
    
    int value;
    CFNumberRef values[2];
    CFDictionaryRef dictionaries[2];

    // Match joysticks.
    value = kHIDPage_GenericDesktop;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);

    value = kHIDUsage_GD_Joystick;
    values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);

    dictionaries[0] = CFDictionaryCreate(kCFAllocatorDefault,
                                         (const void **) keys,
                                         (const void **) values,
                                         2,
                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);

    // Match gamepads.
    value = kHIDPage_GenericDesktop;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);

    value = kHIDUsage_GD_GamePad;
    values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);

    dictionaries[1] = CFDictionaryCreate(kCFAllocatorDefault,
                                         (const void **) keys,
                                         (const void **) values,
                                         2,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);

    CFArrayRef array = CFArrayCreate(   kCFAllocatorDefault,
                                        (const void **) dictionaries,
                                        2,
                                        &kCFTypeArrayCallBacks);
    CFRelease(dictionaries[0]);
    CFRelease(dictionaries[1]);

    IOHIDManagerSetDeviceMatchingMultiple(HidManager, array);
    
    CFRelease(array);
    
    
    IOHIDManagerRegisterDeviceMatchingCallback(HidManager, staticOnDeviceMatched, this);
    IOHIDManagerRegisterDeviceRemovalCallback(HidManager, staticOnDeviceRemoved, this);

}

GamepadManager::~GamepadManager()
{
    CFRelease(HidManager);
}

UInt32 GamepadManager::GetGamepadCount()
{
    return 1;
}
    
bool GamepadManager::GetGamepadState(UInt32 index, GamepadState* pState)
{
    // For now we just support one gamepad.
    OVR_UNUSED(index);
    
    if (!bStateChanged)
    {
        return false;
    }
    
    bStateChanged = false;
//  State.Debug();
    
    *pState = State;
    return true;
}

int GamepadManager::getIntDeviceProperty(IOHIDDeviceRef device, CFStringRef key)
{
    CFTypeRef type = IOHIDDeviceGetProperty(device, key);
    OVR_ASSERT(type != NULL && CFGetTypeID(type) == CFNumberGetTypeID());
    
    int value;
    CFNumberGetValue((CFNumberRef) type, kCFNumberSInt32Type, &value);

    return value;
}

void GamepadManager::staticOnDeviceMatched(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
    GamepadManager* pManager = (GamepadManager*) context;
    pManager->onDeviceMatched(device);
}

void GamepadManager::onDeviceMatched(IOHIDDeviceRef device)
{
    IOHIDDeviceRegisterInputValueCallback(device, staticOnDeviceValueChanged, this);
}

void GamepadManager::staticOnDeviceRemoved(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
    GamepadManager* pManager = (GamepadManager*) context;
    pManager->onDeviceRemoved(device);
}

void GamepadManager::onDeviceRemoved(IOHIDDeviceRef device)
{
    IOHIDDeviceRegisterInputValueCallback(device, NULL, NULL);
}

void GamepadManager::staticOnDeviceValueChanged(void* context, IOReturn result, void* sender, IOHIDValueRef value)
{
    GamepadManager* pManager = (GamepadManager*) context;
    pManager->onDeviceValueChanged(value);
}
    
float GamepadManager::mapAnalogAxis(IOHIDValueRef value, IOHIDElementRef element)
{
    
    CFIndex val = IOHIDValueGetIntegerValue(value);
    CFIndex min = IOHIDElementGetLogicalMin(element);
    CFIndex max = IOHIDElementGetLogicalMax(element);
    
    float v = (float) (val - min) / (float) (max - min);
    v = v * 2.0f - 1.0f;
    
    // Dead zone.
    if (v < 0.1f && v > -0.1f)
    {
        v = 0.0f;
    }
    
    return v;
}
    
bool GamepadManager::setStateIfDifferent(float& state, float newState)
{
    if (state == newState)
        return false;
    
    state = newState;
    
    return true;    
}
    
void GamepadManager::onDeviceValueChanged(IOHIDValueRef value)
{
    
    IOHIDElementRef element = IOHIDValueGetElement(value);
    IOHIDDeviceRef device = IOHIDElementGetDevice(element);

    int vendorID = getIntDeviceProperty(device, CFSTR(kIOHIDVendorIDKey));
    int productID = getIntDeviceProperty(device, CFSTR(kIOHIDProductIDKey));
    
    uint32_t usagePage = IOHIDElementGetUsagePage(element);
    uint32_t usage = IOHIDElementGetUsage(element);

    // The following controller mapping is based on the Logitech F710, however we use it for
    // all Logitech devices on the assumption that they're likely to share the same mapping.
    if (vendorID == Logitech_F710_VendorID)
    {
        // Logitech F710 mapping.
        if (usagePage == kHIDPage_Button)
        {
            bool buttonState = IOHIDValueGetIntegerValue(value);

            switch(usage)
            {
                case kHIDUsage_Button_1:
                    manipulateBitField(State.Buttons, Gamepad_X, buttonState);
                    break;
                case kHIDUsage_Button_2:
                    manipulateBitField(State.Buttons, Gamepad_A, buttonState);
                    break;
                case kHIDUsage_Button_3:
                    manipulateBitField(State.Buttons, Gamepad_B, buttonState);
                    break;
                case kHIDUsage_Button_4:
                    manipulateBitField(State.Buttons, Gamepad_Y, buttonState);
                    break;
                case 0x05:
                    manipulateBitField(State.Buttons, Gamepad_L1, buttonState);
                    break;
                case 0x06:
                    manipulateBitField(State.Buttons, Gamepad_R1, buttonState);
                    break;
                case 0x07:
                    State.LT = buttonState ? 1.0f:0.0f;
                    break;
                case 0x08:
                    State.RT = buttonState ? 1.0f:0.0f;
                    break;
                case 0x09:
                    manipulateBitField(State.Buttons, Gamepad_Back, buttonState);
                    break;
                case 0x0A:
                    manipulateBitField(State.Buttons, Gamepad_Start, buttonState);
                    break;
                case 0x0B:
                    manipulateBitField(State.Buttons, Gamepad_LStick, buttonState);
                    break;
                case 0x0C:
                    manipulateBitField(State.Buttons, Gamepad_RStick, buttonState);
                    break;
                default:
                    return;
            }
        }
        else if (usagePage == kHIDPage_GenericDesktop)
        {
            float v;
            switch(usage)
            {
                case kHIDUsage_GD_X:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.LX, v))
                        return;
                    break;
                case kHIDUsage_GD_Y:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.LY, -v))
                        return;
                    break;
                case kHIDUsage_GD_Z:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.RX, v))
                        return;
                    break;
                case kHIDUsage_GD_Rz:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.RY, -v))
                        return;
                    break;
                case kHIDUsage_GD_Hatswitch:
                    {
                        CFIndex integerValue = IOHIDValueGetIntegerValue(value);
                 
                        manipulateBitField(State.Buttons,
                                           Gamepad_Up,
                                           integerValue == 7 || integerValue == 0 || integerValue == 1);
                        manipulateBitField(State.Buttons,
                                           Gamepad_Down,
                                           integerValue == 3 || integerValue == 4 || integerValue == 5);
                        manipulateBitField(State.Buttons,
                                           Gamepad_Left,
                                           integerValue == 5 || integerValue == 6 || integerValue == 7);
                        manipulateBitField(State.Buttons,
                                           Gamepad_Right,
                                           integerValue == 1 || integerValue == 2 || integerValue == 3);
                    }
                    break;
                default:
                    return;
            }
        }
    }
    // The following controller mapping is based on the Sony DualShock3, however we use it for
    // all Sony devices on the assumption that they're likely to share the same mapping.
    else if (vendorID == Sony_DualShock3_VendorID)
    {
        // PS3 Controller.
        if (usagePage == kHIDPage_Button)
        {
            bool buttonState = IOHIDValueGetIntegerValue(value);
            
            switch(usage)
            {
                case kHIDUsage_Button_1:
                    manipulateBitField(State.Buttons, Gamepad_Back, buttonState);
                    break;
                case kHIDUsage_Button_2:
                    manipulateBitField(State.Buttons, Gamepad_LStick, buttonState);
                    break;
                case kHIDUsage_Button_3:
                    manipulateBitField(State.Buttons, Gamepad_RStick, buttonState);
                    break;
                case kHIDUsage_Button_4:
                    manipulateBitField(State.Buttons, Gamepad_Start, buttonState);
                    break;
                case 0x05:
                    manipulateBitField(State.Buttons, Gamepad_Up, buttonState);
                    break;
                case 0x06:
                    manipulateBitField(State.Buttons, Gamepad_Right, buttonState);
                    break;
                case 0x07:
                    manipulateBitField(State.Buttons, Gamepad_Down, buttonState);
                    break;
                case 0x08:
                    manipulateBitField(State.Buttons, Gamepad_Left, buttonState);
                    break;
                case 0x09:
                    State.LT = buttonState ? 1.0f:0.0f;
                    break;
                case 0x0A:
                    State.RT = buttonState ? 1.0f:0.0f;
                    break;
                case 0x0B:
                    manipulateBitField(State.Buttons, Gamepad_L1, buttonState);
                    break;
                case 0x0C:
                    manipulateBitField(State.Buttons, Gamepad_R1, buttonState);
                    break;
                case 0x0D:
                    // PS3 Triangle.
                    manipulateBitField(State.Buttons, Gamepad_TRIANGLE, buttonState);
                    break;
                case 0x0E:
                    // PS3 Circle
                    manipulateBitField(State.Buttons, Gamepad_CIRCLE, buttonState);
                    break;
                case 0x0F:
                    // PS3 Cross
                    manipulateBitField(State.Buttons, Gamepad_CROSS, buttonState);
                    break;
                case 0x10:
                    // PS3 Square
                    manipulateBitField(State.Buttons, Gamepad_SQUARE, buttonState);
                    break;
                default:
                    return;
            }
        }
        else if (usagePage == kHIDPage_GenericDesktop)
        {
            float v;
            switch(usage)
            {
                case kHIDUsage_GD_X:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.LX, v))
                        return;
                    break;
                case kHIDUsage_GD_Y:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.LY, -v))
                        return;
                    break;
                case kHIDUsage_GD_Z:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.RX, v))
                        return;
                    break;
                case kHIDUsage_GD_Rz:
                    v = mapAnalogAxis(value, element);
                    if (!setStateIfDifferent(State.RY, -v))
                        return;
                    break;
                default:
                    return;
            }
        }
    }
    
    bStateChanged = true;
}
    
void GamepadManager::manipulateBitField(unsigned int& bitfield, unsigned int mask, bool val)
{
    if (val)
    {
        bitfield |= mask;
    }
    else
    {
        bitfield &= ~mask;
    }
}

}}} // OVR::Platform::OSX
