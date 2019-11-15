/*
 * Copyright (c) 2018 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * I2S Driver API (IT9910)
 *
 * @author Hanxuan Li
 * @author I-Chun Lai
 *
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "i2s/i2s_it9910.h"
#include "i2s_reg.h"
#include "i2s_hw.h"
//#define USE_WM8960_ADC

//=============================================================================
//                              Constant Definition
//=============================================================================
/* ************************************************************************** */
/* platform control */

/* wrapper */
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================================================================
//                              Macro Definition
//=============================================================================
#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);           } while(0)
//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static int _i2s_DA_running = 0;
static int _i2s_AD_running = 0;
static pthread_mutex_t I2S_MUTEX = PTHREAD_MUTEX_INITIALIZER;

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
    uint32_t dataL = _REG_Read16(addr16);
    uint32_t dataH = _REG_Read16(addr16 + 2);

    return (dataH << 16 ) | dataL;
}

static inline void
_REG_Write32(
    uint16_t addr16,
    uint32_t data32)
{
    /* must be low then hi for hw design */
    ithWriteRegH(addr16,     (uint16_t)((data32 >>  0) & 0xFFFF));
    ithWriteRegH(addr16 + 2, (uint16_t)((data32 >> 16) & 0xFFFF));
}

static void _i2s_power_on(void);
static void _i2s_power_off(void);
static void _i2s_reset(void);
static void _i2s_aws_sync(void);
static void _i2s_set_clock(u32 demanded_sample_rate);
static void _i2s_setup_io_direction_9910_only(STRC_I2S_SPEC *i2s_spec);

/* export APIs */
void i2s_volume_up(void);
void i2s_volume_down(void);
void i2s_pause_ADC(int pause);
void i2s_pause_DAC(int pause);
void i2s_deinit_ADC(void);
void i2s_deinit_DAC(void);
void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec);
void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec);
void i2s_mute_DAC(int mute);
int i2s_set_direct_volstep(unsigned volstep);
int i2s_set_direct_volperc(unsigned volperc);
unsigned i2s_get_current_volstep(void);
unsigned i2s_get_current_volperc(void);
void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);

//=============================================================================
//                              Public Funtion Definition
//=============================================================================
u32 I2S_DA32_GET_RP(void)
{
    return _I2S_GetTxBufRdPtr();
}

u32 I2S_DA32_GET_WP(void)
{
    return _I2S_GetTxBufWrPtr();
}

void I2S_DA32_SET_WP(u32 data32)
{
    _I2S_SetTxBufWrPtr(data32);
}

u32 I2S_AD32_GET_RP(void) { return _I2S_GetRxBufRdPtr(); }
u32 I2S_AD32_GET_WP(void) { return _I2S_GetRxBufWrPtr(); }
void I2S_AD32_SET_RP(u32 data32) { _I2S_SetRxBufRdPtr(data32); }

static void _i2s_power_on(void)
{
    if(_i2s_DA_running || _i2s_AD_running) {
        DEBUG_PRINT("I2S# power on already, DA:%d, AD:%d\n", _i2s_DA_running, _i2s_AD_running);
        return;
    }

    /* enable audio clock for I2S modules */
    /* NOTE: we SHOULD enable audio clock before toggling 0x003A/3C/3E */
    _REG_WriteMask16(ITH_AUDIO_CLK_REG, 0xFFFF,
        ITH_AUDIO_EN_AMCLK_MSK  /* enable AMCLK (system clock for wolfson chip) */
      | ITH_AUDIO_EN_ZCLK_MSK   /* enable ZCLK  (IIS DAC clock)                 */
      | ITH_AUDIO_EN_M8CLK_MSK  /* enable M8CLK (memory clock for DAC)          */
      | ITH_AUDIO_EN_M9CLK_MSK);/* enable M9CLK (memory clock for ADC)          */

    /* enable PLL3 for audio */
    {
        _REG_ClrBit16(ITH_PLL3_SET3_REG, ITH_PLL1_PWR_STATE_POS);
        _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);
        do
        {
            if (_REG_Read16(ITH_PLL_LOCK_STATUS_REG) & ITH_PLL3_LOCK_MSK)
                break;
            i2s_delay_us(10);
        } while(1);
    }

    DEBUG_PRINT("I2S# %s\n", __func__);
}

