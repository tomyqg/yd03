#ifndef __ISP_H__
#define __ISP_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "mmp_types.h"
#include "mmp_capture.h"
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
//=============================================================================
/**
 * CAP memory Initialize.
 */
//=============================================================================
MMP_RESULT
_Cap_Memory_Initialize(
    MMP_UINT32 width, MMP_UINT32 height);

//=============================================================================
/**
 * CAP memory clear.
 */
//=============================================================================
MMP_RESULT
_Cap_Memory_Clear(
    void);

//=============================================================================
/**
 * Update CAP device.
 */
//=============================================================================
MMP_RESULT
_Cap_Update_Reg(
    CAPTURE_HANDLE *cap_handle);

//=============================================================================
/**
 * CAP default value initialization.
 */
//=============================================================================
MMP_RESULT
_Cap_Initialize(
    CAPTURE_HANDLE *cap_handle);

void
_Cap_SetInputSource(
    CAPTURE_HANDLE *cap_handle, MMP_CAP_DEVICE_ID inputsource);

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
    MMP_INT32               colorEffect[2]);

//=============================================================================
/**
 * Update Capture color matrix device.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of Cap_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
_Cap_UpdateColorMatrix(
    CAPTURE_HANDLE *cap_handle);

#endif

#ifdef __cplusplus
}
#endif

#endif