#define _CRT_SECURE_NO_DEPRECATE
#include <atlbase.h> // for CComPtr implementation
#include <dshow.h> // direct show
#include "SampleGrabber.h" // our own sample grabber header file

#include <sys\timeb.h>


#include "OVR.h"
#include "../../Samples/CommonSrc/Platform/Platform_Default.h"
#include "../../Samples/CommonSrc/Render/Render_Device.h"

#include "../../Samples/CommonSrc/Render/Render_FontEmbed_DejaVu48.h"
#include "../../Samples/CommonSrc/Platform/Gamepad.h"

#include <Kernel\OVR_SysFile.h> // loading of logo


#include <time.h>

#include "ftd2xx.h" // breakout


using namespace OVR;
using namespace OVR::Platform;
using namespace OVR::Render;

const int SEND_DELAY = 15;
const unsigned char SERVO_MAX_BOUNDARY = 160;
const unsigned char SERVO_MIN_BOUNDARY = 0;
const unsigned char SERVO_MEDIAN_BOUNDARY = 80;


//-------------------------------------------------------------------------------------
// ***** OculusWorldDemo Description

// This app renders a simple flat-shaded room allowing the user to move along the
// floor and look around with an HMD, mouse and keyboard. The following keys work:
//
//  'W', 'S', 'A', 'D' and Arrow Keys - Move forward, back; strafe left/right.
//  F1 - No stereo, no distortion.
//  F2 - Stereo, no distortion.
//  F3 - Stereo and distortion.
//  F4 - Toggle MSAA.
//  F9 - Cycle through fullscreen and windowed modes. Necessary for previewing content with Rift.
//
// Important Oculus-specific logic can be found at following locations:
//
//  OculusWorldDemoApp::OnStartup - This function will initialize OVR::DeviceManager and HMD,
//                                    creating SensorDevice and attaching it to SensorFusion.
//                                    This needs to be done before obtaining sensor data.
//
//  OculusWorldDemoApp::OnIdle    - Here we poll SensorFusion for orientation, apply it
//                                    to the scene and handle movement.
//                                    Stereo rendering is also done here, by delegating to
//                                    to Render function for each eye.
//

//-------------------------------------------------------------------------------------
// ***** OculusWorldDemo Application class

// An instance of this class is created on application startup (main/WinMain).
// It then works as follows:
//  - Graphics and HMD setup is done OculusWorldDemoApp::OnStartup(). This function
//    also creates the room model from Slab declarations.
//  - Per-frame processing is done in OnIdle(). This function processes
//    sensor and movement input and then renders the frame.
//  - Additional input processing is done in OnMouse, OnKey.

class OculusWorldDemoApp : public Application
{
public:
    OculusWorldDemoApp();
    ~OculusWorldDemoApp();

    virtual int  OnStartup(int argc, const char** argv);
    virtual void OnIdle();

    virtual void OnKey(OVR::KeyCode key, int chr, bool down, int modifiers);
    virtual void OnResize(int width, int height);

    void         Render(const StereoEyeParams& stereo);

    // Sets temporarily displayed message for adjustments
    void         SetAdjustMessage(const char* format, ...);
    // Overrides current timeout, in seconds (not the future default value);
    // intended to be called right after SetAdjustMessage.
    void         SetAdjustMessageTimeout(float timeout);

    // Stereo setting adjustment functions.
    // Called with deltaTime when relevant key is held.
    void         AdjustFov(float dt);
    void         AdjustAspect(float dt);
    void         AdjustIPD(float dt);

    void         AdjustMotionPrediction(float dt);

    void         AdjustDistortion(float dt, int kIndex, const char* label);
    void         AdjustDistortionK0(float dt)  { AdjustDistortion(dt, 0, "K0"); }
    void         AdjustDistortionK1(float dt)  { AdjustDistortion(dt, 1, "K1"); }
    void         AdjustDistortionK2(float dt)  { AdjustDistortion(dt, 2, "K2"); }
    void         AdjustDistortionK3(float dt)  { AdjustDistortion(dt, 3, "K3"); }

	void         PopulatePreloadScene();

    void         AdjustDistortion(float val, int kIndex);
    void         AdjustEsd(float val);

	// Our Direct Show functions
	HRESULT GetGraph(IGraphBuilder **pGraph, const WCHAR*, int nr);
	HRESULT GetGrabber(ISampleGrabber **grabber, IGraphBuilder *pGraph, int nr);
	HRESULT LoadGraphFile(IGraphBuilder *pGraph, const WCHAR* wszName, int nr);
	HRESULT CheckGrabberStatus(int nr);

	void GrabFrame(ISampleGrabber *grabber, long &pBufferSize, unsigned char *buffer);
	void AdjustPictureSize(float dt);


    // Magnetometer calibration procedure
    void         UpdateManualMagCalibration();

protected:
    RenderDevice*       pRender;
    RendererParams      RenderParams;
    int                 Width, Height;
    int                 Screen;
    int                 FirstScreenInCycle;

	// Our grabber stuff
	int grabberWidth1, grabberHeight1, grabberWidth2, grabberHeight2;
	float screenRatio1, screenRatio2;
	ISampleGrabber *grabber_isg1, *grabber_isg2;
	IMediaControl *pControl1, *pControl2;
	unsigned char *pBuffer1, *pBuffer2;
	long pBufferSize1, pBufferSize2;
	float pictureSize = 1.f;
	time_t lastSampleTest1, lastSampleTest2;
	bool hasStarted = false;

	// breakout
	FT_HANDLE ftHandle;
	DWORD BytesWritten;
	struct timeb lastSend, now;
	int lastSendValue = 2;
	bool shouldSend = false;


    // *** Oculus HMD Variables
    Ptr<DeviceManager>  pManager;
    Ptr<SensorDevice>   pSensor;
    Ptr<HMDDevice>      pHMD;
    Ptr<Profile>        pUserProfile;
    SensorFusion        SFusion;
    HMDInfo             TheHMDInfo;

    double              LastUpdate;
    int                 FPS;
    int                 FrameCounter;
    double              NextFPSUpdate;

    // Loading process displays screenshot in first frame
    // and then proceeds to load until finished.
    enum LoadingStateType
    {
        LoadingState_DoLoad,
        LoadingState_Loading,
        LoadingState_Finished
    };

