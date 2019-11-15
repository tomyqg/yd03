﻿#ifndef __NORWRITER_H_9D134C9D_0143_4c09_83B3_306D6A829BDA__
#define __NORWRITER_H_9D134C9D_0143_4c09_83B3_306D6A829BDA__

/*
#define NOR_WRITER_START_ADDR	(4 * 1024 * 1024)
#define ROM_LENGTH_START_ADDR	(NOR_WRITER_START_ADDR - 4)
#define ROM_LENGTH_READ_LENGTH	(4)
#define ROM_DATA_START_ADDR		(ROM_LENGTH_START_ADDR + ROM_LENGTH_READ_LENGTH)
*/

#define ARM_NOR_WRITER_MAX_BUF_LENGTH	(4 * 1024 * 1024)

#define ARM_NOR_WRITER_STATUS			0x1698
#define ARM_NOR_WRITER_ROM_BUF_ADDR_L	0x169A
#define ARM_NOR_WRITER_ROM_BUF_ADDR_H	0x169C
#define ARM_NOR_WRITER_RB_BUF_ADDR_L	0x169E
#define ARM_NOR_WRITER_RB_BUF_ADDR_H	0x16A0
#define ARM_NOR_WRITER_ROM_LENGTH_L		0x16A2
#define ARM_NOR_WRITER_ROM_LENGTH_H		0x16A4
#define ARM_NOR_WRITER_APP_STATUS		0x16A6
#define ARM_NOR_WRITER_APP_BAR_MIN_L	0x16A8
#define ARM_NOR_WRITER_APP_BAR_MIN_H	0x16AA
#define ARM_NOR_WRITER_APP_BAR_MAX_L	0x16AC
#define ARM_NOR_WRITER_APP_BAR_MAX_H	0x16AE
#define ARM_NOR_WRITER_APP_BAR_CURR_L	0x16B0
#define ARM_NOR_WRITER_APP_BAR_CURR_H	0x16B2

typedef enum _ANW_APP_STATUS
{
	ANW_APP_STATUS_OK,
	ANW_APP_STATUS_NOR_INIT_FAIL,
	ANW_APP_STATUS_OUT_OF_MEMORY,
	ANW_APP_STATUS_ROM_LENGTH_ZERO,
	ANW_APP_STATUS_ROM_LENGTH_OVER_MAX_BUF_LENGTH,
	ANW_APP_STATUS_WRITING_ERASE_NOR,
	ANW_APP_STATUS_WRITING_PAGES,
	ANW_APP_STATUS_READING_PAGES,
	ANW_APP_STATUS_WRITE_FINISH_COMPARE_FAIL,
	ANW_APP_STATUS_WRITE_FINISH_COMPARE_OK,
}ANW_APP_STATUS;

typedef enum _ANW_STATUS
{
	ANW_STATUS_NOT_READY,
	ANW_STATUS_WAIT_TO_WRITE,
	ANW_STATUS_PC_SEND_DATA_FINISH,
}ANW_STATUS;

#endif
