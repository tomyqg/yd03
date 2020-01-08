#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "capture/capture_9850/ite_capture.h"
#include "capture_hw.h"
//=============================================================================
//                              Constant Definition
//=============================================================================

MMP_UINT32          CapMemBuffer[CAPTURE_MEM_BUF_COUNT] = { 0 };

#ifdef TRIPLE_BUFFER
static MMP_UINT16   CapMemTable[CAPTURE_MEM_BUF_COUNT][2] = {
    {0,     0                                                    }, //Y0
    {0, CAP_MEM_BUF_PITCH * 3                                    }, //U0
    {0, CAP_MEM_BUF_PITCH * 3 + (CAP_MEM_BUF_PITCH >> 1) * 3     }, //V0
    {0, CAP_MEM_BUF_PITCH                                        }, //Y1
    {0, CAP_MEM_BUF_PITCH * 3 + (CAP_MEM_BUF_PITCH >> 1)         }, //U1
    {0, CAP_MEM_BUF_PITCH * 3 + (CAP_MEM_BUF_PITCH >> 1) * 4     }, //V1
    {0, CAP_MEM_BUF_PITCH * 2                                    }, //Y2
    {0, CAP_MEM_BUF_PITCH * 3 + (CAP_MEM_BUF_PITCH >> 1) * 2     }, //U2
    {0, CAP_MEM_BUF_PITCH * 3 + (CAP_MEM_BUF_PITCH >> 1) * 5     }, //V2
};
#else
static MMP_UINT16   CapMemTable[CAPTURE_MEM_BUF_COUNT][2] = {
    {0,     0                                                    }, //Y0
    {0, CAP_MEM_BUF_PITCH * 2                                    }, //U0
    {0, CAP_MEM_BUF_PITCH * 2 + (CAP_MEM_BUF_PITCH >> 1) * 2     }, //V0
    {0, CAP_MEM_BUF_PITCH                                        }, //Y1
    {0, CAP_MEM_BUF_PITCH * 2 + (CAP_MEM_BUF_PITCH >> 1)         }, //U1
    {0, CAP_MEM_BUF_PITCH * 2 + (CAP_MEM_BUF_PITCH >> 1) * 3     }, //V1
};
//   CAP_MEM_BUF_PITCH
//     ----------
// 720 |   Y0   |
//     ----------
// 720 |   Y1   |
//     ----------
// 720 |   Y2   |
//     ----------
// 360 |   U0  |
//     ----------
// 360 |   U1  |
//     ----------
// 360 |   U2  |
//     ----------
// 360 |   V0  |
//     ----------
// 360 |   V1  |
//     ----------
// 360 |   V2  |
//     ----------
#endif
//end of capture.c
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
CAP_CONTEXT         *Capctxt            = MMP_NULL;
static MMP_BOOL     gtDeviceReboot      = MMP_TRUE;
static MMP_UINT32   gvideo_vram_addr    = 0;
static MMP_UINT8    *gvideo_sys_addr    = NULL;

//=============================================================================
//                Private Function Definition
//=============================================================================
static CaptureModuleDriver CaptureModuleDrivers = NULL;

///capture.c
//=============================================================================
/**
 * Update CAP device.
 */