    Matrix4f            View;
    Scene               LoadingScene;

    LoadingStateType    LoadingState;

    Ptr<ShaderFill>     LitSolid, LitTextures[4];

    // Stereo view parameters.
    StereoConfig        SConfig;
    PostProcessType     PostProcess;

    // LOD
    String                MainFilePath;

    float               DistortionK0;
    float               DistortionK1;
    float               DistortionK2;
    float               DistortionK3;

    String              AdjustMessage;
    double              AdjustMessageTimeout;

    // Saved distortion state.
    float               SavedK0, SavedK1, SavedK2, SavedK3;
    float               SavedESD, SavedAspect, SavedEyeDistance;

    // Allows toggling color around distortion.
    Color               DistortionClearColor;

    // Stereo settings adjustment state.
    typedef void (OculusWorldDemoApp::*AdjustFuncType)(float);
    bool                ShiftDown;
    AdjustFuncType      pAdjustFunc;
    float               AdjustDirection;

    enum TextScreen
    {
        Text_None,
        Text_Orientation,
        Text_Config,
        Text_Help,
        Text_Count
    };
    TextScreen          TextScreen;

    struct DeviceStatusNotificationDesc
    {
        DeviceHandle    Handle;
        MessageType     Action;

        DeviceStatusNotificationDesc():Action(Message_None) {}
        DeviceStatusNotificationDesc(MessageType mt, const DeviceHandle& dev) 
            : Handle(dev), Action(mt) {}
    };
    Array<DeviceStatusNotificationDesc> DeviceStatusNotificationsQueue; 



    void CycleDisplay();
};

//-------------------------------------------------------------------------------------

OculusWorldDemoApp::OculusWorldDemoApp()
    : pRender(0),
      LastUpdate(0),
      LoadingState(LoadingState_DoLoad),
      // Initial location
      SConfig(),
      PostProcess(PostProcess_Distortion),
      DistortionClearColor(0, 0, 0),

      ShiftDown(false),
      pAdjustFunc(0),
      AdjustDirection(1.0f),
      TextScreen(Text_None)
{
    Width  = 1280;
    Height = 800;
    Screen = 0;
    FirstScreenInCycle = 0;

    FPS = 0;
    FrameCounter = 0;
    NextFPSUpdate = 0;

    AdjustMessageTimeout = 0;
}

OculusWorldDemoApp::~OculusWorldDemoApp()
{
	FT_Close(ftHandle);
	if (DejaVu.fill)
	{
		DejaVu.fill->Release();
	}
    pSensor.Clear();
    pHMD.Clear();
}

int OculusWorldDemoApp::OnStartup(int argc, const char** argv)
{
	// Attach a console to output
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);



    // *** Oculus HMD & Sensor Initialization

	LogText("\nOculus Rift Setup\n-----------------\n");

    // Create DeviceManager and first available HMDDevice from it.
    // Sensor object is created from the HMD, to ensure that it is on the
    // correct device.

    pManager = *DeviceManager::Create();

    pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();
    if (pHMD)
    {
        pSensor = *pHMD->GetSensor();

        // This will initialize HMDInfo with information about configured IPD,
        // screen size and other variables needed for correct projection.
        // We pass HMD DisplayDeviceName into the renderer to select the
        // correct monitor in full-screen mode.
        if(pHMD->GetDeviceInfo(&TheHMDInfo))
        {
            //RenderParams.MonitorName = hmd.DisplayDeviceName;
            SConfig.SetHMDInfo(TheHMDInfo);
        }
    }
    else
    {
        // If we didn't detect an HMD, try to create the sensor directly.
        // This is useful for debugging sensor interaction; it is not needed in
        // a shipping app.
        pSensor = *pManager->EnumerateDevices<SensorDevice>().CreateDevice();
    }

	if (pSensor) {
		SFusion.AttachToSensor(pSensor);
	}



    // Make the user aware which devices are present.
    if(pHMD == NULL && pSensor == NULL)
    {
        SetAdjustMessage("---------------------------------\nNO HMD DETECTED\nNO SENSOR DETECTED\n---------------------------------");
    }
    else if(pHMD == NULL)
    {
        SetAdjustMessage("----------------------------\nNO HMD DETECTED\n----------------------------");
    }
    else if(pSensor == NULL)
    {
        SetAdjustMessage("---------------------------------\nNO SENSOR DETECTED\n---------------------------------");
    }
    else
    {
        SetAdjustMessage("--------------------------------------------\n"
                         "Press F9 for Full-Screen on Rift\n"
                         "--------------------------------------------");
    }

    // First message should be extra-short!
    SetAdjustMessageTimeout(1.0f);


    if(TheHMDInfo.HResolution > 0)
    {
        Width  = TheHMDInfo.HResolution;
        Height = TheHMDInfo.VResolution;
    }

    if(!pPlatform->SetupWindow(Width, Height))
    {
        return 1;
    }

    String Title = "Oculus FPV";
    if(TheHMDInfo.ProductName[0])
    {
        Title += " : ";
        Title += TheHMDInfo.ProductName;
    }
    pPlatform->SetWindowTitle(Title);

    // *** Initialize Rendering

    const char* graphics = "d3d11";

    // Select renderer based on command line arguments.
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-r") && i < argc - 1)
        {
            graphics = argv[i + 1];
        }
        else if(!strcmp(argv[i], "-fs"))
        {
            RenderParams.Fullscreen = true;
        }
    }

    // Enable multi-sampling by default.
    RenderParams.Multisample = 4;
    pRender = pPlatform->SetupGraphics(OVR_DEFAULT_RENDER_DEVICE_SET,
                                       graphics, RenderParams);



    // *** Configure Stereo settings.

    SConfig.SetFullViewport(Viewport(0, 0, Width, Height));
    SConfig.SetStereoMode(Stereo_LeftRight_Multipass);

    // Configure proper Distortion Fit.
    // For 7" screen, fit to touch left side of the view, leaving a bit of
    // invisible screen on the top (saves on rendering cost).
    // For smaller screens (5.5"), fit to the top.
    if (TheHMDInfo.HScreenSize > 0.0f)
    {
        if (TheHMDInfo.HScreenSize > 0.140f)  // 7"
            SConfig.SetDistortionFitPointVP(-1.0f, 0.0f);        
        else        
            SConfig.SetDistortionFitPointVP(0.0f, 1.0f);        
    }

    pRender->SetSceneRenderScale(SConfig.GetDistortionScale());
    //pRender->SetSceneRenderScale(1.0f);

    SConfig.Set2DAreaFov(DegreeToRad(85.0f));


    // *** Identify Scene File & Prepare for Loading
    PopulatePreloadScene();


	// Breakout 
	LogText("\nBreakout Board Setup\n--------------------\n");

	FT_STATUS result;
	result = FT_Open(0, &ftHandle);
	if (result == FT_OK) {
		FT_SetBaudRate(ftHandle, 9600);
		FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

		LogText("Got handler\n");
		shouldSend = true;
	}
	else {
		LogText("Couldn't open handler\n");
	}


	// DIRECT SHOW
	HRESULT hr;
	IGraphBuilder *pGraph1, *pGraph2;


	LogText("\nGraph Setup\n-----------\n");
	hr = GetGraph(&pGraph1, L"D:\\prog\\eit\\FINAL_graph_norender.grf", 1);
	if (FAILED(hr)) {
		LogText("Graph1 failed. \nABORTING...\n");
		return -1;
	}
	hr = GetGraph(&pGraph2, L"D:\\prog\\eit\\FINAL_graph_2_norender.grf", 2);
	if (FAILED(hr)) {
		LogText("Graph2 failed. \nABORTING...\n");
		return -1;
	}

	LogText("\nSample Grabber Setup\n--------------------\n");
	hr = GetGrabber(&grabber_isg1, pGraph1, 1);
	if (FAILED(hr)) {
		LogText("ABORTING...\n");
		return -1;
	}
	hr = GetGrabber(&grabber_isg2, pGraph2, 2);
	if (FAILED(hr)) {
		LogText("ABORTING...\n");
		return -1;
	}


	AM_MEDIA_TYPE am1, am2;
	grabber_isg1->GetConnectedMediaType(&am1);
	grabber_isg2->GetConnectedMediaType(&am2);

	VIDEOINFOHEADER *vh1 = (VIDEOINFOHEADER*)am1.pbFormat;
	grabberWidth1 = vh1->bmiHeader.biWidth;
	grabberHeight1 = vh1->bmiHeader.biHeight;

	VIDEOINFOHEADER *vh2 = (VIDEOINFOHEADER*)am2.pbFormat;
	grabberWidth2 = vh2->bmiHeader.biWidth;
	grabberHeight2 = vh2->bmiHeader.biHeight;

	screenRatio1 = ((float)grabberWidth1) / grabberHeight1;
	screenRatio2 = ((float)grabberWidth2) / grabberHeight2;

	LogText("Size is: %ix%i, ratio: %4.2f (1)\n", grabberWidth1, grabberHeight1, screenRatio1);
	LogText("Size is: %ix%i, ratio: %4.2f (2)\n", grabberWidth2, grabberHeight2, screenRatio2);

	hr = pGraph1->QueryInterface(IID_IMediaControl, (void **)&pControl1);
	hr = pGraph2->QueryInterface(IID_IMediaControl, (void **)&pControl2);

	pBufferSize1 = grabberWidth1 * grabberHeight1 * 4;
	pBufferSize2 = grabberWidth2 * grabberHeight2 * 4;
	LogText("Guessing on buffersize: %i (1)\n", pBufferSize1);
	LogText("Guessing on buffersize: %i (2)\n", pBufferSize2);

	if (SUCCEEDED(hr)) {
		grabber_isg1->SetBufferSamples(true);
		grabber_isg2->SetBufferSamples(true);
		return 0;
	}

	LogText("\nSomething went wrong\n");
    return -1;
}


void OculusWorldDemoApp::OnResize(int width, int height)
{
    Width  = width;
    Height = height;
    SConfig.SetFullViewport(Viewport(0, 0, Width, Height));
}

