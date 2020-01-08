﻿//#include "host/host.h"
#include "math.h"
//#include "mem/mem.h"
//#include "ite/ith.h"

#include "capture/capture_types.h"
#include "capture_config.h"
#include "capture.h"
#include "capture_hw.h"
#include "capture_util.h"
#include "capture_reg.h"
#include "mmp_capture_it970.h"
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
static MMP_FLOAT   ScaleRatio[WEIGHT_NUM] = {(1088.0f/1080), (1920.0f/1920), (1280.0f/1920), (576.0f/1080), (480.0f/1080), (720.0f/1920), (640.0f/1920), (288.0f/1080), (352.0f/1920)};
static MMP_UINT32  WeightMatInt[WEIGHT_NUM][CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];

MMP_UINT32 CapMemBuffer[2][CAPTURE_MEM_BUF_COUNT] = { 0 };
#if 1 //for simulation
static MMP_UINT16 CapMemTable[CAPTURE_MEM_BUF_COUNT][2] = {
    {0,     0                                                         },//Y0
    {0,     CAP_INPUT_MAX_HEIGHT	 								  },//UV0
    {0,     CAP_INPUT_MAX_HEIGHT	 + (CAP_INPUT_MAX_HEIGHT >> 1)    },//Y1
    {0,     CAP_INPUT_MAX_HEIGHT * 2 + (CAP_INPUT_MAX_HEIGHT >> 1)	  },//UV1
    {0,     CAP_INPUT_MAX_HEIGHT * 3                                  },//Y2
    {0,     CAP_INPUT_MAX_HEIGHT * 4                                  } //UV2
};

#else
static MMP_UINT16 CapMemTable[CAPTURE_MEM_BUF_COUNT][2] = {
    {0,     0                                                         },//Y0
    {0,     CAP_INPUT_MAX_HEIGHT * 3                                  },//UV0
    {0,     CAP_INPUT_MAX_HEIGHT                                      },//Y1
    {0,     CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1)    },//UV1
    {0,     CAP_INPUT_MAX_HEIGHT * 2                                  },//Y2
    {0,     CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1) * 2} //UV2
};
#endif

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

//=============================================================================
//                Private Function Definition
//=============================================================================

#if defined (SENSOR_DEV)

	#include "io_sensor_pixelplus_po3100.c"
	#if 0
    #if defined(IT9919_144TQFP)
        #if defined (SENSOR_OMNIVISION_OV7725)
            #include "io_sensor_omnivision_ov7725.c"
        #elif defined (SENSOR_HIMAX_HM1375)
            #include "io_sensor_himax_hm1375.c"
        #elif defined (SENSOR_PIXELPLUS_PO3100)
            #include "io_sensor_pixelplus_po3100.c"
		#elif defined (SENSOR_NOVATEK_NT99141)
            #include "io_sensor_novatek_nt99141.c"	
		#elif defined (SENSOR_AR0130)
			#include "io_sensor_ar0130.c"
        #endif
    #else //IT9917_176TQFP
        #if defined(PIXELPLUS_PH1100K)
            #include "IT9917_176TQFP/io_sensor_pixelplus_ph1100k.c"
        #elif defined (APTINA_MT9M034)
            #include "IT9917_176TQFP/io_sensor_aptina_mt9m034.c"
        #endif
    #endif
	#endif

#else
	#include "io_video.c"
#endif

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
    return (MMP_FLOAT) (((MMP_INT) (16384.0f*Input/(MMP_FLOAT)Output))/16384.0f);
}