static void _i2s_power_off(void)
{
    if(_i2s_DA_running || _i2s_AD_running) {
        DEBUG_PRINT("I2S# module still running, skip power-off, DA:%d, AD:%d\n", _i2s_DA_running, _i2s_AD_running);
        return;
    }

    /* disable audio clock for I2S modules */
    _REG_WriteMask16(ITH_AUDIO_CLK_REG, 0x0000,
        ITH_AUDIO_EN_AMCLK_MSK  /* disable AMCLK (system clock for wolfson chip) */
      | ITH_AUDIO_EN_ZCLK_MSK   /* disable ZCLK  (IIS DAC clock)                 */
      | ITH_AUDIO_EN_M8CLK_MSK  /* disable M8CLK (memory clock for DAC)          */
      | ITH_AUDIO_EN_M9CLK_MSK);/* disable M9CLK (memory clock for ADC)          */

    DEBUG_PRINT("I2S# %s\n", __func__);
}

static void _i2s_reset(void)
{
    if(_i2s_DA_running) {
        DEBUG_PRINT("I2S# DAC is running, skip reset I2S !\n");
        return;
    }
    if(_i2s_AD_running) {
        DEBUG_PRINT("I2S# ADC is running, skip reset I2S !\n");
        return;
    }

    i2s_delay_us(1000); /* FIXME: dummy loop */;
    _REG_SetBit16(ITH_AUDIO_CLK_REG, ITH_AUDIO_RESET_I2S_POS);
    i2s_delay_us(1000); /* FIXME: dummy loop */;
    _REG_ClrBit16(ITH_AUDIO_CLK_REG, ITH_AUDIO_RESET_I2S_POS);
    i2s_delay_us(1000); /* FIXME: dummy loop */;

    I2S_DA32_SET_WP(0);
    I2S_AD32_SET_RP(0);

    DEBUG_PRINT("I2S# %s\n", __func__);
}

static void _i2s_aws_sync(void)
{
    /* AWS & ZWS re-sync */
    _REG_SetBit16(I2S_REG_RX_CTRL, I2S_POS_RESET_WS);
    i2s_delay_us(1000); /* FIXME: dummy loop */;
    _REG_ClrBit16(I2S_REG_RX_CTRL, I2S_POS_RESET_WS);
}

