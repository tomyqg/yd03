/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Screen functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <unistd.h>
#include "itp_cfg.h"

static ITPTvOut screenLastMode;

#ifdef CFG_LCD_ENABLE

#ifdef CFG_LCD_MULTIPLE

static int lcdScriptIndex = 0;
static uint32_t lcdBaseA, lcdBaseB;

#ifdef CFG_VIDEO_ENABLE        
static uint32_t lcdBaseC;
#endif

#if (CFG_CHIP_FAMILY == 970)
typedef uint32_t LCD_SCRIPT_TYPE;
#else
typedef uint16_t LCD_SCRIPT_TYPE;
#endif

#include "lcd.inc"

#else

#if (CFG_CHIP_FAMILY == 970)
static const uint32_t lcdScript[] =
#else
static const uint16_t lcdScript[] =
#endif
{
#include "lcd.inc"
};
#endif // CFG_LCD_MULTIPLE

#endif // CFG_LCD_ENABLE

#ifdef CFG_TVOUT_ENABLE

static const uint16_t tvNtscScript[] =
{
#include "tv_ntsc.inc"
};

static const uint16_t tvPalScript[] =
{
#include "tv_pal.inc"
};

#ifdef CFG_TVOUT_HDMI

static const uint16_t tvNtsc480iScript[] =
{
#include "tv_ntsc_480i.inc"
};

static const uint16_t tvPal576iScript[] =
{
#include "tv_pal_576i.inc"
};

static const uint16_t tvNtsc480pScript[] =
{
#include "tv_ntsc_480p.inc"
};

static const uint16_t tvPal576pScript[] =
{
#include "tv_pal_576p.inc"
};

#endif // CFG_TVOUT_HDMI

static void ScreenTvOut(ITPTvOut mode)
{
    // backup lcd base addresses
#ifdef CFG_LCD_ENABLE
    uint32_t lcdBaseA = ithLcdGetBaseAddrA();
    uint32_t lcdBaseB = ithLcdGetBaseAddrB();
#endif // CFG_LCD_ENABLE

#ifdef CFG_CMDQ_ENABLE
#if (CFG_CHIP_FAMILY == 970) 
    ithCmdQWaitEmpty(ITH_CMDQ0_OFFSET);
    ithCmdQWaitEmpty(ITH_CMDQ1_OFFSET);
#else
    ithCmdQWaitEmpty();
#endif
#endif

    ithTveDisablePower();
    ithTveReset();
    ithIspResetEngine();

#ifdef CFG_LCD_ENABLE
    ithLcdReset();
#endif

    switch (mode)
    {
#ifdef CFG_LCD_ENABLE
    case ITP_TVOUT_LCD:
	#ifdef CFG_LCD_MULTIPLE
		lcdScriptIndex = (int)ptr;
        ithLcdLoadScriptFirst(lcdScript[lcdScriptIndex], lcdScriptSize[lcdScriptIndex]);
	#else
        ithLcdLoadScriptFirst(lcdScript, ITH_COUNT_OF(lcdScript));
	#endif // CFG_LCD_MULTIPLE    	
        break;
#endif // CFG_LCD_ENABLE

#ifdef CFG_TVOUT_ENABLE
    case ITP_TVOUT_NTSC:
        ithLcdLoadScriptFirst(tvNtscScript, ITH_COUNT_OF(tvNtscScript));
        break;

    case ITP_TVOUT_PAL:
        ithLcdLoadScriptFirst(tvPalScript, ITH_COUNT_OF(tvPalScript));
        break;
#endif // CFG_TVOUT_ENABLE

#ifdef CFG_TVOUT_HDMI
    case ITP_TVOUT_NTSC_I:
        ithLcdLoadScriptFirst(tvNtsc480iScript, ITH_COUNT_OF(tvNtsc480iScript));
        break;

    case ITP_TVOUT_PAL_I:
        ithLcdLoadScriptFirst(tvPal576iScript, ITH_COUNT_OF(tvPal576iScript));
        break;

    case ITP_TVOUT_NTSC_P:
        ithLcdLoadScriptFirst(tvNtsc480pScript, ITH_COUNT_OF(tvNtsc480pScript));
        break;

    case ITP_TVOUT_PAL_P:
        ithLcdLoadScriptFirst(tvPal576pScript, ITH_COUNT_OF(tvPal576pScript));
        break;
#endif // CFG_TVOUT_HDMI

    default:
        LOG_ERR "unknown tvout mode: %d\n", mode LOG_END
        return;
    }

#ifdef CFG_LCD_ENABLE
    // restore lcd base addresses
    ithLcdSetBaseAddrA(lcdBaseA);
    ithLcdSetBaseAddrB(lcdBaseB);

    ithSetVram(lcdBaseA, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);
    ithSetVram(lcdBaseB, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);

    ithLcdLoadScriptNext();
#endif // CFG_LCD_ENABLE

    if (mode != ITP_TVOUT_LCD)
        ithTveEnablePower();

    screenLastMode = mode;
}

