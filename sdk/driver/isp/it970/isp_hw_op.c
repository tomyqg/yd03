#include "isp_hw_op.h"
#include "isp_hw.h"
#include "isp_queue_type.h"
#include "isp_queue.h"
#include "isp_reg.h"

//=============================================================================
//                        Private Function Declaration
//=============================================================================
static void _dummy_ReadRegister(MMP_UINT32 addr, MMP_UINT32 *data);
static void _dummy_WriteRegisterMask(ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data, MMP_UINT32 mask);
static void _dummy_WaitQueue(MMP_UINT32 sizeInByte);
static void _dummy_FireQueue(void);
static void _dummy_FireIspQueue(void);
static ISP_RESULT _dummy_WaitIspIdle(ISP_CONTEXT *ISPctxt);

static void _IspQ_ReadRegister(MMP_UINT16 addr, MMP_UINT16 *data);
static void _IspQ_WriteRegister(ISP_CONTEXT *ISPctxt, MMP_UINT16 addr, MMP_UINT16 data);
static void _IspQ_WaitQueue(MMP_UINT32 sizeInByte);
static void _IspQ_FireQueue(void);
static void _IspQ_FireIspQueue(void);

static void _CmdQ_WriteRegister(ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data);
static void _CmdQ_WriteRegisterMask(ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data, MMP_UINT32 mask);

static void _MMIO_ReadRegister(MMP_UINT32 addr, MMP_UINT32 *data);
static void _MMIO_WriteRegister(ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data);
static void _MMIO_WriteRegisterMask(ISP_CONTEXT *ISPctxt, MMP_UINT32 addr, MMP_UINT32 data, MMP_UINT32 mask);
static ISP_RESULT _MMIO_WaitIspIdle(ISP_CONTEXT *ISPctxt);

//=============================================================================
//                        Global Data Definition
//=============================================================================
ISP_HW_OPERATION gtIspHwOpIspQ =
{
    _IspQ_ReadRegister,
    _IspQ_WriteRegister,
    _dummy_WriteRegisterMask,
    _IspQ_WaitQueue,
    _IspQ_FireQueue,
    _IspQ_FireIspQueue,
    _dummy_WaitIspIdle
};

ISP_HW_OPERATION gtIspHwOpCmdQ =
{
    _dummy_ReadRegister,
    _CmdQ_WriteRegister,
    _CmdQ_WriteRegisterMask,
    _dummy_WaitQueue,
    _dummy_FireQueue,
    _dummy_FireIspQueue,
    _dummy_WaitIspIdle
};

ISP_HW_OPERATION gtIspHwOpMMIO =
{
    _MMIO_ReadRegister,
    _MMIO_WriteRegister,
    _MMIO_WriteRegisterMask,
    _dummy_WaitQueue,
    _dummy_FireQueue,
    _dummy_FireIspQueue,
    _MMIO_WaitIspIdle
};

//=============================================================================
//                        Public Function Definition
//=============================================================================
MMP_INLINE void
ISP_CmdSelect(
    ISP_CONTEXT      *ISPctxt,
    ISP_HW_OPERATION *ptIspHwOp)
{
    ISPctxt->gptIspHwOp = ptIspHwOp;
}

MMP_INLINE void
ISP_ReadRegister(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  *data)
{
    ISPctxt->gptIspHwOp->ReadReg(addr, data);
}

MMP_INLINE void
ISP_WriteRegister(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  data)
{
    ISPctxt->gptIspHwOp->WriteReg(ISPctxt, addr, data);
}

MMP_INLINE void
ISP_WriteRegisterMask(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  data,
    MMP_UINT32  mask)
{
    ISPctxt->gptIspHwOp->WriteRegMask(ISPctxt, addr, data, mask);
}

MMP_INLINE void
ISP_CMD_QUEUE_WAIT(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  sizeInByte)
{
    ISPctxt->gptIspHwOp->WaitQueue(sizeInByte);
}

MMP_INLINE void
ISP_CMD_QUEUE_FIRE(
    ISP_CONTEXT *ISPctxt)
{
    ISPctxt->gptIspHwOp->FireQueue();
}

MMP_INLINE void
ISP_FireIspQueue(
    ISP_CONTEXT *ISPctxt)
{
    ISPctxt->gptIspHwOp->FireIspQueue();
}

MMP_INLINE ISP_RESULT
ISP_WaitIspIdle(
    ISP_CONTEXT *ISPctxt)
{
    return ISPctxt->gptIspHwOp->WaitIspIdle(ISPctxt);
}

//=============================================================================
//                        Private Function Definition
//=============================================================================
static void _dummy_ReadRegister(MMP_UINT32 addr, MMP_UINT32 *data) {}

static void
_dummy_WriteRegisterMask(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  data,
    MMP_UINT32  mask)
{
    isp_msg(ISP_MSG_TYPE_ERR, "No Use ISP_WriteRegisterMask!\n");
}

static void _dummy_WaitQueue(MMP_UINT32 sizeInByte) {}
static void _dummy_FireQueue(void) {}
static void _dummy_FireIspQueue(void) {}
static ISP_RESULT _dummy_WaitIspIdle(ISP_CONTEXT *ISPctxt) { return ISP_SUCCESS; }

static void
_IspQ_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *data)
{
    IspQ_WaitIdle(0x0060, 0x0060);  // D[5][6]: S/W CQ Empty
    isp_ReadHwReg(addr, data);
}

static void
_IspQ_WriteRegister(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT16  addr,
    MMP_UINT16  data)
{
    IspQ_PackCommand(addr, data);
}

