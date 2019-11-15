#include <string.h>
#include <sys/time.h>
#include "hdmirx/mmp_hdmirx.h"
#include "capture/mmp_capture.h"
#include "capture_module/capture_module.h"

#ifdef COMPONENT_DEV
    //#include "capture_forntend/cat9883/mmp_cat9883.h"
#endif

#ifdef COMPOSITE_DEV
    //#include "capture_forntend/adv7180/mmp_adv7180.h"
#endif

//=============================================================================
//                Constant Definition
//=============================================================================
extern MMP_UINT32 CapMemBuffer[CAPTURE_MEM_BUF_COUNT];

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static MMP_BOOL   gtDeviceReboot    = MMP_TRUE;
static MMP_UINT16 gtHDMIResolution  = 0xFF;
static MMP_UINT16 gtCVBSResolution  = 0xFF;
static MMP_UINT16 gtYPBPRResolution = 0xFF;
static MMP_UINT16 gtIsSVideoOMode   = MMP_FALSE;
static MMP_UINT16 gdefaultValue = 0x0;

static CaptureModuleDriver gadv7180;
static CaptureModuleDriver gcat9883;

static MMP_CAP_DEVICE_ID inputDeviceID;
extern MMP_CAP_SHARE  DevInfo;
extern pthread_mutex_t gCapmem_mutex;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Cap GetCaptureDevice ID 
 */
//=============================================================================

MMP_CAP_DEVICE_ID ithCapGetCaptureDevice(void)
{
    return inputDeviceID;
}

//=============================================================================
/**
 * Cap Auto Detect Device.
 */
//=============================================================================
MMP_CAP_DEVICE_ID
ithCapAutoDetectDevice(
    void)
{
    MMP_CAP_DEVICE_ID AutoDetectDev;

    switch (inputDeviceID)
    {

#ifdef COMPOSITE_DEV
    case MMP_CAP_DEV_ADV7180:
        if (ithModule_IsSignalStable(gadv7180,gdefaultValue))
        {
            if (ithHDMIRXIsSignalStable())
            {
                AutoDetectDev = MMP_CAP_DEV_HDMIRX;
                break;
            }
#ifdef COMPONENT_DEV
            if (ithModule_IsSignalStable(gcat9883,MMP_FALSE))
            {
                AutoDetectDev = MMP_CAP_DEV_CAT9883;
                break;
            }
#endif
            AutoDetectDev = MMP_CAP_DEV_ADV7180;
        }
        else
            AutoDetectDev = MMP_CAP_UNKNOW_DEVICE;
        break;
#endif

        ////////////////////////////////////////////////////////////////////////////
#ifdef COMPONENT_DEV
    case MMP_CAP_DEV_CAT9883:
        if (ithModule_IsSignalStable(gcat9883,MMP_TRUE))
        {
            if (ithHDMIRXIsSignalStable())
            {
                AutoDetectDev = MMP_CAP_DEV_HDMIRX;
                break;
            }
#ifdef COMPOSITE_DEV
            if (ithModule_IsSignalStable(gadv7180,gdefaultValue))
            {
                AutoDetectDev = MMP_CAP_DEV_ADV7180;
                break;
            }
#endif
            AutoDetectDev = MMP_CAP_DEV_CAT9883;
        }
        else
        {
            ithCapDelayPixelClk(0,MMP_FALSE);
            ithModule_SetProperty(gcat9883,Cat9883Reinit,MMP_TRUE);
            ithCapDelayPixelClk(0,MMP_TRUE);
            AutoDetectDev = MMP_CAP_UNKNOW_DEVICE;
        }
        break;
#endif

    ////////////////////////////////////////////////////////////////////////////
    case MMP_CAP_DEV_HDMIRX:
        if (ithHDMIRXIsSignalStable())
        {
#ifdef COMPONENT_DEV
            if (ithModule_IsSignalStable(gcat9883,MMP_FALSE))
            {
                AutoDetectDev = MMP_CAP_DEV_CAT9883;
                break;
            }
#endif
#ifdef COMPOSITE_DEV
            if (ithModule_IsSignalStable(gadv7180,gdefaultValue))
            {
                AutoDetectDev = MMP_CAP_DEV_ADV7180;
                break;
            }
#endif
            AutoDetectDev = MMP_CAP_DEV_HDMIRX;
        }
        else
            AutoDetectDev = MMP_CAP_UNKNOW_DEVICE;
        break;

    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, "No Match Enable Type !\n");
        break;
    }
    
    ////////////////////////////////////////////////////////////////////////////

    if (AutoDetectDev == MMP_CAP_UNKNOW_DEVICE)
    {
        if (ithHDMIRXIsSignalStable())
        {
            AutoDetectDev = MMP_CAP_DEV_HDMIRX;
        }

#ifdef COMPONENT_DEV
        else if (ithModule_IsSignalStable(gcat9883,MMP_FALSE))
        {
            AutoDetectDev = MMP_CAP_DEV_CAT9883;
        }
#endif

#ifdef COMPOSITE_DEV
        else if (ithModule_IsSignalStable(gadv7180,gdefaultValue))
        {
            AutoDetectDev = MMP_CAP_DEV_ADV7180;
        }
#endif
    }

    return AutoDetectDev;
}