/***********************************************************************************
** Set CODEC Clock
** Assume the oversample rate is 256.
** Suggest system clock is (int)(CLK*2/256/sample_rate+1) / 2 * 256 * sample_rate

** For example:
** In 44100 sample rate, the system clock is 33MHz, the system clock should be
** (33000000*2/256/44100+1) / 2 * 256 * 44100 = 33868800.

** The PLL numurator is (int)(33868800 * 2 * 512 / 13000000 + 1) / 2 - 1 = 0x535.
***********************************************************************************/
static void _i2s_set_clock(u32 demanded_sample_rate)
{
    uint32_t target_sample_rate = 0;
    uint16_t amclk_ratio = 0;
    uint16_t zclk_ratio = 0;

    if(_i2s_DA_running) {
        DEBUG_PRINT("I2S# DAC is running, skip set clock !\n");
        return;
    }
    if(_i2s_AD_running) {
        DEBUG_PRINT("I2S# ADC is running, skip set clock !\n");
        return;
    }

    printf("I2S# %s, demanded_sample_rate: %u\n", __func__, demanded_sample_rate);

    if     ((demanded_sample_rate > 45600) && (demanded_sample_rate < 50400)) { target_sample_rate = 48000; }
    else if((demanded_sample_rate > 41895) && (demanded_sample_rate < 46305)) { target_sample_rate = 44100; }
    else if((demanded_sample_rate > 30400) && (demanded_sample_rate < 33600)) { target_sample_rate = 32000; }
    else if((demanded_sample_rate > 22800) && (demanded_sample_rate < 25200)) { target_sample_rate = 24000; }
    else if((demanded_sample_rate > 20900) && (demanded_sample_rate < 23100)) { target_sample_rate = 22050; }
    else if((demanded_sample_rate > 15200) && (demanded_sample_rate < 16800)) { target_sample_rate = 16000; }
    else if((demanded_sample_rate > 11400) && (demanded_sample_rate < 12600)) { target_sample_rate = 12000; }
    else if((demanded_sample_rate > 10450) && (demanded_sample_rate < 11550)) { target_sample_rate = 11025; }
    else if((demanded_sample_rate >  7600) && (demanded_sample_rate <  8400)) { target_sample_rate =  8000; }
    else
    {
        printf("ERROR# I2S, invalid demanded_sample_rate !\n");
        target_sample_rate = 48000;
    }

    printf("I2S# %s, target_sample_rate: %u\n", __func__, target_sample_rate);

    switch (target_sample_rate)
    {
    case 48000:
    case 24000:
    case 12000:
        {
            /* Xtal: 12 MHz, 48000 Hz */
            /* A:4 B:16.5 C:00811 D:01 E:066 PLLo:12287997.159091 diff:-2.840909 osc:811.008000 */
            _REG_Write16(ITH_PLL3_SET1_REG, 0x9001);
            _REG_Write16(ITH_PLL3_SET2_REG, 0x4040); /* AMCLK select PLL3_OUT? */
            _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL3_SET4_REG, 0x0189);
            _REG_Write16(ITH_PLL3_SET3_REG, 0x3000);
            i2s_delay_us(100); /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);

            switch (target_sample_rate)
            {
            case 48000:
                amclk_ratio = 1 - 1; /* AMCLK = 12.288MHz (256x48KHz)      */
                zclk_ratio  = 4 - 1; /* ZCLK  = 3.07 MHz (zclk ratio=0x1F) */
                break;

            case 24000:
                amclk_ratio = 2 - 1; /* AMCLK = 6.143 MHz */
                zclk_ratio  = 8 - 1; /* ZCLK  = 1.536 MHz */
                break;

            case 12000:
                amclk_ratio =  4 - 1; /* AMCLK = 3.072MHz */
                zclk_ratio  = 16 - 1; /* ZCLK  = 0.768MHz */
                break;
            }
        }
        break;

    case 44100:
    case 22050:
    case 11025:
        {
            /* Xtal: 12 MHz, 44100 Hz */
            /* A:4 B:16.5 C:00408 D:01 E:071 PLLo:11289612.676056 diff:12.676056 osc:801.561600 */
            _REG_Write16(ITH_PLL3_SET1_REG, 0x9001);
            _REG_Write16(ITH_PLL3_SET2_REG, 0x4747); /* AMCLK select PLL3_OUT? */
            _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL3_SET4_REG, 0x3198);
            _REG_Write16(ITH_PLL3_SET3_REG, 0x3000);
            i2s_delay_us(100); /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);

            switch (target_sample_rate)
            {
            case 44100:
                amclk_ratio = 1 - 1; /* AMCLK = 11.29MHz (256x44.1KHz)      */
                zclk_ratio  = 4 - 1; /* ZCLK  = 2.82MHz (zclk ratio=0x1F)   */
                break;

            case 22050:
                amclk_ratio = 2 - 1; /* AMCLK = 5.645MHz */
                zclk_ratio  = 8 - 1; /* ZCLK  = 1.41 MHz */
                break;

            case 11025:
                amclk_ratio =  4 - 1; /* AMCLK = 2.82MHz */
                zclk_ratio  = 16 - 1; /* ZCLK  = 0.70MHz */
                break;
            }
        }
        break;

    case 32000:
    case 16000:
    case  8000:
        {
            /* Xtal: 12 MHz, 32000 Hz */
            /* A:4 B:16.5 C:00112 D:01 E:097 PLLo:8192010.309278 diff:10.309278 osc:794.624000 */
            _REG_Write16(ITH_PLL3_SET1_REG, 0x9001);
            _REG_Write16(ITH_PLL3_SET2_REG, 0x6161); /* AMCLK select PLL3_OUT? */
            _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL3_SET4_REG, 0x3070);
            _REG_Write16(ITH_PLL3_SET3_REG, 0x3000);
            i2s_delay_us(100); /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL3_SET3_REG, 0x2000);

            switch (target_sample_rate)
            {
            case 32000:
                amclk_ratio = 1 - 1; /* AMCLK = 8.192MHz (256x32KHz)       */
                zclk_ratio  = 4 - 1; /* ZCLK  = 2.048MHz (zclk ratio=0x1F) */
                break;

            case 16000:
                amclk_ratio = 2 - 1; /* AMCLK = 4.096MHz */
                zclk_ratio  = 8 - 1; /* ZCLK  = 1.024MHz */
                break;

            case 8000:
                amclk_ratio =  4 - 1; /* AMCLK = 2.048MHz */
                zclk_ratio  = 16 - 1; /* ZCLK  = 0.512MHz */
                break;
            }
        }
        break;

    default:
        printf("ERROR# I2S@ invalid target_sample_rate !\n");
        break;
    }

#ifdef I2S_DEBUG_SET_CLOCK
    printf("I2S# %s, amclk_ratio: 0x%04x, zclk_ratio: 0x%04x\n", __func__, amclk_ratio, zclk_ratio);
