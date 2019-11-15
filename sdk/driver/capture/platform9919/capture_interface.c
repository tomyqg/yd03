#include <ite/ith.h>
#include <string.h>
#include "capture_interface.h"
#include "capture/mmp_capture.h"
#include "capture/sensor_device_table.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define CAP_DATA_BUS_WIDTH 12

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
extern void(*IO_SETTING[MMP_CAP_DEV_MAX])(CAP_CONTEXT * Capctxt);

//=============================================================================
//                Private Function
//=============================================================================

//=============================================================================
//                Public Function
//=============================================================================
void CaptureIO_Init(void)
{
#if defined (CFG_SENSOR_ENABLE)
    #if defined (SENSOR_NOVATEK_NT99141)
    IO_SETTING[MMP_CAP_DEV_SENSOR]  = _NOVATEK_NT99141_Setting;
    #elif defined(SENSOR_OMNIVISION_OV7725)
    IO_SETTING[MMP_CAP_DEV_SENSOR]  = _OMNIVISION_OV7725_Setting;
    #elif defined (SENSOR_PIXELPLUS_PO3100)
    IO_SETTING[MMP_CAP_DEV_SENSOR]  = _PIXELPLUS_PO3100_Setting;
    #elif defined (SENSOR_AR0130)
    IO_SETTING[MMP_CAP_DEV_SENSOR]  = _AR0130_Setting;
    #endif
#else
    IO_SETTING[MMP_CAP_DEV_ADV7180] = _ADV7180_Setting;
    IO_SETTING[MMP_CAP_DEV_CAT9883] = _CAT9883_Setting;
    IO_SETTING[MMP_CAP_DEV_HDMIRX]  = _HDMIRX_LoopThrough_Setting;
#endif
}