#endif // CFG_TVOUT_ENABLE

#ifdef CFG_BACKLIGHT_ENABLE
static const unsigned int blDutyCycleTable[] = { CFG_BACKLIGHT_DUTY_CYCLES };
static unsigned int blLastValue = CFG_BACKLIGHT_DEFAULT_DUTY_CYCLE;

#endif // CFG_BACKLIGHT_ENABLE

#ifdef CFG_POWER_SUSPEND

#ifdef CFG_LCD_ENABLE
static uint32_t screenBaseA, screenBaseB;
#endif

static void ScreenSuspend(void)
{
#ifdef CFG_LCD_ENABLE
    // backup lcd base addresses
    screenBaseA = ithLcdGetBaseAddrA();
    screenBaseB = ithLcdGetBaseAddrB();
#endif // CFG_LCD_ENABLE
}

static void ScreenResume(void)
{
#ifdef CFG_TVOUT_ENABLE
    ithTveDisablePower();
    ithTveReset();
    ithIspResetEngine();
#endif // CFG_TVOUT_ENABLE

#ifdef CFG_LCD_ENABLE
    ithLcdReset();
#endif

    switch (screenLastMode)
    {
#ifdef CFG_LCD_ENABLE
    case ITP_TVOUT_LCD:
	#ifdef CFG_LCD_MULTIPLE
		lcdScriptIndex = (int)ptr;
        ithLcdLoadScriptFirst(lcdScript[lcdScriptIndex], lcdScriptSize[lcdScriptIndex]);
	#else
        ithLcdLoadScriptFirst(lcdScript, ITH_COUNT_OF(lcdScript));
	#endif // CFG_LCD_MULTIPLE
        break;
#endif // CFG_LCD_ENABLE

#ifdef CFG_TVOUT_ENABLE

    case ITP_TVOUT_NTSC:
        ithLcdLoadScriptFirst(tvNtscScript, ITH_COUNT_OF(tvNtscScript));
        break;

    case ITP_TVOUT_PAL:
        ithLcdLoadScriptFirst(tvPalScript, ITH_COUNT_OF(tvPalScript));
        break;

#ifdef CFG_TVOUT_HDMI
    case ITP_TVOUT_NTSC_I:
        ithLcdLoadScriptFirst(tvNtsc480iScript, ITH_COUNT_OF(tvNtsc480iScript));
        break;

    case ITP_TVOUT_PAL_I:
        ithLcdLoadScriptFirst(tvPal576iScript, ITH_COUNT_OF(tvPal576iScript));
        break;

    case ITP_TVOUT_NTSC_P:
        ithLcdLoadScriptFirst(tvNtsc480pScript, ITH_COUNT_OF(tvNtsc480pScript));
        break;

    case ITP_TVOUT_PAL_P:
        ithLcdLoadScriptFirst(tvPal576pScript, ITH_COUNT_OF(tvPal576pScript));
        break;
#endif // CFG_TVOUT_HDMI
#endif // CFG_TVOUT_ENABLE

    default:
        LOG_ERR "unknown screen mode: %d\n", screenLastMode LOG_END
        return;
    }

#ifdef CFG_LCD_ENABLE
    // restore lcd base addresses
    ithLcdSetBaseAddrA(screenBaseA);
    ithLcdSetBaseAddrB(screenBaseB);

    ithSetVram(screenBaseA, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);
    ithSetVram(screenBaseB, 0, CFG_LCD_PITCH * CFG_LCD_HEIGHT);

    ithLcdLoadScriptNext();
#endif // CFG_LCD_ENABLE

#ifdef CFG_TVOUT_ENABLE
    if (screenLastMode != ITP_TVOUT_LCD)
        ithTveEnablePower();
#endif // CFG_TVOUT_ENABLE
}