#endif

    /* setup 0x003A/0x003C */
    {
    int i;
#ifdef I2S_DEBUG_SET_CLOCK
        printf("I2S# %s, setup 0x003A/0x003C !\n", __func__);
#endif

        /* AMCLK divider settings */
        {
            /* PART 1: make sure the divider counter equals to zero */
            _REG_ClrBit16(   ITH_AUDIO_AMCLK_REG, ITH_AUDIO_EN_AMCLK_DIV_POS);
            _REG_WriteMask16(ITH_AUDIO_AMCLK_REG, 0x0000, ITH_AUDIO_AMCLK_DIV_RATIO_MSK);
            _REG_SetBit16(   ITH_AUDIO_AMCLK_REG, ITH_AUDIO_EN_AMCLK_DIV_POS);

            for(i=0; i<100; i++) { asm(""); }
            i2s_delay_us(100); /* unit: micro-sec (us) */

            /* PART 2: hardware asynchronous control */
            _REG_ClrBit16(   ITH_AUDIO_AMCLK_REG, ITH_AUDIO_EN_AMCLK_DIV_POS);

            /* bit 0 ~ 6, AMCLK divide ratio */
            _REG_WriteMask16(ITH_AUDIO_AMCLK_REG,
                amclk_ratio << ITH_AUDIO_AMCLK_DIV_RATIO_POS,
                ITH_AUDIO_AMCLK_DIV_RATIO_MSK);

            _REG_SetBit16(   ITH_AUDIO_AMCLK_REG, ITH_AUDIO_EN_AMCLK_DIV_POS);
        }

        /* ZCLK divider settings */
        {
            /* PART 1: make sure the divider counter equals to zero */
            _REG_ClrBit16(   ITH_AUDIO_ZCLK_REG, ITH_AUDIO_EN_ZCLK_DIV_POS);
            _REG_WriteMask16(ITH_AUDIO_ZCLK_REG, 0x0000, ITH_AUDIO_ZCLK_DIV_RATIO_MSK);
            _REG_SetBit16(   ITH_AUDIO_ZCLK_REG, ITH_AUDIO_EN_ZCLK_DIV_POS);

            for(i=0; i<100; i++) { asm(""); }
            i2s_delay_us(100); /* unit: micro-sec (us) */

            /* PART 2: hardware asynchronous control */
            _REG_ClrBit16(   ITH_AUDIO_ZCLK_REG, ITH_AUDIO_EN_ZCLK_DIV_POS);

            /* bit 0 ~ 9, ZCLK divide ratio */
            _REG_WriteMask16(ITH_AUDIO_ZCLK_REG,
                zclk_ratio << ITH_AUDIO_ZCLK_DIV_RATIO_POS,
                ITH_AUDIO_ZCLK_DIV_RATIO_MSK);

            _REG_SetBit16(   ITH_AUDIO_ZCLK_REG, ITH_AUDIO_EN_ZCLK_DIV_POS);
        }
    }

    /* dummy loop to wait clock divider stablized */
    i2s_delay_us(1000); /* FIXME: dummy loop */;
}

static void _i2s_setup_io_direction_9910_only(STRC_I2S_SPEC *i2s_spec)
{
    u16 data16;
    if(i2s_spec->use_hdmirx) /* NOTE: i2s played the slave role */
    {
        if(i2s_spec->internal_hdmirx)
        {
            if (i2s_spec->use_hdmitx)
                _REG_Write16(I2S_REG_IO_SET, 0x000C);
            else
                _REG_Write16(I2S_REG_IO_SET, 0x00BC);

            _REG_Write16(I2S_REG_IOMUX_CTRL, 0x0000);

        }
        else /* ext. hdmirx */
        {
            if (i2s_spec->use_hdmitx)
                _REG_Write16(I2S_REG_IO_SET, 0x00BF);
            else
                _REG_Write16(I2S_REG_IO_SET, 0x00BF);

            _REG_Write16(I2S_REG_IO_SET, 0x0000);
        }
    }
    else /* no use hdmirx */
    {
        if(i2s_spec->slave_mode)
        {
            _REG_Write16(I2S_REG_IO_SET, 0x00BB);
            _REG_Write16(I2S_REG_IOMUX_CTRL, 0x0200); /* [9:8] pinZD0 select IO's DSD_I */
        }
        else /* i2s master (Wolfson & Cirrus slave mode) */
        {
            _REG_Write16(I2S_REG_IO_SET, 0x22B8);
#if defined (IT9919_144TQFP)
    #if (defined(REF_BOARD_AVSENDER) || defined(EVB_BOARD))
            _REG_Write16(I2S_REG_IOMUX_CTRL, 0x0000); /* [9:8] pinZD0 select IO's ZD0 */
    #endif
#else
            _REG_Write16(I2S_REG_IOMUX_CTRL, 0x0000); /* [9:8] pinZD0 select IO's DSD_I */
#endif
        }
    }
    //disable no-use pin
    data16 = _REG_Read16(I2S_REG_IO_SET);
    data16 &= ~((1 << 5)|(1 << 7));
    _REG_Write16(I2S_REG_IO_SET, data16);
}

