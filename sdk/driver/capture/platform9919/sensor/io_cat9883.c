#include <string.h>
#include "capture_interface.h"
#include "capture/video_device_table.h"

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
 * CAT9883 Set Input Data Pin.
 */
//=============================================================================
static void
_CAT9883_Input_Data_Pin(
    CAP_CONTEXT *Capctxt)
{
    //Reference Design 2012/09/12
    //RGB 24Bit Pin LSB--->MSB (D0--->D11)
    MMP_UINT8 G_Y_24Pin[CAP_DATA_BUS_WIDTH] = { 8,  9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0};
    MMP_UINT8 B_U_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7, 0, 0, 0, 0};
    MMP_UINT8 R_V_24Pin[CAP_DATA_BUS_WIDTH] = {16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0};

    /* Input  pin mux setting */
    if (Capctxt->funen.EnCSFun == MMP_TRUE) //RGB Input
    {
        memcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_V_24Pin, CAP_DATA_BUS_WIDTH);
        memcpy(&Capctxt->inmux_info.U_Pin_Num, &G_Y_24Pin, CAP_DATA_BUS_WIDTH);
        memcpy(&Capctxt->inmux_info.V_Pin_Num, &B_U_24Pin, CAP_DATA_BUS_WIDTH);
    }
    else //YPbPr Input
    {
        memcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_Y_24Pin, CAP_DATA_BUS_WIDTH);
        memcpy(&Capctxt->inmux_info.U_Pin_Num, &B_U_24Pin, CAP_DATA_BUS_WIDTH);
        memcpy(&Capctxt->inmux_info.V_Pin_Num, &R_V_24Pin, CAP_DATA_BUS_WIDTH);
    }
}

//=============================================================================
/**
 * CAT9883 Setting.
 */
//=============================================================================
void
_CAT9883_Setting(
    CAP_CONTEXT *Capctxt)
{
    MMP_UINT16 i;

    _CAT9883_Input_Data_Pin(Capctxt);

    Capctxt->iomode_info.EnInternalRxHDMI = 0x0; // 0: VDO Source from IO Pad.

    Capctxt->inmux_info.UCLKSrc           = 0x0; //0: IO, 1: HDMI, 2: GPIO, 3: PLL3_n1, 4: PLL3_n2
    Capctxt->inmux_info.UCLKInv           = 0x0;
    Capctxt->inmux_info.UCLKDly           = 0x0;
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

    Capctxt->iomode_info.CAPIOFFEn_DE       = 0x0;
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
    // 16bit bus
    //Capctxt->YUVinfo.InputMode = YUV422;
    //Capctxt->YUVinfo.YUV422Format = CAP_IN_YUV422_VU_YY;
    //Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;

    //24bin bus
    Capctxt->YUVinfo.InputMode     = YUV444;
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;

    Capctxt->funen.EnDEMode        = MMP_FALSE;
    Capctxt->funen.EnInBT656       = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE   = MMP_FALSE;
}