void OculusWorldDemoApp::OnKey(OVR::KeyCode key, int chr, bool down, int modifiers)
{
    OVR_UNUSED(chr);

    switch(key)
    {
    case Key_Q:
        if (down && (modifiers & Mod_Control))
        {
            pPlatform->Exit(0);
        }
        break;
    case Key_B:
        if (down)
        {
            if(SConfig.GetDistortionScale() == 1.0f)
            {
                if(SConfig.GetHMDInfo().HScreenSize > 0.140f)  // 7"
                {
                    SConfig.SetDistortionFitPointVP(-1.0f, 0.0f);
                }
                else
                {
                 SConfig.SetDistortionFitPointVP(0.0f, 1.0f);
                }
            }
            else
            {
                // No fitting; scale == 1.0.
                SConfig.SetDistortionFitPointVP(0, 0);
            }
        }
        break;

    // Support toggling background color for distortion so that we can see
    // the effect on the periphery.
    case Key_V:
        if (down)
        {
            if(DistortionClearColor.B == 0)
            {
                DistortionClearColor = Color(0, 128, 255);
            }
            else
            {
                DistortionClearColor = Color(0, 0, 0);
            }

            pRender->SetDistortionClearColor(DistortionClearColor);
        }
        break;


    case Key_F1:
        SConfig.SetStereoMode(Stereo_None);
        PostProcess = PostProcess_None;
        SetAdjustMessage("StereoMode: None");
        break;
    case Key_F2:
        SConfig.SetStereoMode(Stereo_LeftRight_Multipass);
        PostProcess = PostProcess_None;
        SetAdjustMessage("StereoMode: Stereo + No Distortion");
        break;
    case Key_F3:
        SConfig.SetStereoMode(Stereo_LeftRight_Multipass);
        PostProcess = PostProcess_Distortion;
        SetAdjustMessage("StereoMode: Stereo + Distortion");
        break;

    case Key_R:
        SFusion.Reset();
        SetAdjustMessage("Sensor Fusion Reset");
        break;

    case Key_Space:
        if (!down)
        {
            TextScreen = (enum TextScreen)((TextScreen + 1) % Text_Count);
        }
        break;

    case Key_F4:
        if (!down)
        {
            RenderParams = pRender->GetParams();
            RenderParams.Multisample = RenderParams.Multisample > 1 ? 1 : 4;
            pRender->SetParams(RenderParams);
            if(RenderParams.Multisample > 1)
            {
                SetAdjustMessage("Multisampling On");
            }
            else
            {
                SetAdjustMessage("Multisampling Off");
            }
        }
        break;
    case Key_F9:
#ifndef OVR_OS_LINUX    // On Linux F9 does the same as F11.
        if (!down)
        {
            CycleDisplay();
        }
        break;
#endif
#ifdef OVR_OS_MAC
    case Key_F10:  // F11 is reserved on Mac
#else
    case Key_F11:
#endif
        if (!down)
        {
            RenderParams = pRender->GetParams();
            RenderParams.Display = DisplayId(SConfig.GetHMDInfo().DisplayDeviceName,SConfig.GetHMDInfo().DisplayId);
            pRender->SetParams(RenderParams);

            pPlatform->SetMouseMode(Mouse_Normal);            
            pPlatform->SetFullscreen(RenderParams, pRender->IsFullscreen() ? Display_Window : Display_FakeFullscreen);
            pPlatform->SetMouseMode(Mouse_Relative); // Avoid mode world rotation jump.
            // If using an HMD, enable post-process (for distortion) and stereo.
            if(RenderParams.IsDisplaySet() && pRender->IsFullscreen())
            {
                SConfig.SetStereoMode(Stereo_LeftRight_Multipass);
                PostProcess = PostProcess_Distortion;
            }
        }
        break;

    case Key_Escape:
        if(!down)
        {
            // switch to primary screen windowed mode
            pPlatform->SetFullscreen(RenderParams, Display_Window);
            RenderParams.Display = pPlatform->GetDisplay(0);
            pRender->SetParams(RenderParams);
            Screen = 0;
        }
        break;

        // Stereo adjustments.
    case Key_BracketLeft:
	case Key_Num7:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustFov    : 0;
        AdjustDirection = 1;
        break;
    case Key_BracketRight:
	case Key_Num8:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustFov    : 0;
        AdjustDirection = -1;
        break;

    case Key_Insert:
    case Key_Num0:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustIPD    : 0;
        AdjustDirection = 1;
        break;
    case Key_Delete:
    case Key_Num9:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustIPD    : 0;
        AdjustDirection = -1;
        break;

	case Key_W:
		pAdjustFunc = down ? &OculusWorldDemoApp::AdjustPictureSize : 0;
		AdjustDirection = 1;
		break;
	case Key_S:
		pAdjustFunc = down ? &OculusWorldDemoApp::AdjustPictureSize : 0;
		AdjustDirection = -1;
		break;

    case Key_PageUp:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustAspect : 0;
        AdjustDirection = 1;
        break;
    case Key_PageDown:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustAspect : 0;
        AdjustDirection = -1;
        break;

        // Distortion correction adjustments
    case Key_H:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK0 : NULL;
        AdjustDirection = -1;
        break;
    case Key_Y:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK0 : NULL;
        AdjustDirection = 1;
        break;
    case Key_J:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK1 : NULL;
        AdjustDirection = -1;
        break;
    case Key_U:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK1 : NULL;
        AdjustDirection = 1;
        break;
    case Key_K:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK2 : NULL;
        AdjustDirection = -1;
        break;
    case Key_I:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK2 : NULL;
        AdjustDirection = 1;
        break;
    case Key_L:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK3 : NULL;
        AdjustDirection = -1;
        break;
    case Key_O:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK3 : NULL;
        AdjustDirection = 1;
        break;

    case Key_Tab:
        if (down)
        {
            float t0      = SConfig.GetDistortionK(0),
                  t1      = SConfig.GetDistortionK(1),
                  t2      = SConfig.GetDistortionK(2),
                  t3      = SConfig.GetDistortionK(3);
            float tESD    = SConfig.GetEyeToScreenDistance(),
                  taspect = SConfig.GetAspectMultiplier(),
                  tipd    = SConfig.GetIPD();

            if(SavedK0 > 0.0f)
            {
                SConfig.SetDistortionK(0, SavedK0);
                SConfig.SetDistortionK(1, SavedK1);
                SConfig.SetDistortionK(2, SavedK2);
                SConfig.SetDistortionK(3, SavedK3);
                SConfig.SetEyeToScreenDistance(SavedESD);
                SConfig.SetAspectMultiplier(SavedAspect);
                SConfig.SetIPD(SavedEyeDistance);

                if ( ShiftDown )
                {
                    // Swap saved and current values. Good for doing direct comparisons.
                    SetAdjustMessage("Swapped current and saved. New settings:\n"
                                     "ESD:\t120 %.3f\t350 Eye:\t490 %.3f\n"
                                     "K0: \t120 %.4f\t350 K2: \t490 %.4f\n"
                                     "K1: \t120 %.4f\t350 K3: \t490 %.4f\n",
                                     SavedESD, SavedEyeDistance,
                                     SavedK0, SavedK2,
                                     SavedK1, SavedK3);
                    SavedK0 = t0;
                    SavedK1 = t1;
                    SavedK2 = t2;
                    SavedK3 = t3;
                    SavedESD = tESD;
                    SavedAspect = taspect;
                    SavedEyeDistance = tipd;
                }
                else
                {
                    SetAdjustMessage("Restored:\n"
                                     "ESD:\t120 %.3f\t350 Eye:\t490 %.3f\n"
                                     "K0: \t120 %.4f\t350 K2: \t490 %.4f\n"
                                     "K1: \t120 %.4f\t350 K3: \t490 %.4f\n",
                                     SavedESD, SavedEyeDistance,
                                     SavedK0, SavedK2,
                                     SavedK1, SavedK3);
                }
            }
            else
            {
                SetAdjustMessage("Setting Saved");
                SavedK0 = t0;
                SavedK1 = t1;
                SavedK2 = t2;
                SavedK3 = t3;
                SavedESD = tESD;
                SavedAspect = taspect;
                SavedEyeDistance = tipd;
            }

        }
        break; 
 

        // Holding down Shift key accelerates adjustment velocity.
    case Key_Shift:
        ShiftDown = down;
        break;


    case Key_N:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustMotionPrediction : NULL;
        AdjustDirection = -1;
        break;

    case Key_M:
        pAdjustFunc = down ? &OculusWorldDemoApp::AdjustMotionPrediction : NULL;
        AdjustDirection = 1;
        break;

/*
    case Key_N:
        RaiseLOD();
        break;
    case Key_M:
        DropLOD();
        break;
*/

        // Cycle through drift correction options
    case Key_Z:
        if (down)
        {
            if (SFusion.IsYawCorrectionEnabled())
            {
                SFusion.SetGravityEnabled(false);
                SFusion.SetYawCorrectionEnabled(false);
            }
            else if (SFusion.IsGravityEnabled())
            {
                SFusion.SetYawCorrectionEnabled(true);
            }
            else
            {
                SFusion.SetGravityEnabled(true);
            }
            SetAdjustMessage("Tilt Correction %s\nYaw Correction %s", 
                SFusion.IsGravityEnabled() ? "On" : "Off",
                SFusion.IsYawCorrectionEnabled() ? "On" : "Off");
        }
        break;

    case Key_C:
        if (down)
        {
            // Toggle chromatic aberration correction on/off.
            RenderDevice::PostProcessShader shader = pRender->GetPostProcessShader();

            if (shader == RenderDevice::PostProcessShader_Distortion)
            {
                pRender->SetPostProcessShader(RenderDevice::PostProcessShader_DistortionAndChromAb);                
                SetAdjustMessage("Chromatic Aberration Correction On");
            }
            else if (shader == RenderDevice::PostProcessShader_DistortionAndChromAb)
            {
                pRender->SetPostProcessShader(RenderDevice::PostProcessShader_Distortion);                
                SetAdjustMessage("Chromatic Aberration Correction Off");
            }
            else
                OVR_ASSERT(false);
        }
        break;

    case Key_P:
        if (down)
        {
            // Toggle motion prediction.
            if (SFusion.IsPredictionEnabled())
            {
                SFusion.SetPredictionEnabled(false);
                SetAdjustMessage("Motion Prediction Off");
            }
            else
            {
                SFusion.SetPredictionEnabled(true);
                SetAdjustMessage("Motion Prediction On");
            }      
        }
        break;
     default:
        break;
    }
}

