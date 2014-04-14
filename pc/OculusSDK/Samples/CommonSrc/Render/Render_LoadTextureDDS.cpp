/************************************************************************************

Filename    :   WavPlayer_OSX.h
Content     :   A DDS file loader for cross-platform compressed texture support.
Created     :   March 5, 2013
Authors     :   Peter Hoff, Dan Goodman, Bryan Croteau

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
#include "Render_Device.h"

#ifdef OVR_DEFINE_NEW
#undef new
#endif

namespace OVR { namespace Render {

static const UPInt OVR_DDS_PF_FOURCC = 0x4;
static const UInt32 OVR_DTX1_MAGIC_NUMBER = 827611204;
static const UInt32 OVR_DTX5_MAGIC_NUMBER = 894720068;

struct OVR_DDS_PIXELFORMAT
{
    UInt32 Size;
    UInt32 Flags;
    UInt32 FourCC;
    UInt32 RGBBitCount;
    UInt32 RBitMask;
    UInt32 GBitMask;
    UInt32 BBitMask;
    UInt32 ABitMask;
};

struct OVR_DDS_HEADER
{
    UInt32				Size;
    UInt32				Flags;
    UInt32				Height;
    UInt32				Width;
    UInt32				PitchOrLinearSize;
    UInt32				Depth;
    UInt32				MipMapCount;
    UInt32				Reserved1[11];
    OVR_DDS_PIXELFORMAT PixelFormat;
    UInt32				Caps;
    UInt32				Caps2;
    UInt32				Caps3;
    UInt32				Caps4;
    UInt32				Reserved2;
};

Texture* LoadTextureDDS(RenderDevice* ren, File* f)
{
    OVR_DDS_HEADER header;
    unsigned char filecode[4];

    f->Read(filecode, 4);
    if (strncmp((const char*)filecode, "DDS ", 4) != 0)
    {
        return NULL;
    }

    f->Read((unsigned char*)(&header), sizeof(header));

    int width = header.Width;
    int height = header.Height;

    int format = 0;

    UInt32 mipCount = header.MipMapCount;
    if(mipCount <= 0)
    {
        mipCount = 1;
    }
    if(header.PixelFormat.Flags & OVR_DDS_PF_FOURCC)
    {
        if(header.PixelFormat.FourCC == OVR_DTX1_MAGIC_NUMBER)
        {
            format = Texture_DXT1;
        }
        else if(header.PixelFormat.FourCC == OVR_DTX5_MAGIC_NUMBER)
        {
            format = Texture_DXT5;
        }
        else
        {
            return NULL;
        }
    }

    int            byteLen = f->BytesAvailable();
    unsigned char* bytes   = new unsigned char[byteLen];
    f->Read(bytes, byteLen);
    Texture* out = ren->CreateTexture(format, (int)width, (int)height, bytes, mipCount);
    if(strstr(f->GetFilePath(), "_c."))
    {
        out->SetSampleMode(Sample_Clamp);
    }
    OVR_FREE(bytes);
    return out;
}


}}

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif