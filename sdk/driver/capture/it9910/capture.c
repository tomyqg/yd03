#include "math.h"
#include "capture/capture_types.h"
#include "capture_config.h"
#include "capture.h"
#include "capture_hw.h"
#include "capture_util.h"
#include "capture_reg.h"
#include "mmp_capture.h"

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
/*
   Input   Output  Ratio(In/Out)
   1088    1080    1.007407407
   1920    1920    1
   1280    1920    0.666666667
   576     1080    0.533333333
   480     1080    0.444444444
   720     1920    0.375
   640     1920    0.333333333
   288     1080    0.266666667
   352     1920    0.183333333
 */
#define     WEIGHT_NUM 9
static MMP_FLOAT    ScaleRatio[WEIGHT_NUM] = {(1088.0f / 1080), (1920.0f / 1920), (1280.0f / 1920), (576.0f / 1080), (480.0f / 1080), (720.0f / 1920), (640.0f / 1920), (288.0f / 1080), (352.0f / 1920)};
static MMP_UINT16   WeightMatInt[WEIGHT_NUM][CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];

MMP_UINT32          CapMemBuffer[CAPTURE_MEM_BUF_COUNT] = { 0 };


/*static MMP_UINT16 CapMemTable[CAPTURE_MEM_BUF_COUNT] = {
    {0                                                         },//Y0
    {CAP_INPUT_MAX_HEIGHT * 3                                  },//UV0
    {CAP_INPUT_MAX_HEIGHT                                      },//Y1
    {CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1)    },//UV1
    {CAP_INPUT_MAX_HEIGHT * 2                                  },//Y2
    {CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1) * 2} //UV2
};*/

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Calculate Scale Factor
 */
//=============================================================================
static MMP_FLOAT
_Cap_ScaleFactor(
    MMP_UINT16  Input,
    MMP_UINT16  Output)
{
    return (MMP_FLOAT) (((MMP_INT) (16384.0f * Input / (MMP_FLOAT)Output)) / 16384.0f);
}

//=============================================================================
/**
 * Select Color Matrix
 */
//=============================================================================
static void
_Cap_ColorMatrix(
    CAP_CONTEXT *Capctxt, MMP_BOOL isBT709)
{
    if (isBT709 == MMP_TRUE)
    {
        /* RGB to YUV REG */
        Capctxt->RGBtoYUVFun._11    = 0x0037;
        Capctxt->RGBtoYUVFun._12    = 0x00B7;
        Capctxt->RGBtoYUVFun._13    = 0x0012;
        Capctxt->RGBtoYUVFun._21    = 0x07e2;
        Capctxt->RGBtoYUVFun._22    = 0x079b;
        Capctxt->RGBtoYUVFun._23    = 0x0083;
        Capctxt->RGBtoYUVFun._31    = 0x0083;
        Capctxt->RGBtoYUVFun._32    = 0x0789;
        Capctxt->RGBtoYUVFun._33    = 0x07f4;
        Capctxt->RGBtoYUVFun.ConstY = 0x0000;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;
    }
    else
    {
        /* RGB to YUV REG */
        Capctxt->RGBtoYUVFun._11    = 0x004D;
        Capctxt->RGBtoYUVFun._12    = 0x0096;
        Capctxt->RGBtoYUVFun._13    = 0x001D;
        Capctxt->RGBtoYUVFun._21    = 0x07d4;
        Capctxt->RGBtoYUVFun._22    = 0x07a9;
        Capctxt->RGBtoYUVFun._23    = 0x0083;
        Capctxt->RGBtoYUVFun._31    = 0x0083;
        Capctxt->RGBtoYUVFun._32    = 0x0792;
        Capctxt->RGBtoYUVFun._33    = 0x07eb;
        Capctxt->RGBtoYUVFun.ConstY = 0x0000;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;
    }
}

//=============================================================================
/**
 * Create weighting for the matrix of scaling.
 */
//=============================================================================
static void
_Cap_CreateWeighting(
    MMP_FLOAT   scale,
    MMP_UINT8   taps,
    MMP_UINT8   tapSize,
    MMP_FLOAT   weightMatrix[][CAP_SCALE_TAP])
{
    MMP_UINT8   i, j;
    MMP_FLOAT   WW;
    MMP_FLOAT   W[32];
    MMP_INT16   point;
    MMP_UINT8   adjust;
    MMP_INT16   sharp;
    MMP_UINT8   method;
    MMP_FLOAT   precision   = 64.0f;
    MMP_FLOAT   fscale      = 1.0f;

    //Kevin:TODO temp solution
    adjust  = 0;
    method  = 11;
    sharp   = 0;

    switch (sharp)
    {
    case 4:     fscale  = 0.6f;  break;
    case 3:     fscale  = 0.7f;  break;
    case 2:     fscale  = 0.8f;  break;
    case 1:     fscale  = 0.9f;  break;
    case 0:     fscale  = 1.0f;  break;
    case -1:    fscale  = 1.1f;  break;
    case -2:    fscale  = 1.2f;  break;
    case -3:    fscale  = 1.3f;  break;
    case -4:    fscale  = 1.4f;  break;
    default:    fscale  = 1.5f;  break;
    }

    if (adjust == 0)
    {
        if (scale < 1.0f)
        {
            scale = fscale;
        }
        else
        {
            scale *= fscale;
        }
    }
    else if (adjust == 1)
    {
        //Last update (2002/04/24) by WKLIN]
        // For Low Pass
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale > 1.0f)
        {
            scale *= 1.1f;
        }
    }
    else if (adjust == 2)
    {
        //Last update (2003/08/17) by WKLIN in Taipei
        //For including more high frequency details
        if (scale < 1.0f)
        {
            scale = 0.9f;
        }
        else if (scale > 1.0f)
        {
            scale *= 0.9f;
        }
    }
    else if (adjust == 3)
    {
        //Last update (2003/09/10) by WKLIN in Hsin-Chu
        //For excluding more high frequency details
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale >= 1.0f)
        {
            scale *= 1.3f;
        }
    }

    if (method == 10)
    {
        //Sinc
        for (i = 0; i <= (tapSize >> 1); i++)
        {
            WW      = 0.0f;
            point   = (MMP_INT)(taps >> 1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j]    = capSinc(((MMP_FLOAT)point + i / ((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW      += W[j];
            }

            //for (j=0; j< taps; j++)
            //  WeightMat[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps - 1] = 1.0;
            for (j = 0; j < taps - 1; j++)
            {
                weightMatrix[i][j]          = ((MMP_INT)(W[j] / WW * precision + 0.5)) / precision;
                weightMatrix[i][taps - 1]   -= weightMatrix[i][j];
            }
        }

        for (i = ((tapSize >> 1) + 1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize - i][taps - 1 - j];
            }
        }
    }
    else if (method == 11)
    {
        //rcos
        for (i = 0; i <= (tapSize >> 1); i++)
        {
            WW      = 0.0;
            point   = (MMP_INT) (taps >> 1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j]    = capRcos(((MMP_FLOAT)point + i / ((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW      += W[j];
            }

            //for (j=0; j< taps; j++)
            //  weightMatrix[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps - 1] = 1.0;
            for (j = 0; j < taps - 1; j++)
            {
                weightMatrix[i][j]          = ((MMP_INT)(W[j] / WW * precision + 0.5)) / precision;
                weightMatrix[i][taps - 1]   -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize >> 1) + 1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize - i][taps - 1 - j];
            }
        }
    }
    else if (method == 12)
    {
        // Catmull-Rom Cubic interpolation
        for (i = 0; i <= (tapSize >> 1); i++)
        {
            WW      = 0.0f;
            point   = (MMP_INT)(taps >> 1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j]    = capCubic01(((MMP_FLOAT)point + i / ((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW      += W[j];

                //cap_msg_ex(CAP_MSG_TYPE_ERR, "i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
                //  i, W[j], point, WW);
            }

            //Changed: 2004/02/24
            weightMatrix[i][taps - 1] = 1.0;
            for (j = 0; j < taps - 1; j++)
            {
                weightMatrix[i][j]          = ((MMP_INT)(W[j] / WW * precision + 0.5)) / precision;
                weightMatrix[i][taps - 1]   -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize >> 1) + 1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize - i][taps - 1 - j];
            }
        }
    }
    else
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Initialzie the memory space for temporally storing the caputred video data
 * in YUV format.
 */
//=============================================================================

//   CAP_MEM_BUF_PITCH
//     ----------
// 720 |   Y0   |
//     ----------
// 720 |   Y1   |
//     ----------
// 720 |   Y2   |
//     ----------
// 360 |   UV0  |
//     ----------
// 360 |   UV1  |
//     ----------
// 360 |   UV2  |
//     ----------

MMP_RESULT
_Cap_Memory_Initialize(
    MMP_UINT32 width, MMP_UINT32 height)
{
    MMP_RESULT  result  = MMP_SUCCESS;
    MMP_UINT32  startaddr;
    MMP_UINT32  i;
    MMP_UINT32  offset  = 0;
    MMP_UINT16  data;
    MMP_UINT32  Y_BUFFER_BASE = 0;
    MMP_UINT32  UV_BUFFER_BASE = height * 3;
    MMP_UINT32  size    = (height * 3 + (height >> 1) * 3)  * width;

    //ithPrintf("capture allocate szie = %x\n",size);
    if (CapMemBuffer[0])
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " Init Conflict %s (%d) error\n", __FUNCTION__, __LINE__);
        goto end;
    }
    /*alignment*/
    startaddr = (MMP_UINT32) itpVmemAlignedAlloc(32,size);

    if (startaddr == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " create memory pool fail %s (%d) error\n", __FUNCTION__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    /*remap memory for win32,freertos do nothing*/
    CapMemBuffer[0] = (MMP_UINT32)ithMapVram(startaddr, size, ITH_VRAM_WRITE);

    //ithPrintf("CapMemBuffer[0] addr = 0x%x\n", CapMemBuffer[0]);

    /*calculate  each buffer start address*/
    for (i = 0; i < CAPTURE_MEM_BUF_COUNT; i++)
    {
        if(i % 2 == 0)
        {
            offset    = Y_BUFFER_BASE * width;
            Y_BUFFER_BASE += height;
        }
        else
        {
            offset    = UV_BUFFER_BASE * width;
            UV_BUFFER_BASE += (height/2);
        }
        CapMemBuffer[i] = offset + CapMemBuffer[0];
        //ithPrintf("CapMemBuffer[%d] addr = 0x%x\n", i , CapMemBuffer[i]);
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Clear capture buffer
 */
//=============================================================================
MMP_RESULT
_Cap_Memory_Clear(
    void)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  i;
    MMP_UINT8   *pAddr;

    if (CapMemBuffer[0] == 0x0)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " clear memory fail %s (%d) error\n", __FUNCTION__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    itpVmemFree(CapMemBuffer[0]);
    
    for (i = 0; i < CAPTURE_MEM_BUF_COUNT; i++)
        CapMemBuffer[i] = 0;

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Update CAP device.
 */
//=============================================================================
MMP_RESULT
_Cap_Update_Reg(
    CAPTURE_HANDLE *cap_handle)
{
    MMP_RESULT  result      = MMP_SUCCESS;
    MMP_UINT32  index;
    CAP_CONTEXT *Capctxt    = &cap_handle->cap_info;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
    
     // Update Onfly or Memeory mode
    if (Capctxt->EnableOnflyMode == MMP_TRUE)
        ithCap_Set_ISP_HandShaking(cap_handle->cap_id, ONFLY_MODE, 0);
    else
    {
        ithCap_Set_ISP_HandShaking(cap_handle->cap_id, MEMORY_MODE, 0);
        ithCap_Set_Interrupt_Mode(cap_handle->cap_id, CAP_INT_MODE_ERR_FRAMEEND);
    }

    if (cap_handle->source_id == MMP_CAP_DEV_HDMIRX && ithCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
        _Cap_ColorMatrix(Capctxt, MMP_FALSE);   //BT601
    else if (Capctxt->ininfo.capwidth >= 1280)
        _Cap_ColorMatrix(Capctxt, MMP_TRUE);    //BT709
    else
        _Cap_ColorMatrix(Capctxt, MMP_FALSE);   //BT601

                                                // Update Capture Interrupt
    ithCap_Set_Enable_Interrupt(cap_handle->cap_id, Capctxt->EnableInterrupt);

    // Update Capture Write Buffer Address
    ithCap_Set_Buffer_addr_Reg(cap_handle->cap_id, CapMemBuffer, Capctxt->outinfo.OutAddrOffset);

    // Update Input Pin mux
    ithCap_Set_Input_Pin_Mux_Reg(cap_handle->cap_id, &Capctxt->inmux_info);

    // Update Output Pin mux
    ithCap_Set_Output_Pin_Mux_Reg(cap_handle->cap_id, &Capctxt->outpin_info);

    // Update IO Parameter
    ithCap_Set_IO_Mode_Reg(cap_handle->cap_id, &Capctxt->iomode_info);

    // Update Input Parameter
    ithCap_Set_Input_Data_Info_Reg(cap_handle->cap_id, Capctxt);

    // Update Hor Scale
    ithCap_Set_HorScale_Width_Reg(cap_handle->cap_id, &Capctxt->outinfo);

    ithCap_Set_Enable_Reg(cap_handle->cap_id, Capctxt);

    // Update RGB to YUV Matrix
    if (Capctxt->funen.EnCSFun)
        ithCap_Set_RGBtoYUVMatrix_Reg(cap_handle->cap_id, &Capctxt->RGBtoYUVFun);

    // Update Color Format
    ithCap_Set_Color_Format_Reg(cap_handle->cap_id, &Capctxt->YUVinfo);

    //
    //Color Correction Parameter
    //
    if (Capctxt->UpdateFlags & CAP_FLAGS_UPDATE_CCMatrix)
    {
        ithCap_Set_CCMatrix_Reg(cap_handle->cap_id, &Capctxt->CCFun);
        Capctxt->UpdateFlags &= (~CAP_FLAGS_UPDATE_CCMatrix);
    }

    // Update Scale Matrix
    Capctxt->ScaleFun.HCI = _Cap_ScaleFactor(Capctxt->ininfo.ROIWidth, Capctxt->outinfo.OutWidth);
    ithCAP_Set_ScaleParam_Reg(cap_handle->cap_id, &Capctxt->ScaleFun);

    for (index = 0; index < WEIGHT_NUM; index++)
    {
        if (Capctxt->ScaleFun.HCI >= ScaleRatio[index])
        {
            ithCap_Set_IntScaleMatrixH_Reg(cap_handle->cap_id, WeightMatInt[index]);
            break;
        }
        else if (index == WEIGHT_NUM - 1)
            ithCap_Set_IntScaleMatrixH_Reg(cap_handle->cap_id, WeightMatInt[index]);
    }

    // Update Error Handle mode
#ifdef CFG_SENSOR_ENABLE
    ithCap_Set_Error_Handleing(cap_handle->cap_id, 0x8064);
    ithCap_Set_Wait_Error_Reset(cap_handle->cap_id);
#else
    ithCap_Set_Error_Handleing(cap_handle->cap_id, 0xF7FF);
    ithCap_Set_Wait_Error_Reset(cap_handle->cap_id);
#endif

    // Update Skip Pattern
    ithCap_Set_Skip_Pattern_Reg(cap_handle->cap_id, Capctxt->Skippattern, Capctxt->SkipPeriod);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * CAP default value initialization.
 */
//=============================================================================
MMP_RESULT
_Cap_Initialize(CAPTURE_HANDLE *cap_handle)
{
    MMP_RESULT  result = MMP_SUCCESS;

    MMP_UINT16  i, j, index;
    MMP_FLOAT   WeightMat[CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];
    MMP_UINT16  data;
    CAP_CONTEXT *Capctxt = &cap_handle->cap_info;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    //clear flag
    Capctxt->UpdateFlags                = 0x0;

    /* RGB to YUV REG */
    Capctxt->RGBtoYUVFun._11            = 0x004D;
    Capctxt->RGBtoYUVFun._12            = 0x0096;
    Capctxt->RGBtoYUVFun._13            = 0x001D;
    Capctxt->RGBtoYUVFun._21            = 0x07AA;
    Capctxt->RGBtoYUVFun._22            = 0x07b6;
    Capctxt->RGBtoYUVFun._23            = 0x0083;
    Capctxt->RGBtoYUVFun._31            = 0x0083;
    Capctxt->RGBtoYUVFun._32            = 0x0793;
    Capctxt->RGBtoYUVFun._33            = 0x07ec;
    Capctxt->RGBtoYUVFun.ConstY         = 0x0000;
    Capctxt->RGBtoYUVFun.ConstU         = 0x0080;
    Capctxt->RGBtoYUVFun.ConstV         = 0x0080;

    //Capctxt->ininfo.PitchY              = CAP_MEM_BUF_PITCH;
    //Capctxt->ininfo.PitchUV             = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.ColorDepth          = COLOR_DEPTH_8_BITS;

    Capctxt->funen.EnDEMode             = MMP_FALSE;
    Capctxt->funen.EnCSFun              = MMP_FALSE;
    Capctxt->funen.EnInBT656            = MMP_FALSE;

    //Color Contrl
    Capctxt->ColorCtrl.brightness       = 0;
    Capctxt->ColorCtrl.contrast         = 1.0;
    Capctxt->ColorCtrl.hue              = 0;
    Capctxt->ColorCtrl.saturation       = 1.0;
    Capctxt->ColorCtrl.colorEffect[0]   = 0;
    Capctxt->ColorCtrl.colorEffect[1]   = 0;

#if defined (CAP_USE_COLOR_EFFECT)
    Capctxt->funen.EnCCFun              = MMP_TRUE;

    //Color correction matrix
    Capctxt->CCFun.OffsetR              = 0x0000;
    Capctxt->CCFun.OffsetG              = 0x0000;
    Capctxt->CCFun.OffsetB              = 0x0000;
    Capctxt->CCFun._11                  = 0x0100;
    Capctxt->CCFun._12                  = 0x0000;
    Capctxt->CCFun._13                  = 0x0000;
    Capctxt->CCFun._21                  = 0x0000;
    Capctxt->CCFun._22                  = 0x0100;
    Capctxt->CCFun._23                  = 0x0000;
    Capctxt->CCFun._31                  = 0x0000;
    Capctxt->CCFun._32                  = 0x0000;
    Capctxt->CCFun._33                  = 0x0100;
    Capctxt->CCFun.DeltaR               = 0x0000;
    Capctxt->CCFun.DeltaG               = 0x0000;
    Capctxt->CCFun.DeltaB               = 0x0000;

    Capctxt->UpdateFlags                |= CAP_FLAGS_UPDATE_CCMatrix;

#else
    Capctxt->funen.EnCCFun              = MMP_FALSE;
    Capctxt->UpdateFlags                &= (~CAP_FLAGS_UPDATE_CCMatrix);
#endif

    Capctxt->funen.EnUseExtDE           = MMP_FALSE;
    Capctxt->funen.EnUseExtVRst         = MMP_FALSE;
    Capctxt->funen.EnUseExtHRst         = MMP_FALSE;

    Capctxt->funen.EnNoHSyncForSensor   = MMP_FALSE;

    Capctxt->funen.EnProgressiveToField = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE        = MMP_FALSE;

    Capctxt->funen.EnYPbPrTopVSMode     = MMP_FALSE;
    Capctxt->funen.EnDlyVS              = MMP_FALSE;

    Capctxt->funen.EnPort1UV2LineDS     = MMP_TRUE;

    /* Scale */
    Capctxt->ScaleFun.HCI               = 0.0f;

    //Initial Scale Weight Matrix
    for (index = 0; index < WEIGHT_NUM; index++)
    {
        _Cap_CreateWeighting(ScaleRatio[index], CAP_SCALE_TAP, CAP_SCALE_TAP_SIZE, WeightMat);

        for (j = 0; j < CAP_SCALE_TAP_SIZE; j++)
            for (i = 0; i < CAP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT16)CAP_FLOATToFix(WeightMat[j][i], 1, 6);
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Fail\n", __FUNCTION__, __LINE__);

    return result;
}

void _Cap_SetInputSource(CAPTURE_HANDLE *cap_handle, MMP_CAP_DEVICE_ID inputsource)
{
    MMP_RESULT  result      = MMP_SUCCESS;
    MMP_UINT16  Value;
    CAP_CONTEXT *capinfo    = &cap_handle->cap_info;

    if (inputsource < 0 && inputsource > MMP_CAP_DEV_MAX)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
    if (IO_SETTING[inputsource] != NULL)
    {
        IO_SETTING[inputsource](capinfo);
    }
    else
        cap_msg_ex(CAP_MSG_TYPE_ERR, " IO_SETTING = NULL %s (%d) \n", __FUNCTION__, __LINE__);

#if 0
    switch (inputsource)
    {
    #ifdef COMPOSITE_DEV
    case MMP_CAP_DEV_ADV7180:
        _ADV7180_Setting(capinfo);
        break;
    #endif

    #ifdef COMPONENT_DEV
    case MMP_CAP_DEV_CAT9883:
        _CAT9883_Setting(capinfo);
        break;
    #endif

    #ifdef CFG_SENSOR_ENABLE
    case MMP_CAP_DEV_SENSOR:
        _SENSOR_Setting(capinfo);
        break;
    #else
    case MMP_CAP_DEV_HDMIRX:
        #if !defined (EXTERNAL_HDMIRX) && defined(HDMI_LOOPTHROUGH)
        _HDMIRX_LoopThrough_Setting(capinfo);
        #else
        _HDMIRX_Setting(capinfo);
        #endif
        break;
    #endif
    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
        break;
    }
#endif
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Setting Fail\n", __FUNCTION__, __LINE__);
}

//=============================================================================
/**
   // brightness:     -128 ~ 127     default : 0
   // contrast:       0.0 ~ 4.0      default : 1.0
   // hue:            0 ~ 359        default : 0
   // saturation:     0.0 ~ 4.0      default : 1.0
   // colorEffect[2]: -128 ~ 128     default : 0, 0

   // preOff:  S8
   // M:       S4.8
   // postOff: S8
 */
//=============================================================================
#if defined (CAP_USE_COLOR_EFFECT)
void
_Cap_SetColorCorrMatrix(
    CAP_COLOR_CORRECTION    *pColorCorrect,
    MMP_INT32               brightness,
    MMP_FLOAT               contrast,
    MMP_INT32               hue,
    MMP_FLOAT               saturation,
    MMP_INT32               colorEffect[2])
{
    MMP_INT32   preOff[3];
    MMP_INT32   M[3][3];
    MMP_INT32   postOff[3];
    MMP_FLOAT   cosTh, sinTh;

    preOff[0]               = preOff[1] = preOff[2] = 0;

    M[0][0]                 = (int)(contrast * 256 + 0.5);
    M[0][1]                 = M[0][2] = 0;
    capGetSinCos(hue, &sinTh, &cosTh);
    M[1][0]                 = 0;
    M[1][1]                 = (int)(saturation * cosTh * 256 + 0.5);
    M[1][2]                 = (int)(saturation * -sinTh * 256 + 0.5);
    M[2][0]                 = 0;
    M[2][1]                 = (int)(saturation * sinTh * 256 + 0.5);
    M[2][2]                 = (int)(saturation * cosTh * 256 + 0.5);

    postOff[0]              = (int)((contrast * brightness) + 128 * (1.0 - contrast));
    postOff[1]              = 0;
    postOff[2]              = 0;

    pColorCorrect->OffsetR  = preOff[0];
    pColorCorrect->OffsetG  = preOff[1];
    pColorCorrect->OffsetB  = preOff[2];
    pColorCorrect->_11      = M[0][0];
    pColorCorrect->_12      = M[0][1];
    pColorCorrect->_13      = M[0][2];
    pColorCorrect->_21      = M[1][0];
    pColorCorrect->_22      = M[1][1];
    pColorCorrect->_23      = M[1][2];
    pColorCorrect->_31      = M[2][0];
    pColorCorrect->_32      = M[2][1];
    pColorCorrect->_33      = M[2][2];
    pColorCorrect->DeltaR   = postOff[0];
    pColorCorrect->DeltaG   = postOff[1];
    pColorCorrect->DeltaB   = postOff[2];
}

//=============================================================================
/**
 * Update Capture color matrix device.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of Cap_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
_Cap_UpdateColorMatrix(
    CAPTURE_HANDLE *cap_handle)
{
    MMP_RESULT  result      = MMP_SUCCESS;
    CAP_CONTEXT *Capctxt    = &cap_handle->cap_info;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    //
    //Color Correction Parameter
    //
    if (Capctxt->UpdateFlags & CAP_FLAGS_UPDATE_CCMatrix)
    {
        ithCap_Set_CCMatrix_Reg(cap_handle->cap_id, &Capctxt->CCFun);
        Capctxt->UpdateFlags &= (~CAP_FLAGS_UPDATE_CCMatrix);
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}
#endif
