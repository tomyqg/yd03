#ifndef __ISP_HW_OP_H__
#define __ISP_HW_OP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_types.h"

//=============================================================================
//                        Macro Definition
//=============================================================================

//=============================================================================
//                        Structure Definition
//=============================================================================
typedef struct ISP_HW_OPERATION {
    void        (*ReadReg)(MMP_UINT32 addr, MMP_UINT32 *data);
    void        (*WriteReg)(struct ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data);
    void        (*WriteRegMask)(struct ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data, MMP_UINT32 mask);
    void        (*WaitQueue)(MMP_UINT32 sizeInByte);
    void        (*FireQueue)(void);
    void        (*FireIspQueue)(void);
    ISP_RESULT  (*WaitIspIdle)(struct ISP_CONTEXT *ISPctxt);
} ISP_HW_OPERATION;

//=============================================================================
//				          Global Data Definition
//=============================================================================
extern ISP_HW_OPERATION gtIspHwOpIspQ;
extern ISP_HW_OPERATION gtIspHwOpCmdQ;
extern ISP_HW_OPERATION gtIspHwOpMMIO;
extern ISP_HW_OPERATION *gptIspHwOp;

//=============================================================================
//				          Public Function Definition
//=============================================================================
#ifdef WIN32
extern void
ISP_CmdSelect(
    struct ISP_CONTEXT *ISPctxt,
    ISP_HW_OPERATION   *ptIspHwOp);

extern void
ISP_ReadRegister(
    struct ISP_CONTEXT *ISPctxt,
    MMP_UINT32         addr,
    MMP_UINT32         *data);

extern void
ISP_WriteRegister(
    struct ISP_CONTEXT *ISPctxt,
    MMP_UINT32         addr,
    MMP_UINT32         data);

extern void
ISP_WriteRegisterMask(
    struct ISP_CONTEXT *ISPctxt,
    MMP_UINT32         addr,
    MMP_UINT32         data,
    MMP_UINT32         mask);

extern void
ISP_CMD_QUEUE_WAIT(
    struct ISP_CONTEXT *ISPctxt,
    MMP_UINT32         sizeInByte);

extern void
ISP_CMD_QUEUE_FIRE(
    struct ISP_CONTEXT *ISPctxt);

extern void
ISP_FireIspQueue(
    struct ISP_CONTEXT *ISPctxt);

extern ISP_RESULT
ISP_WaitIspIdle(
    struct ISP_CONTEXT *ISPctxt);
#endif

#ifdef __cplusplus
}
#endif

#endif