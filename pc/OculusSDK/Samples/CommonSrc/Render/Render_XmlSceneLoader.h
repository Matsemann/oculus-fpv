/************************************************************************************

Filename    :   Render_XmlSceneLoader.h
Content     :   Imports and exports XML files
Created     :   January 21, 2013
Authors     :   Robotic Arm Software - Peter Hoff, Dan Goodman, Bryan Croteau

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

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

#ifndef INC_Render_XMLSceneLoader_h
#define INC_Render_XMLSceneLoader_h

#include "Render_Device.h"
#include <Kernel/OVR_SysFile.h>
using namespace OVR;
using namespace OVR::Render;

#ifdef OVR_DEFINE_NEW
#undef new
#endif

#include "../../../3rdParty/TinyXml/tinyxml2.h"

namespace OVR { namespace Render {

using namespace tinyxml2;

class XmlHandler
{
public:
    XmlHandler();
    ~XmlHandler();

    bool ReadFile(const char* fileName, OVR::Render::RenderDevice* pRender,
                  OVR::Render::Scene* pScene,
		          OVR::Array<Ptr<CollisionModel> >* pColisions,
                  OVR::Array<Ptr<CollisionModel> >* pGroundCollisions);

protected:
    void ParseVectorString(const char* str, OVR::Array<OVR::Vector3f> *array,
		                   bool is2element = false);

private:
    tinyxml2::XMLDocument* pXmlDocument;
    char                   filePath[250];
    int                    textureCount;
    OVR::Array<Ptr<Texture> > Textures;
    int                    modelCount;
    OVR::Array<Ptr<Model> > Models;
    int                    collisionModelCount;
    int                    groundCollisionModelCount;
};

}} // OVR::Render

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif

#endif // INC_Render_XMLSceneLoader_h