static void
_IspQ_WaitQueue(
    MMP_UINT32 sizeInByte)
{
    IspQ_Lock();    // for lock
    IspQ_WaitSize(sizeInByte);
}

static void
_IspQ_FireQueue(
    void)
{
    IspQ_Fire();
    IspQ_Unlock();  // for unlock
}

static void
_IspQ_FireIspQueue(
    void)
{
    isp_WriteHwReg(ISPQ_REG_FIRE_CMD, (MMP_UINT16)0x1);
}

static void
_CmdQ_WriteRegister(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32 addr,
    MMP_UINT32 data)
{
#ifdef CFG_CMDQ_ENABLE
    if (addr % 4)
    {
        isp_msg(ISP_MSG_TYPE_ERR, "%s addr must be in 4 byte alignment.\n", __FUNCTION__);
        return;
    }

    MMP_UINT32 ptr = ISPctxt->ispCore ? addr - ISP_CORE_1_BASE - ISP_REG_BASE : addr - ISP_REG_BASE;
    MMP_UINT32 cmdqbufAddr = (MMP_UINT32)(ISPctxt->CMDQ_BUF + ptr);
    *(MMP_UINT32 *)cmdqbufAddr = data;

    MMP_UINT32 *cmdqAddr = NULL;
    ithCmdQLock(ITH_CMDQ0_OFFSET);
    cmdqAddr = ithCmdQWaitSize(8, ITH_CMDQ0_OFFSET);
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, addr, *(MMP_UINT32 *)cmdqbufAddr);
    ithCmdQFlush(cmdqAddr, ITH_CMDQ0_OFFSET);
    ithCmdQUnlock(ITH_CMDQ0_OFFSET);
#endif
}

static void
_CmdQ_WriteRegisterMask(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  data,
    MMP_UINT32  mask)
{
#ifdef CFG_CMDQ_ENABLE
    if (addr % 4)
    {
        isp_msg(ISP_MSG_TYPE_ERR, "%s addr must be in 4 byte alignment.\n", __FUNCTION__);
        return;
    }

    MMP_UINT32 ptr = ISPctxt->ispCore ? addr - ISP_CORE_1_BASE - ISP_REG_BASE : addr - ISP_REG_BASE;
    MMP_UINT32 cmdqbufAddr = (MMP_UINT32)(ISPctxt->CMDQ_BUF + ptr);
    *(MMP_UINT32 *)cmdqbufAddr = (*(MMP_UINT32 *)cmdqbufAddr & ~mask) | (data & mask);

    MMP_UINT32 *cmdqAddr = NULL;
    ithCmdQLock(ITH_CMDQ0_OFFSET);
    cmdqAddr = ithCmdQWaitSize(8, ITH_CMDQ0_OFFSET);
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, addr, *(MMP_UINT32 *)cmdqbufAddr);
    ithCmdQFlush(cmdqAddr, ITH_CMDQ0_OFFSET);
    ithCmdQUnlock(ITH_CMDQ0_OFFSET);
#endif
}

static void
_MMIO_ReadRegister(
    MMP_UINT32 addr,
    MMP_UINT32 *data)
{
    isp_ReadHwReg(addr, data);
}

static void
_MMIO_WriteRegister(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  data)
{
#ifndef ACCESS_REG_DIRECT
    if (addr % 4)
    {
        isp_msg(ISP_MSG_TYPE_ERR, "%s addr must be in 4 byte alignment.\n", __FUNCTION__);
        return;
    }

    MMP_UINT32 ptr = ISPctxt->ispCore ? addr - ISP_CORE_1_BASE - ISP_REG_BASE : addr - ISP_REG_BASE;
    MMP_UINT32 cmdqbufAddr = (MMP_UINT32)(ISPctxt->CMDQ_BUF + ptr);
    *(MMP_UINT32 *)cmdqbufAddr = data;

    isp_WriteHwReg(addr, *(MMP_UINT32 *)cmdqbufAddr);
#else
    // update register without CMDQ buffer
    isp_WriteHwReg(addr, data);
#endif
}

static void
_MMIO_WriteRegisterMask(
    ISP_CONTEXT *ISPctxt,
    MMP_UINT32  addr,
    MMP_UINT32  data,
    MMP_UINT32  mask)
{
#ifndef ACCESS_REG_DIRECT
    if (addr % 4)
    {
        isp_msg(ISP_MSG_TYPE_ERR, "%s addr must be in 4 byte alignment.\n", __FUNCTION__);
        return;
    }

    MMP_UINT32 ptr = ISPctxt->ispCore ? addr - ISP_CORE_1_BASE - ISP_REG_BASE : addr - ISP_REG_BASE;
    MMP_UINT32 cmdqbufAddr = (MMP_UINT32)(ISPctxt->CMDQ_BUF + ptr);
    *(MMP_UINT32 *)cmdqbufAddr = (*(MMP_UINT32 *)cmdqbufAddr & ~mask) | (data & mask);

    isp_WriteHwReg(addr, *(MMP_UINT32 *)cmdqbufAddr);
#else
    // update register without CMDQ buffer
    isp_WriteHwRegMask(addr, data, mask);
#endif
}

static ISP_RESULT _MMIO_WaitIspIdle(ISP_CONTEXT *ISPctxt)
{
    ISP_RESULT result = ISP_SUCCESS;
    if (ISPctxt->OutInfo.EnableLcdOnFly == MMP_TRUE)
    {
        result = ISP_WaitISPChangeIdle();
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }
    }
    else
    {
        result = ISP_WaitEngineIdle(ISPctxt);
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }
    }
end:
    return result;
}