void OculusWorldDemoApp::OnIdle()
{

    double curtime = pPlatform->GetAppTime();
    float  dt      = float(curtime - LastUpdate);
    LastUpdate     = curtime;


	if (LoadingState == LoadingState_DoLoad) {
		LogText("\nStarting Loading\n----------------\n");
		LogText("Showing Loading Screen\n");
		LoadingState = LoadingState_Loading;

		ftime(&lastSend); // init last send
	} else if (LoadingState == LoadingState_Loading) {
		if (!hasStarted) {
			pControl1->Run();
			pControl2->Run();
			LogText("Graph is started\n");
			LogText("Waiting for Sample Grabber\n");
			hasStarted = true;
		}
		
		HRESULT hr1 = CheckGrabberStatus(1);
		HRESULT hr2 = CheckGrabberStatus(2);
		if (hr1 >= 0 && hr2 >= 0) {
			LogText("Buffer size is %i (1)\n", pBufferSize1);
			LogText("Buffer size is %i (2)\n", pBufferSize2);
			LogText("Sample Grabbers ready\n");

			LogText("\nLoading Complete\n----------------\nPLAYBACK STARTED\n\n");

			SConfig.SetIPD(0.f);
			LoadingState = LoadingState_Finished;
			pBuffer1 = new unsigned char[pBufferSize1];
			pBuffer2 = new unsigned char[pBufferSize2];
		}
    }
    
    // If one of Stereo setting adjustment keys is pressed, adjust related state.
    if (pAdjustFunc)
    {
        (this->*pAdjustFunc)(dt * AdjustDirection * (ShiftDown ? 5.0f : 1.0f));
    }

	// Counts fps
	if (curtime >= NextFPSUpdate)
	{
		NextFPSUpdate = curtime + 1.0;
		FPS = FrameCounter;
		FrameCounter = 0;
	}
	FrameCounter++;

    // Rotate and position View Camera, using YawPitchRoll in BodyFrame coordinates.
    //
    Matrix4f rollPitchYaw = Matrix4f::RotationY(0) * Matrix4f::RotationX(0) * Matrix4f::RotationZ(0); // YAW PITCH ROLL
	const Vector3f	UpVector(0.0f, 1.0f, 0.0f);
	const Vector3f	ForwardVector(0.0f, 0.0f, -1.0f);
    Vector3f up      = rollPitchYaw.Transform(UpVector);
    Vector3f forward = rollPitchYaw.Transform(ForwardVector);


    // Minimal head modeling; should be moved as an option to SensorFusion.
    float headBaseToEyeHeight     = 0.15f;  // Vertical height of eye from base of head
    float headBaseToEyeProtrusion = 0.09f;  // Distance forward of eye from base of head

    Vector3f eyeCenterInHeadFrame(0.0f, headBaseToEyeHeight, -headBaseToEyeProtrusion);
    Vector3f shiftedEyePos = rollPitchYaw.Transform(eyeCenterInHeadFrame);
    shiftedEyePos.y -= eyeCenterInHeadFrame.y; // Bring the head back down to original height
    View = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + forward, up);

    //  Transformation without head modeling.
    // View = Matrix4f::LookAtRH(EyePos, EyePos + forward, up);

    // This is an alternative to LookAtRH:
    // Here we transpose the rotation matrix to get its inverse.
    //  View = (Matrix4f::RotationY(EyeYaw) * Matrix4f::RotationX(EyePitch) *
    //                                        Matrix4f::RotationZ(EyeRoll)).Transposed() *
    //         Matrix4f::Translation(-EyePos);


	// breakout send stuff
	ftime(&now);

	int msSinceLastSend = (1000 * (now.time - lastSend.time))
		+ (now.millitm - lastSend.millitm);

	if (shouldSend && msSinceLastSend >= SEND_DELAY && LoadingState == LoadingState_Finished) {
		

		Quatf quaternion = SFusion.GetOrientation();
		float yaw, pitch, roll;
		quaternion.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);


		if (lastSendValue == 2) {
			lastSendValue = 0;

			unsigned char start = 255;
			FT_Write(ftHandle, &start, 1, &BytesWritten);
			//LogText("sent startbyte\n");
		}
		else if (lastSendValue == 0) {
			lastSendValue = 1;

			yaw = RadToDegree(yaw);
			unsigned char ayaw;

			if (yaw > SERVO_MEDIAN_BOUNDARY) {
				ayaw = SERVO_MAX_BOUNDARY;
			}
			else if (yaw < -SERVO_MEDIAN_BOUNDARY) {
				ayaw = SERVO_MIN_BOUNDARY;
			}
			else {
				ayaw = SERVO_MEDIAN_BOUNDARY + yaw;
			}

			FT_Write(ftHandle, &ayaw, 1, &BytesWritten);
			//LogText("Sent yaw: %i\n", ayaw);
		}
		else {
			lastSendValue = 2;

			pitch = RadToDegree(pitch);
			unsigned char apitch;


			if (pitch < -SERVO_MEDIAN_BOUNDARY){
				apitch = SERVO_MAX_BOUNDARY;
			}
			else if (pitch > SERVO_MEDIAN_BOUNDARY){
				apitch = SERVO_MIN_BOUNDARY;
			}
			else {
				apitch = SERVO_MEDIAN_BOUNDARY - pitch;
			}

			FT_Write(ftHandle, &apitch, 1, &BytesWritten);
			//LogText("Sent pitch: %i\n", apitch);
		}

		// update last send time
		ftime(&lastSend);
	}


    switch(SConfig.GetStereoMode())
    {
    case Stereo_None:
        Render(SConfig.GetEyeRenderParams(StereoEye_Center));
        break;

    case Stereo_LeftRight_Multipass:
        //case Stereo_LeftDouble_Multipass:
        Render(SConfig.GetEyeRenderParams(StereoEye_Left));
        Render(SConfig.GetEyeRenderParams(StereoEye_Right));
        break;

    }

    pRender->Present();
    // Force GPU to flush the scene, resulting in the lowest possible latency.
    pRender->ForceFlushGPU();
}


static const char* HelpText = 
    "F1         \t100 NoStereo\n"
    "F2         \t100 Stereo                     \t420 Z    \t520 Drift Correction\n"
    "F3         \t100 StereoHMD                  \t420 F6   \t520 Yaw Drift Info\n" 
    "F4         \t100 MSAA                       \t420 R    \t520 Reset SensorFusion\n"
    "F9         \t100 FullScreen                 \t420\n"
    "F11        \t100 Fast FullScreen                   \t500 - +       \t660 Adj EyeHeight\n"                                           
    "C          \t100 Chromatic Ab                      \t500 [ ]       \t660 Adj FOV\n"
    "P          \t100 Motion Pred                       \t500 Shift     \t660 Adj Faster\n"
    "N/M        \t180 Adj Motion Pred\n"
    "( / )      \t180 Adj EyeDistance"
    ;


enum DrawTextCenterType
{
    DrawText_NoCenter= 0,
    DrawText_VCenter = 0x1,
    DrawText_HCenter = 0x2,
    DrawText_Center  = DrawText_VCenter | DrawText_HCenter
};

static void DrawTextBox(RenderDevice* prender, float x, float y,
                        float textSize, const char* text,
                        DrawTextCenterType centerType = DrawText_NoCenter)
{
    float ssize[2] = {0.0f, 0.0f};

    prender->MeasureText(&DejaVu, text, textSize, ssize);

    // Treat 0 a VCenter.
    if (centerType & DrawText_HCenter)
    {
        x = -ssize[0]/2;
    }
    if (centerType & DrawText_VCenter)
    {
        y = -ssize[1]/2;
    }

    prender->FillRect(x-0.02f, y-0.02f, x+ssize[0]+0.02f, y+ssize[1]+0.02f, Color(40,40,100,210));
    prender->RenderText(&DejaVu, text, x, y, textSize, Color(255,255,0,210));
}


HRESULT OculusWorldDemoApp::GetGraph(IGraphBuilder **pGraph, const WCHAR* wszName, int nr) {
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		LogText("CoInitialize failed (%i)", nr);
		return hr;
	}
	else {
		LogText("CoInitialized (%i)\n", nr);
	}


	hr = CoCreateInstance(CLSID_FilterGraph, NULL,
		CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)pGraph);

	if (FAILED(hr)) {
		LogText("Graph creation failed (%i)\n", nr);
		return hr;
	}
	else {
		LogText("Graph created (%i)\n", nr);
	}


	hr = LoadGraphFile(*pGraph, wszName, nr);
	if (FAILED(hr)) {
		LogText("Failed building from file (%i)\n", nr);
		return hr;
	}

	LogText("Graph setup correctly (%i)\n", nr);
	return S_OK;
}