//=============================================================================
/**
 * Cap Device Tri-State.
 */
//=============================================================================
void
ithCapDeviceAllDeviceTriState(
    void)
{
    //Tri-State All Device
    ithPrintf("ithHDMIRX Tri-State Device\n");
    ithHDMIRXOutputPinTriState(MMP_TRUE);

#ifdef COMPONENT_DEV
    ithPrintf("mmpCAT9883 Tri-State Device\n");
    ithModule_OutputPinTriState(gcat9883,MMP_TRUE);
#endif

#ifdef COMPOSITE_DEV
    ithPrintf("mmpADV7180 Tri-State Device\n");
    ithModule_OutputPinTriState(gadv7180,MMP_TRUE);
#endif
}

//=============================================================================
/**
 * Cap Device initialization.
 */
//=============================================================================
MMP_RESULT
ithCapDeviceInitialize(
    MMP_CAP_DEVICE_ID Device_ID)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 Value = 0;
    inputDeviceID = Device_ID;
  
    if (gtDeviceReboot)
    {
        gtHDMIResolution  = 0xFF;
        gtCVBSResolution  = 0xFF;
        gtYPBPRResolution = 0xFF;

        //Initialize All Device and Tri-State All Device
        if (inputDeviceID != MMP_CAP_DEV_HDMIRX)
        {
            ithHDMIRXPowerDown(MMP_TRUE);
            HOST_ReadRegister(GEN_SETTING_REG_34, &Value);
            Value |= ((0x0 & 0x1) << 0) ;//Enable video source from IO pad
            HOST_WriteRegister(GEN_SETTING_REG_34, Value);
        }
        else
        {
            ithHDMIRXInitialize();

            HOST_ReadRegister(GEN_SETTING_REG_34, &Value);
            Value |= ((0x1 & 0x1) << 0) ;//Enable video source from internal HDMI RX
            HOST_WriteRegister(GEN_SETTING_REG_34, Value);           
        }
        ithHDMIRXOutputPinTriState(MMP_TRUE);

#ifdef COMPONENT_DEV
        gcat9883 = ithModule_Create("cat9883_adc.c");
        if (inputDeviceID != MMP_CAP_DEV_CAT9883)
            ithModule_PowerDown(gcat9883,MMP_TRUE);
        else
        {
            gtIsSVideoOMode = MMP_TRUE;
            ithModule_Init(gcat9883,gdefaultValue);
        }
        ithModule_OutputPinTriState(gcat9883,MMP_TRUE);
#endif

#ifdef COMPOSITE_DEV

        gadv7180 = ithModule_Create("adv7180.c");

        if ((inputDeviceID != MMP_CAP_DEV_ADV7180 && inputDeviceID != MMP_CAP_DEV_CAT9883))
            ithModule_PowerDown(gadv7180,MMP_TRUE);
        else
        {
            ithModule_Init(gadv7180,gtIsSVideoOMode);

            if (inputDeviceID == MMP_CAP_DEV_CAT9883)
            {
                ithModule_PowerDown(gadv7180,MMP_TRUE);
                gtIsSVideoOMode = MMP_FALSE;
            }
        }
        ithModule_OutputPinTriState(gadv7180,MMP_TRUE);
#endif

        switch (inputDeviceID)
        {
#ifdef COMPOSITE_DEV
        case MMP_CAP_DEV_ADV7180:
            ithModule_OutputPinTriState(gadv7180,MMP_FALSE);
            break;
#endif

#ifdef COMPONENT_DEV
        case MMP_CAP_DEV_CAT9883:
            ithModule_OutputPinTriState(gcat9883,MMP_FALSE);
            break;
#endif

        case MMP_CAP_DEV_HDMIRX:
            ithHDMIRXOutputPinTriState(MMP_FALSE);
            break;

        default:
            cap_msg_ex(CAP_MSG_TYPE_ERR, "No Match Enable Type !\n");
            break;
        }
    }

    memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));
    {
        struct timeval tv;
        unsigned int t0;
        unsigned int t;
        gettimeofday(&tv, NULL);
        t0=tv.tv_sec * 1000 + tv.tv_usec / 1000;
        ithPrintf("wait ithCapDeviceIsSignalStable...\n");
        while (!ithCapGetDeviceIsSignalStable()) {
            usleep(100);
        }
        gettimeofday(&tv, NULL);
        t=tv.tv_sec * 1000 + tv.tv_usec / 1000;
        ithPrintf("ithCapDeviceIsSignalStable ok, taking %dms\n", t-t0);
        t0=t;
        while (1) {
            usleep(100);
            ithCapGetDeviceIsSignalStable();
            gettimeofday(&tv, NULL);
            t=tv.tv_sec * 1000 + tv.tv_usec / 1000;
            if (t - t0 > 2000) {
                break;
            }
        }
    }

    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Cap Device terminate.
 */
