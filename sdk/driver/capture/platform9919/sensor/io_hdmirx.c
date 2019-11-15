#include <string.h>
#include "capture_interface.h"
#include "capture/sensor_device_table.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define CAP_DATA_BUS_WIDTH  12

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
/**
 * INTERNAL HDMIRX Set Input Data Pin.
 */
//=============================================================================
static void
_HDMIRX_Input_Data_Pin(
    CAP_CONTEXT *Capctxt)
{
    //RGB 36Bit Pin LSB--->MSB (D0--->D11)
    MMP_UINT8 R_Y_36Pin[CAP_DATA_BUS_WIDTH] = {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
    MMP_UINT8 G_U_36Pin[CAP_DATA_BUS_WIDTH] = {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    MMP_UINT8 B_V_36Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11};

    MMP_UINT8 R_Y_30Pin[CAP_DATA_BUS_WIDTH] = {26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 0, 0};
    MMP_UINT8 G_U_30Pin[CAP_DATA_BUS_WIDTH] = {14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 0};
    MMP_UINT8 B_V_30Pin[CAP_DATA_BUS_WIDTH] = { 2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 0, 0};

    MMP_UINT8 R_Y_24Pin[CAP_DATA_BUS_WIDTH] = {28, 29, 30, 31, 32, 33, 34, 35, 0, 0, 0, 0};
    MMP_UINT8 G_U_24Pin[CAP_DATA_BUS_WIDTH] = {16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0};
    MMP_UINT8 B_V_24Pin[CAP_DATA_BUS_WIDTH] = { 4,  5,  6,  7,  8,  9, 10, 11, 0, 0, 0, 0};

    if (Capctxt->funen.EnCSFun) //HDMI Rx RGB444 Output
    {
        if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_12_BITS)
        {
            /* Input  pin mux setting */
            memcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_36Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_36Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_36Pin, CAP_DATA_BUS_WIDTH);
        }
        else if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_10_BITS)
        {
            /* Input  pin mux setting */
            memcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_30Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_30Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_30Pin, CAP_DATA_BUS_WIDTH);
        }
        else
        {
            /* Input  pin mux setting */
            memcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_24Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_24Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_24Pin, CAP_DATA_BUS_WIDTH);
        }
    }
    else //HDMI Rx YUV444/YUV422 Output
    {
        if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_12_BITS)
        {
            /* Input  pin mux setting */
            memcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_U_36Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.U_Pin_Num, &B_V_36Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.V_Pin_Num, &R_Y_36Pin, CAP_DATA_BUS_WIDTH);
        }
        else if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_10_BITS)
        {
            /* Input  pin mux setting */
            memcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_U_30Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.U_Pin_Num, &B_V_30Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.V_Pin_Num, &R_Y_30Pin, CAP_DATA_BUS_WIDTH);
        }
        else
        {
            /* Input  pin mux setting */
            memcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_U_24Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.U_Pin_Num, &B_V_24Pin, CAP_DATA_BUS_WIDTH);
            memcpy(&Capctxt->inmux_info.V_Pin_Num, &R_Y_24Pin, CAP_DATA_BUS_WIDTH);
        }
    }
}