#endif // CFG_POWER_SUSPEND

#define	CFG_GPIO_LCD_CS	   74
#define	CFG_GPIO_LCD_SCL   26
#define	CFG_GPIO_LCD_SDA   25

static void St7701_Write_Byte(unsigned short temp)
{
	unsigned char i;
	for(i=0;i<9;i++)
	{
		ithGpioClear(CFG_GPIO_LCD_SCL);
		if(temp&0x0100)
			ithGpioSet(CFG_GPIO_LCD_SDA);
		else
			ithGpioClear(CFG_GPIO_LCD_SDA);
		usleep(5);
		ithGpioSet(CFG_GPIO_LCD_SCL);
		usleep(5);
		temp = temp<<1;
	}
}

static void St7701_WriteCommand(uint8_t c)
{
	unsigned short temp = 0x00;
	temp = temp | c;
	St7701_Write_Byte(temp);
}

static void St7701_WriteData(uint8_t c)
{
	unsigned short temp = 0x100;
	temp = temp | c;
	St7701_Write_Byte(temp);
}

void LCD_Nreset(unsigned char reset)
{
	//ithGpioSetOut(CFG_GPIO_LCD_RST);
	//ithGpioEnable(CFG_GPIO_LCD_RST);

	ithGpioSetOut(CFG_GPIO_LCD_CS);
	ithGpioEnable(CFG_GPIO_LCD_CS);

	ithGpioSetOut(CFG_GPIO_LCD_SCL);
	ithGpioEnable(CFG_GPIO_LCD_SCL);

	ithGpioSetOut(CFG_GPIO_LCD_SDA);
	ithGpioEnable(CFG_GPIO_LCD_SDA);

	ithGpioSet(CFG_GPIO_LCD_CS);
	ithGpioSet(CFG_GPIO_LCD_SCL);
	ithGpioSet(CFG_GPIO_LCD_SDA);
	//if(reset)
	//	ithGpioSet(CFG_GPIO_LCD_RST);
	//else
	//	ithGpioClear(CFG_GPIO_LCD_RST);

	//ithGpioSetOut(CFG_USB_WIFI_POWER);
	//ithGpioEnable(CFG_USB_WIFI_POWER);
	//ithGpioSet(CFG_USB_WIFI_POWER);
}