void i2s_volume_up(void)
{
    DEBUG_PRINT("I2S# %s\n", __func__);

    itp_codec_playback_amp_volume_up();
}

void i2s_volume_down(void)
{
    DEBUG_PRINT("I2S# %s\n", __func__);

    itp_codec_playback_amp_volume_down();
}

void i2s_pause_ADC(int pause)
{
    printf("I2S# %s(%d)\n", __func__, pause);
    _REG_WriteMask16(I2S_REG_RX_CTRL1,
        (!pause) << I2S_POS_RX_START,
        I2S_MSK_RX_START);
}

void i2s_pause_DAC(int pause)
{
    printf("I2S# %s(%d)\n", __func__, pause);

    if(pause)
    {
        //_I2S_EnableFading(0xF, 0x01); /* fast response */
        _REG_Write16(I2S_REG_DIG_OUT_VOL, 0x0000);
        _REG_ClrBit16(I2S_REG_TX_CTRL2, I2S_POS_TX_START);
    }
    else /* resume */
    {
        //_I2S_EnableFading(0x1, 0xFF); /* slow response */
        _REG_Write16(I2S_REG_DIG_OUT_VOL, 0xFF00);
        _REG_SetBit16(I2S_REG_TX_CTRL2, I2S_POS_TX_START);
    }
}

void i2s_deinit_ADC(void)
{
    if (!_i2s_AD_running) {
        DEBUG_PRINT("I2S# ADC is 'NOT' running, skip deinit ADC !\n");
        return;
    }

    printf("I2S# %s +\n", __func__);
    pthread_mutex_lock(&I2S_MUTEX);

    /* disable hardware I2S */
    _I2S_DisableRx();

    itp_codec_rec_deinit();

    _i2s_AD_running = 0; /* put before _i2s_reset() */
    _i2s_reset();
    _i2s_power_off();

    pthread_mutex_unlock(&I2S_MUTEX);
    printf("I2S# %s -\n", __func__);
}

void i2s_deinit_DAC(void)
{
    u16 out_status_1;
    u16 out_status_2;
    u32 i2s_idle;
    u32 i2s_memcnt;

    pthread_mutex_lock(&I2S_MUTEX);

    if (!_i2s_DA_running) {
        DEBUG_PRINT("I2S# DAC is 'NOT' running, skip deinit DAC !\n");
        pthread_mutex_unlock(&I2S_MUTEX);
        return;
    }

    printf("I2S# %s +\n", __func__);

    itp_codec_playback_deinit();

    /* disable I2S_OUT_FIRE & I2S_OUTPUT_EN */
    _I2S_DisableTx();

    do
    {
        i2s_delay_us(1000); /* FIXME: dummy loop */;

        out_status_1 = _REG_Read16(I2S_REG_TX_STATUS1);
        out_status_2 = _REG_Read16(I2S_REG_TX_STATUS2);

//      printf("I2S# OUTPUT_STATUS 1,2: 0x%04x,0x%04x\n", out_status_1, out_status_2);
        i2s_idle   =  out_status_1 & 0x1;
        i2s_memcnt = (out_status_2 >> 6) & 0x1F;
    } while(!i2s_idle || (i2s_memcnt != 0));

    _i2s_DA_running = 0; /* put before _i2s_reset() */
    _i2s_reset();
    _i2s_power_off();

    printf("I2S# %s -\n", __func__);
    pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec)
{
    int param_fail = 0;
    u16 data16;
    u8 resolution_type;

    pthread_mutex_lock(&I2S_MUTEX);

    if(_i2s_DA_running) {
        DEBUG_PRINT("I2S# DAC is running, skip re-init !\n");
        pthread_mutex_unlock(&I2S_MUTEX);
        return;
    }

    printf("I2S# %s +\n", __func__);

    /* check buffer base/size alignment */
    if((u32)i2s_spec->base_out_i2s_spdif % 8) { param_fail = 1; }
    if(i2s_spec->buffer_size % 8) { param_fail = 4; }

    /* check channels number */
    if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) { param_fail = 5; }

    if(param_fail) {
        printf("ERROR# I2S, param_fail = %d\n", param_fail);
        pthread_mutex_unlock(&I2S_MUTEX);
        return;
    }

    _i2s_power_on();

    _i2s_set_clock(i2s_spec->sample_rate);
    _i2s_reset();
    itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
    _i2s_DA_running = 1;

    /* mastor mode & AWS & ZWS sync */
    switch(i2s_spec->sample_size) {
        case  8: { resolution_type = 3; break; }
        case 16: { resolution_type = 0; break; }
        case 24: { resolution_type = 1; break; }
        case 32:
        default: { resolution_type = 2; break; }
    }

    _REG_WriteMask16(I2S_REG_RX_CTRL,
        0x1F << I2S_POS_RX_CH_LEN,
        I2S_MSK_RX_CH_LEN);

    data16 = (1                         << I2S_POS_SEL_DAC_SRC) /* DAC device selection, 0:External DAC; 1:Internal DAC */
    | (((i2s_spec->slave_mode) ? 1 : 0) << I2S_POS_TX_MODE)     /* 0:Master(ZCLK/ZWS output mode); 1:Slave(ZCLK/ZWS input mode) */
    | (resolution_type                  << I2S_POS_TX_DATA_FMT) /* DAC resolution bits, 00:16bits; 01:24bits; 10:32bits */
    | (0x1F                             << I2S_POS_TX_CH_LEN);  /* sample width (in bits unit) */

    _REG_Write16(I2S_REG_TX_CTRL, data16);

    /* Big Endian Type (for IIS Input and IIS Output use) */
    if(i2s_spec->is_big_endian)
    {
        if ((24 == i2s_spec->sample_size) || (32 == i2s_spec->sample_size))
        {
            _REG_SetBit16(I2S_REG_ADDA_PCM, I2S_POS_DAT_FMT);
        }
        else
        {
            _REG_ClrBit16(I2S_REG_ADDA_PCM, I2S_POS_DAT_FMT);
        }
    }

    /* I2S I/O direction for 9910 only */
    _i2s_setup_io_direction_9910_only(i2s_spec);

    /* I2S Engine IO Mode for 9910 only */
    /* DA only: REG[0x1644] B'7 = 0 ; AD only: REG[0x1644] B'7 = 1 */   
    _REG_ClrBit16(I2S_REG_ADDA_PCM, I2S_POS_SEL_IO_MODE);    
    
    _i2s_aws_sync();
    itp_codec_playback_init(2);

    /* buffer base */
    {
        /* buf 1 (IIS / SPDIF) */
        _I2S_SetTxBufAddr(I2S_TX_BUF_1, (uint32_t)i2s_spec->base_i2s);

        /* SPDIF data channel select */
        _REG_ClrBit16(I2S_REG_SPDIF_VOL, I2S_POS_SEL_SPDIF_DATA_CHANNEL);  
    }

    /* buffer length */
    _I2S_SetTxBufSize(i2s_spec->buffer_size);

    /* DA interrupt threshold: (value = remnant data) to starvation */
    _I2S_SetTxLowWaterMark(0x20040);    /* TODO: need reasonable threshold */

    /* output path */
    {
        _REG_Write16(I2S_REG_TX_CTRL1,
        ( (0 << 15) /* Probe signal select[1] */
        | (0 << 14) /* Probe signal select[0] */
        | (0 << 13) /* Probe high 32bit and low 32bit swap */
        | (1 << 12) /* Enable Probe signal debug */
        | (1 <<  8) /* Enable DA RdWrGap Interrupt */
        | (1 <<  0) /* Enable IIS/SPDIF Data Output */
        ));
    }

#ifdef ENABLE_ITV_AUDIO_BIST_MODE
    /* HDMI audio Functional Test */
    _REG_Write16(0x1676, 0x0000); /* SynWordL */
    _REG_Write16(0x1678, 0x0000); /* SynWordR */
    _REG_Write16(0x167A, 0x0011); /* step_EnHDMI */
