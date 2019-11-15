/*
 * Copyright (c) 2018 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * I2S Driver API (IT9850)
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
#include "i2s/i2s.h"
#include "i2s_reg.h"
#include "i2s_hw.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
/* ************************************************************************** */
/* platform control */

#if   (defined CFG_I2S_USE_GPIO_MODE_1)
static uint32_t Gpio_Pin[5]={32,32,33,36,34};
#elif (defined CFG_I2S_USE_GPIO_MODE_2)
static uint32_t Gpio_Pin[5]={10,11, 9,12,13};
#elif (defined CFG_I2S_USE_GPIO_MODE_3)
static uint32_t Gpio_Pin[5]={10, 7, 9, 6, 5};
#else
static uint32_t Gpio_Pin[5]={ 0, 0, 0, 0, 0};
#endif

#define ENABLE_192KHZ_SAMPLE_RATE
#define I2S_MASTER_MODE

//#define I2S_DEBUG_SET_CLOCK
//#define I2S_DEBUG_DEINIT_DAC_COST

//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================================================================
//                              Macro Definition
//=============================================================================
#ifdef _WIN32
    #define asm()
#endif

#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);           } while(0)
#define TV_CAL_DIFF_MS(TIME2, TIME1) ((((TIME2.tv_sec * 1000000UL) + TIME2.tv_usec) - ((TIME1.tv_sec * 1000000UL) + TIME1.tv_usec)) / 1000)
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
static int _i2s_DA_mute = 0;
static int _bar_mute_flag = 0;
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
static void _i2s_set_GPIO(void); /* GPIO set */

/* FIXME: SPDIF */
static void _init_spdif(u32 sample_rate);
static void _deinit_spdif(void);
static void _show_i2s_spec(STRC_I2S_SPEC *i2s_spec);
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

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

void I2S_DA32_WAIT_RP_EQUAL_WP(void)
{
    uint32_t WPtr;
    _I2S_EnableTxFlush();
    WPtr = _I2S_GetTxBufWrPtr();

    if (ITH_IS_UNALIGNED(WPtr, 8))
        WPtr = ITH_ALIGN_DOWN(WPtr, 8);

    {
        uint32_t cnt=0;
        /* if enable, then wait the RP close th WP: abs(RW-WP) < 128 , */
        while( abs(_I2S_GetTxBufRdPtr() - WPtr) >= 128)
        {
            if(cnt++>5000)  break;
            usleep(1000);
        }
        if(cnt++>5000)  printf("I2S# waring, wait for RP=WP timeout.\n");
    }
    _I2S_DisableTxFlush();
}

u32 I2S_AD32_GET_RP(void) { return _I2S_GetRxBufRdPtr(); }
u32 I2S_AD32_GET_WP(void) { return _I2S_GetRxBufWrPtr(); }
void I2S_AD32_SET_RP(u32 data32) { _I2S_SetRxBufRdPtr(data32); }