void ST7701_PanelInitialCode(void)
{

    LCD_Nreset (1);
    usleep (10000);
    LCD_Nreset (0);
    usleep (10000);
    LCD_Nreset (1);
    usleep (120*1100);
    ithGpioClear(CFG_GPIO_LCD_CS);
    St7701_WriteCommand(0x11);
    usleep (120*1000);
    St7701_WriteCommand(0xFF);
    St7701_WriteData(0x77);
    St7701_WriteData(0x01);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x10);
    St7701_WriteCommand(0xC0);
    St7701_WriteData(0x63);
    St7701_WriteData(0x00);
    St7701_WriteCommand(0xC1);
    St7701_WriteData(0x11);
    St7701_WriteData(0x02);
    St7701_WriteCommand(0xC2);
    St7701_WriteData(0x31);
    St7701_WriteData(0x08);
    St7701_WriteCommand(0xCC);
    St7701_WriteData(0x10);
    St7701_WriteCommand(0xB0);
    St7701_WriteData(0x00);
    St7701_WriteData(0x05);
    St7701_WriteData(0x0F);
    St7701_WriteData(0x0D);
    St7701_WriteData(0x13);
    St7701_WriteData(0x07);
    St7701_WriteData(0x01);
    St7701_WriteData(0x08);
    St7701_WriteData(0x09);
    St7701_WriteData(0x1E);
    St7701_WriteData(0x05);
    St7701_WriteData(0x12);
    St7701_WriteData(0x10);
    St7701_WriteData(0xB2);
    St7701_WriteData(0x2F);
    St7701_WriteData(0x18);
    St7701_WriteCommand(0xB1);
    St7701_WriteData(0x00);
    St7701_WriteData(0x0F);
    St7701_WriteData(0x17);
    St7701_WriteData(0x0C);
    St7701_WriteData(0x0D);
    St7701_WriteData(0x05);
    St7701_WriteData(0x01);
    St7701_WriteData(0x08);
    St7701_WriteData(0x08);
    St7701_WriteData(0x1E);
    St7701_WriteData(0x05);
    St7701_WriteData(0x13);
    St7701_WriteData(0x11);
    St7701_WriteData(0xB2);
    St7701_WriteData(0x2F);
    St7701_WriteData(0x18);
    St7701_WriteCommand(0xFF);
    St7701_WriteData(0x77);
    St7701_WriteData(0x01);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x11);
    St7701_WriteCommand(0xB0);
    St7701_WriteData(0x6C);
    St7701_WriteCommand(0xB1);
    St7701_WriteData(0x5D);
    St7701_WriteCommand(0xB2);
    St7701_WriteData(0x87);
    St7701_WriteCommand(0xB3);
    St7701_WriteData(0x80);
    St7701_WriteCommand(0xB5);
    St7701_WriteData(0x47);
    St7701_WriteCommand(0xB7);
    St7701_WriteData(0x85);
    St7701_WriteCommand(0xB8);
    St7701_WriteData(0x20);
    St7701_WriteCommand(0xB9);
    St7701_WriteData(0x10);
    St7701_WriteCommand(0xC1);
    St7701_WriteData(0x78);
    St7701_WriteCommand(0xC2);
    St7701_WriteData(0x78);
    St7701_WriteCommand(0xD0);
    St7701_WriteData(0x88);
    usleep(120*1000);  
    St7701_WriteCommand(0xEF);
    St7701_WriteData(0x08);
    St7701_WriteData(0x08);
    St7701_WriteData(0x08);
    St7701_WriteData(0x45);
    St7701_WriteData(0x3F);
    St7701_WriteData(0x54);
    usleep(100*1000);
    St7701_WriteCommand(0xE0);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x02);
    St7701_WriteCommand(0xE1);
    St7701_WriteData(0x08);
    St7701_WriteData(0x00);
    St7701_WriteData(0x0A);
    St7701_WriteData(0x00);
    St7701_WriteData(0x07);
    St7701_WriteData(0x00);
    St7701_WriteData(0x09);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x33);
    St7701_WriteData(0x33);
    St7701_WriteCommand(0xE2);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteCommand(0xE3);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x33);
    St7701_WriteData(0x33);
    St7701_WriteCommand(0xE4);
    St7701_WriteData(0x44);
    St7701_WriteData(0x44);
    St7701_WriteCommand(0xE5);
    St7701_WriteData(0x0E);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteData(0x10);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteData(0x0A);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteData(0x0C);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteCommand(0xE6);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x33);
    St7701_WriteData(0x33);
    St7701_WriteCommand(0xE7);
    St7701_WriteData(0x44);
    St7701_WriteData(0x44);
    St7701_WriteCommand(0xE8);
    St7701_WriteData(0x0D);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteData(0x0F);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteData(0x09);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteData(0x0B);
    St7701_WriteData(0x60);
    St7701_WriteData(0xA0);
    St7701_WriteData(0xA0);
    St7701_WriteCommand(0xEB);
    St7701_WriteData(0x02);
    St7701_WriteData(0x01);
    St7701_WriteData(0xE4);
    St7701_WriteData(0xE4);
    St7701_WriteData(0x44);
    St7701_WriteData(0x00);
    St7701_WriteData(0x40);
    St7701_WriteCommand(0xEC);
    St7701_WriteData(0x02);
    St7701_WriteData(0x01);
    St7701_WriteCommand(0xED);
    St7701_WriteData(0xAB);
    St7701_WriteData(0x89);
    St7701_WriteData(0x76);
    St7701_WriteData(0x54);
    St7701_WriteData(0x01);
    St7701_WriteData(0xFF);
    St7701_WriteData(0xFF);
    St7701_WriteData(0xFF);
    St7701_WriteData(0xFF);
    St7701_WriteData(0xFF);
    St7701_WriteData(0xFF);
    St7701_WriteData(0x10);
    St7701_WriteData(0x45);
    St7701_WriteData(0x67);
    St7701_WriteData(0x98);
    St7701_WriteData(0xBA);
    St7701_WriteCommand(0xFF);
    St7701_WriteData(0x77);
    St7701_WriteData(0x01);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    St7701_WriteData(0x00);
    usleep(120*1000);
    St7701_WriteCommand(0x29);
    usleep(30*1000); 


    ithGpioSet(CFG_GPIO_LCD_CS);

}

static int ScreenIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int size;
    
    switch (request)
    {
    case ITP_IOCTL_RESET:
                 ST7701_PanelInitialCode();
#if defined(CFG_LCD_ENABLE) && !defined(CFG_LCD_INIT_ON_BOOTING)

	#ifdef CFG_LCD_MULTIPLE
		lcdScriptIndex = (int)ptr;
        ithLcdLoadScriptFirst(lcdScript[lcdScriptIndex], lcdScriptSize[lcdScriptIndex]);
        
    #ifdef CFG_VIDEO_ENABLE
        if (lcdBaseC)
            itpVmemFree(lcdBaseC);        
    #endif // CFG_VIDEO_ENABLE
        
        if (lcdBaseB)
            itpVmemFree(lcdBaseB);        
        
        if (lcdBaseA)
            itpVmemFree(lcdBaseA);

        size = ithLcdGetPitch() * ithLcdGetHeight();
        lcdBaseA = itpVmemAlloc(size);
        ithLcdSetBaseAddrA(lcdBaseA);
        
        lcdBaseB = itpVmemAlloc(size);
        ithLcdSetBaseAddrB(lcdBaseB);                
        
    #ifdef CFG_VIDEO_ENABLE        
        lcdBaseC = itpVmemAlloc(size);
        ithLcdSetBaseAddrC(lcdBaseC);
        
        LOG_INFO "LCD width:%d height:%d pitch:%d baseA:0x%X baseB:0x%X baseC:0x%X\n", ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), lcdBaseA, lcdBaseB, lcdBaseC LOG_END
    #else
        LOG_INFO "LCD width:%d height:%d pitch:%d baseA:0x%X baseB:0x%X\n", ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), lcdBaseA, lcdBaseB LOG_END
    #endif
        
	#else
        ithLcdLoadScriptFirst(lcdScript, ITH_COUNT_OF(lcdScript));
	#endif // CFG_LCD_MULTIPLE

#endif // defined(CFG_LCD_ENABLE) && !defined(CFG_LCD_INIT_ON_BOOTING)
        break;

    case ITP_IOCTL_POST_RESET:
#if defined(CFG_LCD_ENABLE) && !defined(CFG_LCD_INIT_ON_BOOTING)
        ithLcdLoadScriptNext();
#endif
        break;

#ifdef CFG_TVOUT_ENABLE
    case ITP_IOCTL_TVOUT:
        ScreenTvOut((ITPTvOut)ptr);
        break;
#endif // CFG_TVOUT_ENABLE

#ifdef CFG_POWER_SUSPEND
    case ITP_IOCTL_SUSPEND:
        ScreenSuspend();
        break;

    case ITP_IOCTL_RESUME:
        ScreenResume();
        break;
#endif // CFG_POWER_SUSPEND

    default:
        errno = (ITP_DEVICE_SCREEN << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceScreen =
{
    ":screen",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    ScreenIoctl,
    NULL
};
