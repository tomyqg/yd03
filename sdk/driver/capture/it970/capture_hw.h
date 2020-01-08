#ifndef __CAP_HW_H__
#define __CAP_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capture/capture_types.h"
#include "capture/mmp_capture_it970.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define B_CAP_MEM_MODE                  (1 << 0)
#define B_CAP_ONFLY_MODE                (1 << 1)
#define B_CAP_ERR_RST_STOP_ENGINE       (1 << 16)

#define B_CAP_GET_INTR                  (1 << 31)
#define B_CAP_GET_INTR_FROM_DEV1        (1 << 28)

/* color bar */
#define B_CAP_COLOR_BAR_ROLLING_EN      (1 << 31)
#define B_CAP_COLOR_BAR_VSPOL           (1 << 17)
#define B_CAP_COLOR_BAR_HSPOL           (1 << 16)

/* input data format */
#define CAP_IN_DATA_FORMAT_YUV422       (1 << 12)

#define CAP_PROGRESSIVE                 0
#define CAP_INTERLEVING                 1
#define CAP_BT_601                      0
#define CAP_BT_656                      1

#define CAP_ENCSFUN                     (0x1 << 24)

/* Capture interrupt mode */
#define CAP_INT_MODE_FRAME_END          0x1
#define CAP_INT_MODE_SYNC_ERR           0x10
#define CAP_INT_MODE_DCLK_PHASE_DRIFTED 0x100
#define CAP_INT_MODE_MUTE_DETECT        0x1000

#define CAP_BIT_SCALEWEIGHT             0x00FF      // 0000 0000 1111 1111

#define CAP_SHT_SCALEWEIGHT_H           8
#define CAP_SHT_SCALEWEIGHT_L           0

#define CAP_BIT_RGB_TO_YUV              0x07FF      // 0000 0111 1111 1111
#define CAP_BIT_RGB_TO_YUV_CONST        0x03FF      // 0000 0011 1111 1111

#define MMP_CAP_CLOCK_REG_60            0x0060
#define MMP_CAP_EN_DIV_CAPCLK           0x00008000  // [15]
#define MMP_CAP_RESET                   0x40000000  // [ 30]
#define MMP_CAP_REG_RESET               0x80000000  // [ 31]
#define MMP_CAP_EN_M12CLK               0x00020000  // [ 17] memory clock in Capture

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
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_UnFire(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_Engine_Reset(
    void);

void
ithCap_Set_Engine_Register_Reset(
    void);

void
ithCap_Set_Err_Reset(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_TurnOnClock_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_BOOL flag);

MMP_RESULT
ithCap_Set_Input_Pin_Mux_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_INPUT_MUX_INFO *pininfo);

void
ithCap_Set_Output_Pin_Mux_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_OUTPUT_PIN_SELECT *pininfo);

MMP_RESULT
ithCap_Set_Output_Clk_Mode_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 value);

void
ithCap_Set_Color_Format_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_YUV_INFO *pYUVinfo);

MMP_RESULT
ithCap_Set_IO_Mode_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_IO_MODE_INFO *io_config);

void
ithCap_Set_Input_Data_Info_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_INPUT_INFO *pIninfo);

void
ithCap_Set_HorScale_Width_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_OUTPUT_INFO *pOutInfo);

void
ithCap_Set_Skip_Pattern_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 pattern, MMP_UINT16 period);
void
ithCap_Set_Hsync_Polarity(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 Hsync);
void
ithCap_Set_Vsync_Polarity(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 Vsync);

void
ithCap_Set_Enable_Interrupt(
    CAPTURE_DEV_ID DEV_ID, MMP_BOOL flag);

void
ithCap_Set_Interrupt_Mode(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 Intr_Mode, MMP_BOOL flag);

void
ithCap_Set_Color_Bar(
    CAPTURE_DEV_ID DEV_ID, CAP_COLOR_BAR_CONFIG color_info);

