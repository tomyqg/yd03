/*
 * Copyright (c) 2018 ITE Technology Corp. All Rights Reserved.
 */
/**
 * @file i2s_hw.c
 * I2S Hardware operation API (IT9070)
 *
 * @author Hanxuan Li
 * @author I-Chun Lai
 *
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
#include "i2s_hw.h"
#include "i2s_reg.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================================================================
//                              Macro Definition
//=============================================================================
#ifdef _WIN32
    #define asm(s)
#endif
//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

//=============================================================================
//                              Global Data Definition
//=============================================================================
uint16_t _regTxBufAddr[I2S_TX_BUF_COUNT] =
{
    I2S_REG_TX_BUF_1_ADDR,
    I2S_REG_TX_BUF_2_ADDR,
    I2S_REG_TX_BUF_3_ADDR,
    I2S_REG_TX_BUF_4_ADDR,
    I2S_REG_TX_BUF_5_ADDR,
    I2S_REG_TX_BUF_6_ADDR,
};

uint16_t    _regRxBufAddr[I2S_RX_BUF_COUNT] =
{
    I2S_REG_RX_BUF_1_ADDR
};

//=============================================================================
//                              Private Funtion Declaration
//=============================================================================
static inline uint16_t
_REG_Read16(
    uint16_t addr16)
{
    return ithReadRegH(addr16);
}

static inline void
_REG_Write16(
    uint16_t    addr16,
    uint16_t    data16)
{
    ithWriteRegH(addr16, data16);
}

static inline void
_REG_WriteMask16(
    uint16_t    addr16,
    uint16_t    data16,
    uint16_t    mask16)
{
    ithWriteRegMaskH(addr16, data16, mask16);
}

static inline void
_REG_SetBit16(
    uint16_t        addr,
    unsigned int    bit)
{
    ithSetRegBitH(addr, bit);
}

static inline void
_REG_ClrBit16(
    uint16_t        addr,
    unsigned int    bit)
{
    ithClearRegBitH(addr, bit);
}

static inline uint32_t
_REG_Read32(
    uint16_t addr16)
{
    /* must be low then hi for hw design */
    uint32_t    dataL   = _REG_Read16(addr16);
    uint32_t    dataH   = _REG_Read16(addr16 + 2);

    return (dataH << 16 ) | dataL;
}

static inline void
_REG_Write32(
    uint16_t    addr16,
    uint32_t    data32)
{
    /* must be low then hi for hw design */
    ithWriteRegH(   addr16,     (uint16_t)((data32 >> 0) & 0xFFFF));
    ithWriteRegH(   addr16 + 2, (uint16_t)((data32 >> 16) & 0xFFFF));
}

static inline void
_Delay_US(
    uint32_t us)
{
    ithDelay(us);
}

//=============================================================================
//                              Public Funtion Definition
//=============================================================================
void
_I2S_SetTxBufAddr(
    I2S_TX_BUF_ID   txBufId,
    uint32_t        addr32)
{
    addr32 = ithSysAddr2VramAddr((void*)addr32);
    assert( (I2S_TX_BUF_1 <= txBufId) && (txBufId < I2S_TX_BUF_COUNT));
    assert( ITH_IS_ALIGNED(addr32, 32));
    assert( addr32 > 0);

    _REG_Write32(_regTxBufAddr[txBufId], addr32);
}

uint32_t
_I2S_GetTxBufAddr(
    I2S_TX_BUF_ID txBufId)
{
    assert((I2S_TX_BUF_1 <= txBufId) && (txBufId < I2S_TX_BUF_COUNT));

    return (uint32_t)ithVramAddr2SysAddr(_REG_Read32(_regTxBufAddr[txBufId]));
}

void
_I2S_SetTxBufSize(
    uint32_t size32)
{
    assert( ITH_IS_ALIGNED(size32, 32));
    assert( size32 > 0);

    /* NOTE: minus one for hardware design */
    _REG_Write32(I2S_REG_TX_BUF_SIZE, size32 - 1);
}

uint32_t
_I2S_GetTxBufSize()
{
    /* NOTE: add one for hardware design */
    return _REG_Read32(I2S_REG_TX_BUF_SIZE) + 1;
}

void
_I2S_SetTxBufWrPtr(
    uint32_t ptr32)
{
    _REG_Write32(I2S_REG_TX_BUF_W_PTR, ptr32);
}

uint32_t
_I2S_GetTxBufWrPtr()
{
    return _REG_Read32(I2S_REG_TX_BUF_W_PTR);
}

