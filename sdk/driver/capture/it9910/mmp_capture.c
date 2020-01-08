#include "capture_config.h"
#include "capture_types.h"
#include "capture_reg.h"
#include "capture_hw.h"
#include "capture.h"
#include "capture/video_device_table.h"
#include "capture/mmp_capture.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "capture_interface.h"
 
#ifdef CFG_SENSOR_ENABLE
    #include "device_sensor.c"
#else
    #include "device_video.c"
#endif
 
//=============================================================================
//                Constant Definition
//=============================================================================
 
//=============================================================================
//                Macro Definition
//=============================================================================
 
//=============================================================================
//                Structure Definition
//=============================================================================
 
//=============================================================================
//                Global Data Definition
//=============================================================================
MMP_UINT16      gCapinitNum;    //record how many device opened
pthread_mutex_t gCapnum_mutex;   //for gCapinitNum mutex protect
pthread_mutex_t gCapmem_mutex;   //for mem mutex protect 
MMP_CAP_SHARE   DevInfo;        //Capture and ISP share info
 
//=============================================================================
//                Private Function Definition
//=============================================================================
 
//=============================================================================
//                Public Function Definition
//=============================================================================
 
//=============================================================================
/**
 * Cap context initialization.
 * @param   none.
 * @return  MMP_RESULT :init success or fail
 */
//=============================================================================
MMP_RESULT
ithCapInitialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    
    gCapinitNum = 0;
    /* mutex lock init */
    pthread_mutex_init(&gCapnum_mutex, NULL);
    pthread_mutex_init(&gCapmem_mutex, NULL);
    /* setting io callback function, implement in platform */
    CaptureIO_Init();
    
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);
 
    return (MMP_RESULT)result;
}
 
//=============================================================================
/**
 * Cap connect source and capinfo default init.
 * @param[in]   cap_handle,points to capture_handle structure.
 * @param[in]   CAPTURE_SETTING info
 * @return      MMP_RESULT :connect success or fail.
 */
//=============================================================================
MMP_RESULT ithCapConnect(
    CAPTURE_HANDLE *cap_handle, CAPTURE_SETTING info)
{
    MMP_RESULT result = MMP_SUCCESS;
 
    pthread_mutex_lock(&gCapnum_mutex);
 
    if (cap_handle == NULL || gCapinitNum >= DEVICE_ID_MAX)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
 
    /* reset caphandle mem */
    memset((void *)cap_handle, 0, sizeof(CAPTURE_HANDLE));
 
    /* count capture be init num */
    gCapinitNum++;
    
    /* cap id setting */
    cap_handle->cap_id = gCapinitNum - 1;
 
    pthread_mutex_unlock(&gCapnum_mutex);
 
    /* default capinfo setting */
    _Cap_Initialize(cap_handle);
    
    /* capture io setting */
    _Cap_SetInputSource(cap_handle, info.inputsource);
    
    /* frontend source id setting */
    cap_handle->source_id = info.inputsource;
 
    /* capture onflymode flag setting */
    if (info.OnflyMode_en)
        cap_handle->cap_info.EnableOnflyMode = MMP_TRUE;
    else
    {
        cap_handle->cap_info.EnableOnflyMode = MMP_FALSE;
        pthread_mutex_lock(&gCapmem_mutex);
        _Cap_Memory_Initialize(info.Max_Width,info.Max_Height);
        pthread_mutex_unlock(&gCapmem_mutex);
    }
 
    /* capture interrupt flag setting */
    if (info.Interrupt_en)
        cap_handle->cap_info.EnableInterrupt = MMP_TRUE;
    else
        cap_handle->cap_info.EnableInterrupt = MMP_FALSE;

    /* setting mem pitchY pitchUV */
    cap_handle->cap_info.ininfo.PitchY = info.Max_Width;
    cap_handle->cap_info.ininfo.PitchUV = info.Max_Width;

    ithCap_Set_EnableFrameRate_Reg(cap_handle->cap_id);

    ithCap_Set_Output_Driving_Strength_Reg(cap_handle->cap_id, 0x01);

    
    return result;
 
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);
}
 