#endif

    _REG_Write16(I2S_REG_TX_CTRL2,
    (  0                                 << 6) /* Output Channel active level, 0:Low for Left; 1:High for Left */
    | (0                                 << 5) /* Output Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
    | ((i2s_spec->channels - 1)          << 4) /* Output Channel Select, 0:Single Channel; 1:Two Channels */
    | ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
    | (0                                 << 2) /* 0:NOT_LAST_WPTR; 1:LAST_WPTR */
    | (1                                 << 1) /* Fire the IIS for Audio Output */
    );

    i2s_delay_us(1000); /* FIXME: dummy loop */;

    if (!i2s_spec->postpone_audio_output)
        i2s_pause_DAC(0); /* Enable Audio Output */

    printf("I2S# %s -\n", __func__);
    pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec)
{
    u16 data16;
    u8 resolution_type;
    u16 chipVer = 0;

    chipVer = ithReadRegH(ITH_REVISION_ID_REG);
    if(_i2s_AD_running) {
        DEBUG_PRINT("I2S# ADC is running, skip re-init ADC !\n");
        return;
    }

    printf("I2S# %s +\n", __func__);

    if(((u32)i2s_spec->base_in_i2s % 8) || (i2s_spec->buffer_size % 8)) {
        printf("ERROR# I2S, bufbase/bufsize must be 8-Bytes alignment !\n");
        return;
    }
    if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) {
        printf("ERROR# I2S, only support single or two channels !\n");
        return;
    }
    pthread_mutex_lock(&I2S_MUTEX);
    _i2s_power_on();

    _i2s_set_clock(i2s_spec->sample_rate);
    _i2s_reset();
    itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
    _i2s_AD_running = 1;

    /* mastor mode & AWS & ZWS sync */
    switch(i2s_spec->sample_size) {
        case  8: { resolution_type = 3; break; }
        case 16: { resolution_type = 0; break; }
        case 24: { resolution_type = 1; break; }
        case 32:
        default: { resolution_type = 2; break; }
    }

    /* config AD */
    {
        unsigned short B14;

        B14 = (i2s_spec->use_hdmirx && i2s_spec->internal_hdmirx && i2s_spec->slave_mode) ? 1 : 0;

        data16 = (B14                       << I2S_POS_EN_HDMI_RX_SRC)  /* CLK/WS/DATA1~4 from internal HDMI RX */
        | (0                                << I2S_POS_SEL_ADC_SRC)     /* 0:Ext. ADC; 1:Internal ADC */
        | ((!!(i2s_spec->slave_mode))       << I2S_POS_RX_MODE)         /* hardware not use this bit */
        | (resolution_type                  << I2S_POS_RX_DATA_FMT)     /* ADC resolution, 00:16bit; 01:24bit; 10:32bit */
        | (0x1F                             << I2S_POS_RX_CH_LEN);      /* sample width (in bits unit) */
        _REG_Write16(I2S_REG_RX_CTRL, data16);

        _REG_WriteMask16(I2S_REG_TX_CTRL,
            (1    << I2S_POS_TX_MODE)
          | (0x1F << I2S_POS_TX_CH_LEN),
          I2S_MSK_TX_MODE | I2S_MSK_TX_CH_LEN);
          
        /* Big Endian Type (for IIS Input and IIS Output use) */
        if(i2s_spec->is_big_endian)
        {
            if((24 == i2s_spec->sample_size) || (32 == i2s_spec->sample_size))
            {
                _REG_SetBit16(I2S_REG_ADDA_PCM, I2S_POS_DAT_FMT);
            }
            else
            {
                _REG_ClrBit16(I2S_REG_ADDA_PCM, I2S_POS_DAT_FMT);
            }
        }
    }

    /* I2S I/O direction for 9910 only */
    _i2s_setup_io_direction_9910_only(i2s_spec);

    /* I2S Engine IO Mode for 9910 only */
    /* DA only: REG[0x1644] B'7 = 0 ; AD only: REG[0x1644] B'7 = 1 */   
    _REG_SetBit16(I2S_REG_ADDA_PCM, I2S_POS_SEL_IO_MODE);
    
    _i2s_aws_sync();

    if     ( i2s_spec->from_LineIN && !i2s_spec->from_MIC_IN) { itp_codec_rec_init(1); } /* LineIN only */
    else if(!i2s_spec->from_LineIN &&  i2s_spec->from_MIC_IN) { itp_codec_rec_init(2); } /* MICIN only */
    else                                                      { itp_codec_rec_init(0); } /* both LineIN & MICIN */

    /* buffer base */    
    if (chipVer == 1)
    {
        _I2S_SetRxBufAddr(I2S_RX_BUF_5, (uint32_t)i2s_spec->base_in_i2s);
    }
    else
    {
        _I2S_SetRxBufAddr(I2S_RX_BUF_1, (uint32_t)i2s_spec->base_in_i2s);
    }

    /* buffer length */
    _I2S_SetRxBufSize(i2s_spec->buffer_size);

    /* AD interrupt threshold: (value = available space) to full */
    _I2S_SetRxHighWaterMark(0x20040);   /* TODO: need reasonable threshold */

    if (chipVer == 1)
    {
        _REG_Write16(I2S_REG_RX_CTRL2,
        ( (0 << I2S_POS_EN_RX_LOOP_BACK) /* 0:AD no loopback; 1:Buf1, 7:All loopback(Buf1~5) */
        | (1 << I2S_POS_EN_OVR_RX_H_WAT_MRK_INTR) /* Enable AD RdWrGap Interrupt */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_4) /* Enable HDMI RX Data3 Input */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_3) /* Enable HDMI RX Data2 Input */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_2) /* Enable HDMI RX Data1 Input */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_1) /* Enable HDMI RX Data0 Input */
        | (((!i2s_spec->use_hdmirx) & 0x1) << I2S_POS_EN_RX_BUF_5) /* Enable IIS Data Input */
        ));
    }
    else
    {
        _REG_Write16(I2S_REG_RX_CTRL2,
        ( (0 << I2S_POS_EN_RX_LOOP_BACK) /* 0:AD no loopback; 1:Buf1, 7:All loopback(Buf1~5) */
        | (1 << I2S_POS_EN_OVR_RX_H_WAT_MRK_INTR) /* Enable AD RdWrGap Interrupt */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_5) /* Enable HDMI RX Data3 Input */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_4) /* Enable HDMI RX Data2 Input */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_3) /* Enable HDMI RX Data1 Input */
        | ((i2s_spec->use_hdmirx & 0x1) << I2S_POS_EN_RX_BUF_2) /* Enable HDMI RX Data0 Input */
        | (((!i2s_spec->use_hdmirx) & 0x1) << I2S_POS_EN_RX_BUF_1) /* Enable IIS Data Input */
        ));
    }
    _REG_Write16(I2S_REG_RX_CTRL1,
    ( ((i2s_spec->channels - 1)          << I2S_POS_RX_CH_NUM) /* Input Channel, 0:Single Channel; 1:Two Channels */
    | ((i2s_spec->is_big_endian ? 1 : 0) << I2S_POS_RX_ENDIAN) /* 0:Little Endian; 1:Big Endian */
    | ((i2s_spec->record_mode ? 1 : 0)   << I2S_POS_RX_EN_REC_MODE) /* 0:AV Sync Mode (wait capture to start); 1:Only Record Mode */
    | (1                                 << I2S_POS_RX_ENABLE) /* Fire the IIS for Audio Input */
    | (1                                 << I2S_POS_RX_START) /* Enable Audio Input */
    ));