//=============================================================================
void
ithCapDeviceTerminate(
    void)
{

    //ithCapOnflyResetAllEngine();

    if (gtDeviceReboot)
    {
        //ithCapDeviceAllDeviceTriState();

        ithHDMIRXTerminate();

#ifdef COMPONENT_DEV
        ithModule_DeInit(gcat9883);
        ithModule_Destroy(gcat9883);
#endif

#ifdef COMPOSITE_DEV
        ithModule_DeInit(gadv7180);
        ithModule_Destroy(gadv7180);
#endif

        gtHDMIResolution  = 0xFF;
        gtCVBSResolution  = 0xFF;
        gtYPBPRResolution = 0xFF;

        memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));
    }
}

//=============================================================================
/**
 * Cap Device Signal State.
 */
//=============================================================================
MMP_BOOL
ithCapGetDeviceIsSignalStable(
    void)
{
    MMP_BOOL isSignalChange = MMP_FALSE;

    switch (inputDeviceID)
    {
#ifdef COMPOSITE_DEV
    case MMP_CAP_DEV_ADV7180:
        isSignalChange = ithModule_IsSignalStable(gadv7180,gdefaultValue);
        break;
#endif

#ifdef COMPONENT_DEV
    case MMP_CAP_DEV_CAT9883:
        isSignalChange = ithModule_IsSignalStable(gcat9883,MMP_TRUE);
        if(isSignalChange == MMP_FALSE)
        {
            ithCapDelayPixelClk(0,MMP_FALSE);
            ithModule_SetProperty(gcat9883,Cat9883Reinit,MMP_TRUE);
            ithCapDelayPixelClk(0,MMP_TRUE);
        }
#ifdef COMPOSITE_DEV
        if (isSignalChange && ithModule_GetStatus(gcat9883,IsNTSCorPAL))
        {
            if (ithModule_GetStatus(gadv7180,IsPowerDown))
                ithModule_PowerDown(gadv7180,MMP_FALSE);

            if (ithModule_GetStatus(gadv7180,IsSVideoInput))
                isSignalChange = MMP_FALSE;
            else
                isSignalChange = MMP_TRUE;
        }
        else
        {
            if (!ithModule_GetStatus(gadv7180,IsPowerDown))
                ithModule_PowerDown(gadv7180,MMP_TRUE);
        }
#endif
        break;
#endif

    case MMP_CAP_DEV_HDMIRX:
        isSignalChange = ithHDMIRXIsSignalStable();
        break;

    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
        break;
    }

    return (MMP_BOOL)isSignalChange;
}


//=============================================================================
/**
 * mmpCapGetDeviceInfo
 */