static void
_HDMIRX_Output_Data_Pin(
    CAP_CONTEXT *Capctxt)
{
    MMP_UINT8 CAPIOVDOUTSEL [36] = {
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    memcpy(&Capctxt->outpin_info.CAPIOVDOUTSEL, &CAPIOVDOUTSEL, 36);
}

//=============================================================================
/**
 * INTERNAL HDMIRX Setting.
 */
//=============================================================================
void
_HDMIRX_Setting(
    CAP_CONTEXT *Capctxt)
{
    MMP_UINT16 i;

    _HDMIRX_Input_Data_Pin(Capctxt);

    Capctxt->iomode_info.EnInternalRxHDMI = 0x1; //1: VDO from internal HDMI Rx.

    Capctxt->inmux_info.UCLKSrc           = 0x1; //0: IO, 1: HDMI, 2: GPIO, 3: PLL3_n1, 4: PLL3_n2
    Capctxt->inmux_info.UCLKInv           = 0x0;
    Capctxt->inmux_info.UCLKDly           = 0x1; //delay 1ns
    Capctxt->inmux_info.UCLKRatio         = 0x0;
    Capctxt->inmux_info.EnUCLK            = 0x1;
    Capctxt->inmux_info.UCLKVDSel         = 36;

    /* Output pin mux setting */
    for (i = 0; i < 36; i++)
        Capctxt->outpin_info.CAPIOVDOUTSEL[i] = 0;
    Capctxt->outpin_info.CAPIOVDOUTSEL_X    = 0;

    /* I/O Setting */
    Capctxt->iomode_info.CAPIOEN_VD_15_0    = 0xFFFF; //1111_1111_1111_1111
    Capctxt->iomode_info.CAPIOEN_VD_31_16   = 0xFFFF; //1111_1111_1111_1111
    Capctxt->iomode_info.CAPIOEN_VD_32_35   = 0xF;    //0000_0000_0000_1111
    Capctxt->iomode_info.CAPIOEN_X          = 0x1;
    Capctxt->iomode_info.CAPIOEN_HS         = 0x1;
    Capctxt->iomode_info.CAPIOEN_VS         = 0x1;
    Capctxt->iomode_info.CAPIOEN_DE         = 0x1;
    Capctxt->iomode_info.CAPIOEN_Field      = 0x1;
    Capctxt->iomode_info.CAPIOEN_PMCLK      = 0x1;

    Capctxt->iomode_info.CAPIOFFEn_DE       = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_Field    = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_HS       = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VS       = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync            = BT_601;
    Capctxt->ininfo.Interleave              = Progressive;

    /* Input Data Format Setting */
    Capctxt->YUVinfo.InputMode              = YUV444;
    Capctxt->YUVinfo.ClockPerPixel          = CPP_1P1T;

    Capctxt->funen.EnDEMode                 = MMP_FALSE;
    Capctxt->funen.EnInBT656                = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE            = MMP_FALSE;
}

void
_HDMIRX_LoopThrough_Setting(
    CAP_CONTEXT *Capctxt)
{
    _HDMIRX_Input_Data_Pin(Capctxt);

    Capctxt->iomode_info.EnInternalRxHDMI = 0x1; //1: VDO from internal HDMI Rx.

    Capctxt->inmux_info.UCLKSrc           = 0x1; //0: IO, 1: HDMI, 2: GPIO, 3: PLL3_n1, 4: PLL3_n2
    Capctxt->inmux_info.UCLKInv           = 0x0;
    Capctxt->inmux_info.UCLKDly           = 0x1;
    Capctxt->inmux_info.UCLKRatio         = 0x0;
    Capctxt->inmux_info.EnUCLK            = 0x1;
    Capctxt->inmux_info.UCLKVDSel         = 0x0;

    /* Output pin mux setting */
    _HDMIRX_Output_Data_Pin(Capctxt);
    Capctxt->outpin_info.CAPIOVDOUTSEL_X    = 0x40;

    /* I/O Setting */
    Capctxt->iomode_info.CAPIOEN_VD_15_0    = 0x0; //0000_0000_0000_0000
    Capctxt->iomode_info.CAPIOEN_VD_31_16   = 0x0; //0000_0000_0000_0000
    Capctxt->iomode_info.CAPIOEN_VD_32_35   = 0x0; //0000_0000_0000_0000
    Capctxt->iomode_info.CAPIOEN_X          = 0x0;
    Capctxt->iomode_info.CAPIOEN_HS         = 0x0;
    Capctxt->iomode_info.CAPIOEN_VS         = 0x0;
    Capctxt->iomode_info.CAPIOEN_DE         = 0x1;
    Capctxt->iomode_info.CAPIOEN_Field      = 0x0;
    Capctxt->iomode_info.CAPIOEN_PMCLK      = 0x1;

    Capctxt->iomode_info.CAPIOFFEn_DE       = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_Field    = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_HS       = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VS       = 0x1;
    Capctxt->iomode_info.HDMICLKDly         = 0x0;
    Capctxt->iomode_info.HDMICLKInv         = 0x1;

    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync            = BT_601;
    Capctxt->ininfo.Interleave              = Progressive;

    /* Input Data Format Setting */
    Capctxt->YUVinfo.InputMode              = YUV444;
    Capctxt->YUVinfo.ClockPerPixel          = CPP_1P1T;

    Capctxt->funen.EnDEMode                 = MMP_TRUE;
    Capctxt->funen.EnInBT656                = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE            = MMP_FALSE;
}