//  i2s_pause_ADC(0); /* FIXME: TODO: Enable Audio input */

    printf("I2S# %s -\n", __func__);
    pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_mute_DAC(int mute)
{
    DEBUG_PRINT("I2S# %s(%d)\n", __func__, mute);
    if (mute)
    {
        _I2S_EnableFading(0xF, 0x01); /* fast response */
        _REG_Write16(I2S_REG_DIG_OUT_VOL, 0x0000);
    }
    else /* resume */
    {
        _I2S_EnableFading(0x1, 0xFF); /* slow response */
        _REG_Write16(I2S_REG_DIG_OUT_VOL, 0xFF00);
    }
}

int i2s_set_direct_volstep(unsigned volstep)
{
    itp_codec_playback_set_direct_vol(volstep);
    return 0;
}

int i2s_set_direct_volperc(unsigned volperc)
{
    itp_codec_playback_set_direct_volperc(volperc);
    return 0;
}

unsigned i2s_get_current_volstep(void)
{
    unsigned currvol = 0;
    itp_codec_playback_get_currvol(&currvol);
    return currvol;
}

unsigned i2s_get_current_volperc(void)
{
    unsigned currvolperc = 0;
    itp_codec_playback_get_currvolperc(&currvolperc);
    return currvolperc;
}

void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min)
{
    itp_codec_playback_get_vol_range(max, normal, min);
}

int i2s_ADC_set_direct_volstep(unsigned volstep)
{
    itp_codec_rec_set_direct_vol(volstep);
    return 0;
}

unsigned i2s_ADC_get_current_volstep(void)
{
    unsigned currvol = 0;
    itp_codec_rec_get_currvol(&currvol);
    return currvol;
}

void i2s_ADC_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min)
{
    itp_codec_rec_get_vol_range(max, normal, min);
}

void i2s_mute_ADC(int mute)
{
    DEBUG_PRINT("I2S# %s(%d)\n", __func__, mute);

    if(mute) { itp_codec_rec_mute(); }
    else     { itp_codec_rec_unmute(); }
}