void
ithCap_Set_Interleave(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 Interleave);

void
ithCap_Set_Width_Height(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 width, MMP_UINT32 height);

void
ithCap_Set_ROI_Width_Height(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 width, MMP_UINT32 height);

void
ithCap_Set_Clean_Intr(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_EnableFrameRate_Reg(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_Enable_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_ENFUN_INFO *pFunEn);

void
ithCap_Set_Enable_Dither(
    CAPTURE_DEV_ID DEV_ID, CAP_INPUT_DITHER_INFO *pDitherinfo);

MMP_RESULT
ithCap_Set_ISP_HandShaking(
    CAPTURE_DEV_ID DEV_ID, CAP_ISP_HANDSHAKING_MODE mode, CAP_OUTPUT_INFO *pOutInfo);

MMP_RESULT
ithCap_Set_Error_Handleing(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 errDetectEn);

MMP_RESULT
ithCap_Set_Wait_Error_Reset(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_Memory_AddressLimit_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 memUpBound, MMP_UINT32 memLoBound);

void
ithCap_Set_FrameBase_0_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
ithCap_Set_FrameBase_1_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
ithCap_Set_FrameBase_2_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
ithCap_Set_FrameBase_3_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
ithCap_Set_FrameBase_4_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
ithCap_Set_Buffer_addr_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 *pAddr, MMP_UINT32 addrOffset);

void
ithCap_Set_ScaleParam_Reg(
    CAPTURE_DEV_ID DEV_ID, const CAP_SCALE_CTRL *pScaleFun);

void
ithCap_Set_IntScaleMatrixH_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT32 WeightMatX[][CAP_SCALE_TAP]);

//=============================================================================
/**
 * RGB to YUV transfer matrix.
 */
//=============================================================================
void
ithCap_Set_RGBtoYUVMatrix_Reg(
    CAPTURE_DEV_ID DEV_ID, const CAP_RGB_TO_YUV *pRGBtoYUV);

//=============================================================================
/**
 * Set color correction matrix and constant.
 */
//=============================================================================
void
ithCap_Set_CCMatrix_Reg(
    CAPTURE_DEV_ID DEV_ID, const CAP_COLOR_CORRECTION *pColorCorrect);

void
ithCap_Set_Output_Driving_Strength_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 driving);

void
ithCap_Set_16bit_mode(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_EnableClock(
    void);

void
ithCap_Set_DisableClock(
    void);

void
ithCap_Set_Reset(
    void);

//=============================================================================
/**
 * Audio/Video/Mute Counter control function
 */
//=============================================================================
void
ithAVSync_CounterCtrl(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider);

void
ithAVSync_CounterReset(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode);

MMP_UINT32
ithAVSync_CounterLatch(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode);

MMP_UINT32
ithAVSync_CounterRead(
    CAPTURE_DEV_ID DEV_ID, AV_SYNC_COUNTER_CTRL mode);

MMP_BOOL
ithAVSync_MuteDetect(
    CAPTURE_DEV_ID DEV_ID);

MMP_BOOL
ithCap_Get_IsFire(
    CAPTURE_DEV_ID DEV_ID);

MMP_RESULT
ithCap_Get_WaitEngineIdle(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Lane_status(
    CAPTURE_DEV_ID DEV_ID, CAP_LANE_STATUS lanenum);

MMP_UINT32
ithCap_Get_Hsync_Polarity(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Vsync_Polarity(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Hsync_Polarity_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Vsync_Polarity_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Revision(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_MRawVTotal(
    CAPTURE_DEV_ID DEV_ID);

MMP_RESULT
ithCap_Get_Detected_Region(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Detected_Width(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Detected_Height(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Detected_Interleave(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Sync_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT32
ithCap_Get_Error_Status(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Dump_Reg(
    CAPTURE_DEV_ID DEV_ID);

#ifdef __cplusplus
}
#endif

#endif