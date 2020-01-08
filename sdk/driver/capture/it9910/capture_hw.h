#ifndef __CAP_HW_H__
#define __CAP_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capture/capture_types.h"
#include "capture/mmp_capture.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define B_CAP_INPUT_INTERLEVING     (1 << 9)
#define B_CAP_SYNC_MODE_BT_656      (1 << 1)
#define B_CAP_ONFLY_MODE            (1 << 3)

#define B_CAP_ENABLE_INT            (1 << 15)
#define M_CAP_ENABLE_INT            0x8000

/* color bar */
#define B_CAP_COLOR_BAR_ENABLE      (1 << 15)
#define B_CAP_COLOR_BAR_VSPOL       (1 << 13)
#define B_CAP_COLOR_BAR_HSPOL       (1 << 12)

/* input data format */
#define CAP_IN_DATA_FORMAT_YUV422   (1 << 12)

#define CAP_IN_YUV444               0
#define CAP_IN_YUV422               1
#define CAP_PROGRESSIVE             0
#define CAP_INTERLEVING             1
#define CAP_BT_601                  0
#define CAP_BT_656                  1

#define CAP_ONE_CHANNEL_MODE        0
#define CAP_TWO_CHANNEL_MODE        1
#define CAP_CHANNEL_MODE            (1 << 13)
#define CAP_IN_FORMAT_MASK          3

#define CAP_PRELOADNUM_MASK         3

#define CAP_ERRLD                   (0x7 << 8)
#define CAP_ONCEFULL                (0x1 << 3)
#define CAP_HALTFORFIRE             (0x1 << 2)
#define CAP_SIZEERR                 (0x1 << 1)
#define CAP_ENCSFUN                 (0x1 << 15)

/* Capture interrupt mode */
#define CAP_INT_MODE_ERR            0
#define CAP_INT_MODE_ERR_FRAMEEND   1
#define CAP_INT_MODE_MULTI_CHANNEL  2
#define CAP_INT_MODE_MASK           3

#define CAP_BIT_SCALEWEIGHT         0x00FF          // 0000 0000 1111 1111

#define CAP_SHT_SCALEWEIGHT_H       8
#define CAP_SHT_SCALEWEIGHT_L       0

#define CAP_BIT_RGB_TO_YUV          0x07FF          // 0000 0111 1111 1111
#define CAP_BIT_RGB_TO_YUV_CONST    0x03FF          // 0000 0011 1111 1111

#define MMP_CAP_CLOCK_REG_62        0x0062
#define MMP_CAP_CLOCK_REG_64        0x0064
#define MMP_CAP_EN_DIV_CAPCLK       0x0800          // [11]
#define MMP_CAP_RESET               0x0008          // [ 3]
#define MMP_CAP_REG_RESET           0x0004          // [ 2]
#define MMP_CAP_EN_M17CLK           0x0001          // [ 0] memory clock in Capture

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
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_Engine_Register_Reset(
    void);

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
    CAPTURE_DEV_ID DEV_ID, CAP_CONTEXT *Capctxt);

void
ithCap_Set_HorScale_Width_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_OUTPUT_INFO *pOutInfo);

void
ithCap_Set_Skip_Pattern_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 pattern, MMP_UINT16 period);

void
ithCap_Set_Enable_Interrupt(
    CAPTURE_DEV_ID DEV_ID, MMP_BOOL flag);

void
ithCap_Set_Interrupt_Mode(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT8 Intr_Mode);

void
ithCap_Set_Color_Bar(
    CAPTURE_DEV_ID DEV_ID, CAP_COLOR_BAR_CONFIG color_info);

void
ithCap_Set_EnableFrameRate_Reg(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Set_Enable_Reg(
    CAPTURE_DEV_ID DEV_ID, CAP_CONTEXT *Capctxt);

MMP_RESULT
ithCap_Set_ISP_HandShaking(
    CAPTURE_DEV_ID DEV_ID, CAP_ISP_HANDSHAKING_MODE mode, MMP_UINT8 preloadnum);

MMP_RESULT
ithCap_Set_Error_Handleing(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 errDetectEn);

MMP_RESULT
ithCap_Set_Wait_Error_Reset(
    CAPTURE_DEV_ID DEV_ID);

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
ithCAP_Set_ScaleParam_Reg(
    CAPTURE_DEV_ID DEV_ID, const CAP_SCALE_CTRL *pScaleFun);

void
ithCap_Set_IntScaleMatrixH_Reg(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 WeightMatX[][CAP_SCALE_TAP]);

void
ithCap_Set_Polarity(
    CAPTURE_DEV_ID DEV_ID, MMP_UINT16 HPolarity, MMP_UINT16 VPolarity);

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
ithCap_EnableClock(
    void);

void
ithCap_DisableClock(
    void);

void
ithCap_Reset(
    void);

void
ithCap_Reg_Reset(
    void);

//=============================================================================
/**
 * Audio/Video/Mute Counter control function
 */
//=============================================================================
void
AVSync_CounterCtrl(
    AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider);

void
AVSync_CounterReset(
    AV_SYNC_COUNTER_CTRL mode);

MMP_UINT32
AVSync_CounterLatch(
    AV_SYNC_COUNTER_CTRL mode);

MMP_UINT32
AVSync_CounterRead(
    AV_SYNC_COUNTER_CTRL mode);

MMP_BOOL
AVSync_MuteDetect(
    void);

MMP_BOOL
ithCap_Get_IsFire(
    CAPTURE_DEV_ID DEV_ID);

MMP_RESULT
ithCap_Get_WaitEngineIdle(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Lane_status(
    CAPTURE_DEV_ID DEV_ID, CAP_LANE_STATUS lanenum);

MMP_UINT16
ithCap_Get_Hsync_Polarity(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Vsync_Polarity(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Hsync_Polarity_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Vsync_Polarity_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_RESULT
ithCap_Get_BT601_SyncCnt(
    CAPTURE_DEV_ID DEV_ID);

MMP_RESULT
ithCap_Get_Detected_Region(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Sync_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Error_Status(
    CAPTURE_DEV_ID DEV_ID);

MMP_UINT16
ithCap_Get_Clean_Intr(
    CAPTURE_DEV_ID DEV_ID);

void
ithCap_Dump_Reg(
    CAPTURE_DEV_ID DEV_ID);

#ifdef __cplusplus
}
#endif

#endif