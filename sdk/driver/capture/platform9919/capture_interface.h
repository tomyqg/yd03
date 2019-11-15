#ifndef __CAPTURE_INTERFACE_H__
#define __CAPTURE_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "capture/mmp_capture.h"

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
void CaptureIO_Init();

void _AR0130_Setting(CAP_CONTEXT*);
void _PIXELPLUS_PO3100_Setting(CAP_CONTEXT*);
void _NOVATEK_NT99141_Setting(CAP_CONTEXT*);
void _OMNIVISION_OV7725_Setting(CAP_CONTEXT*);

void _ADV7180_Setting(CAP_CONTEXT*);
void _CAT9883_Setting(CAP_CONTEXT*);
void _HDMIRX_Setting(CAP_CONTEXT*);
void _HDMIRX_LoopThrough_Setting(CAP_CONTEXT*);
void _EXTERNALHDMIRX_Setting(CAP_CONTEXT*);

#ifdef __cplusplus
}
#endif

#endif