//=============================================================================
/**
* Select Color Matrix
*/
//=============================================================================
static void
_Cap_ColorMatrix(
    CAP_CONTEXT *Capctxt,MMP_BOOL isBT709)
{
    if (isBT709 == MMP_TRUE)
    {
        /* RGB to YUV REG */
        Capctxt->RGBtoYUVFun._11 = 0x0037;
        Capctxt->RGBtoYUVFun._12 = 0x00B7;
        Capctxt->RGBtoYUVFun._13 = 0x0012;
        Capctxt->RGBtoYUVFun._21 = 0x07e2;
        Capctxt->RGBtoYUVFun._22 = 0x079b;
        Capctxt->RGBtoYUVFun._23 = 0x0083;
        Capctxt->RGBtoYUVFun._31 = 0x0083;
        Capctxt->RGBtoYUVFun._32 = 0x0789;
        Capctxt->RGBtoYUVFun._33 = 0x07f4;
        Capctxt->RGBtoYUVFun.ConstY = 0x0000;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;
    }
    else
    {
        /* RGB to YUV REG */
        Capctxt->RGBtoYUVFun._11 = 0x004D;
        Capctxt->RGBtoYUVFun._12 = 0x0096;
        Capctxt->RGBtoYUVFun._13 = 0x001D;
        Capctxt->RGBtoYUVFun._21 = 0x07d4;
        Capctxt->RGBtoYUVFun._22 = 0x07a9;
        Capctxt->RGBtoYUVFun._23 = 0x0083;
        Capctxt->RGBtoYUVFun._31 = 0x0083;
        Capctxt->RGBtoYUVFun._32 = 0x0792;
        Capctxt->RGBtoYUVFun._33 = 0x07eb;
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
_CAP_CreateWeighting(
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
    MMP_FLOAT   precision = 64.0f;
    MMP_FLOAT   fscale = 1.0f;

    //Kevin:TODO temp solution
    adjust = 0;
    method = 11;
    sharp = 0;

    switch(sharp)
    {
        case 4:     fscale = 0.6f;  break;
        case 3:     fscale = 0.7f;  break;
        case 2:     fscale = 0.8f;  break;
        case 1:     fscale = 0.9f;  break;
        case 0:     fscale = 1.0f;  break;
        case -1:    fscale = 1.1f;  break;
        case -2:    fscale = 1.2f;  break;
        case -3:    fscale = 1.3f;  break;
        case -4:    fscale = 1.4f;  break;
        default:    fscale = 1.5f;  break;
    };

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
        for (i=0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = capSinc( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  WeightMat[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }

        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 11)
    {
        //rcos
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW =0.0;
            point = (MMP_INT) (taps>>1)-1;
            for (j=0; j < taps; j++)
            {
                W[j] = capRcos( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  weightMatrix[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }

        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 12)
    {
        // Catmull-Rom Cubic interpolation
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1)-1;
            for (j = 0; j < taps; j++)
            {
                W[j] = capCubic01( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];

                //cap_msg_ex(CAP_MSG_TYPE_ERR, "i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
                //  i, W[j], point, WW);
            }


            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
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
 * CAP memory Initialize.
 */
//=============================================================================
MMP_RESULT
Cap_Memory_Initialize(
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *pCapctxt = (CAP_CONTEXT *)ptDev;
    MMP_UINT32 startaddr;
    MMP_UINT32 i;
    MMP_UINT32 offset = 0;
    MMP_UINT16 data;
	
    MMP_UINT32 size = CAP_MEM_BUF_HEIGHT * CAP_MEM_BUF_PITCH + CAP_MEM_BUF_PITCH - 1;

    if (CapMemBuffer[pCapctxt->Cap_ID][0])
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " Init Conflict %s (%d) error\n", __FUNCTION__, __LINE__);
        goto end;
    }

	//Benson modify this.
	pCapctxt->video_vram_addr = itpVmemAlignedAlloc(64, size); //64 bit aligen
	pCapctxt->video_sys_addr = (uint8_t*) ithMapVram(pCapctxt->video_vram_addr, size, ITH_VRAM_WRITE);
	if (pCapctxt->video_vram_addr == MMP_NULL)
	{
		cap_msg_ex(CAP_MSG_TYPE_ERR, " create memory pool fail %s (%d) error\n", __FUNCTION__, __LINE__);
		result = MMP_RESULT_ERROR;
		goto end;
	}
	CapMemBuffer[pCapctxt->Cap_ID][0] = pCapctxt->video_sys_addr;
    printf("CapMemBuffer[0] addr = 0x%x\n", CapMemBuffer[pCapctxt->Cap_ID][0]);

    for (i = 1; i < CAPTURE_MEM_BUF_COUNT; i++)
    {
        offset = CapMemTable[i][0] + CapMemTable[i][1] * CAP_MEM_BUF_PITCH;
        CapMemBuffer[pCapctxt->Cap_ID][i] = offset + CapMemBuffer[pCapctxt->Cap_ID][0];
        printf("CapMemBuffer[%d] addr = 0x%x\n", i , CapMemBuffer[pCapctxt->Cap_ID][i]);
    }
	
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;

}
//=============================================================================
/**
 * CAP memory clear.
 */
//=============================================================================
MMP_RESULT
Cap_Memory_Clear(
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT32 i;
    MMP_UINT8 *pAddr;
	CAP_CONTEXT *pCapctxt = (CAP_CONTEXT *)ptDev;

    if (CapMemBuffer[pCapctxt->Cap_ID][0] == 0x0)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " clear memory fail %s (%d) error\n", __FUNCTION__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

	if (pCapctxt->video_sys_addr)
    {
        itpVmemFree(pCapctxt->video_vram_addr);
        pCapctxt->video_sys_addr  = NULL;
        pCapctxt->video_vram_addr = 0;
		CapMemBuffer[pCapctxt->Cap_ID][0] = 0;
    }

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
Cap_Update_Reg(
    CAP_CONTEXT *Capctxt)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32 index;

    if(Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    // Update Onfly or Memeory mode
    if (Capctxt->EnableOnflyMode == MMP_TRUE && Capctxt->EnableInterrupt == MMP_TRUE)
    {
        ithCap_Set_ISP_HandShaking(Capctxt->Cap_ID,MEMORY_WITH_ONFLY_MODE, &Capctxt->outinfo);
        ithCap_Set_Interrupt_Mode(Capctxt->Cap_ID,(CAP_INT_MODE_FRAME_END | CAP_INT_MODE_SYNC_ERR), MMP_TRUE);
    }	
    else if (Capctxt->EnableOnflyMode == MMP_TRUE)
    {
        ithCap_Set_ISP_HandShaking(Capctxt->Cap_ID,ONFLY_MODE, &Capctxt->outinfo);
		ithCap_Set_Interrupt_Mode(Capctxt->Cap_ID,CAP_INT_MODE_SYNC_ERR, MMP_TRUE);
    }
    else
    {
        ithCap_Set_ISP_HandShaking(Capctxt->Cap_ID,MEMORY_MODE, &Capctxt->outinfo);
        ithCap_Set_Interrupt_Mode(Capctxt->Cap_ID,(CAP_INT_MODE_FRAME_END | CAP_INT_MODE_SYNC_ERR), MMP_TRUE);
    }

	// Update Dither Setting
	if (Capctxt->Ditherinfo.EnDither == MMP_TRUE)
		ithCap_Set_Enable_Dither(Capctxt->Cap_ID, &Capctxt->Ditherinfo);

    if (mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX && mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
        _Cap_ColorMatrix(Capctxt,MMP_FALSE); //BT601
    else if (Capctxt->ininfo.capwidth >= 1280)
        _Cap_ColorMatrix(Capctxt,MMP_TRUE); //BT709
    else
       _Cap_ColorMatrix(Capctxt,MMP_FALSE); //BT601

    // Update Capture Write Buffer Address
    ithCap_Set_Buffer_addr_Reg(Capctxt->Cap_ID,CapMemBuffer[Capctxt->Cap_ID], Capctxt->outinfo.OutAddrOffset);

    // Update Input Pin mux
    ithCap_Set_Input_Pin_Mux_Reg(Capctxt->Cap_ID,&Capctxt->inmux_info);

    // Update Output Pin mux
    ithCap_Set_Output_Pin_Mux_Reg(Capctxt->Cap_ID,&Capctxt->outpin_info);

    // Update IO Parameter
    ithCap_Set_IO_Mode_Reg(Capctxt->Cap_ID,&Capctxt->iomode_info);

    // Update Input Parameter
    ithCap_Set_Input_Data_Info_Reg(Capctxt->Cap_ID,&Capctxt->ininfo);

    // Update Hor Scale
    ithCap_Set_HorScale_Width_Reg(Capctxt->Cap_ID,&Capctxt->outinfo);

    ithCap_Set_Enable_Reg(Capctxt->Cap_ID,&Capctxt->funen);

    // Update RGB to YUV Matrix
    if (Capctxt->funen.EnCSFun)
        ithCap_Set_RGBtoYUVMatrix_Reg(Capctxt->Cap_ID,&Capctxt->RGBtoYUVFun);

    // Update Color Format
    ithCap_Set_Color_Format_Reg(Capctxt->Cap_ID,&Capctxt->YUVinfo);

    //
    //Color Correction Parameter
    //
    if(Capctxt->UpdateFlags & CAP_FLAGS_UPDATE_CCMatrix)
    {
        ithCap_Set_CCMatrix_Reg(Capctxt->Cap_ID,&Capctxt->CCFun);
        Capctxt->UpdateFlags &= (~CAP_FLAGS_UPDATE_CCMatrix);
    }

    // Update Scale Matrix
    Capctxt->ScaleFun.HCI = _Cap_ScaleFactor(Capctxt->ininfo.ROIWidth, Capctxt->outinfo.OutWidth);
    ithCap_Set_ScaleParam_Reg(Capctxt->Cap_ID,&Capctxt->ScaleFun);

    for(index = 0; index < WEIGHT_NUM; index++)
    {
        if(Capctxt->ScaleFun.HCI >= ScaleRatio[index])
        {
            ithCap_Set_IntScaleMatrixH_Reg(Capctxt->Cap_ID,WeightMatInt[index]);
            break;
        }
        else if(index == WEIGHT_NUM - 1)
            ithCap_Set_IntScaleMatrixH_Reg(Capctxt->Cap_ID,WeightMatInt[index]);
    }

    // Update Error Handle mode
#ifdef SENSOR_DEV
    ithCap_Set_Error_Handleing(Capctxt->Cap_ID,0xFFFF); //1080i/720p  turn off  error handling.
    ithCap_Set_Wait_Error_Reset(Capctxt->Cap_ID);
#else
    ithCap_Set_Error_Handleing(Capctxt->Cap_ID,0xFFFF);
    ithCap_Set_Wait_Error_Reset(Capctxt->Cap_ID);
#endif

    // Update Skip Pattern
    ithCap_Set_Skip_Pattern_Reg(Capctxt->Cap_ID,Capctxt->Skippattern, Capctxt->SkipPeriod);

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
Cap_SetSkipMode(
    CAP_CONTEXT *Capctxt,CAP_SKIP_MODE mode)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  SkipPeriod;

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
    	Capctxt->Skippattern = 0x001F;
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

    // Update Skip Pattern
    ithCap_Set_Skip_Pattern_Reg(Capctxt->Cap_ID,Capctxt->Skippattern, Capctxt->SkipPeriod);

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
Cap_Initialize(
    CAP_CONTEXT *Capctxt)
{
    MMP_RESULT result = MMP_SUCCESS;

    MMP_UINT32  i, j, index;
    MMP_FLOAT   WeightMat[CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];
    MMP_UINT32 data;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    //clear flag
    Capctxt->UpdateFlags = 0x0;

    /* RGB to YUV REG */
    Capctxt->RGBtoYUVFun._11 = 0x004D;
    Capctxt->RGBtoYUVFun._12 = 0x0096;
    Capctxt->RGBtoYUVFun._13 = 0x001D;
    Capctxt->RGBtoYUVFun._21 = 0x07AA;
    Capctxt->RGBtoYUVFun._22 = 0x07b6;
    Capctxt->RGBtoYUVFun._23 = 0x0083;
    Capctxt->RGBtoYUVFun._31 = 0x0083;
    Capctxt->RGBtoYUVFun._32 = 0x0793;
    Capctxt->RGBtoYUVFun._33 = 0x07ec;
    Capctxt->RGBtoYUVFun.ConstY = 0x0000;
    Capctxt->RGBtoYUVFun.ConstU = 0x0080;
    Capctxt->RGBtoYUVFun.ConstV = 0x0080;

    Capctxt->ininfo.PitchY = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.PitchUV = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS;
	Capctxt->ininfo.NV12Format = UV;

    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnCSFun = MMP_FALSE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;
	Capctxt->Ditherinfo.EnDither = MMP_FALSE;

    //Color Contrl
    Capctxt->ColorCtrl.brightness = 0;
    Capctxt->ColorCtrl.contrast = 1.0;
    Capctxt->ColorCtrl.hue = 0;
    Capctxt->ColorCtrl.saturation = 1.0;
    Capctxt->ColorCtrl.colorEffect[0] = 0;
    Capctxt->ColorCtrl.colorEffect[1] = 0;

#if defined (CAP_USE_COLOR_EFFECT) 
    Capctxt->funen.EnCCFun = MMP_TRUE;
    //Color correction matrix
    Capctxt->CCFun.OffsetR = 0x0000;
    Capctxt->CCFun.OffsetG = 0x0000;
    Capctxt->CCFun.OffsetB = 0x0000;
    Capctxt->CCFun._11 = 0x0100;
    Capctxt->CCFun._12 = 0x0000;
    Capctxt->CCFun._13 = 0x0000;
    Capctxt->CCFun._21 = 0x0000;
    Capctxt->CCFun._22 = 0x0100;
    Capctxt->CCFun._23 = 0x0000;
    Capctxt->CCFun._31 = 0x0000;
    Capctxt->CCFun._32 = 0x0000;
    Capctxt->CCFun._33 = 0x0100;
    Capctxt->CCFun.DeltaR = 0x0000;
    Capctxt->CCFun.DeltaG = 0x0000;
    Capctxt->CCFun.DeltaB = 0x0000;

    Capctxt->UpdateFlags |= CAP_FLAGS_UPDATE_CCMatrix;

#else
    Capctxt->funen.EnCCFun = MMP_FALSE;
    Capctxt->UpdateFlags &= (~CAP_FLAGS_UPDATE_CCMatrix);
#endif

	Capctxt->funen.EnHSync = MMP_FALSE;
	Capctxt->funen.EnPort1UV2LineDS = MMP_TRUE; // ask H.C.
	Capctxt->funen.EnAutoDetHSPol = MMP_FALSE;
	Capctxt->funen.EnAutoDetVSPol = MMP_FALSE;
	Capctxt->funen.EnDumpMode = MMP_FALSE;
	Capctxt->funen.EnMemContinousDump = MMP_FALSE;
	Capctxt->funen.EnSramNap = MMP_FALSE;
	Capctxt->funen.EnMemLimit = MMP_FALSE;
	Capctxt->ininfo.CheckHS = MMP_FALSE;
	Capctxt->ininfo.CheckDE = MMP_FALSE;
	Capctxt->ininfo.CheckVS = MMP_TRUE; //always turn on.

    /* Scale */
    Capctxt->ScaleFun.HCI = 0.0f;

    //Initial Scale Weight Matrix
    for(index = 0; index < WEIGHT_NUM; index++)
    {
        _CAP_CreateWeighting(ScaleRatio[index], CAP_SCALE_TAP, CAP_SCALE_TAP_SIZE, WeightMat);

          for(j = 0; j < CAP_SCALE_TAP_SIZE; j++)
              for(i = 0; i < CAP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT16)CAP_FLOATToFix(WeightMat[j][i], 1, 6);
    }
	
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Capture Initialize Fail\n", __FUNCTION__, __LINE__);

    return result;
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
Cap_SetColorCorrMatrix(
    CAP_COLOR_CORRECTION  *pColorCorrect,
    MMP_INT32 brightness,
    MMP_FLOAT contrast,
    MMP_INT32 hue,
    MMP_FLOAT saturation,
    MMP_INT32 colorEffect[2])
{
    MMP_INT32 preOff[3];
    MMP_INT32 M[3][3];
    MMP_INT32 postOff[3];
    MMP_FLOAT cosTh, sinTh;

    preOff[0] = preOff[1] = preOff[2] = 0;

    M[0][0] = (int)(contrast * 256 + 0.5);
    M[0][1] = M[0][2] = 0;
    capGetSinCos(hue, &sinTh, &cosTh);
    M[1][0] = 0;
    M[1][1] = (int)(saturation * cosTh * 256 + 0.5);
    M[1][2] = (int)(saturation * -sinTh * 256 + 0.5);
    M[2][0] = 0;
    M[2][1] = (int)(saturation * sinTh * 256 + 0.5);
    M[2][2] = (int)(saturation * cosTh * 256 + 0.5);

    postOff[0] = (int)((contrast * brightness) + 128 * (1.0 - contrast));
    postOff[1] = 0;
    postOff[2] = 0;

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
Cap_UpdateColorMatrix(
    CAP_CONTEXT *Capctxt)
{
    MMP_RESULT  result = MMP_SUCCESS;

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
        ithCap_SetCCMatrix_Reg(Capctxt->Cap_ID,&Capctxt->CCFun);
        Capctxt->UpdateFlags &= (~CAP_FLAGS_UPDATE_CCMatrix);
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}
#endif

