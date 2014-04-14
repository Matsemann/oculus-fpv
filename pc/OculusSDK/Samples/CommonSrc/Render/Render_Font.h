/************************************************************************************

Filename    :   Render_Font_h
Content     :   Font data structure used by renderer
Created     :   September, 2012
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

#ifndef OVR_Render_Font_h
#define OVR_Render_Font_h

namespace OVR { namespace Render {

class Fill;

struct Font
{
    struct Char
    {
        short x, y;       // offset
        short advance;
        float u1, v1, u2, v2;
    };

    int            lineheight, ascent, descent;
    const Char*    chars;
    const short**  kerning;
    int            twidth, theight;
    const
    unsigned char* tex;
    mutable Fill*  fill;
};

}}

#endif