uint32_t
_I2S_GetTxBufRdPtr()
{
    return _REG_Read32(I2S_REG_TX_BUF_R_PTR);
}

void
_I2S_SetRxBufAddr(
    I2S_RX_BUF_ID   rxBufId,
    uint32_t addr32)
{
    addr32 = ithSysAddr2VramAddr((void*)addr32);

    assert( ITH_IS_ALIGNED(addr32, 32));
    assert( addr32 > 0);

    _REG_Write32(_regRxBufAddr[rxBufId], addr32);
}

uint32_t
_I2S_GetRxBufAddr(
    I2S_RX_BUF_ID rxBufId)
{
    return (uint32_t)ithVramAddr2SysAddr(_REG_Read32(_regRxBufAddr[rxBufId]));
}

void
_I2S_SetRxBufSize(
    uint32_t size32)
{
    assert( ITH_IS_ALIGNED(size32, 32));
    assert( size32 > 0);
    assert( size32 <= 0x10000);

    /* NOTE: minus one for hardware design */
    _REG_Write16(I2S_REG_RX_BUF_SIZE, size32 - 1);
}

uint32_t
_I2S_GetRxBufSize()
{
    /* NOTE: add one for hardware design */
    return _REG_Read16(I2S_REG_RX_BUF_SIZE) + 1;
}

void
_I2S_SetRxBufRdPtr(
    uint16_t ptr16)
{
    _REG_Write16(I2S_REG_RX_BUF_R_PTR, ptr16);
}

uint16_t
_I2S_GetRxBufRdPtr(
    void)
{
    return _REG_Read16(I2S_REG_RX_BUF_R_PTR);
}

uint16_t
_I2S_GetRxBufWrPtr(
    void)
{
    return _REG_Read16(I2S_REG_RX_BUF_W_PTR);
}

void
_I2S_DisableTx()
{
    const uint16_t mask = I2S_MSK_TX_ENABLE | I2S_MSK_TX_ENABLE;

    _REG_WriteMask16(I2S_REG_TX_CTRL2, 0x0000, mask);
    while (_REG_Read16(I2S_REG_TX_CTRL2) & mask)
        asm ("");
}

void
_I2S_DisableRx()
{
    _REG_WriteMask16(I2S_REG_RX_CTRL1, 0x0000,
                     I2S_MSK_RX_ENABLE | I2S_MSK_RX_START);
}

void
_I2S_EnableTxFlush()
{
    _REG_SetBit16(I2S_REG_TX_CTRL2, I2S_POS_TX_EN_FLUSH);
}

void
_I2S_DisableTxFlush()
{
    _REG_ClrBit16(I2S_REG_TX_CTRL2, I2S_POS_TX_EN_FLUSH);
}

bool
_I2S_IsTxFlushEnabled()
{
    return !!(_REG_Read16(I2S_REG_TX_CTRL2) & I2S_MSK_TX_EN_FLUSH);
}

void
_I2S_SetTxLowWaterMark(
    uint32_t data32)
{
    _REG_Write32(I2S_REG_TX_L_WATER_MARK, data32);
}

void
_I2S_SetRxHighWaterMark(
    uint32_t data32)
{
    assert(data32 < 0x10000);

    _REG_WriteMask16(I2S_REG_RX_CTRL2,
                     (uint16_t)(data32 & I2S_MSK_RX_H_WATER_MARK),
                     I2S_MSK_RX_H_WATER_MARK);
}

void
_I2S_EnableFading(
    uint32_t fading_step,
    uint32_t fading_duration)
{
    DEBUG_PRINT("I2S# %s\n", __func__);
    _REG_WriteMask16(I2S_REG_FADE_IN_OUT_CTRL,
        ((1               & N01_BITS_MSK) << I2S_POS_EN_FADE_IN_OUT)
      | ((fading_step     & N04_BITS_MSK) << I2S_POS_FADE_STEP)
      | ((fading_duration & N08_BITS_MSK) << I2S_POS_FADE_CYCLE),
        I2S_POS_EN_FADE_IN_OUT | I2S_POS_FADE_STEP | I2S_POS_FADE_CYCLE);
}

void
_I2S_DisableFading(void)
{
    DEBUG_PRINT("I2S# %s\n", __func__);
    _REG_ClrBit16(I2S_REG_FADE_IN_OUT_CTRL, I2S_POS_EN_FADE_IN_OUT);
}

//=============================================================================
//                              Private Funtion Definition
//=============================================================================