//=============================================================================
/**
 * Cap disconnect source and cap_handle reset.
 * @param[in]   cap_handle,points to capture_handle structure.
 * @return      MMP_RESULT :disconnect success or fail.
 */
//=============================================================================
MMP_RESULT ithCapDisConnect(CAPTURE_HANDLE *cap_handle)
{
    MMP_RESULT result = MMP_SUCCESS;
 
    if (cap_handle == NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    if(cap_handle->cap_info.EnableOnflyMode == MMP_FALSE)
    {
        pthread_mutex_lock(&gCapmem_mutex);
        _Cap_Memory_Clear();
        pthread_mutex_unlock(&gCapmem_mutex);
    }
    
    /* reset caphandle mem */
    memset((void *)cap_handle, 0, sizeof(CAPTURE_HANDLE));
    
 
    pthread_mutex_lock(&gCapnum_mutex);
    if (gCapinitNum > 0)
        gCapinitNum--;
    pthread_mutex_unlock(&gCapnum_mutex);
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);
}
 
//=============================================================================
/**
 * Cap terminate all engine,include disable engine(unfire)  and reset engine.
 * @param   none.
 * @return  MMP_RESULT,1 => fail 0 => success
 */
//=============================================================================
MMP_RESULT
ithCapTerminate(
    void)
{
    MMP_RESULT  result  = MMP_SUCCESS;
    MMP_UINT16  index   = 0;
    MMP_UINT16 value;
    /*Disable Cap all engine*/
    for (index = 0; index < DEVICE_ID_MAX; index++)
    {    
        ithCap_Set_UnFire(index);
        result = ithCap_Get_WaitEngineIdle(index);
        if (result)
        {
            cap_msg_ex(CAP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }
 
        ithCap_Set_Engine_Reset(index);
        ithCap_Get_Clean_Intr(index);
    }
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);
 
    return (MMP_RESULT)result;
}
 
//=============================================================================
/**
 * Is Cap engine enable?
 * @param[in]   cap_handle,points to capture_handle structure.
 * @return      MMP_BOOL : True => Enable , Fasle => Disable.
 */
//=============================================================================
MMP_BOOL
ithCapIsFire(
    CAPTURE_HANDLE *cap_handle)
{
    return ithCap_Get_IsFire(cap_handle->cap_id);
}
 
//=============================================================================
/**
 * Get Cap engine Error Status
 * @param[in]   cap_handle,points to capture_handle structure.
 * @param[in]   lanenum,  fornt end source on which lane.
 * @return      status bit0-bit15
 *              [bit0] currunt state of capture engine '1' :idle ,'0':sync
 *              [bit8 -11] error id
 */
//=============================================================================
MMP_UINT16
ithCapGetEngineErrorStatus(
    CAPTURE_HANDLE *cap_handle, MMP_CAP_LANE_STATUS lanenum)
{
    return ithCap_Get_Lane_status(cap_handle->cap_id, lanenum);
}
 
//=============================================================================
/**
 * Check Cap signal is stable or unstable.
 * @param[in]   cap_handle,points to capture_handle structure.
 * @return      true  => stable
 *              false => nonstable
 */
//=============================================================================
MMP_BOOL
ithCapIsSignalStable(
    CAPTURE_HANDLE *cap_handle)
{
    MMP_BOOL    isSignalChange  = MMP_FALSE;
    MMP_UINT16  captureErrState = ithCapGetEngineErrorStatus(cap_handle, MMP_CAP_LANE0_STATUS);
 
    if (captureErrState & 0x0F00)
        isSignalChange = MMP_FALSE;
    else
        isSignalChange = MMP_TRUE;
 
    return (MMP_BOOL)isSignalChange;
}
 
 
//=============================================================================
/**
 * Enable Cap engine,start run!
 * @param[in]   cap_handle,points to capture_handle structure.
 * @return      none.
 */