HRESULT OculusWorldDemoApp::LoadGraphFile(IGraphBuilder *pGraph, const WCHAR* wszName, int nr)
{
	IStorage *pStorage = 0;
	if (S_OK != StgIsStorageFile(wszName))
	{
		LogText("Graph file not found (%i)\n", nr);
		return E_FAIL;
	}
	HRESULT hr = StgOpenStorage(wszName, 0,
		STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE,
		0, 0, &pStorage);
	if (FAILED(hr))
	{
		LogText("Couldn't open storage (%i)\n", nr);
		return hr;
	}
	IPersistStream *pPersistStream = 0;
	hr = pGraph->QueryInterface(IID_IPersistStream,
		reinterpret_cast<void**>(&pPersistStream));
	if (SUCCEEDED(hr))
	{
		IStream *pStream = 0;
		hr = pStorage->OpenStream(L"ActiveMovieGraph", 0,
			STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
		if (SUCCEEDED(hr))
		{
			hr = pPersistStream->Load(pStream);
			pStream->Release();
		}
		pPersistStream->Release();
	}
	pStorage->Release();
	return hr;
}

HRESULT OculusWorldDemoApp::GetGrabber(ISampleGrabber **grabber, IGraphBuilder *pGraph, int nr) {
	HRESULT hr;
	IBaseFilter *sampleFilter;

	hr = pGraph->FindFilterByName(L"SampleGrabber", &sampleFilter);
	if (FAILED(hr)) {
		LogText("Couldn't find SampleGrabber filter in graph (%i)\n", nr);
		return hr;
	}

	LogText("Found it (%i)\n", nr);
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> grabber2(sampleFilter);
	*grabber = grabber2;
	LogText("Hooked into SampleGrabber (%i)\n", nr);
	return S_OK;
}

void OculusWorldDemoApp::AdjustPictureSize(float dt) {
	pictureSize = pictureSize + 0.1f * dt;
	SetAdjustMessage("Picture Size: %6.4f", pictureSize);
}

HRESULT OculusWorldDemoApp::CheckGrabberStatus(int nr) {
	time_t now;
	time(&now);

	if (nr == 1) {
		double diff = difftime(now, lastSampleTest1);
		if (diff > 0.1) {
			LogText("Checking Samplegrabber (1)\n");
			time(&lastSampleTest1);
			return grabber_isg1->GetCurrentBuffer(&pBufferSize1, NULL);
		}
	} else {
		double diff = difftime(now, lastSampleTest2);
		if (diff > 0.1) {
			LogText("Checking Samplegrabber (2)\n");
			time(&lastSampleTest2);
			return grabber_isg2->GetCurrentBuffer(&pBufferSize2, NULL);
		}
	}

	return E_FAIL;
}

void OculusWorldDemoApp::GrabFrame(ISampleGrabber *grabber, long &pBufferSize, unsigned char *buffer) {
	HRESULT hr = grabber->GetCurrentBuffer(&pBufferSize, (long*)buffer);
	if (FAILED(hr)) {
		LogText("HR FAILED");
	}
	//rbga -> rbga
	/*for (int i = 0; i < pBufferSize; i += 4) {
		image1[i] = pBuffer[i+2];
		image1[i + 1] = pBuffer[i + 1];
		image1[i + 2] = pBuffer[i];
		image1[i + 3] = 0xFF;
	}*/
	unsigned char tmp;
	for (int i = 0; i < pBufferSize; i += 4) {
		tmp = buffer[i + 2];
		buffer[i + 2] = buffer[i];
		buffer[i] = tmp;
	}
}


void OculusWorldDemoApp::Render(const StereoEyeParams& stereo)
{

    pRender->BeginScene(PostProcess);

    // *** 3D - Configures Viewport/Projection and Render


    pRender->ApplyStereoParams(stereo);    
    pRender->Clear();

    //pRender->SetDepthMode(true, true);



    // *** 2D Text & Grid - Configure Orthographic rendering.

    // Render UI in 2D orthographic coordinate system that maps [-1,1] range
    // to a readable FOV area centered at your eye and properly adjusted.
    pRender->ApplyStereoParams2D(stereo);    
    pRender->SetDepthMode(false, false);

    float unitPixel = SConfig.Get2DUnitPixel();
    float textHeight= unitPixel * 22; 

    // Display Loading screen-shot in frame 0.
    if (LoadingState != LoadingState_Finished)
    {
        LoadingScene.Render(pRender, Matrix4f());
		String loadMessage = String("Loading ");
        DrawTextBox(pRender, 0.0f, 0.0f, textHeight, loadMessage.ToCStr(), DrawText_HCenter);

	}
	else {

		if /*(1) {*/(stereo.Eye == StereoEye_Left) {
			GrabFrame(grabber_isg1, pBufferSize1, pBuffer1);

			Texture* tex = pRender->CreateTexture(Texture_RGBA, grabberWidth1, grabberHeight1, pBuffer1, 1);
			ShaderFill* image = (ShaderFill*)pRender->CreateTextureFill(tex, false);
			pRender->RenderImage(-pictureSize * screenRatio1, -pictureSize, pictureSize * screenRatio1, pictureSize, image, 255);

			delete image;
			delete tex;
		}
		else {
			GrabFrame(grabber_isg2, pBufferSize2, pBuffer2);

			Texture* tex = pRender->CreateTexture(Texture_RGBA, grabberWidth2, grabberHeight2, pBuffer2, 1);
			ShaderFill* image = (ShaderFill*)pRender->CreateTextureFill(tex, false);
			pRender->RenderImage(-pictureSize * screenRatio2, -pictureSize, pictureSize * screenRatio2, pictureSize, image, 255);

			delete image;
			delete tex;
		}
	}




    if(!AdjustMessage.IsEmpty() && AdjustMessageTimeout > pPlatform->GetAppTime())
    {
        DrawTextBox(pRender,0.0f,0.4f, textHeight, AdjustMessage.ToCStr(), DrawText_HCenter);
    }

    switch(TextScreen)
    {
    case Text_Orientation:
    {
        char buf[256], gpustat[256];
        OVR_sprintf(buf, sizeof(buf),
                    ""
                    " FPS: %d",
                   FPS);
        size_t texMemInMB = pRender->GetTotalTextureMemoryUsage() / 1058576;
        if (texMemInMB)
        {
            OVR_sprintf(gpustat, sizeof(gpustat), "\n GPU Tex: %u MB", texMemInMB);
            OVR_strcat(buf, sizeof(buf), gpustat);
        }
        
        DrawTextBox(pRender, 0.0f, -0.15f, textHeight, buf, DrawText_HCenter);
    }
    break;

    case Text_Config:
    {
        char   textBuff[2048];
         
        OVR_sprintf(textBuff, sizeof(textBuff),
                    "Fov\t300 %9.4f\n"
                    "EyeDistance\t300 %9.4f\n"
                    "DistortionK0\t300 %9.4f\n"
                    "DistortionK1\t300 %9.4f\n"
                    "DistortionK2\t300 %9.4f\n"
                    "DistortionK3\t300 %9.4f\n"
                    "TexScale\t300 %9.4f",
                    SConfig.GetYFOVDegrees(),
                        SConfig.GetIPD(),
                    SConfig.GetDistortionK(0),
                    SConfig.GetDistortionK(1),
                    SConfig.GetDistortionK(2),
                    SConfig.GetDistortionK(3),
                    SConfig.GetDistortionScale());

            DrawTextBox(pRender, 0.0f, 0.0f, textHeight, textBuff, DrawText_Center);
    }
    break;

    case Text_Help:
        DrawTextBox(pRender, 0.0f, -0.1f, textHeight, HelpText, DrawText_Center);
            
    default:
        break;
    }


    pRender->FinishScene();
}


// Sets temporarily displayed message for adjustments
void OculusWorldDemoApp::SetAdjustMessage(const char* format, ...)
{
    Lock::Locker lock(pManager->GetHandlerLock());
    char textBuff[2048];
    va_list argList;
    va_start(argList, format);
    OVR_vsprintf(textBuff, sizeof(textBuff), format, argList);
    va_end(argList);

    // Message will time out in 4 seconds.
    AdjustMessage = textBuff;
    AdjustMessageTimeout = pPlatform->GetAppTime() + 4.0f;
}

void OculusWorldDemoApp::SetAdjustMessageTimeout(float timeout)
{
    AdjustMessageTimeout = pPlatform->GetAppTime() + timeout;
}

// ***** View Control Adjustments

void OculusWorldDemoApp::AdjustFov(float dt)
{
    float esd = SConfig.GetEyeToScreenDistance() + 0.01f * dt;
    SConfig.SetEyeToScreenDistance(esd);
    SetAdjustMessage("ESD:%6.3f  FOV: %6.3f", esd, SConfig.GetYFOVDegrees());
}

void OculusWorldDemoApp::AdjustAspect(float dt)
{
    float rawAspect = SConfig.GetAspect() / SConfig.GetAspectMultiplier();
    float newAspect = SConfig.GetAspect() + 0.01f * dt;
    SConfig.SetAspectMultiplier(newAspect / rawAspect);
    SetAdjustMessage("Aspect: %6.3f", newAspect);
}

void OculusWorldDemoApp::AdjustDistortion(float dt, int kIndex, const char* label)
{
    SConfig.SetDistortionK(kIndex, SConfig.GetDistortionK(kIndex) + 0.03f * dt);
    SetAdjustMessage("%s: %6.4f", label, SConfig.GetDistortionK(kIndex));
}

void OculusWorldDemoApp::AdjustIPD(float dt)
{
    SConfig.SetIPD(SConfig.GetIPD() + 0.025f * dt);
    SetAdjustMessage("EyeDistance: %6.4f", SConfig.GetIPD());
}


void OculusWorldDemoApp::AdjustMotionPrediction(float dt)
{
    float motionPred = SFusion.GetPredictionDelta() + 0.01f * dt;

    if (motionPred < 0.0f)
    {
        motionPred = 0.0f;
    }
    
    SFusion.SetPrediction(motionPred);

    SetAdjustMessage("MotionPrediction: %6.3fs", motionPred);
}

void OculusWorldDemoApp::AdjustDistortion(float val, int kIndex)
{
    SConfig.SetDistortionK(kIndex, val);
    SetAdjustMessage("K%d: %6.4f", kIndex, SConfig.GetDistortionK(kIndex));
}

void OculusWorldDemoApp::AdjustEsd(float val)
{
    SConfig.SetEyeToScreenDistance(val);
    float esd = SConfig.GetEyeToScreenDistance();
    SetAdjustMessage("ESD:%6.3f  FOV: %6.3f", esd, SConfig.GetYFOVDegrees());
}


void OculusWorldDemoApp::PopulatePreloadScene()
{
	LogText("\nPopulating Loading Screen\n-------------------------\n");
    //// Load-screen screen shot image
    //String fileName = MainFilePath;
    //fileName.StripExtension();

    Ptr<File>    imageFile = *new SysFile("logo.tga");
    Ptr<Texture> imageTex;
	if (imageFile->IsValid()) {
		imageTex = *LoadTextureTga(pRender, imageFile);
		LogText("Found logo, added to scene\n");
	}

    // Image is rendered as a single quad.
    if (imageTex)
    {
        imageTex->SetSampleMode(Sample_Anisotropic|Sample_Repeat);
        Ptr<Model> m = *new Model(Prim_Triangles);        
        m->AddVertex(-0.5f,  0.5f,  0.0f, Color(255,255,255,255), 0.0f, 0.0f);
        m->AddVertex( 0.5f,  0.5f,  0.0f, Color(255,255,255,255), 1.0f, 0.0f);
        m->AddVertex( 0.5f, -0.5f,  0.0f, Color(255,255,255,255), 1.0f, 1.0f);
        m->AddVertex(-0.5f, -0.5f,  0.0f, Color(255,255,255,255), 0.0f, 1.0f);
        m->AddTriangle(2,1,0);
        m->AddTriangle(0,3,2);

        Ptr<ShaderFill> fill = *new ShaderFill(*pRender->CreateShaderSet());
        fill->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Vertex, VShader_MVP)); 
        fill->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_Texture)); 
        fill->SetTexture(0, imageTex);
        m->Fill = fill;

        LoadingScene.World.Add(m);
    }
	LogText("Loading Screen ready\n");
}