u32 i2s_is_DAstarvation(void)
{
    uint16_t status_out = _REG_Read16(I2S_REG_TX_STATUS1);
    return (status_out >> 1) & 0x1;
}

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

    /* enable PLL2 for audio */
    {
        _REG_ClrBit16(ITH_PLL2_SET3_REG, ITH_PLL2_PWR_STATE_POS);
        _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);/*initial reset and start,0:Reset PLL2, 1:Start PLL after power is stable*/
        do
        {
            if (_REG_Read16(ITH_PLL_LOCK_STATUS_REG) & ITH_PLL2_LOCK_MSK)
                break;
            ithDelay(10); /* unit: micro-sec (us) */
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

    i2s_delay_us(100); /* FIXME: dummy loop */;
    _REG_WriteMask16(ITH_AUDIO_CLK_REG,
        0xFFFF, 
        ITH_AUDIO_RESET_I2S_TX_MSK | ITH_AUDIO_RESET_I2S_RX_MSK);
    i2s_delay_us(100); /* FIXME: dummy loop */;
    _REG_WriteMask16(ITH_AUDIO_CLK_REG,
        0x0000, 
        ITH_AUDIO_RESET_I2S_TX_MSK | ITH_AUDIO_RESET_I2S_RX_MSK);
    i2s_delay_us(100); /* FIXME: dummy loop */;

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
#ifdef I2S_DEBUG_SET_CLOCK
    printf("I2S# %s, demanded_sample_rate: %u\n", __func__, demanded_sample_rate);
#endif

    #ifdef  ENABLE_192KHZ_SAMPLE_RATE
    if     ((demanded_sample_rate > 182400) && (demanded_sample_rate < 210600)) { target_sample_rate = 192000; }
    else if((demanded_sample_rate > 91200) && (demanded_sample_rate < 100800)) { target_sample_rate = 96000; }
    else if((demanded_sample_rate > 83790) && (demanded_sample_rate < 92610)) { target_sample_rate = 88200; }
    else if ((demanded_sample_rate > 45600) && (demanded_sample_rate < 50400)) { target_sample_rate = 48000; }
    #else
    if     ((demanded_sample_rate > 45600) && (demanded_sample_rate < 50400)) { target_sample_rate = 48000; }
    #endif
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

#ifdef I2S_DEBUG_SET_CLOCK
    printf("I2S# %s, target_sample_rate: %u\n", __func__, target_sample_rate);
#endif

    switch (target_sample_rate)
    {
    #ifdef  ENABLE_192KHZ_SAMPLE_RATE
    case 192000:
    case  96000:
        {
            /* Xtal: 30 MHz, 49152000 Hz */
            /* A:3 B:18.0 C:23 D:03 E:011 PLLo:49152166.19 diff:166.19 osc:540,673,828.13  */
            _REG_Write16(ITH_PLL2_SET1_REG, 0x8003);    //D=3
            _REG_Write16(ITH_PLL2_SET2_REG, 0x0B0B); /* AMCLK select PLL2_OUT1 */   //E=11
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL2_SET4_REG, 0x2017);    //B=16:17:18:16.5=0:1:2:3  C=23
            _REG_Write16(ITH_PLL2_SET3_REG, 0x3000);
            ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);

            switch (target_sample_rate)
            {
            case 192000:
                amclk_ratio = 1 - 1; /* AMCLK = 49.152MHz (256x192KHz)     */
                zclk_ratio  = 4 - 1; /* ZCLK  = 12.288 MHz (zclk ratio=0x1F) */
                break;

            case 96000:
                amclk_ratio = 2 - 1; /* AMCLK = 24.576MHz (256x96KHz)      */
                zclk_ratio  = 8 - 1; /* ZCLK  = 6.144 MHz (zclk ratio=0x1F) */
                break;
            }
        }
        break;
    #endif

    case 48000:
    case 24000:
    case 12000:
        {
            /* Xtal: 12 MHz, 48000 Hz */
            /* A:3 B:16.0 C:-0166 D:02 E:058 PLLo:12288018.588362 diff:18.588362 osc:712.704000 */
            _REG_Write16(ITH_PLL2_SET1_REG, 0x9001);
            _REG_Write16(ITH_PLL2_SET2_REG, 0x4040); /* AMCLK select PLL2_OUT1 */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL2_SET4_REG, 0x0189);
            _REG_Write16(ITH_PLL2_SET3_REG, 0x3000);
            i2s_delay_us(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);

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

    #ifdef  ENABLE_192KHZ_SAMPLE_RATE
    case 88200:
        {
            /* Xtal: 30 MHz, 88200 Hz */
            /* A:3 B:16.5 C:119 D:02 E:033 PLLo:22,579,234.7301  diff:34.7301 osc:745,114,746.0938  */
            _REG_Write16(ITH_PLL2_SET1_REG, 0x8002);    //A=3:4:5:6=[13:12][00][01][10][11] D=2:3=[1:0][10][11]
            _REG_Write16(ITH_PLL2_SET2_REG, 0x2121); /* AMCLK select PLL2_OUT1 */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL2_SET4_REG, 0x3077);    //B=16:17:18:16.5=[13:12]0:1:2:3  C=119(0x77)
            _REG_Write16(ITH_PLL2_SET3_REG, 0x3000);
            ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);

            amclk_ratio = 1 - 1; /* AMCLK = 11.29MHz (256x44.1KHz)     */
            zclk_ratio  = 4 - 1; /* ZCLK  = 2.82MHz (zclk ratio=0x1F) */
        }
        break;
    #endif

    case 44100:
    case 22050:
    case 11025:
        {
            /* Xtal: 12 MHz, 44100 Hz */
            /* A:3 B:18.0 C:-0192 D:02 E:071 PLLo:11289612.676056 diff:12.676056 osc:801.561600 */
            _REG_Write16(ITH_PLL2_SET1_REG, 0x9001);
            _REG_Write16(ITH_PLL2_SET2_REG, 0x4747); /* AMCLK select PLL2_OUT1 */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL2_SET4_REG, 0x3198);
            _REG_Write16(ITH_PLL2_SET3_REG, 0x3000);
            i2s_delay_us(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);

            switch (target_sample_rate)
            {
            case 44100:
                amclk_ratio = 1 - 1; /* AMCLK = 11.29MHz (256x44.1KHz)     */
                zclk_ratio  = 4 - 1; /* ZCLK  = 2.82MHz (zclk ratio=0x1F) */
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
    case 8000:
        {
            /* Xtal: 12 MHz, 32000 Hz */
            /* A:5 B:16.5 C:00098 D:03 E:101 PLLo:8192005.724010 diff:5.724010 osc:827.392000 */
            _REG_Write16(ITH_PLL2_SET1_REG, 0x9001);
            _REG_Write16(ITH_PLL2_SET2_REG, 0x6161); /* AMCLK select PLL2_OUT1 */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);
            _REG_Write16(ITH_PLL2_SET4_REG, 0x3070);
            _REG_Write16(ITH_PLL2_SET3_REG, 0x3000);
            i2s_delay_us(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
            _REG_Write16(ITH_PLL2_SET3_REG, 0x2000);

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

static void _i2s_set_GPIO(void) /* GPIO settings for CFG2 */
{
    ithGpioSetMode(Gpio_Pin[0], ITH_GPIO_MODE2); /* ZCLK     (CODEC-BCLK)   */
    ithGpioSetMode(Gpio_Pin[1], ITH_GPIO_MODE2); /* AMCLK    (CODEC-MCLK)   */
    ithGpioSetMode(Gpio_Pin[2], ITH_GPIO_MODE2); /* ZWS      (CODEC-DACLRC) */
    ithGpioSetMode(Gpio_Pin[3], ITH_GPIO_MODE2); /* DATA-IN  (CODEC-ADCDAT) */

#if (defined CFG_I2S_OUTPUT_PIN_ENABLE)
    ithGpioSetMode(Gpio_Pin[4], ITH_GPIO_MODE2); /* DATA-OUT (CODEC-DACDAT) */
 #endif
    DEBUG_PRINT("I2S# %s\n", __func__);
}

/* ************************************************************************** */
void i2s_CODEC_wake_up(void)
{
    itp_codec_wake_up();
}

void i2s_CODEC_standby(void)
{
    itp_codec_standby();
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

void i2s_enable_fading(int yesno){
    if(yesno){
//      _I2S_EnableFading(0xF, 0x01); /* fast response */
//      _I2S_EnableFading(0x7, 0x7F); /* moderato */
        _I2S_EnableFading(0x1, 0xFF); /* slow response */
        _REG_Write16(I2S_REG_DIG_OUT_VOL, 0xFF00);//fading in
        //_REG_Write16(I2S_REG_DIG_OUT_VOL, 0x0000);//fading out
    }else{
        _I2S_DisableFading();
    }
}

void i2s_pause_DAC(int pause)
{
    printf("I2S# %s(%d)\n", __func__, pause);

    if(pause)
    {
        itp_codec_playback_mute();
        itp_codec_power_off();
        _REG_ClrBit16(I2S_REG_TX_CTRL2, I2S_POS_TX_START);
    }
    else /* resume */
    {
        _REG_SetBit16(I2S_REG_TX_CTRL2, I2S_POS_TX_START);

        itp_codec_power_on();
        if(!_i2s_DA_mute)
        itp_codec_playback_unmute();
    }
}

void i2s_deinit_ADC(void)
{
    if(!_i2s_AD_running) {
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
    u32 pipe_idle;
    u32 i2s_memcnt;
#ifdef I2S_DEBUG_DEINIT_DAC_COST
    static struct timeval tv_pollS;
    static struct timeval tv_pollE;
#endif

    pthread_mutex_lock(&I2S_MUTEX);

    if(!_i2s_DA_running) {
        DEBUG_PRINT("I2S# DAC is 'NOT' running, skip deinit DAC !\n");
        pthread_mutex_unlock(&I2S_MUTEX);
        return;
    }

#ifdef I2S_DEBUG_DEINIT_DAC_COST
    gettimeofday(&tv_pollS, NULL); //-*-
#endif

    printf("I2S# %s +\n", __func__);

    /* FIXME: SPDIF */
    #ifdef I2S_USE_SPDIF
    _deinit_spdif();
    #endif

    itp_codec_playback_deinit();

    /* disable I2S_OUT_FIRE & I2S_OUTPUT_EN */
    _I2S_DisableTx();

    do
    {
        uint16_t out_status_1 = _REG_Read16(I2S_REG_TX_STATUS1);
        uint16_t out_status_2 = _REG_Read16(I2S_REG_TX_STATUS2);

//      printf("I2S# OUTPUT_STATUS 1,2: 0x%04x,0x%04x\n", out_status_1, out_status_2);
//      i2s_idle   =  out_status_1 & 0x1;
        pipe_idle  = (out_status_1 >> 1) & 0x1;
        i2s_memcnt = (out_status_2 >> 0) & 0x7F;
    } while(/*!i2s_idle*/ !pipe_idle || (i2s_memcnt != 0));

    _i2s_DA_running = 0; /* put before _i2s_reset() */
    _i2s_reset();
    _i2s_power_off();

    printf("I2S# %s -\n", __func__);

#ifdef I2S_DEBUG_DEINIT_DAC_COST
    gettimeofday(&tv_pollE, NULL); //-*-
    printf("I2S# DEINIT_DAC_COST: %ld (ms)\n", TV_CAL_DIFF_MS(tv_pollE, tv_pollS));
#endif
    pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec)
{
    int i;
    int param_fail = 0;
    u16 data16;
    u8 resolution_type;
#ifdef I2S_DEBUG_DEINIT_DAC_COST
    static struct timeval tv_pollS;
    static struct timeval tv_pollE;
#endif

    pthread_mutex_lock(&I2S_MUTEX);

    if(_i2s_DA_running) {
        DEBUG_PRINT("I2S# DAC is running, skip re-init !\n");
        pthread_mutex_unlock(&I2S_MUTEX);
        return;
    }
    printf("I2S# %s +\n", __func__);
#ifdef I2S_DEBUG_DEINIT_DAC_COST
    gettimeofday(&tv_pollS, NULL); //-*-
#endif

    if(itp_codec_get_DA_running()) itp_codec_playback_deinit();

    if((u32)i2s_spec->base_spdif % 8 ) { param_fail = 2; }
    if((u32)i2s_spec->base_i2s % 8) { param_fail = 3; }
    if(i2s_spec->buffer_size % 8) { param_fail = 4; }

    /* check channels number */
    if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) { param_fail = 5; }

    if(param_fail) {
        printf("ERROR# I2S, param_fail = %d\n", param_fail);
        pthread_mutex_unlock(&I2S_MUTEX);
        return;
    }

    _i2s_power_on();

    _i2s_set_GPIO();

    _i2s_set_clock(i2s_spec->sample_rate);
    _i2s_reset();
    itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
    /* FIXME: SPDIF */
    #ifdef I2S_USE_SPDIF
    _init_spdif(i2s_spec->sample_rate);
    #endif

    /* mastor mode & AWS & ZWS sync */
    switch(i2s_spec->sample_size) {
        case 8 : { resolution_type = 3; break; }
        case 16: { resolution_type = 0; break; }
        case 24: { resolution_type = 1; break; }
        case 32: 
        default: { resolution_type = 2; break; }
    }

    _REG_WriteMask16(I2S_REG_RX_CTRL,
        0x1F << I2S_POS_RX_CH_LEN,
        I2S_MSK_RX_CH_LEN);

    data16 = (1        << I2S_POS_SEL_DAC_SRC) /* DAC device selection, 0:External DAC; 1:Internal DAC */
#ifdef I2S_MASTER_MODE
    | (0               << I2S_POS_TX_MODE)     /* 0:Master(ZCLK/ZWS output mode); 1:Slave(ZCLK/ZWS input mode) */
#else
    | (1               << I2S_POS_TX_MODE)     /* 0:Master(ZCLK/ZWS output mode); 1:Slave(ZCLK/ZWS input mode) */
#endif
    | (resolution_type << I2S_POS_TX_DATA_FMT) /* DAC resolution bits, 00:16bits; 01:24bits; 10:32bits; 11:8bits */
    | (0x1F            << I2S_POS_TX_CH_LEN);  /* sample width (in bits unit) */

    _REG_Write16(I2S_REG_TX_CTRL, data16);

    /* I2S Engine IO Mode*/
    /* DA only: REG[0x1644] B'7 = 0 ; AD only: REG[0x1644] B'7 = 1 */
    _REG_ClrBit16(I2S_REG_ADDA_PCM, I2S_POS_SEL_IO_MODE);

    _i2s_aws_sync();

    if( i2s_spec->enable_Speaker>=2)
    {
        i2s_spec->enable_Speaker = 0;
        i2s_spec->enable_HeadPhone = 1;
    }
    if( !i2s_spec->enable_Speaker)  i2s_spec->enable_HeadPhone = 1; /* force to set output as headphone if speaker is 0 */

    if( i2s_spec->enable_Speaker &&  i2s_spec->enable_HeadPhone) { itp_codec_playback_init(2); } /* both SEPAKER & HEADPHONE */
    else if(i2s_spec->enable_Speaker)                            { itp_codec_playback_init(1); } /* SPEAKER only */
    else                                                         { itp_codec_playback_init(0); } /* HEADPHONE(line-out) only */

    /* buffer base */
    {
        if(i2s_spec->base_spdif == NULL) { i2s_spec->base_spdif = i2s_spec->base_i2s; }

        /* buf 1 (IIS / SPDIF Data) */
        _I2S_SetTxBufAddr(I2S_TX_BUF_1, (uint32_t)i2s_spec->base_i2s);
    }

    /* buffer length */
    _I2S_SetTxBufSize(i2s_spec->buffer_size);

    /* DA starvation interrupt threshold */
    _I2S_SetTxLowWaterMark(0xFFFF); /* TODO: need reasonable threshold */

    /* output path */
    {
        data16 = (1 << 8); /* DA starvation (DA RdWrGap) Interrupt */
        if(i2s_spec->is_dac_spdif_same_buffer) { data16 |= (1 << 0); }
        //if(i2s_spec->is_dac_spdif_same_buffer) { data16 |= (1 << 4); } /* buf 5 (IIS / SPDIF Data) */
        //else                                   { data16 |= (1 << 5); } /* buf 6 (SPDIF Data) */

        _REG_Write16(I2S_REG_TX_CTRL1, data16);
    }

    _REG_Write16(I2S_REG_TX_CTRL2,
    (  0                                 << 6) /* Output Channel active level, 0:Low for Left; 1:High for Left */
    | (0                                 << 5) /* Output Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
    | ((i2s_spec->channels - 1)          << 4) /* Output Channel Select, 0:Single Channel; 1:Two Channels */
    | ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
    | (0                                 << 2) /* 0:NOT_LAST_WPTR; 1:LAST_WPTR */
    | (1                                 << 1) /* Fire the IIS for Audio Output */
    );

    i2s_delay_us(1000); /* FIXME: dummy loop */;

    if(!i2s_spec->postpone_audio_output)
        i2s_pause_DAC(0); /* Enable Audio Output */
    else
        i2s_pause_DAC(1);

    _i2s_DA_running = 1;

    i2s_mute_DAC(_i2s_DA_mute);

    printf("I2S# %s -\n", __func__);

#ifdef I2S_DEBUG_DEINIT_DAC_COST
    gettimeofday(&tv_pollE, NULL); //-*-
    printf("I2S# INIT_DAC_COST: %ld (ms)\n", TV_CAL_DIFF_MS(tv_pollE, tv_pollS));
#endif
    pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec)
{
    u16 data16;
    u8 resolution_type;

    if(_i2s_AD_running) {
        DEBUG_PRINT("I2S# ADC is running, skip re-init ADC !\n");
        return;
    }

    printf("I2S# %s +\n", __func__);

    //memcpy((void*)&g_curr_AD_Spec, (u8*)i2s_spec, sizeof(g_curr_AD_Spec) );

    if(((u32)i2s_spec->base_i2s % 8) || (i2s_spec->buffer_size % 8)) {
        printf("ERROR# I2S, bufbase/bufsize must be 8-Bytes alignment !\n");
        return;
    }
/*  if(i2s_spec->buffer_size > 0xFFFF) {
        printf("ERROR# I2S, bufsize must be 16-bits width !\n");
        return;
    } */
    if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) {
        printf("ERROR# I2S, only support single or two channels !\n");
        return;
    }
    pthread_mutex_lock(&I2S_MUTEX);
    _i2s_power_on();

    _i2s_set_GPIO();

    _i2s_set_clock(i2s_spec->sample_rate);
    _i2s_reset();
    itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
    _i2s_AD_running = 1;

    /* mastor mode & AWS & ZWS sync */
    switch(i2s_spec->sample_size) {
        case 8 : { resolution_type = 3; break; }
        case 16: { resolution_type = 0; break; }
        case 24: { resolution_type = 1; break; }
        case 32: 
        default: { resolution_type = 2; break; }
    }

    /* config AD */
    {
#ifdef I2S_MASTER_MODE
        const bool isMasterMode = true;
#else
        const bool isMasterMode = false;
#endif
        data16 = (0        << I2S_POS_SEL_ADC_SRC) /* ADC device selection, 0:External ADC; 1:Internal ADC */
        | ((!isMasterMode) << I2S_POS_RX_MODE) /* hardware not use this bit */
        | (resolution_type << I2S_POS_RX_DATA_FMT) /* ADC resolution bits, 00:16bits; 01:24bits; 10:32bits */
        | (0x1F            << I2S_POS_RX_CH_LEN); /* sample width (in bits unit) */
        _REG_Write16(I2S_REG_RX_CTRL, data16);

        _REG_WriteMask16(I2S_REG_TX_CTRL,
            ((!isMasterMode) << I2S_POS_TX_MODE)
          | (0x1F            << I2S_POS_TX_CH_LEN),
            (I2S_MSK_TX_MODE | I2S_MSK_TX_CH_LEN));
    }

    /* I2S Engine IO Mode*/
    /* DA only: REG[0x1644] B'7 = 0 ; AD only: REG[0x1644] B'7 = 1 */
    _REG_SetBit16(I2S_REG_ADDA_PCM, I2S_POS_SEL_IO_MODE);
    _i2s_aws_sync();

    if     ( i2s_spec->from_LineIN && !i2s_spec->from_MIC_IN) { itp_codec_rec_init(1); } /* LineIN only */
    else if(!i2s_spec->from_LineIN &&  i2s_spec->from_MIC_IN) { itp_codec_rec_init(2); } /* MICIN only */
    else                                                      { itp_codec_rec_init(0); } /* both LineIN & MICIN */

    /* buffer base */
    _I2S_SetRxBufAddr(I2S_RX_BUF_1, (uint32_t)i2s_spec->base_i2s);

    /* buffer length */
    _I2S_SetRxBufSize(i2s_spec->buffer_size);

    /* AD starvation interrupt threshold */
    //data16 = (0x8FFF & 0xFFF8) | (1 << 0);
    data16 = (0x0001);
    _REG_Write16(I2S_REG_RX_CTRL2, data16);

    _REG_Write16(I2S_REG_RX_CTRL1,
    (  0                                 << 6) /* Input Channel active level, 0:Low for Left; 1:High for Left */
    | (0                                 << 5) /* Input Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
    | ((i2s_spec->channels - 1)          << 4) /* Input Channel, 0:Single Channel; 1:Two Channels */
    | ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
    | ((i2s_spec->record_mode ? 1 : 0)   << 2) /* 0:AV Sync Mode (wait capture to start); 1:Only Record Mode */
    | (1                                 << 1) /* Fire the IIS for Audio Input */
    | (1                                 << 0) /* Enable Audio Input */
    );

//  i2s_pause_ADC(0); /* FIXME: TODO: Enable Audio input */
    pthread_mutex_unlock(&I2S_MUTEX);
    printf("I2S# %s -\n", __func__);
}

int  i2s_get_DA_running(void)
{
    return _i2s_DA_running;
}

void i2s_mute_DAC(int mute)
{
    DEBUG_PRINT("I2S# %s(%d)\n", __func__, mute);
    if(mute)
    {
        itp_codec_playback_mute();
        _i2s_DA_mute = 1;
    }
    else /* resume */
    {
        itp_codec_playback_unmute();
        _i2s_DA_mute = 0;
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
    usleep(100);//dummy time : for more performance
    if(!volperc)
        i2s_mute_volume(1);//mute
    else if(!_i2s_DA_mute && volperc && _bar_mute_flag)
        i2s_mute_volume(0);//unmute
    else
        //do nothing
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

void i2s_set_linein_bypass(int bypass)
{
    DEBUG_PRINT("I2S# %s(%d)\n", __func__, bypass);
    itp_codec_playback_linein_bypass(bypass);
}

//=============================================================================
//                              Private Funtion Definition
//=============================================================================
#ifdef I2S_USE_SPDIF
static void _init_spdif(u32 sample_rate)
{
    typedef enum SPDIF_MODE_TAG
    {
        SPDIF_LINEAR_PCM = 0,
        SPDIF_NON_LINEAR_DATA
    } SPDIF_MODE;

    printf("I2S# %s\n", __func__);

//  ithWriteRegA( 0xDE000000 | 0x90, 0x22840000); //set gpio padselect
    ithGpioSetMode(16, ITH_GPIO_MODE3); //9850: ithGpioSetMode(16, ITH_GPIO_MODE3)
//  ithWriteRegA( 0xDE000000 | 0xD0, 0x0006); //set spdif clk from 1:pclk 0:amclk
//  ithClearRegBitA(ITH_GPIO_BASE + 0xD0, 7);
    ithWriteRegA(ITH_SSP1_BASE | 0x74, 0<<30);
    /* for SPDIF-LPCM mode */
    {
        #define SPDIF_STATUS_COPY_FREE      (0x1 << 2)
        #define SPDIF_STATUS_STEREO         (0x3 << 20)
        #define SPDIF_SAMPLE_MAX_WORD_LEN   (0x0 << 0)
        #define SPDIF_SAMPLE_WORD_LEN       (0x1 << 1)

        /* Original Sampling frequency. */
        #define ORIGINAL_192K_INFO          (0x1 << 4)
        #define ORIGINAL_12K_INFO           (0x2 << 4)
        #define ORIGINAL_176_40K_INFO       (0x3 << 4)
        #define ORIGINAL_96K_INFO           (0x5 << 4)
        #define ORIGINAL_8K_INFO            (0x6 << 4)
        #define ORIGINAL_88_20K_INFO        (0x7 << 4)
        #define ORIGINAL_16K_INFO           (0x8 << 4)
        #define ORIGINAL_24K_INFO           (0x9 << 4)
        #define ORIGINAL_11_025K_INFO       (0xA << 4)
        #define ORIGINAL_22_05K_INFO        (0xB << 4)
        #define ORIGINAL_32K_INFO           (0xC << 4)
        #define ORIGINAL_48K_INFO           (0xD << 4)
        #define ORIGINAL_44_10K_INFO        (0xF << 4)
        #define ORIGINAL_NO_INDICATION      (0x1 << 4)

        /* STATUS BIT 0 */
        uint32_t data32 = SPDIF_LINEAR_PCM | SPDIF_STATUS_COPY_FREE | SPDIF_STATUS_STEREO;
        ithWriteRegA(ITH_SSP1_BASE | 0x24, data32); /* SPDIF Status Bit Register 0, STUS0 */

        /* STATUS BIT 1 */
        data32 = SPDIF_SAMPLE_MAX_WORD_LEN | SPDIF_SAMPLE_WORD_LEN;
        switch(sample_rate)
        {
            case 192000:
                data32 |= ORIGINAL_192K_INFO;
                break;
            case 96000:
                data32 |= ORIGINAL_96K_INFO;
                break;
            case 88200:
                data32 |= ORIGINAL_88_20K_INFO;
                break;
            case 11025:
                data32 |= ORIGINAL_11_025K_INFO;
                break;
            case 22050:
                data32 |= ORIGINAL_22_05K_INFO;
                break;
            case 44100:
                data32 |= ORIGINAL_44_10K_INFO;
                break;
            case 12000:
                data32 |= ORIGINAL_12K_INFO;
                break;
            case 24000:
                data32 |= ORIGINAL_24K_INFO;
                break;
            case 48000:
                data32 |= ORIGINAL_48K_INFO;
                break;
            case 8000:
                data32 |= ORIGINAL_8K_INFO;
                break;
            case 16000:
                data32 |= ORIGINAL_16K_INFO;
                break;
            case 32000:
                data32 |= ORIGINAL_32K_INFO;
                break;
            default:
                SERR();
                break;
        }
        ithWriteRegA(ITH_SSP1_BASE | 0x28, data32); /* SPDIF Status Bit Register 1, STUS1 */
    }

    ithWriteRegA( ITH_SSP1_BASE | 0x00, 0x0000504c); /* SSP Control Register 0, CR0 */
    ithWriteRegA( ITH_SSP1_BASE | 0x04, 0x000f0000); /* SSP Control Register 1, CR1 */
    ithWriteRegA( ITH_SSP1_BASE | 0x10, 0x00100020); /* Interrupt Control Register, ICR */
//  ithWriteRegA( ITH_SSP1_BASE | 0x24, 0x00000004); /* SPDIF Status Bit Register 0, STUS0 */
//  ithWriteRegA( ITH_SSP1_BASE | 0x28, 0x000000d2); /* SPDIF Status Bit Register 1, STUS1 */
    ithWriteRegA( ITH_SSP1_BASE | 0x5C, 0x00040001); /* I2S and AC3 Control Register, CR3 */
    ithWriteRegA( ITH_SSP1_BASE | 0x08, 0x0000000f); /* SSP Control Register 2, CR2 */
}

static void _deinit_spdif(void)
{
    printf("I2S# %s\n", __func__);
}
#endif

static void _show_i2s_spec(STRC_I2S_SPEC *i2s_spec)
{
    printf("    @channels   %x\n",i2s_spec->channels);
    printf("    @sample_rate    %x\n",i2s_spec->sample_rate);
    printf("    @buffer_size    %x\n",i2s_spec->buffer_size);
    printf("    @is_big_endian  %x\n",i2s_spec->is_big_endian);
    printf("    @sample_size    %x\n",i2s_spec->sample_size);
    printf("    @from_LineIN    %x\n",i2s_spec->from_LineIN);
    printf("    @from_MIC_IN    %x\n",i2s_spec->from_MIC_IN);
    printf("    @num_hdmi_buf   %x\n",i2s_spec->num_hdmi_audio_buffer);
    printf("    @base_i2s   %x\n",i2s_spec->base_i2s);
    printf("    @spdif_same_buf %x\n",i2s_spec->is_dac_spdif_same_buffer);
    printf("    @base_spdif %x\n",i2s_spec->base_spdif);
    printf("    @aud_out    %x\n",i2s_spec->postpone_audio_output);
    printf("    @record_mode    %x\n",i2s_spec->record_mode);
    printf("    @Speaker    %x\n",i2s_spec->enable_Speaker);
    printf("    @HeadPhone  %x\n",i2s_spec->enable_HeadPhone);
}

void i2s_mute_volume(int mute)
{
    if(mute){
        _bar_mute_flag = 1;
        itp_codec_playback_mute();
    }
    else{
        _bar_mute_flag = 0;
        itp_codec_playback_unmute();
    }
}

#if 0 //for DAC alc5628
void i2s_select_datain_path(int i2sin_yesno,int linein_yesno)
{
    if(_i2s_DA_running){
        itp_codec_set_i2sin_enable(i2sin_yesno);
        itp_codec_set_linein_enable(linein_yesno);
        printf("I2S# select DAC data in path : i2sIn (%d) ,lineIn (%d)\n",i2sin_yesno,linein_yesno);
    }else{
        printf("I2S# DAC is 'NOT' running, skip select DAC data in path!\n");
    }

}
#endif