//=============================================================================
void
ithCapFire(
    CAPTURE_HANDLE *cap_handle, const MMP_CAP_SHARE *data)
{
    MMP_RESULT  result      = MMP_SUCCESS;
    CAP_CONTEXT *cap_info    = &cap_handle->cap_info;
 
    if (cap_info == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }
 
    /*Set Active Region*/
    cap_info->ininfo.capwidth        = data->Width;
    cap_info->ininfo.capheight       = data->Height;
 
    /*Set Interlace or Prograssive*/
    cap_info->ininfo.Interleave      = data->IsInterlaced;
 
    /*Set Polarity*/
    cap_info->ininfo.HSyncPol        = data->HSyncPol;
    cap_info->ininfo.VSyncPol        = data->VSyncPol;
 
    /*Set Output Parameter*/
    cap_info->outinfo.OutWidth       = data->OutWidth;
    cap_info->outinfo.OutHeight      = data->OutHeight;
 
    cap_info->outinfo.OutAddrOffset  = data->OutAddrOffset;
 
    /*Update parameter*/
    result                          = _Cap_Update_Reg(cap_handle);
    
    /*enable capture hw*/
    ithCap_Set_Fire(cap_handle->cap_id);
 
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);    
}
 
//=============================================================================
/**
 * Register  an interrupt handler[Only OPENRTOS]
 * @param[in]   user define caphandler
 * @return      none.
 */
//=============================================================================
 
void
ithCapRegisterIRQ(
    ITHIntrHandler caphandler)
{
    /*Initialize Capture IRQ*/
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);
 
#if defined (__OPENRTOS__)
    /*register NAND Handler to IRQ*/
    ithIntrRegisterHandlerIrq(ITH_INTR_CAPTURE, caphandler, MMP_NULL);
#endif     // defined (__OPENRTOS__)
 
    /*set IRQ to edge trigger*/
    ithIntrSetTriggerModeIrq(ITH_INTR_CAPTURE, ITH_INTR_EDGE);
 
    /*set IRQ to detect rising edge*/
    ithIntrSetTriggerLevelIrq(ITH_INTR_CAPTURE, ITH_INTR_HIGH_RISING);
 
    /* Enable IRQ*/
    ithIntrEnableIrq(ITH_INTR_CAPTURE);
}
 
//=============================================================================
/**
 * Clear  an interrupt handler[Only OPENRTOS]
 * @param   none.
 * @return  none.
 */
//=============================================================================
 
void
ithCapDisableIRQ(
    void)
{
    /* Initialize Capture IRQ */
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);
}
 
//=============================================================================
/**
 * Control Capture Interrupt
 * @param[in]   flag,True => Enable interrupt, False =>Disable interrupt.
 * @return      none.
 */
//=============================================================================
void
ithCapEnableInterrupt(
    CAPTURE_HANDLE *cap_handle, MMP_BOOL flag)
{
    ithCap_Set_Enable_Interrupt(cap_handle->cap_id, flag);
}
 
//=============================================================================
/**
 * ithCapClearInterrupt
 */
//=============================================================================
MMP_UINT16
ithCapClearInterrupt(
    CAPTURE_HANDLE *cap_handle)
{
    MMP_UINT16 value, CapErrorType;
 
    value           = ithCap_Get_Clean_Intr(cap_handle->cap_id);
 
    CapErrorType    = value & 0x000F;
 
    return CapErrorType;
}
 
//=============================================================================
/**
 * Check current number of memory ring buffer
 * @param[in]   cap_handle,points to capture_handle structure.
 * @return      current buffer number.
 */
//=============================================================================
MMP_UINT16
ithCapReturnWrBufIndex(
    CAPTURE_HANDLE *cap_handle)
{
    MMP_UINT16  CapWrBufIndex;
    MMP_UINT16  status = ithCap_Get_Lane_status(cap_handle->cap_id, CAP_LANE0_STATUS);
 
    CapWrBufIndex = ((status & 0x7000) >> 12);
 
    return CapWrBufIndex;
}
 
//=============================================================================
/**
 * Get current input Source Frame Rate
 * @param       none.
 * @return      MMP_CAP_FRAMERATE type.
 */
//=============================================================================
MMP_CAP_FRAMERATE
ithCapGetInputFrameRate(
    void)
{
    MMP_UINT16  RawVTotal;
    MMP_UINT32  framerate;
    MMP_UINT16  FrameRate_mode;
    MMP_UINT32  MemClk;
    MMP_FLOAT   MCLK_Freq;
 
    RawVTotal   = ithReadRegH(ENGINE_STATUS_REG_14);
 
    MemClk      = ithGetMemClock();
    MCLK_Freq   = (MMP_FLOAT)(MemClk / 1000.0);
 
    framerate   = (MMP_UINT32)((3906 * MCLK_Freq) / RawVTotal);
    //printf("RawVTotal = %x MCLK_Freq = %f framerate = %d\n", RawVTotal, MCLK_Freq, framerate);
 
    if ((23988 > framerate) && (framerate > 23946))       // 23.976fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_23_97HZ;
    }
    else if ((24030 > framerate) && (framerate > 23987))  // 24fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_24HZ;
    }
    else if ((25030 > framerate) && (framerate > 24970))  // 25fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
    }
    else if ((29985 > framerate) && (framerate > 29940))  // 29.97fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
    }
    else if ((30030 > framerate) && (framerate > 29984))  // 30fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
    }
    else if ((50030 > framerate) && (framerate > 49970))  // 50fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_50HZ;
    }
    else if ((57000 > framerate) && (framerate > 55000))  // 56fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_56HZ;
    }
    else if ((59970 > framerate) && (framerate > 57001))  // 59.94fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_59_94HZ;
    }
    else if ((62030 > framerate) && (framerate > 59969))  // 60fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }
    else if ((70999 > framerate) && (framerate > 69000))  // 70fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_70HZ;
    }
    else if ((73000 > framerate) && (framerate > 71000))  // 72fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_72HZ;
    }
    else if ((76000 > framerate) && (framerate > 74000))  // 75fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_75HZ;
    }
    else if ((86000 > framerate) && (framerate > 84000))  // 85fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_85HZ;
    }
    else
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_UNKNOW;
    }
 
    return FrameRate_mode;
}
 
//=============================================================================
/**
 * Get current output Frame Rate,according to skip mode
 * @param[out]  output frame rate detail value
 * @return      Capture output frame rate
 */
//=============================================================================
 
MMP_CAP_FRAMERATE
ithCapGetOutputFrameRate(
    CAPTURE_HANDLE *cap_handle, MMP_UINT32 *FramePeriod)
{
    MMP_UINT16          RawVTotal;
    MMP_UINT32          framerate;
    MMP_UINT32          MemClk;
    MMP_FLOAT              MCLK_Freq;
    MMP_CAP_FRAMERATE   FrameRate_mode;
    CAP_CONTEXT         *Capctxt = &cap_handle->cap_info;
 
    RawVTotal = ithReadRegH(ENGINE_STATUS_REG_14);
    
    if (cap_handle->source_id == MMP_CAP_DEV_SENSOR)
    {
#if defined (SENSOR_OMNIVISION_OV7725)
         FrameRate_mode  = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_HIMAX_HM1375)
         FrameRate_mode  = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_PIXELPLUS_PO3100)
         FrameRate_mode  = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_NOVATEK_NT99141)
         FrameRate_mode  = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_AR0130)
         FrameRate_mode  = MMP_CAP_FRAMERATE_25HZ;