//=============================================================================
void
ithCapGetDeviceInfo(
    CAPTURE_HANDLE *cap_handle,MMP_CAP_SHARE *data)
{
    MMP_UINT32         HTotal, VTotal, ColorDepth;
    MMP_UINT16         i, rate, Index;
    MMP_BOOL           matchResolution = MMP_FALSE;
    MMP_CAP_INPUT_INFO infoIdx;
    CAP_CONTEXT *Capctxt = &cap_handle->cap_info;


    Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS;

    if (cap_handle->source_id == MMP_CAP_DEV_HDMIRX)
    {
        data->IsInterlaced = (MMP_BOOL)ithHDMIRXGetProperty(HDMIRX_IS_INTERLACE);

        if (data->IsInterlaced)
            data->Height = (MMP_UINT16)ithHDMIRXGetProperty(HDMIRX_HEIGHT) << 1;
        else
            data->Height = (MMP_UINT16)ithHDMIRXGetProperty(HDMIRX_HEIGHT);

        data->Width = (MMP_UINT16)ithHDMIRXGetProperty(HDMIRX_WIDTH);

        HTotal      = ithHDMIRXGetProperty(HDMIRX_HTOTAL);
        VTotal      = ithHDMIRXGetProperty(HDMIRX_VTOTAL);

        for (i = 0; i < (sizeof(HDMI_TABLE) / sizeof(CAP_HDMI_TIMINFO_TABLE)); ++i)
        {
            if ((HTotal == HDMI_TABLE[i].HTotal) &&
                (VTotal == HDMI_TABLE[i].VTotal || VTotal == HDMI_TABLE[i].VTotal + 1) )
            {
                if (i == 6 || i == 7) //1080p60/1080p50 or 1080p60/1080p25
                {
                    MMP_UINT32 hdmirxPCLK = ithHDMIRXGetProperty(HDMIRX_PCLK);
                    if (hdmirxPCLK < 100000000) //100MHz
                        i = i + 6;              //1080p60/1080p25
                }

                data->HSyncPol       = HDMI_TABLE[i].HPolarity;
                data->VSyncPol       = HDMI_TABLE[i].VPolarity;
                Capctxt->Skippattern = HDMI_TABLE[i].Skippattern;
                Capctxt->SkipPeriod  = HDMI_TABLE[i].SkipPeriod;
                data->FrameRate      = HDMI_TABLE[i].FrameRate;
                data->inputPAR       = ithHDMIRXGetProperty(HDMIRX_ASPECT_RATIO);
                matchResolution      = MMP_TRUE;
                gtHDMIResolution     = i;
                break;
            }
        }

#ifdef IT9913_128LQFP
        Capctxt->funen.EnCSFun = MMP_FALSE;
#else
        if (ithHDMIRXGetProperty(HDMIRX_OUTPUT_VIDEO_MODE) == 0)
            Capctxt->funen.EnCSFun = MMP_TRUE;  //HDMI Rx RGB444 Output
        else
            Capctxt->funen.EnCSFun = MMP_FALSE; //HDMI Rx YUV444/YUV422 Output
#endif

        if (matchResolution == MMP_FALSE)
        {
            MMP_CAP_INPUT_INFO infoIdx;
            data->HSyncPol           = 1;
            data->VSyncPol           = 1;
            Capctxt->Skippattern     = 0xAAAA;
            Capctxt->SkipPeriod      = 0xF;
            data->FrameRate          = MMP_CAP_FRAMERATE_60HZ;
            data->inputPAR           = MMP_CAP_PAR_OTHER;
            gtHDMIResolution         = CAP_HDMI_INPUT_VESA;
            Capctxt->ininfo.HNum1    = 0;
            Capctxt->ininfo.HNum2    = 0;
            Capctxt->ininfo.LineNum1 = 0;
            Capctxt->ininfo.LineNum2 = 0;
            Capctxt->ininfo.LineNum3 = 0;
            Capctxt->ininfo.LineNum4 = 0;

            Capctxt->funen.EnDEMode  = MMP_TRUE;
            matchResolution          = ithCapVESATimingCheck(data->Width, data->Height, &infoIdx);
        }
        else
        {
            Capctxt->ininfo.HNum1    = HDMI_TABLE[i].HStar;
            Capctxt->ininfo.HNum2    = HDMI_TABLE[i].HEnd;
            Capctxt->ininfo.LineNum1 = HDMI_TABLE[i].VStar1;
            Capctxt->ininfo.LineNum2 = HDMI_TABLE[i].VEnd1;
            Capctxt->ininfo.LineNum3 = HDMI_TABLE[i].VStar2;
            Capctxt->ininfo.LineNum4 = HDMI_TABLE[i].VEnd2;

            Capctxt->funen.EnDEMode  = MMP_FALSE;
        }

        printf("Htotal = %d, Vtotal = %d, w = %d h = %d, res = %d,colorMode = %d\n", HTotal, VTotal, data->Width, data->Height, gtHDMIResolution,ithHDMIRXGetProperty(HDMIRX_OUTPUT_VIDEO_MODE));

        /* Set ROI */
        Capctxt->ininfo.ROIPosX = 0;
        Capctxt->ininfo.ROIPosY = 0;
        Capctxt->ininfo.ROIWidth = ((data->Width >> 2) << 2);
        Capctxt->ininfo.ROIHeight = data->Height;

        data->OutWidth  = ((data->Width >> 2) << 2);
        data->OutHeight = data->Height;

        ColorDepth      = ithHDMIRXGetProperty(HDMIRX_COLOR_DEPTH);

        if (ColorDepth == 36)
            Capctxt->ininfo.ColorDepth = COLOR_DEPTH_12_BITS;
        else if (ColorDepth == 30)
            Capctxt->ininfo.ColorDepth = COLOR_DEPTH_10_BITS;
        else //(ColorDepth == 24)
            Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS;
    }

#ifdef COMPONENT_DEV
    if (cap_handle->source_id == MMP_CAP_DEV_CAT9883)
    {
        data->IsInterlaced = (MMP_BOOL)ithModule_GetProperty(gcat9883,GetModuleIsInterlace);
        data->Height       = (MMP_UINT16)ithModule_GetProperty(gcat9883,GetHeight);
        data->Width        = (MMP_UINT16)ithModule_GetProperty(gcat9883,GetWidth);
        rate               = (MMP_UINT16)ithModule_GetProperty(gcat9883,Rate);

        for (i = 0; i < (sizeof(CAT9883_TABLE) / sizeof(CAP_CAT9883_TIMINFO_TABLE)); ++i)
        {
            if ((data->Width == CAT9883_TABLE[i].HActive) &&
                (data->Height == CAT9883_TABLE[i].VActive) &&
                rate == CAT9883_TABLE[i].Rate)
            {
                data->HSyncPol       = CAT9883_TABLE[i].HPolarity;
                data->VSyncPol       = CAT9883_TABLE[i].VPolarity;
                data->FrameRate      = CAT9883_TABLE[i].FrameRate;
                Capctxt->Skippattern = CAT9883_TABLE[i].Skippattern;
                Capctxt->SkipPeriod  = CAT9883_TABLE[i].SkipPeriod;
                matchResolution      = MMP_TRUE;
                gtYPBPRResolution    = Index = i;
                break;
            }
        }

        if (data->Width >= 1280)
            data->inputPAR = MMP_CAP_PAR_16_9;
        else
            data->inputPAR = MMP_CAP_PAR_4_3;

        if (i == (sizeof(CAT9883_TABLE) / sizeof(CAP_CAT9883_TIMINFO_TABLE)))
            cap_msg_ex(CAP_MSG_TYPE_ERR, "No Support Resolution! %dx%d @%dHz\n", data->Width, data->Height, (rate / 100));

        if (ithModule_GetStatus(gcat9883,IsTVMODE))
            Capctxt->funen.EnCSFun = MMP_FALSE;
        else
            Capctxt->funen.EnCSFun = MMP_TRUE;

        Capctxt->ininfo.HNum1           = CAT9883_TABLE[i].HStar;
        Capctxt->ininfo.HNum2           = CAT9883_TABLE[i].HEnd;
        Capctxt->ininfo.LineNum1        = CAT9883_TABLE[i].VStar1;
        Capctxt->ininfo.LineNum2        = CAT9883_TABLE[i].VEnd1;
        Capctxt->ininfo.LineNum3        = CAT9883_TABLE[i].VStar2;
        Capctxt->ininfo.LineNum4        = CAT9883_TABLE[i].VEnd2;

        Capctxt->funen.EnCrossLineDE    = CAT9883_TABLE[i].CrossLineDE;
        Capctxt->funen.EnYPbPrTopVSMode = CAT9883_TABLE[i].YPbPrTopVSMode;
        Capctxt->funen.EnDlyVS          = CAT9883_TABLE[i].DlyVS;
        Capctxt->funen.EnHSPosEdge      = CAT9883_TABLE[i].HSPosEdge;

        /* Set ROI */
        Capctxt->ininfo.ROIPosX = CAT9883_TABLE[i].ROIPosX;
        Capctxt->ininfo.ROIPosY = CAT9883_TABLE[i].ROIPosY;
        Capctxt->ininfo.ROIWidth = ((CAT9883_TABLE[i].ROIWidth >> 2) << 2);
        Capctxt->ininfo.ROIHeight = CAT9883_TABLE[i].ROIHeight;

        data->OutWidth                  = ((CAT9883_TABLE[i].ROIWidth >> 2) << 2);
        data->OutHeight                 = CAT9883_TABLE[i].ROIHeight;

        Capctxt->inmux_info.UCLKInv     = CAT9883_TABLE[i].UCLKInv;
    }
#endif

#ifdef COMPOSITE_DEV
    if (cap_handle->source_id == MMP_CAP_DEV_ADV7180)
    {
        data->IsInterlaced = (MMP_BOOL)ithModule_GetProperty(gadv7180,GetModuleIsInterlace);
        data->Height       = (MMP_UINT16)ithModule_GetProperty(gadv7180,GetHeight);
        data->Width        = (MMP_UINT16)ithModule_GetProperty(gadv7180,GetWidth);
        rate               = (MMP_UINT16)ithModule_GetProperty(gadv7180,Rate);

        for (i = 0; i < (sizeof(ADV7180_TABLE) / sizeof(CAP_ADV7180_TIMINFO_TABLE)); ++i)
        {
            if ((data->Width == ADV7180_TABLE[i].HActive) &&
                (data->Height == ADV7180_TABLE[i].VActive) &&
                rate == ADV7180_TABLE[i].Rate)
            {
                data->HSyncPol       = ADV7180_TABLE[i].HPolarity;
                data->VSyncPol       = ADV7180_TABLE[i].VPolarity;
                data->FrameRate      = ADV7180_TABLE[i].FrameRate;
                Capctxt->Skippattern = ADV7180_TABLE[i].Skippattern;
                Capctxt->SkipPeriod  = ADV7180_TABLE[i].SkipPeriod;
                matchResolution      = MMP_TRUE;
                gtCVBSResolution     = i;
                break;
            }
        }

        data->inputPAR = MMP_CAP_PAR_4_3;

        if (i == (sizeof(ADV7180_TABLE) / sizeof(CAP_ADV7180_TIMINFO_TABLE)))
            cap_msg_ex(CAP_MSG_TYPE_ERR, "No Support Resolution! %dx%d @%dHz\n", data->Width, data->Height, (rate / 100));

        Capctxt->funen.EnCSFun   = MMP_FALSE;

        Capctxt->ininfo.HNum1    = ADV7180_TABLE[i].HStar;
        Capctxt->ininfo.HNum2    = ADV7180_TABLE[i].HEnd;
        Capctxt->ininfo.LineNum1 = ADV7180_TABLE[i].VStar1;
        Capctxt->ininfo.LineNum2 = ADV7180_TABLE[i].VEnd1;
        Capctxt->ininfo.LineNum3 = ADV7180_TABLE[i].VStar2;
        Capctxt->ininfo.LineNum4 = ADV7180_TABLE[i].VEnd2;

        /* Set ROI */
        Capctxt->ininfo.ROIPosX = ADV7180_TABLE[i].ROIPosX;
        Capctxt->ininfo.ROIPosY = ADV7180_TABLE[i].ROIPosY;
        Capctxt->ininfo.ROIWidth = ((ADV7180_TABLE[i].ROIWidth >> 2) << 2);
        Capctxt->ininfo.ROIHeight = ADV7180_TABLE[i].ROIHeight;
        

        data->OutWidth           = ((ADV7180_TABLE[i].ROIWidth >> 2) << 2);
        data->OutHeight          = ADV7180_TABLE[i].ROIHeight;
    }
#endif

    pthread_mutex_lock(&gCapmem_mutex);
    data->OutAddrY[0]      = CapMemBuffer[CAP_MEM_Y0];
    data->OutAddrUV[0]     = CapMemBuffer[CAP_MEM_UV0];
    data->OutAddrY[1]      = CapMemBuffer[CAP_MEM_Y1];
    data->OutAddrUV[1]     = CapMemBuffer[CAP_MEM_UV1];
    data->OutAddrY[2]      = CapMemBuffer[CAP_MEM_Y2];
    data->OutAddrUV[2]     = CapMemBuffer[CAP_MEM_UV2];
    pthread_mutex_unlock(&gCapmem_mutex);

    data->bMatchResolution = matchResolution;

    _Cap_SetInputSource(cap_handle, cap_handle->source_id);
    
    memcpy(&DevInfo, data, sizeof(MMP_CAP_SHARE));

    if (matchResolution == MMP_FALSE)
        printf("---Resolutin not Suppott !---\n");
}

