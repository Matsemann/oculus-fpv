/************************************************************************************

Filename    :   Render_GL_Device.cpp
Content     :   RenderDevice implementation for OpenGL
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

#include "../Render/Render_GL_Device.h"
#include "Kernel/OVR_Log.h"

namespace OVR { namespace Render { namespace GL {



// GL Hooks for PC.
#if defined(OVR_OS_WIN32)

PFNGLGENFRAMEBUFFERSEXTPROC              glGenFramebuffersEXT;
PFNGLDELETESHADERPROC                    glDeleteShader;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC       glCheckFramebufferStatusEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC      glFramebufferRenderbufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC         glFramebufferTexture2DEXT;
PFNGLBINDFRAMEBUFFEREXTPROC              glBindFramebufferEXT;
PFNGLACTIVETEXTUREPROC                   glActiveTexture;
PFNGLDISABLEVERTEXATTRIBARRAYPROC        glDisableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC             glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC         glEnableVertexAttribArray;
PFNGLBINDBUFFERPROC                      glBindBuffer;
PFNGLUNIFORMMATRIX4FVPROC                glUniformMatrix4fv;
PFNGLDELETEBUFFERSPROC                   glDeleteBuffers;
PFNGLBUFFERDATAPROC                      glBufferData;
PFNGLGENBUFFERSPROC                      glGenBuffers;
PFNGLMAPBUFFERPROC                       glMapBuffer;
PFNGLUNMAPBUFFERPROC                     glUnmapBuffer;
PFNGLGETSHADERINFOLOGPROC                glGetShaderInfoLog;
PFNGLGETSHADERIVPROC                     glGetShaderiv;
PFNGLCOMPILESHADERPROC                   glCompileShader;
PFNGLSHADERSOURCEPROC                    glShaderSource;
PFNGLCREATESHADERPROC                    glCreateShader;
PFNGLCREATEPROGRAMPROC                   glCreateProgram;
PFNGLATTACHSHADERPROC                    glAttachShader;
PFNGLDETACHSHADERPROC                    glDetachShader;
PFNGLDELETEPROGRAMPROC                   glDeleteProgram;
PFNGLUNIFORM1IPROC                       glUniform1i;
PFNGLGETUNIFORMLOCATIONPROC              glGetUniformLocation;
PFNGLGETACTIVEUNIFORMPROC                glGetActiveUniform;
PFNGLUSEPROGRAMPROC                      glUseProgram;
PFNGLGETPROGRAMINFOLOGPROC               glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC                    glGetProgramiv;
PFNGLLINKPROGRAMPROC                     glLinkProgram;
PFNGLBINDATTRIBLOCATIONPROC              glBindAttribLocation;
PFNGLUNIFORM4FVPROC                      glUniform4fv;
PFNGLUNIFORM3FVPROC                      glUniform3fv;
PFNGLUNIFORM2FVPROC                      glUniform2fv;
PFNGLUNIFORM1FVPROC                      glUniform1fv;
PFNGLCOMPRESSEDTEXIMAGE2DPROC            glCompressedTexImage2D;
PFNGLRENDERBUFFERSTORAGEEXTPROC          glRenderbufferStorageEXT;
PFNGLBINDRENDERBUFFEREXTPROC             glBindRenderbufferEXT;
PFNGLGENRENDERBUFFERSEXTPROC             glGenRenderbuffersEXT;
PFNGLDELETERENDERBUFFERSEXTPROC          glDeleteRenderbuffersEXT;

PFNGLGENVERTEXARRAYSPROC                 glGenVertexArrays;

void InitGLExtensions()
{
    if (glGenFramebuffersEXT)
        return;

    glGenFramebuffersEXT =              (PFNGLGENFRAMEBUFFERSEXTPROC)              wglGetProcAddress("glGenFramebuffersEXT");
    glDeleteShader =                    (PFNGLDELETESHADERPROC)                    wglGetProcAddress("glDeleteShader");
    glCheckFramebufferStatusEXT =       (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)       wglGetProcAddress("glCheckFramebufferStatusEXT");
    glFramebufferRenderbufferEXT =      (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)      wglGetProcAddress("glFramebufferRenderbufferEXT");
    glFramebufferTexture2DEXT =         (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)         wglGetProcAddress("glFramebufferTexture2DEXT");
    glBindFramebufferEXT =              (PFNGLBINDFRAMEBUFFEREXTPROC)              wglGetProcAddress("glBindFramebufferEXT");
    glActiveTexture =                   (PFNGLACTIVETEXTUREPROC)                   wglGetProcAddress("glActiveTexture");
    glDisableVertexAttribArray =        (PFNGLDISABLEVERTEXATTRIBARRAYPROC)        wglGetProcAddress("glDisableVertexAttribArray");
    glVertexAttribPointer =             (PFNGLVERTEXATTRIBPOINTERPROC)             wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray =         (PFNGLENABLEVERTEXATTRIBARRAYPROC)         wglGetProcAddress("glEnableVertexAttribArray");
    glBindBuffer =                      (PFNGLBINDBUFFERPROC)                      wglGetProcAddress("glBindBuffer");
    glUniformMatrix4fv =                (PFNGLUNIFORMMATRIX4FVPROC)                wglGetProcAddress("glUniformMatrix4fv");
    glDeleteBuffers =                   (PFNGLDELETEBUFFERSPROC)                   wglGetProcAddress("glDeleteBuffers");
    glBufferData =                      (PFNGLBUFFERDATAPROC)                      wglGetProcAddress("glBufferData");
    glGenBuffers =                      (PFNGLGENBUFFERSPROC)                      wglGetProcAddress("glGenBuffers");
    glMapBuffer =                       (PFNGLMAPBUFFERPROC)                       wglGetProcAddress("glMapBuffer");
    glUnmapBuffer =                     (PFNGLUNMAPBUFFERPROC)                     wglGetProcAddress("glUnmapBuffer");
    glGetShaderInfoLog =                (PFNGLGETSHADERINFOLOGPROC)                wglGetProcAddress("glGetShaderInfoLog");
    glGetShaderiv =                     (PFNGLGETSHADERIVPROC)                     wglGetProcAddress("glGetShaderiv");
    glCompileShader =                   (PFNGLCOMPILESHADERPROC)                   wglGetProcAddress("glCompileShader");
    glShaderSource =                    (PFNGLSHADERSOURCEPROC)                    wglGetProcAddress("glShaderSource");
    glCreateShader =                    (PFNGLCREATESHADERPROC)                    wglGetProcAddress("glCreateShader");
    glCreateProgram =                   (PFNGLCREATEPROGRAMPROC)                   wglGetProcAddress("glCreateProgram");
    glAttachShader =                    (PFNGLATTACHSHADERPROC)                    wglGetProcAddress("glAttachShader");
    glDetachShader =                    (PFNGLDETACHSHADERPROC)                    wglGetProcAddress("glDetachShader");
    glDeleteProgram =                   (PFNGLDELETEPROGRAMPROC)                   wglGetProcAddress("glDeleteProgram");
    glUniform1i =                       (PFNGLUNIFORM1IPROC)                       wglGetProcAddress("glUniform1i");
    glGetUniformLocation =              (PFNGLGETUNIFORMLOCATIONPROC)              wglGetProcAddress("glGetUniformLocation");
    glGetActiveUniform =                (PFNGLGETACTIVEUNIFORMPROC)                wglGetProcAddress("glGetActiveUniform");
    glUseProgram =                      (PFNGLUSEPROGRAMPROC)                      wglGetProcAddress("glUseProgram");
    glGetProgramInfoLog =               (PFNGLGETPROGRAMINFOLOGPROC)               wglGetProcAddress("glGetProgramInfoLog");
    glGetProgramiv =                    (PFNGLGETPROGRAMIVPROC)                    wglGetProcAddress("glGetProgramiv");
    glLinkProgram =                     (PFNGLLINKPROGRAMPROC)                     wglGetProcAddress("glLinkProgram");
    glBindAttribLocation =              (PFNGLBINDATTRIBLOCATIONPROC)              wglGetProcAddress("glBindAttribLocation");
    glUniform4fv =                      (PFNGLUNIFORM4FVPROC)                      wglGetProcAddress("glUniform4fv");
    glUniform3fv =                      (PFNGLUNIFORM3FVPROC)                      wglGetProcAddress("glUniform3fv");
    glUniform2fv =                      (PFNGLUNIFORM2FVPROC)                      wglGetProcAddress("glUniform2fv");
    glUniform1fv =                      (PFNGLUNIFORM1FVPROC)                      wglGetProcAddress("glUniform1fv");
    glCompressedTexImage2D =            (PFNGLCOMPRESSEDTEXIMAGE2DPROC)            wglGetProcAddress("glCompressedTexImage2D");
    glRenderbufferStorageEXT =          (PFNGLRENDERBUFFERSTORAGEEXTPROC)          wglGetProcAddress("glRenderbufferStorageEXT");
    glBindRenderbufferEXT =             (PFNGLBINDRENDERBUFFEREXTPROC)             wglGetProcAddress("glBindRenderbufferEXT");
    glGenRenderbuffersEXT =             (PFNGLGENRENDERBUFFERSEXTPROC)             wglGetProcAddress("glGenRenderbuffersEXT");
    glDeleteRenderbuffersEXT =          (PFNGLDELETERENDERBUFFERSEXTPROC)          wglGetProcAddress("glDeleteRenderbuffersEXT");


    glGenVertexArrays =                 (PFNGLGENVERTEXARRAYSPROC)                 wglGetProcAddress("glGenVertexArrays");
}

#endif


static const char* StdVertexShaderSrc =
    "uniform mat4 Proj;\n"
    "uniform mat4 View;\n"
    "attribute vec4 Position;\n"
    "attribute vec4 Color;\n"
    "attribute vec2 TexCoord;\n"
    "attribute vec2 TexCoord1;\n"
    "attribute vec3 Normal;\n"
    "varying  vec4 oColor;\n"
    "varying  vec2 oTexCoord;\n"
    "varying  vec2 oTexCoord1;\n"
    "varying  vec3 oNormal;\n"
    "varying  vec3 oVPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = Proj * (View * Position);\n"
    "   oNormal = vec3(View * vec4(Normal,0));\n"
    "   oVPos = vec3(View * Position);\n"
    "   oTexCoord = TexCoord;\n"
    "   oTexCoord1 = TexCoord1;\n"
    "   oColor = Color;\n"
    "}\n";

static const char* DirectVertexShaderSrc =
    "uniform mat4 View;\n"
    "attribute vec4 Position;\n"
    "attribute vec4 Color;\n"
    "attribute vec2 TexCoord;\n"
    "attribute vec3 Normal;\n"
    "varying  vec4 oColor;\n"
    "varying  vec2 oTexCoord;\n"
    "varying  vec3 oNormal;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = View * Position;\n"
    "   oTexCoord = TexCoord;\n"
    "   oColor = Color;\n"
    "   oNormal = vec3(View * vec4(Normal,0));\n"
    "}\n";

static const char* SolidFragShaderSrc =
    "uniform vec4 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = Color;\n"
    "}\n";

static const char* GouraudFragShaderSrc =
    "varying vec4 oColor;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = oColor;\n"
    "}\n";

static const char* TextureFragShaderSrc =
    "uniform sampler2D Texture0;\n"
    "varying vec4 oColor;\n"
    "varying vec2 oTexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = oColor * texture2D(Texture0, oTexCoord);\n"
    "   if (gl_FragColor.a < 0.4)\n"
    "       discard;\n"
    "}\n";

#define LIGHTING_COMMON                                                 \
    "uniform   vec3 Ambient;\n"                                               \
    "uniform   vec4 LightPos[8];\n"                                           \
    "uniform   vec4 LightColor[8];\n"                                         \
    "uniform   float LightCount;\n"                                          \
    "varying   vec4 oColor;\n"                                                  \
    "varying   vec2 oTexCoord;\n"                                               \
    "varying   vec3 oNormal;\n"                                                 \
    "varying   vec3 oVPos;\n"                                                   \
    "vec4 DoLight()\n"                                        \
    "{\n"                                                               \
    "   vec3 norm = normalize(oNormal);\n"                             \
    "   vec3 light = Ambient;\n"                                        \
    "   for (int i = 0; i < int(LightCount); i++)\n"                \
    "   {\n"                                                            \
    "       vec3 ltp = (LightPos[i].xyz - oVPos);\n"              \
    "       float  ldist = length(ltp);\n"                             \
    "       ltp = normalize(ltp);\n"                             \
    "       light += clamp(LightColor[i].rgb * oColor.rgb * (dot(norm, ltp) / ldist), 0.0,1.0);\n" \
    "   }\n"                                                            \
    "   return vec4(light, oColor.a);\n"                               \
    "}\n"

static const char* LitSolidFragShaderSrc =
    LIGHTING_COMMON
    "void main()\n"
    "{\n"
    "   gl_FragColor = DoLight() * oColor;\n"
    "}\n";

static const char* LitTextureFragShaderSrc =
    "uniform sampler2D Texture0;\n"
    LIGHTING_COMMON
    "void main()\n"
    "{\n"
    "   gl_FragColor = DoLight() * texture2D(Texture0, oTexCoord);\n"
    "}\n";

static const char* AlphaTextureFragShaderSrc =
    "uniform sampler2D Texture0;\n"
    "varying vec4 oColor;\n"
    "varying vec2 oTexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = oColor * vec4(1,1,1,texture2D(Texture0, oTexCoord).a);\n"
    "}\n";

static const char* MultiTextureFragShaderSrc =
    "uniform sampler2D Texture0;\n"
    "uniform sampler2D Texture1;\n"
    "varying vec4 oColor;\n"
    "varying vec2 oTexCoord;\n"
    "varying vec2 oTexCoord1;\n"
    "void main()\n"
    "{\n"
	"	vec4 color1 = texture2D(Texture0, oTexCoord);\n"
	"	vec4 color2 = texture2D(Texture1, oTexCoord1);\n"
	"	color2.rgb = color2.rgb * mix(1.9, 1.2, clamp(length(color2.rgb),0.0,1.0));\n"
	"	color2 = color1 * color2;\n"
	"   if (color2.a <= 0.6)\n"
	"		discard;\n"
	"	gl_FragColor = color2;\n"
    "}\n";

static const char* PostProcessVertexShaderSrc =
    "uniform mat4 View;\n"
    "uniform mat4 Texm;\n"
    "attribute vec4 Position;\n"
    "attribute vec2 TexCoord;\n"
    "varying  vec2 oTexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = View * Position;\n"
    "   oTexCoord = vec2(Texm * vec4(TexCoord,0,1));\n"
    "   oTexCoord.y = 1.0-oTexCoord.y;\n"
    "}\n";

static const char* PostProcessFragShaderSrc =
    "uniform vec2 LensCenter;\n"
    "uniform vec2 ScreenCenter;\n"
    "uniform vec2 Scale;\n"
    "uniform vec2 ScaleIn;\n"
    "uniform vec4 HmdWarpParam;\n"
    "uniform sampler2D Texture0;\n"
    "varying vec2 oTexCoord;\n"
    "\n"
    "vec2 HmdWarp(vec2 in01)\n"
    "{\n"
    "   vec2  theta = (in01 - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
    "   float rSq = theta.x * theta.x + theta.y * theta.y;\n"
    "   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
    "                           HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
    "   return LensCenter + Scale * theta1;\n"
    "}\n"
    "void main()\n"
    "{\n"
    "   vec2 tc = HmdWarp(oTexCoord);\n"
    "   if (!all(equal(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tc)))\n"
    "       gl_FragColor = vec4(0);\n"
    "   else\n"
    "       gl_FragColor = texture2D(Texture0, tc);\n"
    "}\n";

// Shader with lens distortion and chromatic aberration correction.
static const char* PostProcessFullFragShaderSrc =
    "uniform vec2 LensCenter;\n"
    "uniform vec2 ScreenCenter;\n"
    "uniform vec2 Scale;\n"
    "uniform vec2 ScaleIn;\n"
    "uniform vec4 HmdWarpParam;\n"
    "uniform vec4 ChromAbParam;\n"
    "uniform sampler2D Texture0;\n"
    "varying vec2 oTexCoord;\n"
    "\n"
    // Scales input texture coordinates for distortion.
    // ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
    // larger due to aspect ratio.
    "void main()\n"
    "{\n"
    "   vec2  theta = (oTexCoord - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
    "   float rSq= theta.x * theta.x + theta.y * theta.y;\n"
    "   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
    "                  HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
    "   \n"
    "   // Detect whether blue texture coordinates are out of range since these will scaled out the furthest.\n"
    "   vec2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);\n"
    "   vec2 tcBlue = LensCenter + Scale * thetaBlue;\n"
    "   if (!all(equal(clamp(tcBlue, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tcBlue)))\n"
    "   {\n"
    "       gl_FragColor = vec4(0);\n"
    "       return;\n"
    "   }\n"
    "   \n"
    "   // Now do blue texture lookup.\n"
    "   float blue = texture2D(Texture0, tcBlue).b;\n"
    "   \n"
    "   // Do green lookup (no scaling).\n"
    "   vec2  tcGreen = LensCenter + Scale * theta1;\n"
    "   vec4  center = texture2D(Texture0, tcGreen);\n"
    "   \n"
    "   // Do red scale and lookup.\n"
    "   vec2  thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);\n"
    "   vec2  tcRed = LensCenter + Scale * thetaRed;\n"
    "   float red = texture2D(Texture0, tcRed).r;\n"
    "   \n"
    "   gl_FragColor = vec4(red, center.g, blue, center.a);\n"
    "}\n";

static const char* VShaderSrcs[VShader_Count] =
{
    DirectVertexShaderSrc,
    StdVertexShaderSrc,
    PostProcessVertexShaderSrc
};
static const char* FShaderSrcs[FShader_Count] =
{
    SolidFragShaderSrc,
    GouraudFragShaderSrc,
    TextureFragShaderSrc,
    AlphaTextureFragShaderSrc,
    PostProcessFragShaderSrc,
    PostProcessFullFragShaderSrc,
    LitSolidFragShaderSrc,
    LitTextureFragShaderSrc,
    MultiTextureFragShaderSrc
};



RenderDevice::RenderDevice(const RendererParams&)
{
    for (int i = 0; i < VShader_Count; i++)
        VertexShaders[i] = *new Shader(this, Shader_Vertex, VShaderSrcs[i]);

    for (int i = 0; i < FShader_Count; i++)
        FragShaders[i] = *new Shader(this, Shader_Fragment, FShaderSrcs[i]);

    Ptr<ShaderSet> gouraudShaders = *new ShaderSet();
    gouraudShaders->SetShader(VertexShaders[VShader_MVP]);
    gouraudShaders->SetShader(FragShaders[FShader_Gouraud]);
    DefaultFill = *new ShaderFill(gouraudShaders);

    glGenFramebuffersEXT(1, &CurrentFbo);
}

Shader *RenderDevice::LoadBuiltinShader(ShaderStage stage, int shader)
{
    switch (stage)
    {
    case Shader_Vertex: return VertexShaders[shader];
    case Shader_Fragment: return FragShaders[shader];
    default:
        return NULL;
    }
}


void RenderDevice::BeginRendering()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    glLineWidth(3.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RenderDevice::SetDepthMode(bool enable, bool write, CompareFunc func)
{
    if (enable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(write);
        switch (func)
        {
        case Compare_Always:  glDepthFunc(GL_ALWAYS); break;
        case Compare_Less:    glDepthFunc(GL_LESS); break;
        case Compare_Greater: glDepthFunc(GL_GREATER); break;
        default: assert(0);
        }
    }
    else
        glDisable(GL_DEPTH_TEST);
}

void RenderDevice::SetRealViewport(const Viewport& vp)
{
    int wh;
    if (CurRenderTarget)
        wh = CurRenderTarget->Height;
    else
        wh = WindowHeight;
    glViewport(vp.x, wh-vp.y-vp.h, vp.w, vp.h);

    glEnable(GL_SCISSOR_TEST);
    glScissor(vp.x, wh-vp.y-vp.h, vp.w, vp.h);
}

void RenderDevice::Clear(float r, float g, float b, float a, float depth)
{
    glClearColor(r,g,b,a);
    glClearDepth(depth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

RBuffer* RenderDevice::GetDepthBuffer(int w, int h, int ms)
{
    for (unsigned i = 0; i < DepthBuffers.GetSize(); i++)
        if (w == DepthBuffers[i]->Width && h == DepthBuffers[i]->Height)// && ms == DepthBuffers[i]->Samples)
            return DepthBuffers[i];

    //Ptr<Texture> newDepth = *CreateTexture(Texture_Depth|Texture_RenderTarget|ms, w, h, NULL);
    Ptr<RBuffer> newDepth = *new RBuffer(GL_DEPTH24_STENCIL8, w, h); // combined depth stencil
    DepthBuffers.PushBack(newDepth);
    return newDepth.GetPtr();
}

void RenderDevice::SetRenderTarget(Render::Texture* color, Render::Texture*, Render::Texture* stencil)
{
    OVR_UNUSED(stencil);

    CurRenderTarget = (Texture*)color;
    if (color == NULL)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        return;
    }
    //if (depth == NULL)
    RBuffer* depth = GetDepthBuffer(color->GetWidth(), color->GetHeight(), 0); //CurRenderTarget->Samples);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, CurrentFbo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ((Texture*)color)->TexId, 0);
    if (depth)
        //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, ((Texture*)depth)->TexId, 0);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, ((RBuffer*)depth)->BufId);
    else
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        OVR_DEBUG_LOG(("framebuffer not complete: %x", status));
}


void RenderDevice::SetWorldUniforms(const Matrix4f& proj)
{
    Proj = proj.Transposed();
}

void RenderDevice::SetTexture(Render::ShaderStage, int slot, const Texture* t)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ((Texture*)t)->TexId);
    glActiveTexture(GL_TEXTURE0);
}

Buffer* RenderDevice::CreateBuffer()
{
    return new Buffer(this);
}

Fill* RenderDevice::CreateSimpleFill(int flags)
{
    OVR_UNUSED(flags);
    return DefaultFill;
}


void RenderDevice::Render(const Matrix4f& matrix, Model* model)
{
    // Store data in buffers if not already
    if (!model->VertexBuffer)
    {
        Ptr<Render::Buffer> vb = *CreateBuffer();
        vb->Data(Buffer_Vertex, &model->Vertices[0], model->Vertices.GetSize() * sizeof(Vertex));
        model->VertexBuffer = vb;
    }
    if (!model->IndexBuffer)
    {
        Ptr<Render::Buffer> ib = *CreateBuffer();
        ib->Data(Buffer_Index, &model->Indices[0], model->Indices.GetSize() * 2);
        model->IndexBuffer = ib;
    }

    Render(model->Fill ? (const Fill*)model->Fill : (const Fill*)DefaultFill,
           model->VertexBuffer, model->IndexBuffer,
           matrix, 0, (int)model->Indices.GetSize(), model->GetPrimType());
}

void RenderDevice::Render(const Fill* fill, Render::Buffer* vertices, Render::Buffer* indices,
                      const Matrix4f& matrix, int offset, int count, PrimitiveType rprim)
{
    ShaderSet* shaders = (ShaderSet*) ((ShaderFill*)fill)->GetShaders();

    GLenum prim;
    switch (rprim)
    {
    case Prim_Triangles:
        prim = GL_TRIANGLES;
        break;
    case Prim_Lines:
        prim = GL_LINES;
        break;
    case Prim_TriangleStrip:
        prim = GL_TRIANGLE_STRIP;
        break;
    default:
        assert(0);
        return;
    }

    fill->Set();
    if (shaders->ProjLoc >= 0)
        glUniformMatrix4fv(shaders->ProjLoc, 1, 0, &Proj.M[0][0]);
    if (shaders->ViewLoc >= 0)
        glUniformMatrix4fv(shaders->ViewLoc, 1, 0, &matrix.Transposed().M[0][0]);

    if (shaders->UsesLighting && Lighting->Version != shaders->LightingVer)
    {
        shaders->LightingVer = Lighting->Version;
        Lighting->Set(shaders);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ((Buffer*)vertices)->GLBuffer);
    for (int i = 0; i < 5; i++)
        glEnableVertexAttribArray(i);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (char*)offset + offsetof(Vertex, Pos));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (char*)offset + offsetof(Vertex, C));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (char*)offset + offsetof(Vertex, U));
    glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(Vertex), (char*)offset + offsetof(Vertex, U2));
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), (char*)offset + offsetof(Vertex, Norm));

    if (indices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((Buffer*)indices)->GLBuffer);
        glDrawElements(prim, count, GL_UNSIGNED_SHORT, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
        glDrawArrays(prim, 0, count);
    }

    for (int i = 0; i < 5; i++)
        glDisableVertexAttribArray(i);
}

void RenderDevice::RenderWithAlpha(const Fill* fill, Render::Buffer* vertices, Render::Buffer* indices,
                                   const Matrix4f& matrix, int offset, int count, PrimitiveType rprim)
{
    //glEnable(GL_BLEND);
    Render(fill, vertices, indices, matrix, offset, count, rprim);
    //glDisable(GL_BLEND);
}

void RenderDevice::SetLighting(const LightingParams* lt)
{
    Lighting = lt;
}

Buffer::~Buffer()
{
    if (GLBuffer)
        glDeleteBuffers(1, &GLBuffer);
}

bool Buffer::Data(int use, const void* buffer, size_t size)
{
    switch (use & Buffer_TypeMask)
    {
    case Buffer_Index:     Use = GL_ELEMENT_ARRAY_BUFFER; break;
    default:               Use = GL_ARRAY_BUFFER; break;
    }

    if (!GLBuffer)
        glGenBuffers(1, &GLBuffer);

    int mode = GL_DYNAMIC_DRAW;
    if (use & Buffer_ReadOnly)
        mode = GL_STATIC_DRAW;

    glBindBuffer(Use, GLBuffer);
    glBufferData(Use, size, buffer, mode);
    glBindBuffer(Use, 0);
    return 1;
}

void* Buffer::Map(size_t start, size_t size, int flags)
{
    int mode = GL_WRITE_ONLY;
    //if (flags & Map_Unsynchronized)
    //    mode |= GL_MAP_UNSYNCHRONIZED;
    
    glBindBuffer(Use, GLBuffer);
    void* v = glMapBuffer(Use, mode);
    glBindBuffer(Use, 0);
    return v;
}

bool Buffer::Unmap(void*)
{
    glBindBuffer(Use, GLBuffer);
    int r = glUnmapBuffer(Use);
    glBindBuffer(Use, 0);
    return r;
}

bool Shader::Compile(const char* src)
{
    if (!GLShader)
        GLShader = glCreateShader(GLStage());

    glShaderSource(GLShader, 1, &src, 0);
    glCompileShader(GLShader);
    GLint r;
    glGetShaderiv(GLShader, GL_COMPILE_STATUS, &r);
    if (!r)
    {
        GLchar msg[1024];
        glGetShaderInfoLog(GLShader, sizeof(msg), 0, msg);
        if (msg[0])
            OVR_DEBUG_LOG(("Compiling shader\n%s\nfailed: %s\n", src, msg));
        if (!r)
            return 0;
    }
    return 1;
}

ShaderSet::ShaderSet()
{
    Prog = glCreateProgram();
}
ShaderSet::~ShaderSet()
{
    glDeleteProgram(Prog);
}

bool ShaderSet::Link()
{
    glBindAttribLocation(Prog, 0, "Position");
    glBindAttribLocation(Prog, 1, "Color");
    glBindAttribLocation(Prog, 2, "TexCoord");
    glBindAttribLocation(Prog, 3, "TexCoord1");
    glBindAttribLocation(Prog, 4, "Normal");

    glLinkProgram(Prog);
    GLint r;
    glGetProgramiv(Prog, GL_LINK_STATUS, &r);
    if (!r)
    {
        GLchar msg[1024];
        glGetProgramInfoLog(Prog, sizeof(msg), 0, msg);
        OVR_DEBUG_LOG(("Linking shaders failed: %s\n", msg));
        if (!r)
            return 0;
    }
    glUseProgram(Prog);

    UniformInfo.Clear();
    LightingVer = 0;
    UsesLighting = 0;
    GLuint i = 0;
    for(;; i++)
    {
        GLsizei namelen;
        GLint size = 0;
        GLenum type;
        GLchar name[32];
        glGetActiveUniform(Prog, i, sizeof(name), &namelen, &size, &type, name);
        if (size)
        {
            int l = glGetUniformLocation(Prog, name);
            char *np = name;
            while (*np)
            {
                if (*np == '[')
                    *np = 0;
                np++;
            }
            Uniform u;
            u.Name = name;
            u.Location = l;
            u.Size = size;
            switch (type)
            {
            case GL_FLOAT:      u.Type = 1; break;
            case GL_FLOAT_VEC2: u.Type = 2; break;
            case GL_FLOAT_VEC3: u.Type = 3; break;
            case GL_FLOAT_VEC4: u.Type = 4; break;
            case GL_FLOAT_MAT4: u.Type = 16; break;
            default:
                continue;
            }
            UniformInfo.PushBack(u);
            if (!strcmp(name, "LightCount"))
                UsesLighting = 1;
        }
        else
            break;
    }

    ProjLoc = glGetUniformLocation(Prog, "Proj");
    ViewLoc = glGetUniformLocation(Prog, "View");
    for (int i = 0; i < 8; i++)
    {
        char texv[32];
        sprintf(texv, "Texture%d", i);
        TexLoc[i] = glGetUniformLocation(Prog, texv);
        if (TexLoc[i] < 0)
            break;

        glUniform1i(TexLoc[i], i);
    }
    if (UsesLighting)
        OVR_ASSERT(ProjLoc >= 0 && ViewLoc >= 0);
    return 1;
}

void ShaderSet::Set(PrimitiveType) const
{
    glUseProgram(Prog);
}

bool ShaderSet::SetUniform(const char* name, int n, const float* v)
{
    for (UPInt i = 0; i < UniformInfo.GetSize(); i++)
        if (!strcmp(UniformInfo[i].Name.ToCStr(), name))
        {
            OVR_ASSERT(UniformInfo[i].Location >= 0);
            glUseProgram(Prog);
            switch (UniformInfo[i].Type)
            {
            case 1:   glUniform1fv(UniformInfo[i].Location, n, v); break;
            case 2:   glUniform2fv(UniformInfo[i].Location, n/2, v); break;
            case 3:   glUniform3fv(UniformInfo[i].Location, n/3, v); break;
            case 4:   glUniform4fv(UniformInfo[i].Location, n/4, v); break;
            default: OVR_ASSERT(0);
            }
            return 1;
        }

    OVR_DEBUG_LOG(("Warning: uniform %s not present in selected shader", name));
    return 0;
}

bool ShaderSet::SetUniform4x4f(const char* name, const Matrix4f& m)
{
    for (UPInt i = 0; i < UniformInfo.GetSize(); i++)
        if (!strcmp(UniformInfo[i].Name.ToCStr(), name))
        {
            glUseProgram(Prog);
            glUniformMatrix4fv(UniformInfo[i].Location, 1, 1, &m.M[0][0]);
            return 1;
        }

    OVR_DEBUG_LOG(("Warning: uniform %s not present in selected shader", name));
    return 0;
}

Texture::Texture(RenderDevice* r, int w, int h) : Ren(r), Width(w), Height(h)
{
    glGenTextures(1, &TexId);
}

Texture::~Texture()
{
    if (TexId)
        glDeleteTextures(1, &TexId);
}

void Texture::Set(int slot, Render::ShaderStage stage) const
{
    Ren->SetTexture(stage, slot, this);
}

void Texture::SetSampleMode(int sm)
{
    glBindTexture(GL_TEXTURE_2D, TexId);
    switch (sm & Sample_FilterMask)
    {
    case Sample_Linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
        break;

    case Sample_Anisotropic:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
        break;

    case Sample_Nearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
        break;
    }

    switch (sm & Sample_AddressMask)
    {
    case Sample_Repeat:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;

    case Sample_Clamp:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;

    case Sample_ClampBorder:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        break;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture* RenderDevice::CreateTexture(int format, int width, int height, const void* data, int mipcount)
{
    GLenum   glformat, gltype = GL_UNSIGNED_BYTE;
    switch(format & Texture_TypeMask)
    {
    case Texture_RGBA:  glformat = GL_RGBA; break;
    case Texture_R:     glformat = GL_ALPHA; break;
    case Texture_Depth: glformat = GL_DEPTH; gltype = GL_DEPTH_COMPONENT; break;
    case Texture_DXT1:  glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
    case Texture_DXT3:  glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
    case Texture_DXT5:  glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    default:
        return NULL;
    }
    Texture* NewTex = new Texture(this, width, height);
    glBindTexture(GL_TEXTURE_2D, NewTex->TexId);
    glGetError();
    
    if (format & Texture_Compressed)
    {
        const unsigned char* level = (const unsigned char*)data;
        int w = width, h = height;
        for (int i = 0; i < mipcount; i++)
        {
            int mipsize = GetTextureSize(format, w, h);
            glCompressedTexImage2D(GL_TEXTURE_2D, i, glformat, w, h, 0, mipsize, level);

            level += mipsize;
            w >>= 1;
            h >>= 1;
            if (w < 1) w = 1;
            if (h < 1) h = 1;
        }
    }
    else
        glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, gltype, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (format == (Texture_RGBA|Texture_GenMipmaps)) // not render target
    {
        int srcw = width, srch = height;
        int level = 0;
        UByte* mipmaps = NULL;
        do
        {
            level++;
            int mipw = srcw >> 1; if (mipw < 1) mipw = 1;
            int miph = srch >> 1; if (miph < 1) miph = 1;
            if (mipmaps == NULL)
                mipmaps = (UByte*)OVR_ALLOC(mipw * miph * 4);
            FilterRgba2x2(level == 1 ? (const UByte*)data : mipmaps, srcw, srch, mipmaps);
            glTexImage2D(GL_TEXTURE_2D, level, glformat, mipw, miph, 0, glformat, gltype, mipmaps);
            srcw = mipw;
            srch = miph;
        } while (srcw > 1 || srch > 1);
        if (mipmaps)
            OVR_FREE(mipmaps);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipcount-1);
    }

    OVR_ASSERT(!glGetError());
    glBindTexture(GL_TEXTURE_2D, 0);
    return NewTex;
}

bool RenderDevice::SetFullscreen(DisplayMode fullscreen)
{
    Params.Fullscreen = fullscreen;
    return true;
}

RBuffer::RBuffer(GLenum format, GLint w, GLint h)
{
    Width = w;
    Height = h;
    glGenRenderbuffersEXT(1, &BufId);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, BufId);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, format, w, h);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}

RBuffer::~RBuffer()
{
    glDeleteRenderbuffersEXT(1, &BufId);
}

}}}
