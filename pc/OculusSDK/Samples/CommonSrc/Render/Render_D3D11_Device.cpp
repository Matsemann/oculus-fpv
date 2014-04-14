/************************************************************************************

Filename    :   Renderer_D3D11_Device.cpp
Content     :   Builds D3D11 renderer versions (to avoid duplication).
Created     :   September 10, 2012
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

#define OVR_D3D_VERSION 11
#include "Render_D3D1X_Device.cpp"
#undef OVR_D3D_VERSION