#endif
        return FrameRate_mode;
    }
 
    MemClk      = ithGetMemClock();
    MCLK_Freq = (MMP_FLOAT)(MemClk / 1000.0);
 
    framerate   = ((3906 * MCLK_Freq) / RawVTotal);
    //ithPrintf("RawVTotal = %x MCLK_Freq = %d framerate = %d\n", RawVTotal, MCLK_Freq, framerate);
 
    if (cap_handle->source_id == MMP_CAP_DEV_HDMIRX && ithCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
    {
        if (Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_VESA_30HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_VESA_60HZ;
    }
    else if ((23988 > framerate) && (framerate > 23946))  // 23.976fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_23_97HZ;
    }
    else if ((24030 > framerate) && (framerate > 23987))  // 24fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_24HZ;
    }
    else if ((25030 > framerate) && (framerate > 24970))  // 25fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
    }
    else if ((29985 > framerate) && (framerate > 29940))  // 29.97fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
    }
    else if ((30030 > framerate) && (framerate > 29984))  // 30fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
    }
    else if ((50030 > framerate) && (framerate > 49970))  // 50fps
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_50HZ;
    }
    else if ((59970 > framerate) && (framerate > 59910))  // 59.94fps
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_59_94HZ;
    }
    else if ((60030 > framerate) && (framerate > 59969))  // 60fps
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }
    else
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }
 
    if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
        *FramePeriod = (framerate >> 1);
    else
        *FramePeriod = framerate;

    return FrameRate_mode;
}
 
//=============================================================================
/**
 * ithCapGetInputSrcInfo
 */
//=============================================================================
MMP_CAP_INPUT_INFO
ithCapGetInputSrcInfo(
    CAPTURE_HANDLE *cap_handle)
{
    MMP_CAP_INPUT_INFO  info;
    MMP_CAP_FRAMERATE   frameRateMode;
 
    if (DevInfo.bMatchResolution == MMP_FALSE)
        return MMP_CAP_INPUT_INFO_UNKNOWN;
 
    frameRateMode = ithCapGetInputFrameRate();
 
    if (cap_handle->source_id == MMP_CAP_DEV_HDMIRX && ithCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
    {
        ithCapVESATimingCheck(DevInfo.Width, DevInfo.Height, &info);
    }
    else
    {
        switch (frameRateMode)
        {
        case MMP_CAP_FRAMERATE_23_97HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_23P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
 
        case MMP_CAP_FRAMERATE_24HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_24P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
 
        case MMP_CAP_FRAMERATE_25HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_25P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
 
        case MMP_CAP_FRAMERATE_29_97HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_29P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
 
        case MMP_CAP_FRAMERATE_30HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_30P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
 
        case MMP_CAP_FRAMERATE_50HZ:
            if (DevInfo.Width == 720 && DevInfo.Height == 576)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_720X576_50I;
                else
                    info = MMP_CAP_INPUT_INFO_720X576_50P;
            }
            else if (DevInfo.Width == 1280 && DevInfo.Height == 720)
            {
                info = MMP_CAP_INPUT_INFO_1280X720_50P;
            }
            else if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_1920X1080_50I;
                else
                    info = MMP_CAP_INPUT_INFO_1920X1080_50P;
            }
            else
            {
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;
 
        case MMP_CAP_FRAMERATE_59_94HZ:
            if (DevInfo.Width == 720 && DevInfo.Height == 480)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_720X480_59I;
                else
                    info = MMP_CAP_INPUT_INFO_720X480_59P;
            }
            else if (DevInfo.Width == 1280 && DevInfo.Height == 720)
            {
                info = MMP_CAP_INPUT_INFO_1280X720_59P;
            }
            else if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_1920X1080_59I;
                else
                    info = MMP_CAP_INPUT_INFO_1920X1080_59P;
            }
            else if (DevInfo.Width == 640 && DevInfo.Height == 480)
            {
                info = MMP_CAP_INPUT_INFO_640X480_60P;
            }
            else
            {
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;
 
        case MMP_CAP_FRAMERATE_60HZ:
            if (DevInfo.Width == 720 && DevInfo.Height == 480)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_720X480_60I;
                else
                    info = MMP_CAP_INPUT_INFO_720X480_60P;
            }
            else if (DevInfo.Width == 1280 && DevInfo.Height == 720)
            {
                info = MMP_CAP_INPUT_INFO_1280X720_60P;
            }
            else if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_1920X1080_60I;
                else
                    info = MMP_CAP_INPUT_INFO_1920X1080_60P;
            }
            else if (DevInfo.Width == 640 && DevInfo.Height == 480)
            {
                info = MMP_CAP_INPUT_INFO_640X480_60P;
            }
            else
            {
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;
 
        default:
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
        }
    }
 
    return info;
}
 
