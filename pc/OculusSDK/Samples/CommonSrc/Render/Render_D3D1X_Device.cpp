/************************************************************************************

Filename    :   Renderer_D3D1x.cpp
Content     :   RenderDevice implementation  for D3DX10/11.
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

#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_Std.h"

#include "Render_D3D1X_Device.h"

#include <d3dcompiler.h>

#if (OVR_D3D_VERSION == 10)
namespace OVR { namespace Render { namespace D3D10 {
#else
namespace OVR { namespace Render { namespace D3D11 {
#endif

static D3D1x_(INPUT_ELEMENT_DESC) ModelVertexDesc[] =
{
    {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos),   D3D1x_(INPUT_PER_VERTEX_DATA), 0},
    {"Color",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof(Vertex, C),     D3D1x_(INPUT_PER_VERTEX_DATA), 0},
    {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, U),     D3D1x_(INPUT_PER_VERTEX_DATA), 0},
    {"TexCoord", 1, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, U2),	 D3D1x_(INPUT_PER_VERTEX_DATA), 0},
    {"Normal",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Norm),  D3D1x_(INPUT_PER_VERTEX_DATA), 0},
};


static const char* StdVertexShaderSrc =
    "float4x4 Proj;\n"
    "float4x4 View;\n"
    "struct Varyings\n"
    "{\n"
    "   float4 Position : SV_Position;\n"
    "   float4 Color    : COLOR0;\n"
    "   float2 TexCoord : TEXCOORD0;\n"
    "   float2 TexCoord1 : TEXCOORD1;\n"
    "   float3 Normal   : NORMAL;\n"
    "   float3 VPos     : TEXCOORD4;\n"
    "};\n"
    "void main(in float4 Position : POSITION, in float4 Color : COLOR0, in float2 TexCoord : TEXCOORD0, in float2 TexCoord1 : TEXCOORD1, in float3 Normal : NORMAL,\n"
    "          out Varyings ov)\n"
    "{\n"
    "   ov.Position = mul(Proj, mul(View, Position));\n"
    "   ov.Normal = mul(View, Normal);\n"
    "   ov.VPos = mul(View, Position);\n"
    "   ov.TexCoord = TexCoord;\n"
    "   ov.TexCoord1 = TexCoord1;\n"
    "   ov.Color = Color;\n"
    "}\n";

static const char* DirectVertexShaderSrc =
    "float4x4 View : register(c4);\n"
    "void main(in float4 Position : POSITION, in float4 Color : COLOR0, in float2 TexCoord : TEXCOORD0, in float2 TexCoord1 : TEXCOORD1, in float3 Normal : NORMAL,\n"
    "          out float4 oPosition : SV_Position, out float4 oColor : COLOR, out float2 oTexCoord : TEXCOORD0, out float2 oTexCoord1 : TEXCOORD1, out float3 oNormal : NORMAL)\n"
    "{\n"
    "   oPosition = mul(View, Position);\n"
    "   oTexCoord = TexCoord;\n"
    "   oTexCoord1 = TexCoord1;\n"
    "   oColor = Color;\n"
    "   oNormal = mul(View, Normal);\n"
    "}\n";

static const char* SolidPixelShaderSrc =
    "float4 Color;\n"
    "struct Varyings\n"
    "{\n"
    "   float4 Position : SV_Position;\n"
    "   float4 Color    : COLOR0;\n"
    "   float2 TexCoord : TEXCOORD0;\n"
    "};\n"
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "   return Color;\n"
    "}\n";

static const char* GouraudPixelShaderSrc =
    "struct Varyings\n"
    "{\n"
    "   float4 Position : SV_Position;\n"
    "   float4 Color    : COLOR0;\n"
    "   float2 TexCoord : TEXCOORD0;\n"
    "};\n"
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "   return ov.Color;\n"
    "}\n";

static const char* TexturePixelShaderSrc =
    "Texture2D Texture : register(t0);\n"
    "SamplerState Linear : register(s0);\n"
    "struct Varyings\n"
    "{\n"
    "   float4 Position : SV_Position;\n"
    "   float4 Color    : COLOR0;\n"
    "   float2 TexCoord : TEXCOORD0;\n"
    "};\n"
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "	float4 color2 = ov.Color * Texture.Sample(Linear, ov.TexCoord);\n"
    "   if (color2.a <= 0.4)\n"
    "		discard;\n"
    "   return color2;\n"
    "}\n";

static const char* MultiTexturePixelShaderSrc =
    "Texture2D Texture[2] : register(t0);\n"
    "SamplerState Linear : register(s0);\n"
    "struct Varyings\n"
    "{\n"
    "   float4 Position : SV_Position;\n"
    "   float4 Color    : COLOR0;\n"
    "   float2 TexCoord : TEXCOORD0;\n"
    "   float2 TexCoord1 : TEXCOORD1;\n"
    "};\n"
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "float4 color1;\n"
    "float4 color2;\n"
    "	color1 = Texture[0].Sample(Linear, ov.TexCoord);\n"
    "	color2 = Texture[1].Sample(Linear, ov.TexCoord1);\n"
    "	color2.rgb = color2.rgb * lerp(1.2, 1.9, saturate(length(color2.rgb)));\n"
    "	color2 = color1 * color2;\n"
    "   if (color2.a <= 0.4)\n"
    "		discard;\n"
    "	return color2;\n"
    "}\n";

#define LIGHTING_COMMON                 \
    "cbuffer Lighting : register(b1)\n" \
    "{\n"                               \
    "    float3 Ambient;\n"             \
    "    float3 LightPos[8];\n"         \
    "    float4 LightColor[8];\n"       \
    "    float  LightCount;\n"          \
    "};\n"                              \
    "struct Varyings\n"                 \
    "{\n"                                       \
    "   float4 Position : SV_Position;\n"       \
    "   float4 Color    : COLOR0;\n"            \
    "   float2 TexCoord : TEXCOORD0;\n"         \
    "   float3 Normal   : NORMAL;\n"            \
    "   float3 VPos     : TEXCOORD4;\n"         \
    "};\n"                                      \
    "float4 DoLight(Varyings v)\n"              \
    "{\n"                                       \
    "   float3 norm = normalize(v.Normal);\n"   \
    "   float3 light = Ambient;\n"              \
    "   for (uint i = 0; i < LightCount; i++)\n"\
    "   {\n"                                        \
    "       float3 ltp = (LightPos[i] - v.VPos);\n" \
    "       float  ldist = dot(ltp,ltp);\n"         \
    "       ltp = normalize(ltp);\n"                \
    "       light += saturate(LightColor[i] * v.Color.rgb * dot(norm, ltp) / sqrt(ldist));\n"\
    "   }\n"                                        \
    "   return float4(light, v.Color.a);\n"         \
    "}\n"

static const char* LitSolidPixelShaderSrc =
    LIGHTING_COMMON
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "   return DoLight(ov) * ov.Color;\n"
    "}\n";

static const char* LitTexturePixelShaderSrc =
    "Texture2D Texture : register(t0);\n"
    "SamplerState Linear : register(s0);\n"
    LIGHTING_COMMON
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "   return DoLight(ov) * Texture.Sample(Linear, ov.TexCoord);\n"
    "}\n";

static const char* AlphaTexturePixelShaderSrc =
    "Texture2D Texture : register(t0);\n"
    "SamplerState Linear : register(s0);\n"
    "struct Varyings\n"
    "{\n"
    "   float4 Position : SV_Position;\n"
    "   float4 Color    : COLOR0;\n"
    "   float2 TexCoord : TEXCOORD0;\n"
    "};\n"
    "float4 main(in Varyings ov) : SV_Target\n"
    "{\n"
    "   return ov.Color * float4(1,1,1,Texture.Sample(Linear, ov.TexCoord).r);\n"
    "}\n";


// ***** PostProcess Shader

static const char* PostProcessVertexShaderSrc =
    "float4x4 View : register(c4);\n"
    "float4x4 Texm : register(c8);\n"
    "void main(in float4 Position : POSITION, in float4 Color : COLOR0, in float2 TexCoord : TEXCOORD0, in float2 TexCoord1 : TEXCOORD1,\n"
    "          out float4 oPosition : SV_Position, out float4 oColor : COLOR, out float2 oTexCoord : TEXCOORD0)\n"
    "{\n"
    "   oPosition = mul(View, Position);\n"
    "   oTexCoord = mul(Texm, float4(TexCoord,0,1));\n"
    "   oColor = Color;\n"
    "}\n";

// Shader with just lens distortion correction.
static const char* PostProcessPixelShaderSrc =
    "Texture2D Texture : register(t0);\n"
    "SamplerState Linear : register(s0);\n"
    "float2 LensCenter;\n"
    "float2 ScreenCenter;\n"
    "float2 Scale;\n"
    "float2 ScaleIn;\n"
    "float4 HmdWarpParam;\n"
    "\n"

    // Scales input texture coordinates for distortion.
    // ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
    // larger due to aspect ratio.
    "float2 HmdWarp(float2 in01)\n"
    "{\n"
    "   float2 theta = (in01 - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
    "   float  rSq = theta.x * theta.x + theta.y * theta.y;\n"
    "   float2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
    "                   HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
    "   return LensCenter + Scale * theta1;\n"
    "}\n"

    "float4 main(in float4 oPosition : SV_Position, in float4 oColor : COLOR,\n"
    " in float2 oTexCoord : TEXCOORD0) : SV_Target\n"
    "{\n"
    "   float2 tc = HmdWarp(oTexCoord);\n"
    "   if (any(clamp(tc, ScreenCenter-float2(0.25,0.5), ScreenCenter+float2(0.25, 0.5)) - tc))\n"
    "       return 0;\n"
    "   return Texture.Sample(Linear, tc);\n"
    "}\n";

// Shader with lens distortion and chromatic aberration correction.
static const char* PostProcessPixelShaderWithChromAbSrc =
    "Texture2D Texture : register(t0);\n"
    "SamplerState Linear : register(s0);\n"
    "float2 LensCenter;\n"
    "float2 ScreenCenter;\n"
    "float2 Scale;\n"
    "float2 ScaleIn;\n"
    "float4 HmdWarpParam;\n"
    "float4 ChromAbParam;\n"
    "\n"

    // Scales input texture coordinates for distortion.
    // ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
    // larger due to aspect ratio.
    "float4 main(in float4 oPosition : SV_Position, in float4 oColor : COLOR,\n"
    "            in float2 oTexCoord : TEXCOORD0) : SV_Target\n"
    "{\n"
    "   float2 theta = (oTexCoord - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
    "   float  rSq = theta.x * theta.x + theta.y * theta.y;\n"
    "   float2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
    "                   HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
    "   \n"
    "   // Detect whether blue texture coordinates are out of range since these will scaled out the furthest.\n"
    "   float2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);\n"
    "   float2 tcBlue = LensCenter + Scale * thetaBlue;\n"
    "   if (any(clamp(tcBlue, ScreenCenter-float2(0.25,0.5), ScreenCenter+float2(0.25, 0.5)) - tcBlue))\n"
    "       return 0;\n"
    "   \n"
    "   // Now do blue texture lookup.\n"
    "   float  blue = Texture.Sample(Linear, tcBlue).b;\n"
    "   \n"
    "   // Do green lookup (no scaling).\n"
    "   float2 tcGreen = LensCenter + Scale * theta1;\n"
    "   float4 greenColor = Texture.Sample(Linear, tcGreen);\n"
    "   float  green = greenColor.g;\n"
    "   float  alpha = greenColor.a;\n"
    "   \n"
    "   // Do red scale and lookup.\n"
    "   float2 thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);\n"
    "   float2 tcRed = LensCenter + Scale * thetaRed;\n"
    "   float  red = Texture.Sample(Linear, tcRed).r;\n"
    "   \n"
    "   return float4(red, green, blue, alpha);\n"
    "}\n";


static const char* VShaderSrcs[VShader_Count] =
{
    DirectVertexShaderSrc,
    StdVertexShaderSrc,
    PostProcessVertexShaderSrc
};
static const char* FShaderSrcs[FShader_Count] =
{
    SolidPixelShaderSrc,
    GouraudPixelShaderSrc,
    TexturePixelShaderSrc,
    AlphaTexturePixelShaderSrc,
    PostProcessPixelShaderSrc,
    PostProcessPixelShaderWithChromAbSrc,
    LitSolidPixelShaderSrc,
    LitTexturePixelShaderSrc,
    MultiTexturePixelShaderSrc
};

RenderDevice::RenderDevice(const RendererParams& p, HWND window)
{
    RECT rc;
    GetClientRect(window, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    WindowWidth = width;
    WindowHeight = height;
    Window = window;

    Params = p;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&DXGIFactory.GetRawRef()));
    if (FAILED(hr))    
        return;

    // Find the adapter & output (monitor) to use for fullscreen, based on the reported name of the HMD's monitor.
    if (Params.Display.MonitorName.GetLength() > 0)
    {
        for(UINT AdapterIndex = 0; ; AdapterIndex++)
        {
            HRESULT hr = DXGIFactory->EnumAdapters(AdapterIndex, &Adapter.GetRawRef());
            if (hr == DXGI_ERROR_NOT_FOUND)
                break;

            DXGI_ADAPTER_DESC Desc;
            Adapter->GetDesc(&Desc);

            UpdateMonitorOutputs();

            if (FullscreenOutput)
                break;
        }

        if (!FullscreenOutput)
            Adapter = NULL;
    }

    if (!Adapter)
    {
        DXGIFactory->EnumAdapters(0, &Adapter.GetRawRef());
        UpdateMonitorOutputs();
    }

    int flags = 0;

#if (OVR_D3D_VERSION == 10)
    hr = D3D10CreateDevice(Adapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, flags, D3D1x_(SDK_VERSION),
                           &Device.GetRawRef());
    Context = Device;
    Context->AddRef();
#else //11
    hr = D3D11CreateDevice(Adapter, Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                           NULL, flags, NULL, 0, D3D1x_(SDK_VERSION),
                           &Device.GetRawRef(), NULL, &Context.GetRawRef());
#endif
    if (FAILED(hr))
        return;

    if (!RecreateSwapChain())
        return;

    if (Params.Fullscreen)
        SwapChain->SetFullscreenState(1, FullscreenOutput);

    CurRenderTarget = NULL;
    for(int i = 0; i < Shader_Count; i++)
    {
        UniformBuffers[i] = *CreateBuffer();
        MaxTextureSet[i] = 0;
    }

    ID3D10Blob* vsData = CompileShader("vs_4_0", DirectVertexShaderSrc);
    VertexShaders[VShader_MV] = *new VertexShader(this, vsData);
    for(int i = 1; i < VShader_Count; i++)
    {
        VertexShaders[i] = *new VertexShader(this, CompileShader("vs_4_0", VShaderSrcs[i]));
    }

    for(int i = 0; i < FShader_Count; i++)
    {
        PixelShaders[i] = *new PixelShader(this, CompileShader("ps_4_0", FShaderSrcs[i]));
    }

    SPInt bufferSize = vsData->GetBufferSize();
    const void* buffer = vsData->GetBufferPointer();
    ID3D1xInputLayout** objRef = &ModelVertexIL.GetRawRef();

    HRESULT validate = Device->CreateInputLayout(ModelVertexDesc, 5, buffer, bufferSize, objRef);
    OVR_UNUSED(validate);

    Ptr<ShaderSet> gouraudShaders = *new ShaderSet();
    gouraudShaders->SetShader(VertexShaders[VShader_MVP]);
    gouraudShaders->SetShader(PixelShaders[FShader_Gouraud]);
    DefaultFill = *new ShaderFill(gouraudShaders);

#if (OVR_D3D_VERSION == 10)
    D3D1x_(BLEND_DESC) bm;
    memset(&bm, 0, sizeof(bm));
    bm.BlendEnable[0] = true;
    bm.BlendOp      = bm.BlendOpAlpha   = D3D1x_(BLEND_OP_ADD);
    bm.SrcBlend     = bm.SrcBlendAlpha  = D3D1x_(BLEND_SRC_ALPHA);
    bm.DestBlend    = bm.DestBlendAlpha = D3D1x_(BLEND_INV_SRC_ALPHA);
    bm.RenderTargetWriteMask[0]         = D3D1x_(COLOR_WRITE_ENABLE_ALL);
    Device->CreateBlendState(&bm, &BlendState.GetRawRef());
#else
    D3D1x_(BLEND_DESC) bm;
    memset(&bm, 0, sizeof(bm));
    bm.RenderTarget[0].BlendEnable = true;
    bm.RenderTarget[0].BlendOp     = bm.RenderTarget[0].BlendOpAlpha    = D3D1x_(BLEND_OP_ADD);
    bm.RenderTarget[0].SrcBlend    = bm.RenderTarget[0].SrcBlendAlpha   = D3D1x_(BLEND_SRC_ALPHA);
    bm.RenderTarget[0].DestBlend   = bm.RenderTarget[0].DestBlendAlpha  = D3D1x_(BLEND_INV_SRC_ALPHA);
    bm.RenderTarget[0].RenderTargetWriteMask = D3D1x_(COLOR_WRITE_ENABLE_ALL);
    Device->CreateBlendState(&bm, &BlendState.GetRawRef());
#endif

    D3D1x_(RASTERIZER_DESC) rs;
    memset(&rs, 0, sizeof(rs));
    rs.AntialiasedLineEnable = true;
    rs.CullMode = D3D1x_(CULL_BACK);
    // rs.CullMode = D3D1x_(CULL_NONE);
    rs.DepthClipEnable = true;
    rs.FillMode = D3D1x_(FILL_SOLID);
    Device->CreateRasterizerState(&rs, &Rasterizer.GetRawRef());

    QuadVertexBuffer = *CreateBuffer();
    const Render::Vertex QuadVertices[] =
    { Vertex(Vector3f(0, 1, 0)), Vertex(Vector3f(1, 1, 0)),
      Vertex(Vector3f(0, 0, 0)), Vertex(Vector3f(1, 0, 0)) };
    QuadVertexBuffer->Data(Buffer_Vertex, QuadVertices, sizeof(QuadVertices));

    SetDepthMode(0, 0);
}

RenderDevice::~RenderDevice()
{
    if (SwapChain && Params.Fullscreen)
    {
        SwapChain->SetFullscreenState(false, NULL);
    }
}


// Implement static initializer function to create this class.
Render::RenderDevice* RenderDevice::CreateDevice(const RendererParams& rp, void* oswnd)
{
    return new RenderDevice(rp, (HWND)oswnd);
}


// Fallback monitor enumeration in case newly plugged in monitor wasn't detected.
// Added originally for the FactoryTest app.
// New Outputs don't seem to be detected unless adapter is re-created, but that would also
// require us to re-initialize D3D10 (recreating objects, etc). This bypasses that for "fake"
// fullscreen modes.
BOOL CALLBACK MonitorEnumFunc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
    RenderDevice* renderer = (RenderDevice*)dwData;

    MONITORINFOEX monitor;
    monitor.cbSize = sizeof(monitor);

    if (::GetMonitorInfo(hMonitor, &monitor) && monitor.szDevice[0])
    {
        DISPLAY_DEVICE dispDev;
        memset(&dispDev, 0, sizeof(dispDev));
        dispDev.cb = sizeof(dispDev);

        if (::EnumDisplayDevices(monitor.szDevice, 0, &dispDev, 0))
        {
            if (strstr(String(dispDev.DeviceName).ToCStr(), renderer->GetParams().Display.MonitorName.ToCStr()))
            {
                renderer->FSDesktopX = monitor.rcMonitor.left;
                renderer->FSDesktopY = monitor.rcMonitor.top;
                return FALSE;
            }
        }
    }

    return TRUE;
}


void RenderDevice::UpdateMonitorOutputs(bool needRecreate)
{
    HRESULT hr;

    if (needRecreate)
    {
        // need to recreate DXGIFactory and Adapter in order 
        // to get latest info about monitors.
        if (SwapChain)
        {
            SwapChain->SetFullscreenState(FALSE, NULL);
            SwapChain->Release();
            SwapChain = NULL;
        }

        DXGIFactory = NULL;
        Adapter = NULL;
        hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&DXGIFactory.GetRawRef()));
        if (FAILED(hr))    
            return;
        DXGIFactory->EnumAdapters(0, &Adapter.GetRawRef());
    }

    bool deviceNameFound = false;

    for(UINT OutputIndex = 0; ; OutputIndex++)
    {
        Ptr<IDXGIOutput> Output;
        hr = Adapter->EnumOutputs(OutputIndex, &Output.GetRawRef());
        if (hr == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

        DXGI_OUTPUT_DESC OutDesc;
        Output->GetDesc(&OutDesc);

        MONITORINFOEX monitor;
        monitor.cbSize = sizeof(monitor);
        if (::GetMonitorInfo(OutDesc.Monitor, &monitor) && monitor.szDevice[0])
        {
            DISPLAY_DEVICE dispDev;
            memset(&dispDev, 0, sizeof(dispDev));
            dispDev.cb = sizeof(dispDev);

            if (::EnumDisplayDevices(monitor.szDevice, 0, &dispDev, 0))
            {
                if (strstr(String(dispDev.DeviceName).ToCStr(), Params.Display.MonitorName.ToCStr()))
                {
                    deviceNameFound = true;
                    FullscreenOutput = Output;
                    FSDesktopX = monitor.rcMonitor.left;
                    FSDesktopY = monitor.rcMonitor.top;
                    break;
                }
            }
        }
    }

    if (!deviceNameFound && !Params.Display.MonitorName.IsEmpty())
    {
        EnumDisplayMonitors(0, 0, MonitorEnumFunc, (LPARAM)this);
    }
}

bool RenderDevice::RecreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC scDesc;
    memset(&scDesc, 0, sizeof(scDesc));
    scDesc.BufferCount = 1;
    scDesc.BufferDesc.Width = WindowWidth;
    scDesc.BufferDesc.Height = WindowHeight;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferDesc.RefreshRate.Numerator = 60;
    scDesc.BufferDesc.RefreshRate.Denominator = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = Window;
    scDesc.SampleDesc.Count = Params.Multisample;
    scDesc.SampleDesc.Quality = 0;
    scDesc.Windowed = Params.Fullscreen != Display_Fullscreen;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if (SwapChain)
	{
		SwapChain->SetFullscreenState(FALSE, NULL);
		SwapChain->Release();
		SwapChain = NULL;
	}

    Ptr<IDXGISwapChain> newSC;
    if (FAILED(DXGIFactory->CreateSwapChain(Device, &scDesc, &newSC.GetRawRef())))
        return false;    
    SwapChain = newSC;

    BackBuffer = NULL;
    BackBufferRT = NULL;
    HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D1xTexture2D), (void**)&BackBuffer.GetRawRef());
    if (FAILED(hr))
        return false;

    hr = Device->CreateRenderTargetView(BackBuffer, NULL, &BackBufferRT.GetRawRef());
    if (FAILED(hr))
        return false;

    Texture* depthBuffer = GetDepthBuffer(WindowWidth, WindowHeight, Params.Multisample);
    CurDepthBuffer = depthBuffer;
    if (CurRenderTarget == NULL)
    {
        Context->OMSetRenderTargets(1, &BackBufferRT.GetRawRef(), depthBuffer->TexDsv);
    }
    return true;
}

bool RenderDevice::SetParams(const RendererParams& newParams)
{
    String oldMonitor = Params.Display.MonitorName;

    Params = newParams;
    if (newParams.Display.MonitorName != oldMonitor)
    {
        UpdateMonitorOutputs(true);
    }

    // Cause this to be recreated with the new multisample mode.
    pSceneColorTex = NULL;
    return RecreateSwapChain();
}

void RenderDevice::SetWindowSize(int w, int h)
{
    if (w == WindowWidth && h == WindowHeight)
        return;

    WindowWidth  = w;
    WindowHeight = h;
    Context->OMSetRenderTargets(0, NULL, NULL);
    BackBuffer   = NULL;
    BackBufferRT = NULL;
    if (SwapChain)
    {
        SwapChain->ResizeBuffers(2, WindowWidth, WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        SwapChain->GetBuffer(0, __uuidof(ID3D1xTexture2D), (void**)&BackBuffer.GetRawRef());
    }
    Device->CreateRenderTargetView(BackBuffer, NULL, &BackBufferRT.GetRawRef());
}

bool RenderDevice::SetFullscreen(DisplayMode fullscreen)
{
    if (fullscreen == Params.Fullscreen)
    {
        return true;
    }

    if (Params.Fullscreen == Display_FakeFullscreen)
    {
        SetWindowLong(Window, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS);
        SetWindowPos(Window, NULL, PreFullscreenX, PreFullscreenY,
                     PreFullscreenW, PreFullscreenH, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    if (fullscreen == Display_FakeFullscreen)
    {
        // Get WINDOWPLACEMENT before changing style to get OVERLAPPED coordinates,
        // which we will restore.
        WINDOWPLACEMENT wp;
        wp.length = sizeof(wp);
        GetWindowPlacement(Window, &wp);
        PreFullscreenW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
        PreFullscreenH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        PreFullscreenX = wp.rcNormalPosition.left;
        PreFullscreenY = wp.rcNormalPosition.top;
        // Warning: SetWindowLong sends message computed based on old size (incorrect).
        // A proper work-around would be to mask that message out during window frame change in Platform.
        SetWindowLong(Window, GWL_STYLE, WS_OVERLAPPED | WS_VISIBLE | WS_CLIPSIBLINGS);
        SetWindowPos(Window, NULL, FSDesktopX, FSDesktopY, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        // Relocate cursor into the window to avoid losing focus on first click.
        POINT oldCursor;
        if (GetCursorPos(&oldCursor) &&
                ((oldCursor.x < FSDesktopX) || (oldCursor.x > (FSDesktopX + PreFullscreenW)) ||
                 (oldCursor.y < FSDesktopY) || (oldCursor.x > (FSDesktopY + PreFullscreenH))))
        {
            // TBD: FullScreen window logic should really be in platform; it causes world rotation
            // in relative mouse mode.
            ::SetCursorPos(FSDesktopX, FSDesktopY);
        }
    }
    else
    {
        HRESULT hr = SwapChain->SetFullscreenState(fullscreen, fullscreen ? FullscreenOutput : NULL);
        if (FAILED(hr))
        {
            return false;
        }
    }

    Params.Fullscreen = fullscreen;
    return true;
}

void RenderDevice::SetMultipleViewports(int n, const Viewport* vps)
{
    if (n > 2)
    {
        n = 2;
    }
    for(int i = 0; i < n; i++)
    {
#if (OVR_D3D_VERSION == 10)
        Viewports[i].Width = vps[i].w;
        Viewports[i].Height = vps[i].h;
        Viewports[i].MinDepth = 0;
        Viewports[i].MaxDepth = 1;
        Viewports[i].TopLeftX = vps[i].x;
        Viewports[i].TopLeftY = vps[i].y;
#else
        Viewports[i].Width = (float)vps[i].w;
        Viewports[i].Height = (float)vps[i].h;
        Viewports[i].MinDepth = 0;
        Viewports[i].MaxDepth = 1;
        Viewports[i].TopLeftX = (float)vps[i].x;
        Viewports[i].TopLeftY = (float)vps[i].y;
#endif
    }
    NumViewports = n;
    Context->RSSetViewports(n, Viewports);
}

static int GetDepthStateIndex(bool enable, bool write, RenderDevice::CompareFunc func)
{
    if (!enable)
    {
        return 0;
    }
    return 1 + int(func) * 2 + write;
}

void RenderDevice::SetDepthMode(bool enable, bool write, CompareFunc func)
{
    int index = GetDepthStateIndex(enable, write, func);
    if (DepthStates[index])
    {
        CurDepthState = DepthStates[index];
        Context->OMSetDepthStencilState(DepthStates[index], 0);
        return;
    }

    D3D1x_(DEPTH_STENCIL_DESC) dss;
    memset(&dss, 0, sizeof(dss));
    dss.DepthEnable = enable;
    switch(func)
    {
    case Compare_Always:  dss.DepthFunc = D3D1x_(COMPARISON_ALWAYS);  break;
    case Compare_Less:    dss.DepthFunc = D3D1x_(COMPARISON_LESS);    break;
    case Compare_Greater: dss.DepthFunc = D3D1x_(COMPARISON_GREATER); break;
    default:
        assert(0);
    }
    dss.DepthWriteMask = write ? D3D1x_(DEPTH_WRITE_MASK_ALL) : D3D1x_(DEPTH_WRITE_MASK_ZERO);
    Device->CreateDepthStencilState(&dss, &DepthStates[index].GetRawRef());
    Context->OMSetDepthStencilState(DepthStates[index], 0);
    CurDepthState = DepthStates[index];
}

Texture* RenderDevice::GetDepthBuffer(int w, int h, int ms)
{
    for(unsigned i = 0; i < DepthBuffers.GetSize(); i++)
    {
        if (w == DepthBuffers[i]->Width && h == DepthBuffers[i]->Height &&
            ms == DepthBuffers[i]->Samples)
            return DepthBuffers[i];
    }

    Ptr<Texture> newDepth = *CreateTexture(Texture_Depth | Texture_RenderTarget | ms, w, h, NULL);
    if (newDepth == NULL)
    {
        OVR_DEBUG_LOG(("Failed to get depth buffer."));
        return NULL;
    }

    DepthBuffers.PushBack(newDepth);
    return newDepth.GetPtr();
}

void RenderDevice::Clear(float r, float g, float b, float a, float depth)
{
    const float color[] = {r, g, b, a};

	// save state that is affected by clearing this way
    ID3D1xDepthStencilState* oldDepthState = CurDepthState;
	StandardUniformData      clearUniforms;

    SetDepthMode(true, true, Compare_Always);
		
    Context->IASetInputLayout(ModelVertexIL);
#if (OVR_D3D_VERSION == 10)
    Context->GSSetShader(NULL);
#else
    Context->GSSetShader(NULL, NULL, 0);
#endif
    //Shader<Shader_Geometry,ID3D1xGeometryShader> NullGS(this,(ID3D1xGeometryShader*)NULL);
    //NullGS.Set(Prim_TriangleStrip);

    ID3D1xShaderResourceView* sv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (MaxTextureSet[Shader_Fragment])
    {
        Context->PSSetShaderResources(0, MaxTextureSet[Shader_Fragment], sv);
    }

    ID3D1xBuffer* vertexBuffer = QuadVertexBuffer->GetBuffer();
    UINT vertexStride = sizeof(Vertex);
    UINT vertexOffset = 0;
    Context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);

    clearUniforms.View = Matrix4f(2, 0, 0, 0,
                                    0, 2, 0, 0,
                                    0, 0, 0, 0,
                                    -1, -1, depth, 1);
    UniformBuffers[Shader_Vertex]->Data(Buffer_Uniform, &clearUniforms, sizeof(clearUniforms));

    ID3D1xBuffer* vertexConstants = UniformBuffers[Shader_Vertex]->GetBuffer();
    Context->VSSetConstantBuffers(0, 1, &vertexConstants);
    Context->IASetPrimitiveTopology(D3D1x_(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
    VertexShaders[VShader_MV]->Set(Prim_TriangleStrip);
    PixelShaders[FShader_Solid]->Set(Prim_TriangleStrip);

    UniformBuffers[Shader_Pixel]->Data(Buffer_Uniform, color, sizeof(color));
    PixelShaders[FShader_Solid]->SetUniformBuffer(UniformBuffers[Shader_Pixel]);

    if (NumViewports > 1)
    {
        for(int i = 0; i < NumViewports; i++)
        {
            Context->RSSetViewports(1, &Viewports[i]);
            Context->OMSetBlendState(NULL, NULL, 0xffffffff);
            Context->Draw(4, 0);
        }
        Context->RSSetViewports(NumViewports, Viewports);
    }
    else
    {
		Context->OMSetBlendState(NULL, NULL, 0xffffffff);
		Context->Draw(4, 0);
    }

    // reset
    CurDepthState = oldDepthState;
    Context->OMSetDepthStencilState(CurDepthState, 0);
}

// Buffers

Buffer* RenderDevice::CreateBuffer()
{
    return new Buffer(this);
}

Buffer::~Buffer()
{
}

bool   Buffer::Data(int use, const void *buffer, size_t size)
{
    if (D3DBuffer && Size >= size)
    {
        if (Dynamic)
        {
            if (!buffer)
                return true;

            void* v = Map(0, size, Map_Discard);
            if (v)
            {
                memcpy(v, buffer, size);
                Unmap(v);
                return true;
            }
        }
        else
        {
            Ren->Context->UpdateSubresource(D3DBuffer, 0, NULL, buffer, 0, 0);
            return true;
        }
    }
    if (D3DBuffer)
    {
        D3DBuffer = NULL;
        Size = 0;
        Use = 0;
        Dynamic = 0;
    }

    D3D1x_(BUFFER_DESC) desc;
    memset(&desc, 0, sizeof(desc));
    if (use & Buffer_ReadOnly)
    {
        desc.Usage = D3D1x_(USAGE_IMMUTABLE);
        desc.CPUAccessFlags = 0;
    }
    else
    {
        desc.Usage = D3D1x_(USAGE_DYNAMIC);
        desc.CPUAccessFlags = D3D1x_(CPU_ACCESS_WRITE);
        Dynamic = 1;
    }

    switch(use & Buffer_TypeMask)
    {
    case Buffer_Vertex:  desc.BindFlags = D3D1x_(BIND_VERTEX_BUFFER); break;
    case Buffer_Index:   desc.BindFlags = D3D1x_(BIND_INDEX_BUFFER);  break;
    case Buffer_Uniform:
        desc.BindFlags = D3D1x_(BIND_CONSTANT_BUFFER);
        size += ((size + 15) & ~15) - size;
        break;
    case Buffer_Feedback:
        desc.BindFlags = D3D1x_(BIND_STREAM_OUTPUT);
        desc.Usage     = D3D1x_(USAGE_DEFAULT);
        desc.CPUAccessFlags = 0;
        size += ((size + 15) & ~15) - size;
        break;
    }

    desc.ByteWidth = (unsigned)size;

    D3D1x_(SUBRESOURCE_DATA) sr;
    sr.pSysMem = buffer;
    sr.SysMemPitch = 0;
    sr.SysMemSlicePitch = 0;

    HRESULT hr = Ren->Device->CreateBuffer(&desc, buffer ? &sr : NULL, &D3DBuffer.GetRawRef());
    if (SUCCEEDED(hr))
    {
        Use = use;
        Size = desc.ByteWidth;
        return 1;
    }
    return 0;
}

void*  Buffer::Map(size_t start, size_t size, int flags)
{
    OVR_UNUSED(size);

    D3D1x_(MAP) mapFlags = D3D1x_(MAP_WRITE);
    if (flags & Map_Discard)    
        mapFlags = D3D1x_(MAP_WRITE_DISCARD);    
    if (flags & Map_Unsynchronized)    
        mapFlags = D3D1x_(MAP_WRITE_NO_OVERWRITE);    

#if (OVR_D3D_VERSION == 10)
    void* map;
    if (SUCCEEDED(D3DBuffer->Map(mapFlags, 0, &map)))    
        return ((char*)map) + start;    
    else    
        return NULL;
#else
    D3D1x_(MAPPED_SUBRESOURCE) map;
    if (SUCCEEDED(Ren->Context->Map(D3DBuffer, 0, mapFlags, 0, &map)))    
        return ((char*)map.pData) + start;    
    else    
        return NULL;    
#endif
}

bool   Buffer::Unmap(void *m)
{
    OVR_UNUSED(m);

#if (OVR_D3D_VERSION == 10)
    D3DBuffer->Unmap();
#else
    Ren->Context->Unmap(D3DBuffer, 0);
#endif
    return true;
}


// Shaders

#if (OVR_D3D_VERSION == 10)
template<> bool Shader<Render::Shader_Vertex, ID3D10VertexShader>::Load(void* shader, size_t size)
{
    return SUCCEEDED(Ren->Device->CreateVertexShader(shader, size, &D3DShader));
}
template<> bool Shader<Render::Shader_Pixel, ID3D10PixelShader>::Load(void* shader, size_t size)
{
    return SUCCEEDED(Ren->Device->CreatePixelShader(shader, size, &D3DShader));
}
template<> bool Shader<Render::Shader_Geometry, ID3D10GeometryShader>::Load(void* shader, size_t size)
{
    return SUCCEEDED(Ren->Device->CreateGeometryShader(shader, size, &D3DShader));
}

template<> void Shader<Render::Shader_Vertex, ID3D10VertexShader>::Set(PrimitiveType) const
{
    Ren->Context->VSSetShader(D3DShader);
}
template<> void Shader<Render::Shader_Pixel, ID3D10PixelShader>::Set(PrimitiveType) const
{
    Ren->Context->PSSetShader(D3DShader);
}
template<> void Shader<Render::Shader_Geometry, ID3D10GeometryShader>::Set(PrimitiveType) const
{
    Ren->Context->GSSetShader(D3DShader);
}

#else // 11
template<> bool Shader<Render::Shader_Vertex, ID3D11VertexShader>::Load(void* shader, size_t size)
{
    return SUCCEEDED(Ren->Device->CreateVertexShader(shader, size, NULL, &D3DShader));
}
template<> bool Shader<Render::Shader_Pixel, ID3D11PixelShader>::Load(void* shader, size_t size)
{
    return SUCCEEDED(Ren->Device->CreatePixelShader(shader, size, NULL, &D3DShader));
}
template<> bool Shader<Render::Shader_Geometry, ID3D11GeometryShader>::Load(void* shader, size_t size)
{
    return SUCCEEDED(Ren->Device->CreateGeometryShader(shader, size, NULL, &D3DShader));
}

template<> void Shader<Render::Shader_Vertex, ID3D11VertexShader>::Set(PrimitiveType) const
{
    Ren->Context->VSSetShader(D3DShader, NULL, 0);
}
template<> void Shader<Render::Shader_Pixel, ID3D11PixelShader>::Set(PrimitiveType) const
{
    Ren->Context->PSSetShader(D3DShader, NULL, 0);
}
template<> void Shader<Render::Shader_Geometry, ID3D11GeometryShader>::Set(PrimitiveType) const
{
    Ren->Context->GSSetShader(D3DShader, NULL, 0);
}
#endif

template<> void Shader<Render::Shader_Vertex, ID3D1xVertexShader>::SetUniformBuffer(Render::Buffer* buffer, int i)
{
    Ren->Context->VSSetConstantBuffers(i, 1, &((Buffer*)buffer)->D3DBuffer.GetRawRef());
}
template<> void Shader<Render::Shader_Pixel, ID3D1xPixelShader>::SetUniformBuffer(Render::Buffer* buffer, int i)
{
    Ren->Context->PSSetConstantBuffers(i, 1, &((Buffer*)buffer)->D3DBuffer.GetRawRef());
}
template<> void Shader<Render::Shader_Geometry, ID3D1xGeometryShader>::SetUniformBuffer(Render::Buffer* buffer, int i)
{
    Ren->Context->GSSetConstantBuffers(i, 1, &((Buffer*)buffer)->D3DBuffer.GetRawRef());
}

ID3D10Blob* RenderDevice::CompileShader(const char* profile, const char* src, const char* mainName)
{
    ID3D10Blob* shader;
    ID3D10Blob* errors;
    HRESULT hr = D3DCompile(src, strlen(src), NULL, NULL, NULL, mainName, profile,
                            0, 0, &shader, &errors);
    if (FAILED(hr))
    {
        OVR_DEBUG_LOG(("Compiling D3D shader for %s failed\n%s\n\n%s",
                       profile, src, errors->GetBufferPointer()));
        OutputDebugStringA((char*)errors->GetBufferPointer());
        return NULL;
    }
    if (errors)
    {
        errors->Release();
    }
    return shader;
}


ShaderBase::ShaderBase(RenderDevice* r, ShaderStage stage)
    : Render::Shader(stage), Ren(r), UniformData(0)
{
}
ShaderBase::~ShaderBase()
{
    if (UniformData)
    {
        OVR_FREE(UniformData);
    }
}

bool ShaderBase::SetUniform(const char* name, int n, const float* v)
{
    for(unsigned i = 0; i < UniformInfo.GetSize(); i++)
        if (!strcmp(UniformInfo[i].Name.ToCStr(), name))
        {
            memcpy(UniformData + UniformInfo[i].Offset, v, n * sizeof(float));
            return 1;
        }
    return 0;
}

void ShaderBase::InitUniforms(ID3D10Blob* s)
{
    ID3D10ShaderReflection* ref = NULL;
    D3D10ReflectShader(s->GetBufferPointer(), s->GetBufferSize(), &ref);
    ID3D10ShaderReflectionConstantBuffer* buf = ref->GetConstantBufferByIndex(0);
    D3D10_SHADER_BUFFER_DESC bufd;
    if (FAILED(buf->GetDesc(&bufd)))
    {
        UniformsSize = 0;
        if (UniformData)
        {
            OVR_FREE(UniformData);
            UniformData = 0;
        }
        return;
    }

    for(unsigned i = 0; i < bufd.Variables; i++)
    {
        ID3D10ShaderReflectionVariable* var = buf->GetVariableByIndex(i);
        if (var)
        {
            D3D10_SHADER_VARIABLE_DESC vd;
            if (SUCCEEDED(var->GetDesc(&vd)))
            {
                Uniform u;
                u.Name = vd.Name;
                u.Offset = vd.StartOffset;
                u.Size = vd.Size;
                UniformInfo.PushBack(u);
            }
        }
    }

    UniformsSize = bufd.Size;
    UniformData = (unsigned char*)OVR_ALLOC(bufd.Size);
}

void ShaderBase::UpdateBuffer(Buffer* buf)
{
    if (UniformsSize)
    {
        buf->Data(Buffer_Uniform, UniformData, UniformsSize);
    }
}

void RenderDevice::SetCommonUniformBuffer(int i, Render::Buffer* buffer)
{
    CommonUniforms[i] = (Buffer*)buffer;

    Context->PSSetConstantBuffers(1, 1, &CommonUniforms[1]->D3DBuffer.GetRawRef());
    Context->VSSetConstantBuffers(1, 1, &CommonUniforms[1]->D3DBuffer.GetRawRef());
}

Render::Shader *RenderDevice::LoadBuiltinShader(ShaderStage stage, int shader)
{
    switch(stage)
    {
    case Shader_Vertex:
        return VertexShaders[shader];
    case Shader_Pixel:
        return PixelShaders[shader];
    default:
        return NULL;
    }
}

ShaderBase* RenderDevice::CreateStereoShader(PrimitiveType prim, Render::Shader* vs)
{
    if (pStereoShaders[prim])
    {
        return pStereoShaders[prim];
    }

    OVR_UNUSED(vs);
    const char* varyings =
        "   float4 Position : SV_Position;\n"
        "   float4 Color    : COLOR0;\n"
        "   float2 TexCoord : TEXCOORD0;\n"
        "   float3 Normal   : NORMAL;\n";
    const char* copyVaryings =
        "       o.Color = iv[i].Color;\n"
        "       o.Normal = iv[i].Normal;\n"
        "       o.TexCoord = iv[i].TexCoord;\n";

    StringBuffer src =
        "float4x4 Proj[2]     : register(c0);\n"
        "float4   ViewOffset  : register(c8);\n"
        "struct Varyings\n"
        "{\n";
    src += varyings;
    src += "};\n"
           "struct OutVaryings\n"
           "{\n";
    src += varyings;
    src +=
        "   float3 VPos     : TEXCOORD4;\n"
        "   uint   Viewport : SV_ViewportArrayIndex;\n"
        "};\n";

    if (prim == Prim_Lines)
        src +=
            "[maxvertexcount(4)]\n"
            "void main(line Varyings iv[2], inout LineStream<OutVaryings> v)\n";
    else
        src +=
            "[maxvertexcount(6)]\n"
            "void main(triangle Varyings iv[3], inout TriangleStream<OutVaryings> v)\n";

    char ivsize[6];
    OVR_sprintf(ivsize, 6, "%d", (prim == Prim_Lines) ? 2 : 3);

    src +=
        "{\n"
        "   OutVaryings o;\n"
        "   for (uint i = 0; i < ";
    src += ivsize;
    src += "; i++)\n"
           "   {\n"
           "       o.Position = mul(Proj[0], iv[i].Position - ViewOffset);\n"
           "       o.VPos = iv[i].Position;\n"
           "       o.Viewport = 0;\n";
    src += copyVaryings;
    src +=
        "       v.Append(o);\n"
        "   }\n"
        "   v.RestartStrip();\n"
        "   for (uint i = 0; i < ";
    src += ivsize;
    src += "; i++)\n"
           "   {\n"
           "       o.Position = mul(Proj[1], iv[i].Position + ViewOffset);\n"
           "       o.VPos = iv[i].Position;\n"
           "       o.Viewport = 1;\n";
    src += copyVaryings;
    src +=
        "       v.Append(o);\n"
        "   }\n"
        "   v.RestartStrip();\n"
        "}\n";

    pStereoShaders[prim] = *new GeomShader(this, CompileShader("gs_4_0", src.ToCStr()));
    return pStereoShaders[prim];
}

Fill* RenderDevice::CreateSimpleFill(int flags)
{
    OVR_UNUSED(flags);
    return DefaultFill;
}

// Textures

ID3D1xSamplerState* RenderDevice::GetSamplerState(int sm)
{
    if (SamplerStates[sm])    
        return SamplerStates[sm];

    D3D1x_(SAMPLER_DESC) ss;
    memset(&ss, 0, sizeof(ss));
    if (sm & Sample_Clamp)    
        ss.AddressU = ss.AddressV = ss.AddressW = D3D1x_(TEXTURE_ADDRESS_CLAMP);    
    else if (sm & Sample_ClampBorder)    
        ss.AddressU = ss.AddressV = ss.AddressW = D3D1x_(TEXTURE_ADDRESS_BORDER);    
    else    
        ss.AddressU = ss.AddressV = ss.AddressW = D3D1x_(TEXTURE_ADDRESS_WRAP);
    
    if (sm & Sample_Nearest)
    {
        ss.Filter = D3D1x_(FILTER_MIN_MAG_MIP_POINT);
    }
    else if (sm & Sample_Anisotropic)
    {
        ss.Filter = D3D1x_(FILTER_ANISOTROPIC);
        ss.MaxAnisotropy = 8;
    }
    else
    {
        ss.Filter = D3D1x_(FILTER_MIN_MAG_MIP_LINEAR);
    }
    ss.MaxLOD = 15;
    Device->CreateSamplerState(&ss, &SamplerStates[sm].GetRawRef());
    return SamplerStates[sm];
}

Texture::Texture(RenderDevice* ren, int fmt, int w, int h) : Ren(ren), Tex(NULL), TexSv(NULL), TexRtv(NULL), TexDsv(NULL), Width(w), Height(h)
{
    OVR_UNUSED(fmt);
    Sampler = Ren->GetSamplerState(0);
}

Texture::~Texture()
{
}

void Texture::Set(int slot, Render::ShaderStage stage) const
{
    Ren->SetTexture(stage, slot, this);
}

void Texture::SetSampleMode(int sm)
{
    Sampler = Ren->GetSamplerState(sm);
}

void RenderDevice::SetTexture(Render::ShaderStage stage, int slot, const Texture* t)
{
    if (MaxTextureSet[stage] <= slot)
        MaxTextureSet[stage] = slot + 1;    

    ID3D1xShaderResourceView* sv = t ? t->TexSv : NULL;
    switch(stage)
    {
    case Shader_Fragment:
        Context->PSSetShaderResources(slot, 1, &sv);
        if (t)
        {
            Context->PSSetSamplers(slot, 1, &t->Sampler.GetRawRef());
        }
        break;

    case Shader_Vertex:
        Context->VSSetShaderResources(slot, 1, &sv);
        break;
    }
}

void RenderDevice::GenerateSubresourceData(
    unsigned imageWidth, unsigned imageHeight, int format, unsigned imageDimUpperLimit,
    const void* rawBytes, D3D1x_(SUBRESOURCE_DATA)* subresData,
    unsigned& largestMipWidth, unsigned& largestMipHeight, unsigned& byteSize, unsigned& effectiveMipCount)
{
    largestMipWidth  = 0;
    largestMipHeight = 0;

    unsigned sliceLen   = 0;
    unsigned rowLen     = 0;
    unsigned numRows    = 0;
    const byte* mipBytes = static_cast<const byte*>(rawBytes);

    unsigned index        = 0;
    unsigned subresWidth  = imageWidth;
    unsigned subresHeight = imageHeight;
    unsigned numMips      = effectiveMipCount;

    for(unsigned i = 0; i < numMips; i++)
    {
        unsigned bytesPerBlock = 0;
        if (format == DXGI_FORMAT_BC1_UNORM)
        {
            bytesPerBlock = 8;
        }
        else if (format == DXGI_FORMAT_BC3_UNORM)
        {
            bytesPerBlock = 16;
        }

        unsigned blockWidth = 0;
        blockWidth = (subresWidth + 3) / 4;
        if (blockWidth < 1)
        {
            blockWidth = 1;
        }

        unsigned blockHeight = 0;
        blockHeight = (subresHeight + 3) / 4;
        if (blockHeight < 1)
        {
            blockHeight = 1;
        }

        rowLen = blockWidth * bytesPerBlock;
        numRows = blockHeight;
        sliceLen = rowLen * numRows;

        if (imageDimUpperLimit == 0 || (effectiveMipCount == 1) ||
            (subresWidth <= imageDimUpperLimit && subresHeight <= imageDimUpperLimit))
        {
            if(!largestMipWidth)
            {
                largestMipWidth = subresWidth;
                largestMipHeight = subresHeight;
            }

            subresData[index].pSysMem = (const void*)mipBytes;
            subresData[index].SysMemPitch = static_cast<UINT>(rowLen);
            subresData[index].SysMemSlicePitch = static_cast<UINT>(sliceLen);
            byteSize += sliceLen;
            ++index;
        }
        else
        {
            effectiveMipCount--;
        }

        mipBytes += sliceLen;

        subresWidth = subresWidth >> 1;
        subresHeight = subresHeight >> 1;
        if (subresWidth <= 0)
        {
            subresWidth = 1;
        }
        if (subresHeight <= 0)
        {
            subresHeight = 1;
        }
    }
}

#define _256Megabytes 268435456
#define _512Megabytes 536870912

Texture* RenderDevice::CreateTexture(int format, int width, int height, const void* data, int mipcount)
{
    UPInt gpuMemorySize = 0;
    {
        IDXGIDevice* pDXGIDevice;
        Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
        IDXGIAdapter * pDXGIAdapter;
        pDXGIDevice->GetAdapter(&pDXGIAdapter);
        DXGI_ADAPTER_DESC adapterDesc;
        pDXGIAdapter->GetDesc(&adapterDesc);
        gpuMemorySize = adapterDesc.DedicatedVideoMemory;
        pDXGIAdapter->Release();
        pDXGIDevice->Release();
    }
 
    unsigned imageDimUpperLimit = 0;
    if (gpuMemorySize <= _256Megabytes)
    {
        imageDimUpperLimit = 512;
    }    
    else if (gpuMemorySize <= _512Megabytes)
    {
        imageDimUpperLimit = 1024;
    } 

    if (format == Texture_DXT1 || format == Texture_DXT5)
    {
        int      convertedFormat   = (format == Texture_DXT1) ? DXGI_FORMAT_BC1_UNORM : DXGI_FORMAT_BC3_UNORM;
        unsigned largestMipWidth   = 0;
        unsigned largestMipHeight  = 0;
        unsigned effectiveMipCount = mipcount;
        unsigned textureSize       = 0;

#ifdef OVR_DEFINE_NEW
#undef new
#endif
        
        D3D1x_(SUBRESOURCE_DATA)* subresData = (D3D1x_(SUBRESOURCE_DATA)*)
                                                OVR_ALLOC(sizeof(D3D1x_(SUBRESOURCE_DATA)) * mipcount);
        GenerateSubresourceData(width, height, convertedFormat, imageDimUpperLimit, data, subresData, largestMipWidth,
                                largestMipHeight, textureSize, effectiveMipCount);
        TotalTextureMemoryUsage += textureSize;

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif

        if (!Device || !subresData)
        {
            return NULL;
        }
        int samples = (Texture_RGBA & Texture_SamplesMask);
        if (samples < 1)
        {
            samples = 1;
        }

        Texture* NewTex = new Texture(this, format, largestMipWidth, largestMipHeight);
        NewTex->Samples = samples;

        D3D1x_(TEXTURE2D_DESC) desc;
        desc.Width      = largestMipWidth;
        desc.Height     = largestMipHeight;
        desc.MipLevels  = effectiveMipCount;
        desc.ArraySize  = 1;
        desc.Format     = static_cast<DXGI_FORMAT>(convertedFormat);
        desc.SampleDesc.Count = samples;
        desc.SampleDesc.Quality = 0;
        desc.Usage      = D3D1x_(USAGE_DEFAULT);
        desc.BindFlags  = D3D1x_(BIND_SHADER_RESOURCE);
        desc.CPUAccessFlags = 0;
        desc.MiscFlags  = 0;

        HRESULT hr = Device->CreateTexture2D(&desc, static_cast<D3D1x_(SUBRESOURCE_DATA)*>(subresData),
                                             &NewTex->Tex.GetRawRef());
        OVR_FREE(subresData);

        if (SUCCEEDED(hr) && NewTex != 0)
        {
            D3D1x_(SHADER_RESOURCE_VIEW_DESC) SRVDesc;
            memset(&SRVDesc, 0, sizeof(SRVDesc));
            SRVDesc.Format = static_cast<DXGI_FORMAT>(format);
            SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = desc.MipLevels;

            hr = Device->CreateShaderResourceView(NewTex->Tex, NULL, &NewTex->TexSv.GetRawRef());

            if (FAILED(hr))
            {
                NewTex->Release();
                return NULL;
            }
            return NewTex;
        }

        return NULL;
    }
    else
    {
        DXGI_FORMAT d3dformat;
        int         bpp;
        switch(format & Texture_TypeMask)
        {
        case Texture_RGBA:
            bpp = 4;
            d3dformat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        case Texture_R:
            bpp = 1;
            d3dformat = DXGI_FORMAT_R8_UNORM;
            break;
        case Texture_Depth:
            bpp = 0;
            d3dformat = DXGI_FORMAT_D32_FLOAT;
            break;
        default:
            return NULL;
        }

        int samples = (format & Texture_SamplesMask);
        if (samples < 1)
        {
            samples = 1;
        }

        Texture* NewTex = new Texture(this, format, width, height);
        NewTex->Samples = samples;

        D3D1x_(TEXTURE2D_DESC) dsDesc;
        dsDesc.Width     = width;
        dsDesc.Height    = height;
        dsDesc.MipLevels = (format == (Texture_RGBA | Texture_GenMipmaps) && data) ? GetNumMipLevels(width, height) : 1;
        dsDesc.ArraySize = 1;
        dsDesc.Format    = d3dformat;
        dsDesc.SampleDesc.Count = samples;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage     = D3D1x_(USAGE_DEFAULT);
        dsDesc.BindFlags = D3D1x_(BIND_SHADER_RESOURCE);
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags      = 0;

        if (format & Texture_RenderTarget)
        {
            if ((format & Texture_TypeMask) == Texture_Depth)
                // We don't use depth textures, and creating them in d3d10 requires different options.
            {
                dsDesc.BindFlags = D3D1x_(BIND_DEPTH_STENCIL);
            }
            else
            {
                dsDesc.BindFlags |= D3D1x_(BIND_RENDER_TARGET);
            }
        }

        HRESULT hr = Device->CreateTexture2D(&dsDesc, NULL, &NewTex->Tex.GetRawRef());
        if (FAILED(hr))
        {
            OVR_DEBUG_LOG_TEXT(("Failed to create 2D D3D texture."));
            NewTex->Release();
            return NULL;
        }
        if (dsDesc.BindFlags & D3D1x_(BIND_SHADER_RESOURCE))
        {
            Device->CreateShaderResourceView(NewTex->Tex, NULL, &NewTex->TexSv.GetRawRef());
        }

        if (data)
        {
            Context->UpdateSubresource(NewTex->Tex, 0, NULL, data, width * bpp, width * height * bpp);
            if (format == (Texture_RGBA | Texture_GenMipmaps))
            {
                int srcw = width, srch = height;
                int level = 0;
                UByte* mipmaps = NULL;
                do
                {
                    level++;
                    int mipw = srcw >> 1;
                    if (mipw < 1)
                    {
                        mipw = 1;
                    }
                    int miph = srch >> 1;
                    if (miph < 1)
                    {
                        miph = 1;
                    }
                    if (mipmaps == NULL)
                    {
                        mipmaps = (UByte*)OVR_ALLOC(mipw * miph * 4);
                    }
                    FilterRgba2x2(level == 1 ? (const UByte*)data : mipmaps, srcw, srch, mipmaps);
                    Context->UpdateSubresource(NewTex->Tex, level, NULL, mipmaps, mipw * bpp, miph * bpp);
                    srcw = mipw;
                    srch = miph;
                }
                while(srcw > 1 || srch > 1);

                if (mipmaps != NULL)
                {
                    OVR_FREE(mipmaps);
                }
            }
        }

        if (format & Texture_RenderTarget)
        {
            if ((format & Texture_TypeMask) == Texture_Depth)
            {
                Device->CreateDepthStencilView(NewTex->Tex, NULL, &NewTex->TexDsv.GetRawRef());
            }
            else
            {
                Device->CreateRenderTargetView(NewTex->Tex, NULL, &NewTex->TexRtv.GetRawRef());
            }
        }

        return NewTex;
    }
}

// Rendering

void RenderDevice::BeginRendering()
{
    Context->RSSetState(Rasterizer);
}

void RenderDevice::SetRenderTarget(Render::Texture* color, Render::Texture* depth, Render::Texture* stencil)
{
    OVR_UNUSED(stencil);

    CurRenderTarget = (Texture*)color;
    if (color == NULL)
    {
        Texture* newDepthBuffer = GetDepthBuffer(WindowWidth, WindowHeight, Params.Multisample);
        if (newDepthBuffer == NULL)
        {
            OVR_DEBUG_LOG(("New depth buffer creation failed."));
        }
        if (newDepthBuffer != NULL)
        {
            CurDepthBuffer = GetDepthBuffer(WindowWidth, WindowHeight, Params.Multisample);
            Context->OMSetRenderTargets(1, &BackBufferRT.GetRawRef(), CurDepthBuffer->TexDsv);
        }
        return;
    }
    if (depth == NULL)
    {
        depth = GetDepthBuffer(color->GetWidth(), color->GetHeight(), CurRenderTarget->Samples);
    }

    ID3D1xShaderResourceView* sv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (MaxTextureSet[Shader_Fragment])
    {
        Context->PSSetShaderResources(0, MaxTextureSet[Shader_Fragment], sv);
    }
    memset(MaxTextureSet, 0, sizeof(MaxTextureSet));

    CurDepthBuffer = (Texture*)depth;
    Context->OMSetRenderTargets(1, &((Texture*)color)->TexRtv.GetRawRef(), ((Texture*)depth)->TexDsv);
}


void RenderDevice::SetWorldUniforms(const Matrix4f& proj)
{
    StdUniforms.Proj = proj.Transposed();
    // Shader constant buffers cannot be partially updated.
}


void RenderDevice::Render(const Matrix4f& matrix, Model* model)
{
    // Store data in buffers if not already
    if (!model->VertexBuffer)
    {
        Ptr<Buffer> vb = *CreateBuffer();
        vb->Data(Buffer_Vertex, &model->Vertices[0], model->Vertices.GetSize() * sizeof(Vertex));
        model->VertexBuffer = vb;
    }
    if (!model->IndexBuffer)
    {
        Ptr<Buffer> ib = *CreateBuffer();
        ib->Data(Buffer_Index, &model->Indices[0], model->Indices.GetSize() * 2);
        model->IndexBuffer = ib;
    }

    Render(model->Fill ? model->Fill : DefaultFill,
           model->VertexBuffer, model->IndexBuffer,
           matrix, 0, (unsigned)model->Indices.GetSize(), model->GetPrimType());
}

void RenderDevice::RenderWithAlpha(	const Fill* fill, Render::Buffer* vertices, Render::Buffer* indices,
									const Matrix4f& matrix, int offset, int count, PrimitiveType rprim)
{
	Context->OMSetBlendState(BlendState, NULL, 0xffffffff);
    Render(fill, vertices, indices, matrix, offset, count, rprim);
    Context->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void RenderDevice::Render(const Fill* fill, Render::Buffer* vertices, Render::Buffer* indices,
                          const Matrix4f& matrix, int offset, int count, PrimitiveType rprim)
{
    Context->IASetInputLayout(ModelVertexIL);
    if (indices)
    {
        Context->IASetIndexBuffer(((Buffer*)indices)->GetBuffer(), DXGI_FORMAT_R16_UINT, 0);
    }

    ID3D1xBuffer* vertexBuffer = ((Buffer*)vertices)->GetBuffer();
    UINT vertexStride = sizeof(Vertex);
    UINT vertexOffset = offset;
    Context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);

    ShaderSet* shaders = ((ShaderFill*)fill)->GetShaders();

    ShaderBase* vshader = ((ShaderBase*)shaders->GetShader(Shader_Vertex));
    unsigned char* vertexData = vshader->UniformData;
    if (vertexData)
    {
        StandardUniformData* stdUniforms = (StandardUniformData*) vertexData;
        stdUniforms->View = matrix.Transposed();
        stdUniforms->Proj = StdUniforms.Proj;
        UniformBuffers[Shader_Vertex]->Data(Buffer_Uniform, vertexData, vshader->UniformsSize);
        vshader->SetUniformBuffer(UniformBuffers[Shader_Vertex]);
    }

    for(int i = Shader_Vertex + 1; i < Shader_Count; i++)
        if (shaders->GetShader(i))
        {
            ((ShaderBase*)shaders->GetShader(i))->UpdateBuffer(UniformBuffers[i]);
            ((ShaderBase*)shaders->GetShader(i))->SetUniformBuffer(UniformBuffers[i]);
        }

    D3D1x_(PRIMITIVE_TOPOLOGY) prim;
    switch(rprim)
    {
    case Prim_Triangles:
        prim = D3D1x_(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case Prim_Lines:
        prim = D3D1x_(PRIMITIVE_TOPOLOGY_LINELIST);
        break;
    case Prim_TriangleStrip:
        prim = D3D1x_(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        break;
    default:
        assert(0);
        return;
    }
    Context->IASetPrimitiveTopology(prim);

    fill->Set(rprim);
    if (ExtraShaders)
    {
        ExtraShaders->Set(rprim);
    }

    if (indices)
    {
        Context->DrawIndexed(count, 0, 0);
    }
    else
    {
        Context->Draw(count, 0);
    }
}

UPInt RenderDevice::QueryGPUMemorySize()
{
	IDXGIDevice* pDXGIDevice;
	Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
	IDXGIAdapter * pDXGIAdapter;
	pDXGIDevice->GetAdapter(&pDXGIAdapter);
	DXGI_ADAPTER_DESC adapterDesc;
	pDXGIAdapter->GetDesc(&adapterDesc);
	return adapterDesc.DedicatedVideoMemory;
}


void RenderDevice::Present()
{
    SwapChain->Present(0, 0);
}

void RenderDevice::ForceFlushGPU()
{
    D3D1x_QUERY_DESC queryDesc = { D3D1x_(QUERY_EVENT), 0 };
    Ptr<ID3D1xQuery> query;
    BOOL             done = FALSE;

    if (Device->CreateQuery(&queryDesc, &query.GetRawRef()) == S_OK)
    {

#if (OVR_D3D_VERSION == 10)
        // Begin() not used for EVENT query.
        query->End();
        // GetData will returns S_OK for both done == TRUE or FALSE.
        // Exit on failure to avoid infinite loop.
        do { }
        while(!done && !FAILED(query->GetData(&done, sizeof(BOOL), 0)));
#else
        Context->End(query);
        do { }
        while(!done && !FAILED(Context->GetData(query, &done, sizeof(BOOL), 0)));
#endif

    }
}


void RenderDevice::FillRect(float left, float top, float right, float bottom, Color c)
{
    Context->OMSetBlendState(BlendState, NULL, 0xffffffff);
    OVR::Render::RenderDevice::FillRect(left, top, right, bottom, c);
    Context->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void RenderDevice::FillGradientRect(float left, float top, float right, float bottom, Color col_top, Color col_btm)
{
    Context->OMSetBlendState(BlendState, NULL, 0xffffffff);
    OVR::Render::RenderDevice::FillGradientRect(left, top, right, bottom, col_top, col_btm);
    Context->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void RenderDevice::RenderText(const struct Font* font, const char* str, float x, float y, float size, Color c)
{
	Context->OMSetBlendState(BlendState, NULL, 0xffffffff);
	OVR::Render::RenderDevice::RenderText(font, str, x, y, size, c);
	Context->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void RenderDevice::RenderImage(float left, float top, float right, float bottom, ShaderFill* image)
{
    Context->OMSetBlendState(BlendState, NULL, 0xffffffff);
    OVR::Render::RenderDevice::RenderImage(left, top, right, bottom, image);
    Context->OMSetBlendState(NULL, NULL, 0xffffffff);
}

}}}