//=============================================================================
MMP_RESULT
_Cap_Update_Reg(
    void)
{
    MMP_RESULT  result          = MMP_SUCCESS;
    MMP_BOOL    WriteBufferTest = MMP_FALSE;
    MMP_BOOL    TripleBuffer    = MMP_FALSE;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

#ifdef TRIPLE_BUFFER
    TripleBuffer = MMP_TRUE;
#endif

    // Update Capture Write Buffer Address
    ithCap_Set_Output_Buf_Addr(CapMemBuffer);

    // Update Input Parameter
    ithCap_Set_Input_Data_Info_Reg(&Capctxt->ininfo);

    ithCap_Set_Input_Protocol(Capctxt->input_protocol);

    ithCap_Set_Use_Triple_Buf(TripleBuffer);

    ithCap_Set_Interleave(Capctxt->Interleave);

    ithCap_Set_Use_Data_Enable_Mode(Capctxt->EnDEMode);

    // Update Color Format
    ithCap_Set_Color_Format_Reg(Capctxt->YUV422Format);

    CaptureControllerHardware_SetInputSignalSource(&Capctxt->input_video_source);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

///ite9070_capture.c
//=============================================================================
/**
 * Cap terminate.
 */
//=============================================================================
MMP_RESULT
mmpCapTerminate(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        return (MMP_RESULT)result;
    }

    //
    // Disable Cap engine
    //
    ithCap_Set_Reset();

    memset((void *)Capctxt, 0, sizeof(CAP_CONTEXT));

    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * mmpCapResetEngine
 */
//=============================================================================
MMP_RESULT
mmpCapResetEngine(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    ithCap_Set_UnFire();
    ithCap_Set_Reset();

    return result;
}

//=============================================================================
/**
 * mmpCapParameterSetting
 */
//=============================================================================
MMP_RESULT
mmpCapParameterSetting(void)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    ithModule_ForCaptureDriverSetting(CaptureModuleDrivers, Capctxt);
    //Update parameter
    result = _Cap_Update_Reg();

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);

    return result;
}

//=============================================================================
/**
 * mmpUnCapFire
 */
//=============================================================================
void
mmpUnCapFire(
    void)
{
    ithCap_Set_UnFire();
}

//=============================================================================
/**
 * mmpCapFire
 */
//=============================================================================
void
mmpCapFire(
    void)
{
    ithCap_Set_Fire();
}

//=============================================================================
/**
 * mmpDumpReg
 */
//=============================================================================
void
mmpDumpReg(
    void)
{
    ithCap_Dump_Reg();
}

//=============================================================================
/**
 * mmpCapMemoryClear
 */
//=============================================================================

//=============================================================================
/**
 * Capture Power Up.
 */
//=============================================================================
void
mmpCapPowerUp(
    void)
{
    ithCap_Set_EnableClock();
}

//=============================================================================
/**
 * Capture Power Down.
 */
//=============================================================================
void
mmpCapPowerDown(
    void)
{
    ithCap_Set_UnFire();
    ithCap_Set_DisableClock();
}

//=============================================================================
/**
 * Cap Device initialization.
 */
//=============================================================================
MMP_RESULT
mmpCapDeviceInitialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (gtDeviceReboot)
    {
        ithModule_Init(CaptureModuleDrivers, 0);
        ithModule_OutputPinTriState(  CaptureModuleDrivers,   MMP_TRUE);
        ithModule_OutputPinTriState(  CaptureModuleDrivers,   MMP_FALSE);
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
mmpCapDeviceTerminate(
    void)
{
    ithCap_Set_UnFire();

    if (gtDeviceReboot)
    {
        //Tri-State All Device
        ithModule_OutputPinTriState(CaptureModuleDrivers, MMP_TRUE);

        //Benson
        ithModule_DeInit(CaptureModuleDrivers);
        ithModule_Destroy(CaptureModuleDrivers);
        CaptureModuleDrivers = NULL;
    }
}

//=============================================================================
/**
 * mmpCapGetDeviceInfo
 */
//=============================================================================
void
mmpCapGetDeviceInfo()
{
    MMP_UINT16          rate;
    MMP_BOOL            matchRes = MMP_FALSE;

    rate                        = ithModule_GetProperty(CaptureModuleDrivers,Rate);
    matchRes                    = ithModule_GetProperty(CaptureModuleDrivers,matchResolution);

                                                                                        // Set Active Region
    Capctxt->ininfo.capwidth    = ithModule_GetProperty(CaptureModuleDrivers,GetWidth); // this two parameters
    Capctxt->ininfo.capheight   = ithModule_GetProperty(CaptureModuleDrivers,GetHeight);// can change to AutoDetected.

                                                            
    Capctxt->ininfo.HSyncPol    = ithModule_GetProperty(CaptureModuleDrivers,HPolarity);// Set Polarity
    Capctxt->ininfo.VSyncPol    = ithModule_GetProperty(CaptureModuleDrivers,VPolarity);
    Capctxt->ininfo.TopFieldPol = ithModule_GetProperty(CaptureModuleDrivers,GetTopFieldPolarity);// fingerprint will set it false.

    Capctxt->ininfo.PitchY      = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.PitchUV     = CAP_MEM_BUF_PITCH >> 1;

    // Set Interlace or Prograssive
    Capctxt->Interleave         = ithModule_GetProperty(CaptureModuleDrivers,GetModuleIsInterlace);

    Capctxt->ininfo.HNum1       = ithModule_GetProperty(CaptureModuleDrivers,HStar);
    Capctxt->ininfo.HNum2       = ithModule_GetProperty(CaptureModuleDrivers,HEnd);
    Capctxt->ininfo.LineNum1    = ithModule_GetProperty(CaptureModuleDrivers,VStar1);
    Capctxt->ininfo.LineNum2    = ithModule_GetProperty(CaptureModuleDrivers,VEnd1);
    Capctxt->ininfo.LineNum3    = ithModule_GetProperty(CaptureModuleDrivers,VStar2);
    Capctxt->ininfo.LineNum4    = ithModule_GetProperty(CaptureModuleDrivers,VEnd2);

    if (matchRes != MMP_TRUE)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "No Support Resolution! %dx%d @%dHz\n", ithModule_GetProperty(CaptureModuleDrivers,GetWidth),  ithModule_GetProperty(CaptureModuleDrivers,GetHeight), (rate / 100));

    // Benson
    Capctxt->OutAddrY[0]        = CapMemBuffer[CAP_MEM_Y0];
    Capctxt->OutAddrU[0]        = CapMemBuffer[CAP_MEM_U0];
    Capctxt->OutAddrV[0]        = CapMemBuffer[CAP_MEM_V0];
    Capctxt->OutAddrY[1]        = CapMemBuffer[CAP_MEM_Y1];
    Capctxt->OutAddrU[1]        = CapMemBuffer[CAP_MEM_U1];
    Capctxt->OutAddrV[1]        = CapMemBuffer[CAP_MEM_V1];
#ifdef TRIPLE_BUFFER
    Capctxt->OutAddrY[2]        = CapMemBuffer[CAP_MEM_Y2];
    Capctxt->OutAddrU[2]        = CapMemBuffer[CAP_MEM_U2];
    Capctxt->OutAddrV[2]        = CapMemBuffer[CAP_MEM_V2];
#endif

    Capctxt->bMatchResolution   = matchRes;
    if (matchRes == MMP_FALSE)
        printf("---Resolutin not Suppott !---\n");
}

///end of ite9070_capture.c
//capture.c
//=============================================================================
/**
 * CAP memory Initialize.
 */
//=============================================================================
int
mmpCap_Memory_Initialize()
{
    MMP_RESULT  result  = MMP_SUCCESS;
    uint32_t    i;
    uint32_t    offset  = 0;
    uint32_t    size    = CAP_MEM_BUF_HEIGHT * CAP_INPUT_MAX_HEIGHT;

    if (CapMemBuffer[0])
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " Init Conflict %s (%d) error\n", __FUNCTION__, __LINE__);
        goto end;
    }

    gvideo_vram_addr    = itpVmemAlignedAlloc(64, size); //64 bit aligen
    gvideo_sys_addr     = (uint8_t *) ithMapVram(gvideo_vram_addr, size, /*ITH_VRAM_READ |*/ ITH_VRAM_WRITE);

    if (gvideo_vram_addr == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " create memory pool fail %s (%d) error\n", __FUNCTION__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    CapMemBuffer[0] = gvideo_sys_addr;
    for (i = 1; i < CAPTURE_MEM_BUF_COUNT; i++)
    {
        offset          = CapMemTable[i][0] + CapMemTable[i][1] * CAP_INPUT_MAX_HEIGHT;
        CapMemBuffer[i] = offset + CapMemBuffer[0];
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
mmpCap_Memory_Clear(
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

    if (gvideo_sys_addr)
    {
        itpVmemFree(gvideo_vram_addr);
        gvideo_sys_addr     = NULL;
        gvideo_vram_addr    = 0;
        CapMemBuffer[0]     = 0;
    }
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
mmpCap_Initialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        Capctxt = malloc(sizeof(CAP_CONTEXT));
        if (Capctxt == MMP_NULL)
        {
            result = MMP_RESULT_ERROR;
            cap_msg_ex(CAP_MSG_TYPE_ERR, "Allocate memeory fail\n");
            goto end;
        }
    }
    ithCap_Set_Reset();

    memset((void *)Capctxt, 0, sizeof(CAP_CONTEXT));

    Capctxt->ininfo.PitchY              = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.PitchUV             = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.TopFieldPol         = MMP_FALSE;

    Capctxt->EnDEMode                   = MMP_FALSE;
    Capctxt->input_protocol             = BT_601;

    Capctxt->input_video_source.LCDSRC  = MMP_FALSE;
    Capctxt->input_video_source.HSYNCDE = MMP_FALSE;
    Capctxt->input_video_source.DEPOL   = MMP_FALSE;

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Capture Initialize Fail\n", __FUNCTION__, __LINE__);

    return result;
}

//=============================================================================
/**
 * Cap Device Signal State.
 */
//=============================================================================
int ithCapControllerSetBT601(uint16_t capwidth, uint16_t capheight, CAP_INPUT_DATA_FORMAT input_dFmt)
{
    uint32_t result = 0;
    printf("%s\n", __FUNCTION__);

    Capctxt->EnDEMode                   = MMP_TRUE;
    Capctxt->input_protocol             = BT_601;

    Capctxt->input_video_source.HSYNCDE = MMP_FALSE;
    Capctxt->input_video_source.LCDSRC  = MMP_FALSE;
    Capctxt->input_video_source.DEPOL   = MMP_FALSE;

                                                        // Set Active Region
    Capctxt->ininfo.capwidth            = capwidth;     // this two parameters
    Capctxt->ininfo.capheight           = capheight;    // can change to AutoDetected.

                                                        // Set Polarity
    Capctxt->ininfo.HSyncPol            = 0;
    Capctxt->ininfo.VSyncPol            = 0;
    Capctxt->ininfo.TopFieldPol         = 0; // fingerprint will set it false.

    Capctxt->ininfo.PitchY              = capwidth;
    Capctxt->ininfo.PitchUV             = capwidth >> 1;

    // Set Interlace or Prograssive
    Capctxt->Interleave                 = Progressive;

    Capctxt->ininfo.HNum1               = 0;
    Capctxt->ininfo.HNum2               = 0;
    Capctxt->ininfo.LineNum1            = 0;
    Capctxt->ininfo.LineNum2            = 0;
    Capctxt->ininfo.LineNum3            = 0;
    Capctxt->ininfo.LineNum4            = 0;

    Capctxt->YUV422Format               = input_dFmt;

    Capctxt->OutAddrY[0]                = CapMemBuffer[CAP_MEM_Y0];
    Capctxt->OutAddrU[0]                = CapMemBuffer[CAP_MEM_U0];
    Capctxt->OutAddrV[0]                = CapMemBuffer[CAP_MEM_V0];
    Capctxt->OutAddrY[1]                = CapMemBuffer[CAP_MEM_Y1];
    Capctxt->OutAddrU[1]                = CapMemBuffer[CAP_MEM_U1];
    Capctxt->OutAddrV[1]                = CapMemBuffer[CAP_MEM_V1];
#ifdef TRIPLE_BUFFER
    Capctxt->OutAddrY[2]                = CapMemBuffer[CAP_MEM_Y2];
    Capctxt->OutAddrU[2]                = CapMemBuffer[CAP_MEM_U2];
    Capctxt->OutAddrV[2]                = CapMemBuffer[CAP_MEM_V2];
#endif

    result                              = _Cap_Update_Reg();

    //Capture Fire
    mmpCapFire();
}

int ithCapControllerSetBT601Href(uint16_t capwidth, uint16_t capheight, CAP_INPUT_DATA_FORMAT input_dFmt)
{
    uint32_t result = 0;
    printf("%s\n", __FUNCTION__);

    Capctxt->EnDEMode                   = MMP_TRUE;
    Capctxt->input_protocol             = BT_601;

    Capctxt->input_video_source.HSYNCDE = MMP_TRUE;
    Capctxt->input_video_source.LCDSRC  = MMP_FALSE;
    Capctxt->input_video_source.DEPOL   = MMP_FALSE;

                                                        // Set Active Region
    Capctxt->ininfo.capwidth            = capwidth;     // this two parameters
    Capctxt->ininfo.capheight           = capheight;    // can change to AutoDetected.

                                                        // Set Polarity
    Capctxt->ininfo.HSyncPol            = 0;
    Capctxt->ininfo.VSyncPol            = 0;
    Capctxt->ininfo.TopFieldPol         = 0; // fingerprint will set it false.

    Capctxt->ininfo.PitchY              = capwidth;
    Capctxt->ininfo.PitchUV             = capwidth >> 1;

    // Set Interlace or Prograssive
    Capctxt->Interleave                 = Progressive;

    Capctxt->ininfo.HNum1               = 0;
    Capctxt->ininfo.HNum2               = 0;
    Capctxt->ininfo.LineNum1            = 0;
    Capctxt->ininfo.LineNum2            = 0;
    Capctxt->ininfo.LineNum3            = 0;
    Capctxt->ininfo.LineNum4            = 0;

    Capctxt->YUV422Format               = input_dFmt;

    Capctxt->OutAddrY[0]                = CapMemBuffer[CAP_MEM_Y0];
    Capctxt->OutAddrU[0]                = CapMemBuffer[CAP_MEM_U0];
    Capctxt->OutAddrV[0]                = CapMemBuffer[CAP_MEM_V0];
    Capctxt->OutAddrY[1]                = CapMemBuffer[CAP_MEM_Y1];
    Capctxt->OutAddrU[1]                = CapMemBuffer[CAP_MEM_U1];
    Capctxt->OutAddrV[1]                = CapMemBuffer[CAP_MEM_V1];
#ifdef TRIPLE_BUFFER
    Capctxt->OutAddrY[2]                = CapMemBuffer[CAP_MEM_Y2];
    Capctxt->OutAddrU[2]                = CapMemBuffer[CAP_MEM_U2];
    Capctxt->OutAddrV[2]                = CapMemBuffer[CAP_MEM_V2];
#endif

    result                              = _Cap_Update_Reg();

    //Capture Fire
    mmpCapFire();
}

int ithCapControllerSetBT601WithoutDE(uint16_t capwidth, uint16_t capheight, CAP_INPUT_DATA_FORMAT input_dFmt, uint16_t HS_AreaStart, uint16_t HS_AreaEnd, uint16_t VS_TopAreaStart, uint16_t VS_TopAreaEnd, uint16_t VS_BtmAreaStart, uint16_t VS_BtmAreaEnd)
{
    uint32_t result = 0;
    printf("%s\n", __FUNCTION__);

    Capctxt->EnDEMode                   = MMP_FALSE;
    Capctxt->input_protocol             = BT_601;

    Capctxt->input_video_source.HSYNCDE = MMP_FALSE;
    Capctxt->input_video_source.LCDSRC  = MMP_FALSE;
    Capctxt->input_video_source.DEPOL   = MMP_FALSE;

                                                        // Set Active Region
    Capctxt->ininfo.capwidth            = capwidth;     // this two parameters
    Capctxt->ininfo.capheight           = capheight;    // can change to AutoDetected.

                                                        // Set Polarity
    Capctxt->ininfo.HSyncPol            = 0;
    Capctxt->ininfo.VSyncPol            = 0;
    Capctxt->ininfo.TopFieldPol         = 0; // fingerprint will set it false.

    Capctxt->ininfo.PitchY              = capwidth;
    Capctxt->ininfo.PitchUV             = capwidth >> 1;

    // Set Interlace or Prograssive
    Capctxt->Interleave                 = Progressive;

    Capctxt->ininfo.HNum1               = HS_AreaStart;
    Capctxt->ininfo.HNum2               = HS_AreaEnd;
    Capctxt->ininfo.LineNum1            = VS_TopAreaStart;
    Capctxt->ininfo.LineNum2            = VS_TopAreaEnd;
    Capctxt->ininfo.LineNum3            = VS_BtmAreaStart;
    Capctxt->ininfo.LineNum4            = VS_BtmAreaEnd;

    Capctxt->YUV422Format               = input_dFmt;

    Capctxt->OutAddrY[0]                = CapMemBuffer[CAP_MEM_Y0];
    Capctxt->OutAddrU[0]                = CapMemBuffer[CAP_MEM_U0];
    Capctxt->OutAddrV[0]                = CapMemBuffer[CAP_MEM_V0];
    Capctxt->OutAddrY[1]                = CapMemBuffer[CAP_MEM_Y1];
    Capctxt->OutAddrU[1]                = CapMemBuffer[CAP_MEM_U1];
    Capctxt->OutAddrV[1]                = CapMemBuffer[CAP_MEM_V1];
#ifdef TRIPLE_BUFFER
    Capctxt->OutAddrY[2]                = CapMemBuffer[CAP_MEM_Y2];
    Capctxt->OutAddrU[2]                = CapMemBuffer[CAP_MEM_U2];
    Capctxt->OutAddrV[2]                = CapMemBuffer[CAP_MEM_V2];
#endif

    result                              = _Cap_Update_Reg();

    //Capture Fire
    mmpCapFire();
}

int ithCapControllerSetBT656(uint16_t capwidth, uint16_t capheight, CAP_INPUT_DATA_FORMAT input_dFmt)
{
    uint32_t result = 0;
    printf("%s\n", __FUNCTION__);

    Capctxt->EnDEMode                   = MMP_FALSE;
    Capctxt->input_protocol             = BT_656;

    Capctxt->input_video_source.LCDSRC  = MMP_FALSE;
    Capctxt->input_video_source.HSYNCDE = MMP_FALSE;
    Capctxt->input_video_source.DEPOL   = MMP_FALSE;

                                                        // Set Active Region
    Capctxt->ininfo.capwidth            = capwidth;     // this two parameters
    Capctxt->ininfo.capheight           = capheight;    // can change to AutoDetected.

                                                        // Set Polarity
    Capctxt->ininfo.HSyncPol            = 0;
    Capctxt->ininfo.VSyncPol            = 0;
    Capctxt->ininfo.TopFieldPol         = 0; // fingerprint will set it false.

    Capctxt->ininfo.PitchY              = capwidth;
    Capctxt->ininfo.PitchUV             = capwidth >> 1;

    // Set Interlace or Prograssive
    Capctxt->Interleave                 = Progressive;

    Capctxt->ininfo.HNum1               = 0;
    Capctxt->ininfo.HNum2               = 0;
    Capctxt->ininfo.LineNum1            = 0;
    Capctxt->ininfo.LineNum2            = 0;
    Capctxt->ininfo.LineNum3            = 0;
    Capctxt->ininfo.LineNum4            = 0;

    Capctxt->YUV422Format               = input_dFmt;

    Capctxt->OutAddrY[0]                = CapMemBuffer[CAP_MEM_Y0];
    Capctxt->OutAddrU[0]                = CapMemBuffer[CAP_MEM_U0];
    Capctxt->OutAddrV[0]                = CapMemBuffer[CAP_MEM_V0];
    Capctxt->OutAddrY[1]                = CapMemBuffer[CAP_MEM_Y1];
    Capctxt->OutAddrU[1]                = CapMemBuffer[CAP_MEM_U1];
    Capctxt->OutAddrV[1]                = CapMemBuffer[CAP_MEM_V1];
#ifdef TRIPLE_BUFFER
    Capctxt->OutAddrY[2]                = CapMemBuffer[CAP_MEM_Y2];
    Capctxt->OutAddrU[2]                = CapMemBuffer[CAP_MEM_U2];
    Capctxt->OutAddrV[2]                = CapMemBuffer[CAP_MEM_V2];
#endif

    result                              = _Cap_Update_Reg();

    //Capture Fire
    mmpCapFire();
}

bool
ithCaptureGetDetectedStatus()
{
    MMP_BOOL    isCaptureStatusGood = MMP_TRUE;
    MMP_UINT16  DetectedHTotal, DetectedVTotal;

    DetectedHTotal  = CaptureControllerHardwareGetDetectedHSyncLineNums();
    DetectedVTotal  = CaptureControllerHardwareGetDetectedVSyncLineNums();

    if (!DetectedHTotal || !DetectedVTotal)
    {
        isCaptureStatusGood = MMP_FALSE;
        return isCaptureStatusGood;
    }
    else
        return isCaptureStatusGood;
}

bool
ithCapDeviceIsSignalStable()
{
    MMP_BOOL isSignalChange = MMP_FALSE;
    isSignalChange = ithModule_IsSignalStable(CaptureModuleDrivers, 0);
    return (MMP_BOOL)isSignalChange;
}

int ithCaptureSetModule(CaptureModuleDriver CapturemoduleDriver)
{
    //memset(CaptureModuleDrivers, 0, sizeof(CaptureModuleDriver));
    ithModule_Destroy(CaptureModuleDrivers);
    CaptureModuleDrivers = CapturemoduleDriver;

    //Device Init
    mmpCapDeviceInitialize();

    return 0;
}

int ithCaptureGetNewFrame(
    ITE_CAP_VIDEO_INFO *Outdata)
{
    static int      cap_idx;
    static int      NewBufferIdx        = 0;
    static int      OldBufferIdx        = 0;
    static MMP_BOOL GetFirstBufferIdx   = MMP_FALSE;
    MMP_UINT16      timeOut             = 0;
    MMP_UINT16      FirstBufferIdx      = 1;

    if (!GetFirstBufferIdx)
    {
        OldBufferIdx        = ithCap_Get_Write_Buf_Index();
        GetFirstBufferIdx   = MMP_TRUE;
    }

    while (NewBufferIdx == OldBufferIdx)
    {
        NewBufferIdx = ithCap_Get_Write_Buf_Index();
        usleep(1000);
        timeOut++;
        if (timeOut > 1000)
        {
            printf("Capture controller not moving!!\n");
            break;
        }
    }
    OldBufferIdx            = NewBufferIdx;

    Outdata->OutHeight      = Capctxt->ininfo.capheight;
    Outdata->OutWidth       = Capctxt->ininfo.capwidth;
    Outdata->IsInterlaced   = Capctxt->Interleave;
    Outdata->PitchY         = Capctxt->ininfo.PitchY;
    Outdata->PitchUV        = Capctxt->ininfo.PitchUV;
#ifdef TRIPLE_BUFFER
    if (!OldBufferIdx)
        cap_idx = 2;
    else
        cap_idx = OldBufferIdx - 1;

    switch (cap_idx)
    {
    case 0:
        Outdata->DisplayAddrY   = Capctxt->OutAddrY[0];
        Outdata->DisplayAddrU   = Capctxt->OutAddrU[0];
        Outdata->DisplayAddrV   = Capctxt->OutAddrV[0];
        break;

    case 1:
        Outdata->DisplayAddrY   = Capctxt->OutAddrY[1];
        Outdata->DisplayAddrU   = Capctxt->OutAddrU[1];
        Outdata->DisplayAddrV   = Capctxt->OutAddrV[1];
        break;

    case 2:
        Outdata->DisplayAddrY   = Capctxt->OutAddrY[2];
        Outdata->DisplayAddrU   = Capctxt->OutAddrU[2];
        Outdata->DisplayAddrV   = Capctxt->OutAddrV[2];
        break;
    }
#else
    if (!OldBufferIdx)
        cap_idx = 1;
    else
        cap_idx = OldBufferIdx - 1;

    switch (cap_idx)
    {
    case 0:
        Outdata->DisplayAddrY   = Capctxt->OutAddrY[0];
        Outdata->DisplayAddrU   = Capctxt->OutAddrU[0];
        Outdata->DisplayAddrV   = Capctxt->OutAddrV[0];
        break;

    case 1:
        Outdata->DisplayAddrY   = Capctxt->OutAddrY[1];
        Outdata->DisplayAddrU   = Capctxt->OutAddrU[1];
        Outdata->DisplayAddrV   = Capctxt->OutAddrV[1];
        break;
    }
#endif

    return 0;
}

int ithCapStop()
{
    MMP_RESULT result = MMP_SUCCESS;

    mmpUnCapFire();
    return result;
}

//end of capture.c
int ithCapFire()
{
    MMP_RESULT result = MMP_SUCCESS;

    //Capture setting
    //get device information
    mmpCapGetDeviceInfo();
    if (Capctxt->bMatchResolution == MMP_FALSE)
    {
        printf("MatchResolution fail!!\n");
        return MMP_RESULT_ERROR;
    }

    //set capture parameter
    mmpCapParameterSetting();

    //Capture Fire
    mmpCapFire();

    return result;
}

int ithCapTerminate()
{
    MMP_RESULT result = MMP_SUCCESS;

    mmpCapDeviceTerminate();

    mmpCapTerminate();

    mmpCap_Memory_Clear();

    ithCap_Set_DisableClock();

    if (Capctxt)
    {
        free(Capctxt);
        Capctxt = NULL;
    }

    return result;
}

int
ithCapInitialize()
{
    MMP_UINT32  i;
    MMP_RESULT  result = MMP_SUCCESS;

    for (i = ITH_GPIO_PIN69; i <= ITH_GPIO_PIN81; i++)   //GPIO70 ~ GPIO81  for capture setting
    {
        ithGpioSetMode(i, ITH_GPIO_MODE1);
        ithGpioSetIn(i);
    }

    //usleep(1000);
    //it can add ithCapResetEngine() here. //Benson
    //ithCapEnableClock();

    result  = mmpCap_Memory_Initialize();

    //1st Capture Init, switch IO direction
    result  = mmpCap_Initialize();

    ithCap_Set_EnableClock(); //Add for fingerprint recognition

    if (result != MMP_SUCCESS)
    {
        printf("[Capture] _CaptureAndCaptureModule_Init Fail\n");
        mmpCapPowerDown();
        return -1;
    }

    return 0;
}

/*  setting color space mode */
void ithCapSetMode(
    MMP_UINT8 mode)
{
    CaptureControllerHardware_SetMode(mode);
}

void ithCapGetBufferaddr(MMP_UINT32* membuffer)
{
    memcpy(membuffer,CapMemBuffer, sizeof(MMP_UINT32) *CAPTURE_MEM_BUF_COUNT);
}