//-----------------------------------------------------------------------------
void OculusWorldDemoApp::CycleDisplay()
{
    int screenCount = pPlatform->GetDisplayCount();

    // If Windowed, switch to the HMD screen first in Full-Screen Mode.
    // If already Full-Screen, cycle to next screen until we reach FirstScreenInCycle.

    if (pRender->IsFullscreen())
    {
        // Right now, we always need to restore window before going to next screen.
        pPlatform->SetFullscreen(RenderParams, Display_Window);

        Screen++;
        if (Screen == screenCount)
            Screen = 0;

        RenderParams.Display = pPlatform->GetDisplay(Screen);

        if (Screen != FirstScreenInCycle)
        {
            pRender->SetParams(RenderParams);
            pPlatform->SetFullscreen(RenderParams, Display_Fullscreen);
        }
    }
    else
    {
        // Try to find HMD Screen, making it the first screen in full-screen Cycle.        
        FirstScreenInCycle = 0;

        if (pHMD)
        {
            DisplayId HMD (SConfig.GetHMDInfo().DisplayDeviceName, SConfig.GetHMDInfo().DisplayId);
            for (int i = 0; i< screenCount; i++)
            {   
                if (pPlatform->GetDisplay(i) == HMD)
                {
                    FirstScreenInCycle = i;
                    break;
                }
            }            
        }







        // Switch full-screen on the HMD.
        Screen = FirstScreenInCycle;
        RenderParams.Display = pPlatform->GetDisplay(Screen);
        pRender->SetParams(RenderParams);
        pPlatform->SetFullscreen(RenderParams, Display_Fullscreen);
    }
}


//-------------------------------------------------------------------------------------

OVR_PLATFORM_APP(OculusWorldDemoApp);





