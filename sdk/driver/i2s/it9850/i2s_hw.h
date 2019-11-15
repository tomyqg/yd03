/*
 * Copyright (c) 2018 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * I2S Hardware operation API (IT9850)
 *
 * @author Hanxuan Li
 * @author I-Chun Lai
 *
 * @version 0.1
 */
#ifndef __I2S_HW_H__
#define __I2S_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <assert.h>
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum I2S_TX_BUF_ID
{
    I2S_TX_BUF_1,
    I2S_TX_BUF_COUNT
} I2S_TX_BUF_ID;

typedef enum I2S_RX_BUF_ID
{
    I2S_RX_BUF_1,
    I2S_RX_BUF_COUNT
} I2S_RX_BUF_ID;

//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================================================================
//                              Macro Definition
//=============================================================================
//=============================================================================
//                              Funtion Declaration
//=============================================================================
void
_I2S_SetTxBufAddr(
    I2S_TX_BUF_ID   txBufId,
    uint32_t        addr32);

uint32_t
_I2S_GetTxBufAddr(
    I2S_TX_BUF_ID   txBufId);

void 
_I2S_SetTxBufSize(
    uint32_t        size32);

uint32_t
_I2S_GetTxBufSize();

void
_I2S_SetTxBufWrPtr(
    uint32_t ptr32);

uint32_t
_I2S_GetTxBufWrPtr();

uint32_t
_I2S_GetTxBufRdPtr();

void
_I2S_SetRxBufAddr(
    I2S_RX_BUF_ID   rxBufId,
    uint32_t        addr32);

uint32_t
_I2S_GetRxBufAddr(
    I2S_RX_BUF_ID rxBufId);

void
_I2S_SetRxBufSize(
    uint32_t        size32);

uint32_t
_I2S_GetRxBufSize();

void
_I2S_SetRxBufRdPtr(
    uint32_t ptr32);

uint32_t
_I2S_GetRxBufRdPtr(
    void);

uint32_t
_I2S_GetRxBufWrPtr(
    void);

void
_I2S_DisableRx();

void
_I2S_DisableTx();

void
_I2S_EnableTxFlush();

void
_I2S_DisableTxFlush();

bool
_I2S_IsTxFlushEnabled();

void
_I2S_SetTxLowWaterMark(
    uint32_t data32);

void
_I2S_SetRxHighWaterMark(
    uint32_t data32);

void 
_I2S_EnableFading(
    uint32_t fading_step, 
    uint32_t fading_duration);

void 
_I2S_DisableFading(void);

#ifdef __cplusplus
}
#endif

#endif