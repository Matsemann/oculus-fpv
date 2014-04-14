/************************************************************************************

Filename    :   Render_LoadeTextureTGA.cpp
Content     :   Loading of TGA implementation
Created     :   October, 2012
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


#include "Render_Device.h"

namespace OVR { namespace Render {

Texture* LoadTextureTga(RenderDevice* ren, File* f, unsigned char alpha)
{
    f->SeekToBegin();
    
    int desclen = f->ReadUByte();
    int palette = f->ReadUByte();
    OVR_UNUSED(palette);
    int imgtype = f->ReadUByte();
    f->ReadUInt16();
    int palCount = f->ReadUInt16();
    int palSize = f->ReadUByte();
    f->ReadUInt16();
    f->ReadUInt16();
    int width = f->ReadUInt16();
    int height = f->ReadUInt16();
    int bpp = f->ReadUByte();
    f->ReadUByte();
    int imgsize = width * height * 4;
    unsigned char* imgdata = (unsigned char*) OVR_ALLOC(imgsize);
    unsigned char buf[16];
    f->Read(imgdata, desclen);
    f->Read(imgdata, palCount * (palSize + 7) >> 3);
    int bpl = width * 4;

    switch (imgtype)
    {
    case 2:
        switch (bpp)
        {
        case 24:
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                {
                    f->Read(buf, 3);
                    imgdata[y*bpl+x*4+0] = buf[2];
                    imgdata[y*bpl+x*4+1] = buf[1];
                    imgdata[y*bpl+x*4+2] = buf[0];
                    imgdata[y*bpl+x*4+3] = alpha;
                }
            break;
        case 32:
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                {
                    f->Read(buf, 4);
                    imgdata[y*bpl+x*4+0] = buf[2];
                    imgdata[y*bpl+x*4+1] = buf[1];
                    imgdata[y*bpl+x*4+2] = buf[0];
                    if (buf[3] == 255)
                        imgdata[y*bpl+x*4+3] = alpha;
                    else
                    imgdata[y*bpl+x*4+3] = buf[3];
                }
            break;

        default:
            OVR_FREE(imgdata);
            return NULL;
        }
        break;

    default:
        OVR_FREE(imgdata);
        return NULL;
    }

	Texture* out = ren->CreateTexture(Texture_RGBA|Texture_GenMipmaps, width, height, imgdata);

	// check for clamp based on texture name
	if(strstr(f->GetFilePath(), "_c."))
	{
		out->SetSampleMode(Sample_Clamp);
	}

    OVR_FREE(imgdata);
    return out;
}

}}
