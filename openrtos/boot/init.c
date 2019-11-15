﻿/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Initialize functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "openrtos/FreeRTOS.h"
#include "openrtos/semphr.h"
#include "ite/itp.h"
#include <stdio.h>
#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/swUart/swUart.h"

// FIXME: remove inline the tlb.c here. (Kuoping)
// I don't know why it cannot compile the tlb.c standalone. It must
// include inlined here, else the linker do not link the tlb.o.

extern uint32_t __mem_end;

#define VMEM_START  (64) // keep 0 for NULL

int int_init(void);
int int_enable(unsigned long vect);
void BSP_InitIntCtrl(void);

void __init BootInit(void)
{
    static ITHVmem vmem;

    uint32_t bootTime = ithTimerGetTime(portTIMER);

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_TRACE)
    vTraceEnable(TRC_START);
#endif

#ifdef CFG_OPENRTOS_MEMPOOL_ENABLE
    // init openrtos memory pool
    vPortInitialiseBlocks();
#endif

    // init video memory management for write-back memory
#if defined(CFG_CPU_WB) && CFG_WT_SIZE > 0
    vmem.startAddr      = VMEM_START;
    vmem.totalSize      = CFG_WT_SIZE;
    vmem.mutex          = xSemaphoreCreateMutex();
    vmem.usedMcbCount = vmem.freeSize = 0;

    ithVmemInit(&vmem);
#endif // defined(CFG_CPU_WB) && CFG_WT_SIZE > 0

#ifdef __SM32__
    int_init();
    int_enable(3);

#elif defined(__NDS32__)
    BSP_InitIntCtrl();

#endif // __SM32__

    // init hal module
    ithInit();

#if defined(CFG_DBG_PRINTBUF)
    // init print buffer device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDevicePrintBuf);
    itpRegisterDevice(ITP_DEVICE_PRINTBUF, &itpDevicePrintBuf);
    ioctl(ITP_DEVICE_PRINTBUF, ITP_IOCTL_INIT, NULL);

//#elif defined(CFG_DBG_SWUART)
    // init sw uart device
//    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceSwUart);
//    itpRegisterDevice(ITP_DEVICE_SWUART, &itpDeviceSwUart);
//    ioctl(ITP_DEVICE_SWUART, ITP_IOCTL_INIT, (void*)CFG_SWUART_BAUDRATE);

#elif defined(CFG_DBG_UART0)
    // init uart device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart0);
    itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
#if defined (CFG_UART_DMA)	
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_UART_SET_BOOT, (void*)true);
#endif	
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, (void*)CFG_UART0_BAUDRATE);
#elif defined(CFG_DBG_UART1)
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart1);
    itpRegisterDevice(ITP_DEVICE_UART1, &itpDeviceUart1);
#if defined(CFG_UART_DMA)	
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_UART_SET_BOOT, (void*)true);
#endif
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, (void*)CFG_UART1_BAUDRATE);
#elif defined(CFG_DBG_SWUART_CODEC)
#if defined (CFG_SW_UART) && !defined(CFG_RS485_4_ENABLE)
	//init Risc
	iteRiscInit();

	//init alt cpu
	itpRegisterDevice(ITP_DEVICE_ALT_CPU, &itpDeviceAltCpu);

	{
		//NOTICE: alt_cpu only for chip 9850
		int altCpuEngineType = ALT_CPU_SW_UART;
		SW_UART_INIT_DATA tUartInitData = {0};

		itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceAltCpu);
		
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_DBG_UART, NULL);

		tUartInitData.cpuClock = ithGetRiscCpuClock();
		tUartInitData.baudrate = CFG_SWUARTDBGPRINTF_BAUDRATE;
		tUartInitData.uartRxGpio = -1;
		tUartInitData.uartTxGpio = CFG_SWUARTDBGPRINTF_GPIO;
		tUartInitData.parity = NONE;
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_UART_PARAM, &tUartInitData);
	}
#else
    int swuart_gpio;
    int swuart_baudrate;
   //int swuart_parity;
    iteRiscInit();
    iteRiscOpenEngine(1, 1);
    {
        int i;
        for (i=0; i<1000000; i++)
            asm("");
    }

    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceSwUartCodecDbg);
    itpRegisterDevice(ITP_DEVICE_SWUARTDBG, &itpDeviceSwUartCodecDbg);

    swuart_gpio = CFG_SWUARTDBGPRINTF_GPIO;
    ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_SET_GPIO_PIN, &swuart_gpio);
    swuart_baudrate = CFG_SWUARTDBGPRINTF_BAUDRATE;
    ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_SET_BAUDRATE, &swuart_baudrate);
    //swuart_parity = ITP_SWUART_NONE;
    //ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_SET_PARITY, &swuart_parity);
    ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_INIT, NULL);
#endif
#endif // defined(CFG_DBG_PRINTBUF)

    // init memleak tool
#ifdef CFG_DBG_MEMLEAK
    dbg_init(CFG_DBG_MEMLEAK_LEN);
#endif

    ithPrintf(CFG_SYSTEM_NAME "/" CFG_PROJECT_NAME " ver " CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR "\n");

#if (CFG_CHIP_FAMILY != 970)  //970 using this code will hang it.
    ithPrintRegA(0xd090002c, 28);
#endif

    // booting time
    ithPrintf("booting time: %ums\r\n", bootTime / 1000);
}
