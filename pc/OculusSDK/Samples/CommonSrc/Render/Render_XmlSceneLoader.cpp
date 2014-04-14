/************************************************************************************

Filename    :   Render_XmlSceneLoader.cpp
Content     :   Imports and exports XML files - implementation
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

#include "Render_XmlSceneLoader.h"
#include <Kernel/OVR_Log.h>

#ifdef OVR_DEFINE_NEW
#undef new
#endif

namespace OVR { namespace Render {

XmlHandler::XmlHandler() : pXmlDocument(NULL)
{
    pXmlDocument = new tinyxml2::XMLDocument();
}

XmlHandler::~XmlHandler()
{
    delete pXmlDocument;
}

bool XmlHandler::ReadFile(const char* fileName, OVR::Render::RenderDevice* pRender,
	                      OVR::Render::Scene* pScene,
                          OVR::Array<Ptr<CollisionModel> >* pCollisions,
	                      OVR::Array<Ptr<CollisionModel> >* pGroundCollisions)
{
    if(pXmlDocument->LoadFile(fileName) != 0)
    {
        return false;
    }

    // Extract the relative path to our working directory for loading textures
    filePath[0] = 0;
    SPInt pos = 0;
	SPInt len = strlen(fileName);
    for(SPInt i = len; i > 0; i--)
    {
        if (fileName[i-1]=='\\' || fileName[i-1]=='/')
        {
            memcpy(filePath, fileName, i);
            filePath[i] = 0;
            break;
        }        
    }    

    // Load the textures
	OVR_DEBUG_LOG_TEXT(("Loading textures..."));
    XMLElement* pXmlTexture = pXmlDocument->FirstChildElement("scene")->FirstChildElement("textures");
    if (pXmlTexture)
    {
        pXmlTexture->QueryIntAttribute("count", &textureCount);
        pXmlTexture = pXmlTexture->FirstChildElement("texture");
    }

    for(int i = 0; i < textureCount; ++i)
    {
        const char* textureName = pXmlTexture->Attribute("fileName");
		SPInt       dotpos = strcspn(textureName, ".");
        char        fname[300];

		if (pos == len)
		{            
			OVR_sprintf(fname, 300, "%s", textureName);
		}
		else
		{
			OVR_sprintf(fname, 300, "%s%s", filePath, textureName);
		}

        SysFile* pFile = new SysFile(fname);
		Ptr<Texture> texture;
		if (textureName[dotpos + 1] == 'd' || textureName[dotpos + 1] == 'D')
		{
			// DDS file
			texture.SetPtr(*LoadTextureDDS(pRender, pFile));
		}
		else
		{
			texture.SetPtr(*LoadTextureTga(pRender, pFile));
		}

        Textures.PushBack(texture);
		pFile->Close();
		pFile->Release();
        pXmlTexture = pXmlTexture->NextSiblingElement("texture");
    }
	OVR_DEBUG_LOG_TEXT(("Done.\n"));

    // Load the models
	pXmlDocument->FirstChildElement("scene")->FirstChildElement("models")->
		          QueryIntAttribute("count", &modelCount);
	
		OVR_DEBUG_LOG(("Loading models... %i models to load...", modelCount));
    XMLElement* pXmlModel = pXmlDocument->FirstChildElement("scene")->
		                                  FirstChildElement("models")->FirstChildElement("model");
    for(int i = 0; i < modelCount; ++i)
    {
		if (i % 15 == 0)
		{
			OVR_DEBUG_LOG_TEXT(("%i models remaining...", modelCount - i));
		}
		Models.PushBack(*new Model(Prim_Triangles));
        bool isCollisionModel = false;
        pXmlModel->QueryBoolAttribute("isCollisionModel", &isCollisionModel);
        Models[i]->IsCollisionModel = isCollisionModel;
		if (isCollisionModel)
		{
			Models[i]->Visible = false;
		}

        //read the vertices
        OVR::Array<Vector3f> *vertices = new OVR::Array<Vector3f>();
        ParseVectorString(pXmlModel->FirstChildElement("vertices")->FirstChild()->
			              ToText()->Value(), vertices);

		for (unsigned int vertexIndex = 0; vertexIndex < vertices->GetSize(); ++vertexIndex)
		{
			vertices->At(vertexIndex).x *= -1.0f;
		}

        //read the normals
        OVR::Array<Vector3f> *normals = new OVR::Array<Vector3f>();
        ParseVectorString(pXmlModel->FirstChildElement("normals")->FirstChild()->
			              ToText()->Value(), normals);

		for (unsigned int normalIndex = 0; normalIndex < normals->GetSize(); ++normalIndex)
		{
			normals->At(normalIndex).z *= -1.0f;
		}

        //read the textures
        OVR::Array<Vector3f> *diffuseUVs = new OVR::Array<Vector3f>();
        OVR::Array<Vector3f> *lightmapUVs = new OVR::Array<Vector3f>();
        int         diffuseTextureIndex = -1;
        int         lightmapTextureIndex = -1;
        XMLElement* pXmlCurMaterial = pXmlModel->FirstChildElement("material");

        while(pXmlCurMaterial != NULL)
        {
            if(pXmlCurMaterial->Attribute("name", "diffuse"))
            {
                pXmlCurMaterial->FirstChildElement("texture")->
					             QueryIntAttribute("index", &diffuseTextureIndex);
                if(diffuseTextureIndex > -1)
                {
                    ParseVectorString(pXmlCurMaterial->FirstChildElement("texture")->
						              FirstChild()->ToText()->Value(), diffuseUVs, true);
                }
            }
            else if(pXmlCurMaterial->Attribute("name", "lightmap"))
            {
                pXmlCurMaterial->FirstChildElement("texture")->
					                               QueryIntAttribute("index", &lightmapTextureIndex);
                if(lightmapTextureIndex > -1)
                {
                    XMLElement* firstChildElement = pXmlCurMaterial->FirstChildElement("texture");
                    XMLNode* firstChild = firstChildElement->FirstChild();
                    XMLText* text = firstChild->ToText();
                    const char* value = text->Value();
                    ParseVectorString(value, lightmapUVs, true);
                }
            }

            pXmlCurMaterial = pXmlCurMaterial->NextSiblingElement("material");
        }

        //set up the shader
        Ptr<ShaderFill> shader = *new ShaderFill(*pRender->CreateShaderSet());
        shader->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Vertex, VShader_MVP));
        if(diffuseTextureIndex > -1)
        {
            shader->SetTexture(0, Textures[diffuseTextureIndex]);
            if(lightmapTextureIndex > -1)
            {
                shader->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_MultiTexture));
                shader->SetTexture(1, Textures[lightmapTextureIndex]);
            }
            else
            {
                shader->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_Texture));
            }
        }
        else
        {
            shader->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_LitGouraud));
        }
        Models[i]->Fill = shader;

        //add all the vertices to the model
        const UPInt numVerts = vertices->GetSize();
        for(UPInt v = 0; v < numVerts; ++v)
        {
            if(diffuseTextureIndex > -1)
            {
                if(lightmapTextureIndex > -1)
                {
                    Models[i]->AddVertex(vertices->At(v).z, vertices->At(v).y, vertices->At(v).x, Color(255, 255, 255),
                                          diffuseUVs->At(v).x, diffuseUVs->At(v).y, lightmapUVs->At(v).x, lightmapUVs->At(v).y,
                                          normals->At(v).x, normals->At(v).y, normals->At(v).z);
                }
                else
                {
                    Models[i]->AddVertex(vertices->At(v).z, vertices->At(v).y, vertices->At(v).x, Color(255, 255, 255),
                                          diffuseUVs->At(v).x, diffuseUVs->At(v).y, 0, 0,
                                          normals->At(v).x, normals->At(v).y, normals->At(v).z);
                }
            }
            else
            {
                Models[i]->AddVertex(vertices->At(v).z, vertices->At(v).y, vertices->At(v).x, Color(255, 0, 0, 128),
                                      0, 0, 0, 0,
                                      normals->At(v).x, normals->At(v).y, normals->At(v).z);
            }
        }

        // Read the vertex indices for the triangles
        const char* indexStr = pXmlModel->FirstChildElement("indices")->
                                          FirstChild()->ToText()->Value();
        UPInt stringLength = strlen(indexStr);

        for(UPInt j = 0; j < stringLength; )
        {
            UPInt k = j + 1;
            for(; k < stringLength; ++k)
            {
                if (indexStr[k] == ' ')
                    break;                
            }
            char text[20];
            for(UPInt l = 0; l < k - j; ++l)
            {
                text[l] = indexStr[j + l];
            }
            text[k - j] = '\0';

            Models[i]->Indices.PushBack((unsigned short)atoi(text));
            j = k + 1;
        }

        // Reverse index order to match original expected orientation
        Array<UInt16>& indices    = Models[i]->Indices;
        UPInt          indexCount = indices.GetSize();         

        for (UPInt revIndex = 0; revIndex < indexCount/2; revIndex++)
        {
            unsigned short itemp               = indices[revIndex];
            indices[revIndex]                  = indices[indexCount - revIndex - 1];
            indices[indexCount - revIndex - 1] = itemp;            
        }

        delete vertices;
        delete normals;
        delete diffuseUVs;
        delete lightmapUVs;

        pScene->World.Add(Models[i]);
        pScene->Models.PushBack(Models[i]);
        pXmlModel = pXmlModel->NextSiblingElement("model");
    }
	OVR_DEBUG_LOG(("Done."));

    //load the collision models
	OVR_DEBUG_LOG(("Loading collision models... "));
    XMLElement* pXmlCollisionModel = pXmlDocument->FirstChildElement("scene")->FirstChildElement("collisionModels");
    if (pXmlCollisionModel)
    {
		pXmlCollisionModel->QueryIntAttribute("count", &collisionModelCount);
        pXmlCollisionModel = pXmlCollisionModel->FirstChildElement("collisionModel");
    }

    XMLElement* pXmlPlane = NULL;
    for(int i = 0; i < collisionModelCount; ++i)
    {
        Ptr<CollisionModel> cm = *new CollisionModel();
        int planeCount = 0;
        pXmlCollisionModel->QueryIntAttribute("planeCount", &planeCount);

        pXmlPlane = pXmlCollisionModel->FirstChildElement("plane");
        for(int j = 0; j < planeCount; ++j)
        {
            Vector3f norm;
            pXmlPlane->QueryFloatAttribute("nx", &norm.x);
            pXmlPlane->QueryFloatAttribute("ny", &norm.y);
            pXmlPlane->QueryFloatAttribute("nz", &norm.z);
            float D;
            pXmlPlane->QueryFloatAttribute("d", &D);
            D -= 0.5f;
            Planef p(norm.z, norm.y, norm.x * -1.0f, D);
            cm->Add(p);
            pXmlPlane = pXmlPlane->NextSiblingElement("plane");
        }

        pCollisions->PushBack(cm);
        pXmlCollisionModel = pXmlCollisionModel->NextSiblingElement("collisionModel");
    }
	OVR_DEBUG_LOG(("done."));

    //load the ground collision models
	OVR_DEBUG_LOG(("Loading ground collision models..."));
    pXmlCollisionModel = pXmlDocument->FirstChildElement("scene")->FirstChildElement("groundCollisionModels");
    if (pXmlCollisionModel)
    {
		pXmlCollisionModel->QueryIntAttribute("count", &groundCollisionModelCount);
        pXmlCollisionModel = pXmlCollisionModel->FirstChildElement("collisionModel");
    }
    pXmlPlane = NULL;
    for(int i = 0; i < groundCollisionModelCount; ++i)
    {
        Ptr<CollisionModel> cm = *new CollisionModel();
        int planeCount = 0;
        pXmlCollisionModel->QueryIntAttribute("planeCount", &planeCount);

        pXmlPlane = pXmlCollisionModel->FirstChildElement("plane");
        for(int j = 0; j < planeCount; ++j)
        {
            Vector3f norm;
            pXmlPlane->QueryFloatAttribute("nx", &norm.x);
            pXmlPlane->QueryFloatAttribute("ny", &norm.y);
            pXmlPlane->QueryFloatAttribute("nz", &norm.z);
            float D;
            pXmlPlane->QueryFloatAttribute("d", &D);
            Planef p(norm.z, norm.y, norm.x * -1.0f, D);
            cm->Add(p);
            pXmlPlane = pXmlPlane->NextSiblingElement("plane");
        }

        pGroundCollisions->PushBack(cm);
        pXmlCollisionModel = pXmlCollisionModel->NextSiblingElement("collisionModel");
    }
	OVR_DEBUG_LOG(("done."));
	return true;
}

void XmlHandler::ParseVectorString(const char* str, OVR::Array<OVR::Vector3f> *array,
	                               bool is2element)
{
    UPInt stride = is2element ? 2 : 3;
    UPInt stringLength = strlen(str);
    UPInt element = 0;
    float v[3];

    for(UPInt j = 0; j < stringLength;)
    {
        UPInt k = j + 1;
        for(; k < stringLength; ++k)
        {
            if(str[k] == ' ')
            {
                break;
            }
        }
        char text[20];
        for(UPInt l = 0; l < k - j; ++l)
        {
            text[l] = str[j + l];
        }
        text[k - j] = '\0';
        v[element] = (float)atof(text);

        if(element == (stride - 1))
        {
            //we've got all the elements of our vertex, so store them
            OVR::Vector3f vect;
            vect.x = v[0];
            vect.y = v[1];
            vect.z = is2element ? 0.0f : v[2];
            array->PushBack(vect);
        }

        j = k + 1;
        element = (element + 1) % stride;
    }
}

}} // OVR::Render

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif
