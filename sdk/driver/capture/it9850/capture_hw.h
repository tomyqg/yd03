#ifndef __CAP_HW_H__
#define __CAP_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capture_types.h"

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

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

void
ithCap_Set_Fire(
    void);

void
ithCap_Set_UnFire(
    void);


void
CaptureControllerHardware_SetInputSignalSource(
    CAP_INPUT_VIDEO_SOURCE_INFO *pInVideoinfo);

void
ithCap_Set_Color_Format_Reg(
    MMP_UINT8 YUV422Format);

void
CaptureControllerHardware_SetMode(
    MMP_UINT8 Mode);

void
ithCap_Set_Interleave(
    CAP_INPUT_VIDEO_FORMAT Interleave);

void
ithCap_Set_Input_Data_Info_Reg(
    CAP_INPUT_INFO *pIninfo);

void
ithCap_Set_Reset(
    void);

void
ithCap_Dump_Reg(
    void);

void
ithCap_Set_Use_Triple_Buf(
    MMP_BOOL flag);

void
ithCap_Set_Use_Data_Enable_Mode(
    MMP_BOOL EnDEMode);

void 
ithCap_Set_Input_Protocol(
    CAP_INPUT_PROTOCOL_INFO pInputProtocol);

void
ithCap_Set_Output_Buf_Addr (
    MMP_UINT32 *pAddr);

void
ithCap_Set_EnableClock(
    void);

void
ithCap_Set_DisableClock(
    void);

MMP_UINT16
ithCap_Get_Write_Buf_Index(
    void);

MMP_UINT16
CaptureControllerHardwareGetDetectedDisplayWidth(
	void);

MMP_UINT16
CaptureControllerHardwareGetDetectedDisplayHeight(
	void);

MMP_UINT16
CaptureControllerHardwareGetDetectedHSyncLineNums(
	void);

MMP_UINT16
CaptureControllerHardwareGetDetectedVSyncLineNums(
	void);

#ifdef __cplusplus
}
#endif

#endif