//=============================================================================
/**
 * Set Hsync Vsync Polarity convert
 * @param[in]   cap_handle,points to capture_handle structure.
 * @param[in]   HPolarity,default '0' blanking is low.
 * @param[in]   VPolarity,default '0' blanking is low.
 * @return      none.
 */
//=============================================================================
void
ithCapSetPolarity(CAPTURE_HANDLE *cap_handle, MMP_UINT16 HPolarity, MMP_UINT16 VPolarity)
{
    /* Set Hsync & Vsync Porlarity */
    ithCap_Set_Polarity(cap_handle->cap_id, HPolarity, VPolarity);
}
 
//=============================================================================
/**
 * ithAVSyncCounterInit
 */
//=============================================================================
void
ithAVSyncCounterCtrl(AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    AVSync_CounterCtrl(mode, divider);
}
 
//=============================================================================
/**
 * ithAVSyncCounterReset
 */
//=============================================================================
void
ithAVSyncCounterReset(AV_SYNC_COUNTER_CTRL mode)
{
    AVSync_CounterReset(mode);
}
 
//=============================================================================
/**
 * ithAVSyncCounterLatch
 */
//=============================================================================
MMP_UINT32
ithAVSyncCounterLatch(AV_SYNC_COUNTER_CTRL cntSel)
{
    AVSync_CounterLatch(cntSel);
}
 
//=============================================================================
/**
 * ithAVSyncCounterRead
 */
//=============================================================================
MMP_UINT32
ithAVSyncCounterRead(AV_SYNC_COUNTER_CTRL mode)
{
    return AVSync_CounterRead(mode);
}
 
//=============================================================================
/**
 * ithAVSyncMuteDetect
 */
//=============================================================================
MMP_BOOL
ithAVSyncMuteDetect(void)
{
    return AVSync_MuteDetect();
}
 
//=============================================================================
/**
 * ithCapSetSkipPattern
 */
//=============================================================================
MMP_RESULT
ithCapSetSkipMode(
    CAPTURE_HANDLE *cap_handle, MMP_CAP_SKIP_MODE mode)
{
    MMP_RESULT  result      = MMP_SUCCESS;
    CAP_CONTEXT *Capctxt    = &cap_handle->cap_info;
 
    if (Capctxt == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }
    
    if(Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
 
    if (mode == CAPTURE_SKIP_BY_TWO)
    {
        Capctxt->skip_mode = CAPTURE_SKIP_BY_TWO;
        Capctxt->Skippattern = 0xAAAA;
        Capctxt->SkipPeriod = 0xF;
    }
    else  if (mode == CAPTURE_SKIP_BY_THREE)
    {
        Capctxt->skip_mode = CAPTURE_SKIP_BY_THREE;
        Capctxt->Skippattern = 0x0003;
        Capctxt->SkipPeriod = 0x2;
    }
    else if (mode == CAPTURE_SKIP_30FPS_TO_25FPS)
    {
        Capctxt->skip_mode = CAPTURE_SKIP_30FPS_TO_25FPS;
        Capctxt->Skippattern = 0x0001;
        Capctxt->SkipPeriod = 0x5;
    }
    else if (mode == CAPTURE_SKIP_60FPS_TO_25FPS)
    {
        Capctxt->skip_mode = CAPTURE_SKIP_60FPS_TO_25FPS;
        Capctxt->Skippattern = 0x05ad;
        Capctxt->SkipPeriod = 0xb;
    }
    else
    {
        Capctxt->skip_mode = CAPTURE_NO_DROP;
        Capctxt->Skippattern = 0;
        Capctxt->SkipPeriod = 0xF;
    }
 
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);
}
 