//=============================================================================
/**
 * Device reboot.
 */
//=============================================================================
MMP_RESULT
ithCapSetDeviceReboot(
    MMP_BOOL flag)
{
    gtDeviceReboot = flag;

    if (!gtDeviceReboot)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "------Not Reboot Device-------\n");
}

MMP_BOOL
ithCapGetDeviceReboot(
    void)
{
    return gtDeviceReboot;
}

//=============================================================================
/**
 * Capture Resolution.
 */
//=============================================================================
MMP_UINT16
ithCapGetResolutionIndex(
    MMP_CAP_DEVICE_ID id)
{
    MMP_UINT16 resIndex;

    switch (id)
    {
    case MMP_CAP_DEV_HDMIRX:
        resIndex = gtHDMIResolution;
        break;

    case MMP_CAP_DEV_CAT9883:
        resIndex = gtYPBPRResolution;
        break;

    case MMP_CAP_DEV_ADV7180:
        resIndex = gtCVBSResolution;
        break;

    default:
        resIndex = 0;
        break;
    }

    return resIndex;
}

//=============================================================================
/**
 * Capture VESA Timing Check.
 */
//=============================================================================
MMP_BOOL
ithCapVESATimingCheck(
    MMP_UINT16         width,
    MMP_UINT16         height,
    MMP_CAP_INPUT_INFO *info)
{
    MMP_CAP_INPUT_INFO infoIdx;
    MMP_CAP_FRAMERATE  frameRateMode;

    frameRateMode = ithCapGetInputFrameRate();

    switch (frameRateMode)
    {
    case MMP_CAP_FRAMERATE_59_94HZ:
    case MMP_CAP_FRAMERATE_60HZ:
        if (width == 800 && height == 600)
        {
            infoIdx = MMP_CAP_INPUT_INFO_800X600_60P;
        }
        else if (width == 1024 && height == 768)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1024X768_60P;
        }
        else if (width == 1280 && height == 720)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1280X720_60P;
        }
        else if (width == 1280 && height == 768)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1280X768_60P;
        }
        else if (width == 1280 && height == 800)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1280X800_60P;
        }
        else if (width == 1280 && height == 960)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1280X960_60P;
        }
        else if (width == 1280 && height == 1024)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1280X1024_60P;
        }
        else if (width == 1360 && height == 768)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1360X768_60P;
        }
        else if (width == 1366 && height == 768)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1366X768_60P;
        }
        else if (width == 1440 && height == 900)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1440X900_60P;
        }
        else if (width == 1400 && height == 1050)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1400X1050_60P;
        }
        else if (width == 1440 && height == 1050)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1440X1050_60P;
        }
        else if (width == 1600 && height == 900)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1600X900_60P;
        }
        //else if (width == 1600 && height == 1200)
        //{
        //    infoIdx = MMP_CAP_INPUT_INFO_1600X1200_60P;
        //}
        else if (width == 1680 && height == 1050)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1680X1050_60P;
        }
        else if (width == 1920 && height == 1080)
        {
            infoIdx = MMP_CAP_INPUT_INFO_1920X1080_60P;
        }
        else
        {
            infoIdx = MMP_CAP_INPUT_INFO_UNKNOWN;
        }
        break;

    default:
        infoIdx = MMP_CAP_INPUT_INFO_UNKNOWN;
        break;
    }

    (*info) = infoIdx;

    if (infoIdx == MMP_CAP_INPUT_INFO_UNKNOWN)
        return MMP_FALSE;
    else
        return MMP_TRUE;
}