//=============================================================================
/**
 * ithCapDelayPixelClk
 */
//=============================================================================
MMP_BOOL
ithCapDelayPixelClk(
    CAPTURE_HANDLE *cap_handle, MMP_BOOL flag)
{
    ithCap_Set_TurnOnClock_Reg(cap_handle->cap_id, flag);
}
 
//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
void
ithCapSetColorCtrl(
    CAPTURE_HANDLE *cap_handle, const MMP_CAP_COLOR_CTRL *data)
{
    CAP_CONTEXT *Capctxt = &cap_handle->cap_info;
#if defined (CAP_USE_COLOR_EFFECT)
    if (data->brightness > 127)
        Capctxt->ColorCtrl.brightness = 127;
    else if (data->brightness < -128)
        Capctxt->ColorCtrl.brightness = -128;
    else
        Capctxt->ColorCtrl.brightness = data->brightness;
 
    if (data->contrast > 4.0)
        Capctxt->ColorCtrl.contrast = 4.0;
    else if (data->contrast < 0.0)
        Capctxt->ColorCtrl.contrast = 0.0;
    else
        Capctxt->ColorCtrl.contrast = data->contrast;
 
    if (data->hue > 359)
        Capctxt->ColorCtrl.hue = 359;
    else if (data->hue < 0)
        Capctxt->ColorCtrl.hue = 0;
    else
        Capctxt->ColorCtrl.hue = data->hue;
 
    if (data->saturation > 4.0)
        Capctxt->ColorCtrl.saturation = 4.0;
    else if (data->saturation < 0.0)
        Capctxt->ColorCtrl.saturation = 0.0;
    else
        Capctxt->ColorCtrl.saturation = data->saturation;
 
    _Cap_SetColorCorrMatrix(
        &Capctxt->CCFun,
        Capctxt->ColorCtrl.brightness,
        Capctxt->ColorCtrl.contrast,
        Capctxt->ColorCtrl.hue,
        Capctxt->ColorCtrl.saturation,
        Capctxt->ColorCtrl.colorEffect);
 
    Capctxt->UpdateFlags |= CAP_FLAGS_UPDATE_CCMatrix;
#endif
}
 
//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
void
ithCapGetColorCtrl(
    CAPTURE_HANDLE *cap_handle, MMP_CAP_COLOR_CTRL *data)
{
    CAP_CONTEXT *Capctxt = &cap_handle->cap_info;
#if defined (CAP_USE_COLOR_EFFECT)
    data->brightness    = Capctxt->ColorCtrl.brightness;
    data->contrast      = Capctxt->ColorCtrl.contrast;
    data->hue           = Capctxt->ColorCtrl.hue;
    data->saturation    = Capctxt->ColorCtrl.saturation;
#endif
}
 
//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
void
ithCapUpdateColorMatrix(
    CAPTURE_HANDLE *cap_handle)
{
#if defined (CAP_USE_COLOR_EFFECT)
    _Cap_UpdateColorMatrix(cap_handle);
#endif
}
 
//=============================================================================
/**
 * Capture Power Up.
 */
//=============================================================================
void
ithCapPowerUp(
    void)
{
    ithCap_Set_Reset();
    ithCap_Set_EnableClock();
}
 
//=============================================================================
/**
 * Capture Power Down.
 */
//=============================================================================
void
ithCapPowerDown(
    void)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT16  id;
    for (id = 0; id < DEVICE_ID_MAX; id++)
    {
        ithCap_Set_UnFire(id);
 
        result = ithCap_Get_WaitEngineIdle(id);
        if (result)
            cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! capture [%d] not idle !!!!\n", id);
 
        ithCap_Get_Clean_Intr(id);
    }
    ithCap_Set_DisableClock();
}
