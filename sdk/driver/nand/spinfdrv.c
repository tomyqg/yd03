/*
 * Copyright (c) 2015 iTE Corp. All Rights Reserved.
 */
/** @file
 * SMedia SPI NAND Driver API header file.
 *
 */
 
#include "spinfdrv.h"

#ifdef USE_MMP_DRIVER
#include "ssp/mmp_spi.h"
#else
#include <stdio.h>
#include <string.h>
#include "stdint.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define EN_NO_RTC

//=============================================================================
//                              Include Files
//=============================================================================
#if (CFG_CHIP_FAMILY == 9850)
#define USE_NEW_MACRO
#endif

/***************************
 *
 **************************/ 

#ifdef	USE_MMP_DRIVER
#ifdef	CFG_SPI_NAND_USE_SPI0
#define SPI_PORT	SPI_0
#endif

#ifdef	CFG_SPI_NAND_USE_SPI1
#define SPI_PORT	SPI_1
#endif

#ifndef SPI_PORT
    #error "ERROR: spinfdrv.c MUST define the SPI NAND bus(SPI0 or SPI1)"
#endif

#ifdef USE_NEW_MACRO
    #define SPI_OK		(1)	//(true)
#else
    #define SPI_OK		(0)	//(true)
    #define ENABLE_CTRL_1
#endif
#else
#define SPI_OK		(0)	//(true)
#endif

#define SPINF_OK		(0)
#define SPINF_FAIL		(1)

#define SPINF_REMAP_SPARE_FOR_READ		(1)	//
#define SPINF_REMAP_SPARE_FOR_WRITE		(0)	//

//=============================================================================
//                              spi Function WRAP
//=============================================================================

//=============================================================================
//                              spi Function WRAP
//=============================================================================
static SPI_NF_INFO gSpiNfInfo;

static SPINF_CFG g_SpiNfCfgArray[] = 
{
	{ "GD_5F1GQ4UC", 0xC8, 0xB1, 0x48, 1, 2048, 64, 1024, 128 },
	{ "GD_5F2GQ4UC", 0xC8, 0xB2, 0x48, 1, 2048, 64, 2048, 128 },
	{ "GD_5F4GQ4UC", 0xC8, 0xB4, 0x68, 1, 4096, 64, 2048, 256 },
	{ "GD_5F1GQ4UB", 0xC8, 0xD1, 0x00, 1, 2048, 64, 1024, 128 },
	{ "GD_5F2GQ4UB", 0xC8, 0xD2, 0x00, 1, 2048, 64, 2048, 128 },
	{ "MX35LF1GE4AB", 0xC2, 0x12, 0x00, 2, 2048, 64, 1024, 128 },
	{ "MX35LF2GE4AB", 0xC2, 0x22, 0x00, 2, 2048, 64, 2048, 128 },
	{ "PN26G01AWSIUG", 0xA1, 0xE1, 0x00, 4, 2048, 64, 1024, 128 },
	{ "XT26G01A", 0x0B, 0xE1, 0x00, 4, 2048, 64, 1024, 64 },
	{ "XT26G02A", 0x0B, 0xE2, 0x00, 4, 2048, 64, 2048, 64 },
	{ "W25N01GV", 0xEF, 0xAA, 0x21, 3, 2048, 64, 1024, 64 },
	{ "ATO25D1GA", 0x9B, 0x12, 0x00, 1, 2048, 64, 1024, 64 },
	{ "TC58CVG2S0HRAIG", 0x98, 0xCD, 0x00, 5, 4096, 64, 2048, 128 },
};

//re-arrange the spare data
//0~3 -> 4~7, 4~7 -> 20~23, 8~11 -> 36~39, 12~15 -> 52~55,
//16~19(good block code) -> 0~3(NO ECC protect); 20~23 -> 16~19(NO ECC protect)
static uint8_t gWbSprMapTable[] = { 0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,
									0x24,0x25,0x26,0x27,0x34,0x35,0x36,0x37,
									0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13 };
								
//0~3(4B) -> NO ECC(for bad block check)
//4~5(2B) -> has ECC(user meta data1),	6~12(13B) -> has ECC(H/W ECC data)
//13h~14h(2B) -> has ECC(user meta data1),	15h~21h(13B) -> has ECC(H/W ECC data)
//22h~23h(2B) -> has ECC(user meta data1),	24h~30h(13B) -> has ECC(H/W ECC data)
//31h~32h(2B) -> has ECC(user meta data1),	33h~3Fh(13B) -> has ECC(H/W ECC data)
//40h~7Fh(64B) -> NO ECC(user meta data2)
//
//A[]->PARAGON's spr; B[]:FTL's spr
//A[0~3] <=B[0~3]	
//A[4] <= B[4], A[5] <= B[6], A[13h~14h] <= B[8~9],
//A[22h,23h,31h,32h] <=B [16~19] (for good block code)
//A[64~] <=B [12~15,20~23]				
static uint8_t gPrgSprMapTable[] = {0x00,0x01,0x02,0x03,0x04,0x48,0x05,0x49,
									0x13,0x14,0x4A,0x4B,0x40,0x41,0x42,0x43,
									0x22,0x23,0x31,0x32,0x44,0x45,0x46,0x47};

//0~7(8B) -> NO ECC protect(for bad block check)
//8~48(40B) -> has ECC protect(user meta data),	0~24(24B) -> has ECC(H/W ECC data)
//49~63(16B) -> NO ECC(this area contains Internal ECC Data
//please check datasheet "XTX_SPI Nand_3.3V_1G_XT26G01A_Rev 0.1.pdf"
//
//A[]->XTX's spr; B[]:FTL's spr
//A[8~31] <=B[0~23]									
static uint8_t gXtxSprMapTable[] = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
									0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
									0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};

static int gTimeOutInterval = 0;

static int gHasDummyByteAddr = 0;

#ifdef	USE_MMP_DRIVER
struct timeval startT, endT;
static int gCurrTv = 0;
#else
static int gCurrTv = 0;
#endif

#ifdef	ENABLE_CTRL_1
static int gEnableRecoverNorGpioPin = 0;
#endif

#ifdef USE_NEW_MACRO
static uint32_t gEnSpiCsCtrl = 0;	//default: diable GPIO control SPI CS pin
#else
static uint32_t gEnSpiCsCtrl = 1;	//default: enable GPIO control SPI CS pin
#endif

static uint32_t gAddrHasDummyByte = 0;
static uint32_t gUseDummyReadId = 0;

static int gForceToStopErasing = 0;
static int gEnWinbondContinueRead = 0;

//=============================================================================
//                              spi Function WRAP
//=============================================================================
#ifdef	USE_MMP_DRIVER
static int spi_read(uint8_t *cBuf, uint32_t cLen, uint8_t *dBuf, uint32_t dLen)	//SPI_COMMAND_INFO *cmd)
{
	uint32_t rst;

	#ifdef	ENABLE_CTRL_1
	if(gEnSpiCsCtrl)	mmpSpiSetControlMode(SPI_CONTROL_NAND);
	#endif	
	
	if(dLen<16)	rst = mmpSpiPioRead(SPI_PORT, cBuf, cLen, dBuf, dLen, 8);
	else	rst = mmpSpiDmaRead(SPI_PORT, cBuf, cLen, dBuf, dLen, 8);
	
	#ifdef	ENABLE_CTRL_1
	if(gEnSpiCsCtrl)
	{
		mmpSpiResetControl();
		if(gEnableRecoverNorGpioPin)
		{
			mmpSpiSetControlMode(SPI_CONTROL_NOR);
			mmpSpiResetControl();
		}
	}
	#endif

	if(rst==SPI_OK)	return SPINF_OK;
	else			return SPINF_FAIL;
}

static int spi_write(uint8_t *cBuf, uint32_t cLen, uint8_t *dBuf, uint32_t dLen)	//(SPI_COMMAND_INFO *cmd)
{
	uint32_t rst;

	#ifdef	ENABLE_CTRL_1
	if(gEnSpiCsCtrl)	mmpSpiSetControlMode(SPI_CONTROL_NAND);
	#endif
	
	if(dLen<16)	rst = mmpSpiPioWrite(SPI_PORT, cBuf, cLen, dBuf, dLen, 8);
	else	rst = mmpSpiDmaWrite(SPI_PORT, cBuf, cLen, dBuf, dLen, 8);
	
	#ifdef	ENABLE_CTRL_1
	if(gEnSpiCsCtrl)
	{
		mmpSpiResetControl();
		if(gEnableRecoverNorGpioPin)
		{
			mmpSpiSetControlMode(SPI_CONTROL_NOR);
			mmpSpiResetControl();
		}
	}
	#endif
	
	if(rst==SPI_OK)	return SPINF_OK;
	else			return SPINF_FAIL;
}

static void spinf_error(uint8_t errCode)
{
	printf("[SPINF ERROR] error code = %x\n",errCode);
}
#else	//use win32 spi driver
static int spi_read(uint8_t *cBuf, uint32_t cLen, uint8_t *dBuf, uint32_t dLen)	//(SPI_COMMAND_INFO *cmd)
{
	uint32_t rst;
	rst = SpiRead(cLen, (PWriteControlByteBuffer)cBuf, dLen, (PReadDataByteBuffer)dBuf);
	if(rst==SPI_OK)	return SPINF_OK;
	else		return SPINF_FAIL;
}

static int spi_write(uint8_t *cBuf, uint32_t cLen, uint8_t *dBuf, uint32_t dLen)	//(SPI_COMMAND_INFO *cmd)
{
	uint32_t rst;
	rst = SpiWrite(cLen, (PWriteControlByteBuffer)cBuf, dLen, (PWriteDataByteBuffer)dBuf);

	if(rst==SPI_OK)	return SPINF_OK;
	else		return SPINF_FAIL;
}

static void spinf_error(uint8_t errCode)
{
	printf("[SPINF ERROR] error code = %x\n",errCode);
}
#endif

void _setTimeOutT(uint32_t tv)
{
#ifdef	USE_MMP_DRIVER
	#ifdef	EN_NO_RTC
	gCurrTv = 0;
	gTimeOutInterval = tv*100;
	#else
	gettimeofday(&startT, NULL);
	gTimeOutInterval = tv;
	#endif	
#else	
	gCurrTv = 0;
	gTimeOutInterval = tv*10;
#endif
}

static int _waitTimeOut(void)
{
#ifdef	USE_MMP_DRIVER
	#ifdef	EN_NO_RTC
	gCurrTv++;
	if(gCurrTv==gTimeOutInterval)	return 1;
	else
	{
		usleep(100);
		return 0;
	}
	#else
    gettimeofday(&endT, NULL);
    if(itpTimevalDiff(&startT, &endT) > gTimeOutInterval)    return 1;
	else	return 0;
	#endif
#else
	gCurrTv++;
	if(gCurrTv==gTimeOutInterval)	return 1;
	else							return 0;
#endif
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
void _SpiNf_InitAttr(unsigned char *pBuf)
{
	memset( (unsigned char*)&gSpiNfInfo, 0, sizeof(SPI_NF_INFO) );
	
    if(pBuf[0]==0xC8)	gSpiNfInfo.MID = (unsigned char)pBuf[0];
    else	printf("Error: MID not support!!\n");
    
    if(pBuf[1]==0xB1)	gSpiNfInfo.DID0 = (unsigned char)pBuf[1];
    else	printf("Error: DID0 not support!!\n");

    if(pBuf[2]==0x48)	gSpiNfInfo.DID1 = (unsigned char)pBuf[2];
    else	printf("Error: DID1 not support!!\n");
    
    if(gSpiNfInfo.MID!=0xC8)
    {
		//vendor not support
    }
    
    if( (gSpiNfInfo.DID0==0xB1) && (gSpiNfInfo.DID1==0x48) )
    {
    	printf("SPI NF ID is VALID!!\n");
    	gSpiNfInfo.Init = 1;
    	gSpiNfInfo.PageSize = 2048;
    	gSpiNfInfo.PageInBlk = 64;
    	gSpiNfInfo.TotalBlk = 256;
    	gSpiNfInfo.SprSize = 128;    	
    }
}

static int _setAttribute(SPI_NF_INFO *cInfo, unsigned char *id)
{
	SPINF_CFG *snCfg;
	int snCfgCnt = 0;
	int isGotAttr = 0;
	int i = 0;

	//printf("setInfo:[%x,%x][%x,%x]\n",cInfo,cInfo->fromCfg,id,id[0]);

	if(cInfo->fromCfg)
	{
		if(!cInfo->CfgArray)	return (0x18);

		if(!cInfo->CfgCnt)	return (0x19);

		snCfg = (SPINF_CFG*)cInfo->CfgArray;
		snCfgCnt = cInfo->CfgCnt;
	}
	else
	{
		snCfg = g_SpiNfCfgArray;
		snCfgCnt = sizeof(g_SpiNfCfgArray)/sizeof(SPINF_CFG);
	}

	for(i=0; i<snCfgCnt; i++)
	{
		if( (id[0]==snCfg[i].cfgMID) && (id[1]==snCfg[i].cfgDID0) && (id[2]==snCfg[i].cfgDID1) )
		{
			isGotAttr = 1;
			break;
		}
	}

	if(gUseDummyReadId && isGotAttr)    gHasDummyByteAddr = 1;

	if(!isGotAttr)	return (0x1A);	//ID not support

	gSpiNfInfo.Init = 1;
	gSpiNfInfo.MID = id[0];
	gSpiNfInfo.DID0 = id[1];
	gSpiNfInfo.DID1 = id[2];
	gSpiNfInfo.BBM_type = snCfg[i].cfgBBM_type;
	gSpiNfInfo.BootRomSize = 0;	//16MB = 2048*64*32;
	gSpiNfInfo.PageInBlk = snCfg[i].cfgPageInBlk;
	gSpiNfInfo.PageSize = snCfg[i].cfgPageSize;
	gSpiNfInfo.TotalBlk = snCfg[i].cfgTotalBlk;
	gSpiNfInfo.SprSize = snCfg[i].cfgSprSize;
	gSpiNfInfo.FtlSprSize = 24;

	memcpy((char*)gSpiNfInfo.name, (char*)&snCfg[i].cfgDevName, 32);

	memcpy( (unsigned char*)cInfo, (unsigned char*)&gSpiNfInfo, sizeof(SPI_NF_INFO) );

	printf("isGotAttr=%x, dummyAddr=%x\n",isGotAttr, gHasDummyByteAddr);

	return (0);
}

static uint8_t _cmdReset(void)
{
	unsigned char SpiCmd[1];
	unsigned char dBuf;
	int spiret;
	uint8_t  status=0x01;
	uint8_t  result = 0x31;
	
	//printf("_cmdReset.1\n");
	
    //FFH (reset)
    SpiCmd[0] = 0xFF;
    
    spiret = spi_write(&SpiCmd[0], 1, &dBuf, 0);	
    
    //0F (get feature)
	_setTimeOutT(3000);
	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))	
	    {
	    	printf(" get feature FAIL(reset1)!!\n");
	    	goto resetEnd;
	    }
	    //else                            	printf(" status = %x\n",status);
	    
	    if(_waitTimeOut())
	    {
	    	printf("reset command time out!!\n");
	    	goto resetEnd;
	    }
	    
	    usleep(0); 
	}

	result = 0;

resetEnd:
		
	//printf("leave reset cmd R=%x\n",status);
	return result;
}

static uint8_t _checkEccStatus_GD(uint32_t blk, uint32_t ppo, uint8_t s)
{
    if(gUseDummyReadId)
    {
        switch(s>>4)
        {
            case 0x3:
                printf("spiNfRdGdE3(%x,%x), s=%x\n",blk,ppo,s);
                break;
                
            case 0x2:
                printf("spiNfRdGdE2(%x,%x), s=%x\n",blk,ppo,s);
                break;
                
            case 0x1:
                {
                	uint8_t sF0 = 0;
                	
                	if(!spiNf_getFeature(0xF0, &sF0))
                	{
                		uint8_t s1 = (sF0>>4)&0x03;
                		if(s1)	printf("NfRdGdE2(%x,%x,%x)(%x,%x)\n",blk,ppo,s1+4,s,sF0);
                		//else	printf("NfRdGdE1(%x,%x)(%x,%x)\n",blk,ppo,s,sF0);
                	}
                }
                break;
                
            case 0x0:
                //printf("spiNfRdGdE0(%x,%x), b=%d, s=%x\n",blk,ppo,s);
                break;
                
            default:
                printf("incorrect status of feature register!!!reg[C0]=%x\n",s);
                return (22);
                break;
        }  
    }
    else
    {
        switch((s>>4) & 0x7)
        {
            case 0x7:
                printf("spiNfRdGdE3(%x,%x,%x)\n",blk,ppo,s);
                return (20);
                break;
            case 0x6:
            case 0x5:
            case 0x4:
                printf("spiNfRdGdE2(%x,%x), b=%d, s=%x\n",blk,ppo,(s>>4)+2,s);
                break;                
            case 0x3:
            case 0x2:
            case 0x1:
                //printf("spiNfRdGdE1(%x,%x), b=%d, s=%x\n",blk,ppo,(s>>4)+2,s);
                break;
            case 0x0:
                //printf("spiNfRdGdE0(%x,%x), b=%d, s=%x\n",blk,ppo,s);
                break;
        }  
    }

    return (0);
}

static uint8_t _checkEccStatus_MXIC(uint32_t blk, uint32_t ppo, uint8_t s)
{
    switch(s>>4)
    {
        case 0x2:
            printf("NfRdMxicE2(%x,%x,%x)\n",blk,ppo,s);
            return (23);
        case 0x1:
            printf("NfRdMxicE1(%x,%x,%x)\n",blk,ppo,s);
            break;
        case 0x0:
            //printf("NfRdMxicE0(%x,%x,%x)\n",blk,ppo,s);
            break;

        case 0x3:            
        default:
            printf("incorrect status of feature register!!!reg[C0]=%x\n",s);
            return (22);
            break;
    }  
    return (0);
}

static uint8_t _checkEccStatus_WINBOND(uint32_t blk, uint32_t ppo, uint8_t s)
{
    switch(s>>4)
    {
        case 0x3:      
        case 0x2:
            printf("NfRdWbE2(%x,%x,%x)\n",blk,ppo,s);
            return (23);
        case 0x1:
            printf("NfRdWbE1(%x,%x,%x)\n",blk,ppo,s);
            break;
        case 0x0:
            //printf("NfRdWbE0(%x,%x,%x)\n",blk,ppo,s);
            break;
          
        default:
            printf("incorrect status of feature register!!!reg[C0]=%x\n",s);
            return (22);
            break;
    }  
    return (0);
}

/*
ref: page ?
00b = No bit errors were detected during the previous read algorithm. 
01b = bit error was detected and corrected, error bit number = 1~7. 
10b = bit error was detected and not corrected.
11b = bit error was detected and corrected, error bit number = 8.
*/
static uint8_t _checkEccStatus_XTX(uint32_t blk, uint32_t ppo, uint8_t s)
{
    switch(s>>4)
    {
        case 0x3:
            printf("NfRdXtxE3(%x,%x,%x)\n",blk,ppo,s);
            //means ECC error and corrected (8 bits error)
            //return (4);
            break;   
        case 0x2:
            printf("NfRdXtxE2(%x,%x,%x)\n",blk,ppo,s);
            //means ECC error and NOT be corrected
            return (23);
        case 0x1:
            //printf("NfRdXtxE1(%x,%x,%x)\n",blk,ppo,s);
            //means ECC error and corrected (1~7 bits error)
            break;
        case 0x0:
            //printf("NfRdXtxE0(%x,%x,%x)\n",blk,ppo,s);
            //mean no ECC error
            break;          
        default:
            printf("incorrect status of feature register!!!reg[C0]=%x\n",s);
            return (22);
            break;
    }  

    return (0);
}

/*
ref: page ? NO status for checking ECC 
*/
static uint8_t _checkEccStatus_ATO(uint32_t blk, uint32_t ppo, uint8_t s)
{	
    return (0);
}

static uint8_t _checkEccStatus_TOSHIBA(uint32_t blk, uint32_t ppo, uint8_t s)
{
    switch(s>>4)
    {
        case 0x3:
            printf("NfRdTsbE3(%x,%x,%x), data has been corrected\n",blk,ppo,s);
            //means ECC error and corrected (8 bits error)
            //return (4);
            break;   
        case 0x2:
            printf("NfRdTsbE2(%x,%x,%x)\n",blk,ppo,s);
            //means ECC error and NOT be corrected
            return (23);
        case 0x1:
            //printf("NfRdXtxE1(%x,%x,%x)\n",blk,ppo,s);
            //means ECC error and corrected (1~7 bits error)
            break;
        case 0x0:
            //printf("NfRdXtxE0(%x,%x,%x)\n",blk,ppo,s);
            //mean no ECC error
            break;          
        default:
            printf("incorrect status of feature register!!!reg[C0]=%x\n",s);
            return (22);
            break;
    }  

    return (0);
}

static uint8_t _checkEccStatus(uint32_t b, uint32_t p)
{
	uint8_t rst=0;
	uint8_t status=0;	
	
	if(!spiNf_getFeature(0xC0, &status))
	{	
		//printf(" get feature OK for ECC check, status=%x!!\n",status);
	    switch(gSpiNfInfo.MID)
	    {
	    	case 0xC8:
	   			rst = _checkEccStatus_GD(b,p,status);
	   			break;
	   			
	   		case 0xC2:
	   			rst = _checkEccStatus_MXIC(b,p,status);
	   			break;
	   			
	   		case 0xEF:
	   			rst = _checkEccStatus_WINBOND(b,p,status);
	   			break;
	   				
	   		case 0x98:
	   			rst = _checkEccStatus_TOSHIBA(b,p,status);
	   			break;
	   			
	   		case 0xA1:
	   		case 0x0B:
	   			rst = _checkEccStatus_XTX(b,p,status);
	   			break;
	   					
	   		case 0x9B:
	   			rst = _checkEccStatus_ATO(b,p,status);
	   			break;
	   				
	   		default:
	   			printf("_checkEccStatus(): incorrect Factory ID!!\n",gSpiNfInfo.MID);
	   			rst = 0x23;
	   			break;
	   	}
   	}

   	return rst;
}

/*
remapping spare data structure
*/
static void _reMapSpare(uint8_t *ptr1,uint8_t *ptr2, uint8_t mapType)
{
	int i;
	uint8_t *reMap;
	
	if( (gSpiNfInfo.MID!=0xEF) && (gSpiNfInfo.MID!=0xA1) && (gSpiNfInfo.MID!=0x0B) )
	{
		printf("SPI NAND ERROR!! this MID(%x) is NOT suitable for spare data re-mapping\n",gSpiNfInfo.MID);
		return;
	}
	
	if(gSpiNfInfo.MID==0xEF)	reMap = (uint8_t*)gWbSprMapTable;
		
	if(gSpiNfInfo.MID==0xA1)	reMap = (uint8_t*)gPrgSprMapTable;

	if(gSpiNfInfo.MID==0x0B)	reMap = (uint8_t*)gXtxSprMapTable;

	if(mapType == SPINF_REMAP_SPARE_FOR_READ)
	{
		//from NAND to Memory(read)
		for(i=0; i<24; i++)
		{			
			ptr1[i] = ptr2[reMap[i]];
			//printf("i=[%d][%x,%x,%x]\n",i,reMap[i],ptr2[reMap[i]],ptr1[i]);
		}
			
	}
	else
	{
		//from Memory to NAND(write)
		for(i=0; i<24; i++)	
		{			
			ptr1[reMap[i]] = ptr2[i];
			//printf("i=[%d][%x,%x,%x]\n", i, reMap[i], ptr2[i], ptr1[reMap[i]]);
		}
	}
}

#define TOSHIBA_HSE_BIT         (0x02)
#define TOSHIBA_HSE_ENABLE      (1)
#define TOSHIBA_HSE_DISABLE     (0)

static void _enHighSpeedMode(uint8_t mode)
{
    unsigned char regStatus;
    
    if( spiNf_getFeature(0xB0, &regStatus) )
    {
	    printf("[SPINF ERR] get feature commnad fail\n");
	    return;
    }
    
    if(mode)
    {
        if( !(regStatus & TOSHIBA_HSE_BIT) )
        {
            //go to enable TOSHIBA_HSE_BIT
            spiNf_setFeature(0xB0, (regStatus | TOSHIBA_HSE_BIT));
        }
    }
    else
    {
        if(regStatus & TOSHIBA_HSE_BIT)
        {
            //go to disable TOSHIBA_HSE_BIT
            spiNf_setFeature(0xB0, (regStatus & ~TOSHIBA_HSE_BIT));
        }
    }
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
* read ID of SPI NAND
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_ReadId(unsigned char *id)
{
	unsigned char SpiCmd[2];
	unsigned char SpiData[3];
	unsigned char regStatus;
	int spiret,i;	
    
	//reset 0xFF
    if(_cmdReset())
    {
        printf("[SPINF ERR] reset commnad fail:0\n");
        spiret = SPINF_ERROR_RESET_CMD_FAIL;
        goto errEnd;
    }
    
    for(i=0; i<4; i++)
    {
	      if( spiNf_getFeature(0xA0+0x10*i, &regStatus) )
	      {
	          printf("[SPINF ERR] get feature commnad fail\n");
	          spiret = SPINF_ERROR_GET_FEATURE_CMD_FAIL;
	          goto errEnd;
	      }    
    }
    
	SpiData[0]=0;SpiData[1]=0;SpiData[2]=0;
	SpiCmd[0] = 0x9F;
	spiret = spi_read(SpiCmd, 1, SpiData, 3);
	
    id[0] = SpiData[0];
    id[1] = SpiData[1];
    id[2] = SpiData[2];

	if(spiret==SPINF_OK)
	{
		printf("SPI READ ID PASS, data=%02x,%02x,%02x\n",id[0],id[1],id[2]);
		spiret = 0;
	}
	else	
	{
		printf("SPI READ ID FAIL,result=%x\n",spiret);  
	}

errEnd:
	
	return (spiret);
}

//=============================================================================
/**
* read ID with dummy byte of SPI NAND
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_DummyReadId(unsigned char *id)
{
	unsigned char SpiCmd[2];
	unsigned char SpiData[3];
	unsigned char regStatus;
	int spiret,i;
	
	//reset 0xFF
    if(_cmdReset())
    {
        printf("[SPINF ERR] (dummy read ID)reset commnad fail:0\n");
        spiret = SPINF_ERROR_RESET_CMD_FAIL;
        goto errEnd;
    }
	
    for(i=0; i<4; i++)
    {
	      if( spiNf_getFeature(0xA0+0x10*i, &regStatus) )
	      {
	          //printf("[SPINF ERR] get feature commnad fail\n");
	          spiret = SPINF_ERROR_GET_FEATURE_CMD_FAIL;
	          goto errEnd;
	      }
    }
	
	SpiData[0]=0;SpiData[1]=0;SpiData[2]=0;	
	SpiCmd[0] = 0x9F; SpiCmd[1] = 0x00;
	spiret = spi_read(SpiCmd, 2, SpiData, 3);

	id[0] = SpiData[0];
	id[1] = SpiData[1];
	
    //GD & MXIC has 2 IDs, but Winbond has 3 IDs.
	if(SpiData[0]!=0xEF)
	    id[2] = 00;
	else
	    id[2] = SpiData[2];

	if(spiret==SPINF_OK)
	{
		printf("SPI READ ID(with dummy byte) PASS, data=%02x,%02x,%02x\n",id[0],id[1],id[2]);
		gUseDummyReadId = 1;
		spiret = 0;
	}
	else	
	{
		//printf("SPI READ ID FAIL,result=%x\n",spiret);  
	}
	
errEnd:

	return (spiret);
}

//=============================================================================
/**
* initialization of SPI NAND
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_Initial(SPI_NF_INFO *info)
{
	unsigned char idBuf[4];

	gUseDummyReadId = 0;

	//readId
	if( spiNf_ReadId(idBuf) )  return 0x11;

	if( idBuf[0]==0xFF || idBuf[0]==0x00 )
	{
		if( spiNf_DummyReadId(idBuf) )	return 0x11;
	}

	//get attribute from cfg(cfgArray)
	if(_setAttribute(info, idBuf))	return 0x12;

	if(gSpiNfInfo.MID==0xEF)
	{
	    uint8_t  status = 0;
	    //do winbond initial flow
	    if(!spiNf_getFeature(0xA0, &status))   printf(" get feature(A0)=%x!!\n",status);
	    if(!spiNf_getFeature(0xB0, &status))   printf(" get feature(B0)=%x!!\n",status);
	    if( !(status&0x08) )	spiNf_setFeature(0xB0, 0x18);
	    if(!spiNf_getFeature(0xC0, &status))   printf(" get feature(C0)=%x!!\n",status);
	}
	printf("[Spinfdrv.c] MID=%x,MID0=%x,MID1=%x!\n",gSpiNfInfo.MID,gSpiNfInfo.DID0,gSpiNfInfo.DID1);
	if( (gSpiNfInfo.MID==0xA1) || (gSpiNfInfo.MID==0x0B) )
	{
	    uint8_t  status = 0;
	    //do XTX initial flow
	    if(!spiNf_getFeature(0xA0, &status))   printf(" get feature(A0)=%x!!\n",status);
	    if(!spiNf_getFeature(0xB0, &status))   printf(" get feature(B0)=%x!!\n",status);
	    if( !(status&0x10) )	spiNf_setFeature(0xB0, (status|0x10) );
	    if(!spiNf_getFeature(0xC0, &status))   printf(" get feature(C0)=%x!!\n",status);
	    if(!spiNf_getFeature(0x90, &status))   printf(" get feature(90)=%x!!\n",status);
	}
	
	return 0x00;
}
//=============================================================================
/**
* Get feature of SPI NAND
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_getFeature(uint8_t addr, uint8_t *buf)
{
	unsigned char SpiCmd[2];
	unsigned char SpiData[4];
	int spiret;
	
	SpiCmd[0] = 0x0F;	

	if(!buf)	return (4);
	
	SpiCmd[1] = addr;
	spiret = spi_read(SpiCmd, 2, &SpiData[0], 1);

	if(spiret==SPINF_OK)
	{
	    *buf = SpiData[0];
	    //printf("SPI NF GET feature OK, data[%02X]=%02x\n",addr,SpiData[0]);
	    return (0);
	}
	else
	{
	    printf("SPI NAND GET feature FAIL,result=%x\n",spiret); 
	    return (5);
	}
}

//=============================================================================
/**
* Set feature of SPI NAND
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_setFeature(uint8_t addr, uint8_t Reg)
{
	unsigned char SpiCmd[2];
	unsigned char SpiData[2];
	int spiret;
	uint8_t  mask[4] = {0xBE,0xD1,0x00,0xC0};	//reg[C0] is read only (status)
	
	SpiCmd[0] = 0x1F;
	SpiCmd[1] = addr;

	if(gSpiNfInfo.MID==0xC8)
	{
	    SpiData[0] = Reg & mask[(addr - 0xA0) >> 4];		
	    if(SpiData[0]!=Reg)    printf("warning: THE VALUE=%x,set=%x, index=%x\n",Reg,SpiData[0],(addr - 0xA0) >> 4);
	}
	else
	{
	    SpiData[0] = Reg;	
	}

	spiret = spi_write(SpiCmd, 2, &SpiData[0], 1);
    
    if(spiret==SPINF_OK)
	{
	    //printf("Set Feature OK!!\n");
	    return (0);
	}
	else
	{
	    printf("SPI NAND SET feature FAIL,result=%x\n",spiret);
	    return (1);
	}
}

uint8_t spiNf_ByteRead(uint32_t blk, uint32_t ppo, uint8_t *dBuf, uint32_t offset, uint32_t rLen)
{
	unsigned char SpiCmd[4];
	unsigned char SpiData[3];
	int spiret;
	uint8_t  status=0x01;
	uint32_t addr = blk*gSpiNfInfo.PageInBlk + ppo;
	uint8_t result = 1;
	uint8_t reMapSpr = 0;
	uint32_t oriOffset = offset;
	uint32_t oriRdLen = rLen;
	uint32_t tsbRdOffset = 4;
	unsigned char tmpData[128];	

	status = NF_STATUS_OIP;
	_setTimeOutT(2000);
	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))   printf(" get feature FAIL(PgRd)!!\n");
	    if(_waitTimeOut())
	    {
	    	printf("readbyte command time out[1]!!\n");
	    	goto errRdEnd;
	    }
	    
	    usleep(0); 
	}

	if( ((gSpiNfInfo.MID==0xEF) || (gSpiNfInfo.MID==0xA1) || (gSpiNfInfo.MID==0x0B) ) && (offset>=gSpiNfInfo.PageSize) )
	{
		reMapSpr = 1;
		oriOffset = offset;
		offset = gSpiNfInfo.PageSize;	
	}
	
	//to workaround the toshiba random read page issue
	if( (gSpiNfInfo.MID==0x98) && (offset >= gSpiNfInfo.PageSize) )
	{
        //1.read the last 4-bytes of last sector(sector 7) 
        //2.and then read the 24-bytes of 0th spare data
        //3.please reference to datasheet "Definition of 528 bytes Data Pair"
	    offset = gSpiNfInfo.PageSize - tsbRdOffset;
	    rLen = tsbRdOffset + gSpiNfInfo.FtlSprSize;
	}
		
	//if(gSpiNfInfo.MID==0x98) _enHighSpeedMode(0);
	
	SpiCmd[0] = 0x13;
	SpiCmd[1] = (addr>>16)&0xFF;
	SpiCmd[2] = (addr>>8)&0xFF;
	SpiCmd[3] = addr&0xFF;
	spiret = spi_write(SpiCmd, 4, &SpiData[0], 0);
	if(spiret!=SPINF_OK)
	{
		printf("send cmd 0x13 FAIL,R1=(%x,%x)\n",spiret,SPINF_OK);
		goto errRdEnd;
	}
	
	status = NF_STATUS_OIP;
	_setTimeOutT(2000);
	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))   printf(" get feature FAIL(PgRd)!!\n");
	    if(_waitTimeOut())
	    {
	    	printf("readbyte command time out[2]!!\n");
	    	goto errRdEnd;
	    }
	    
	    usleep(0); 
	}
	
	SpiCmd[0] = 0x03;

	if(gHasDummyByteAddr)
	{
		SpiCmd[1] = (unsigned char)((offset>>8) & 0x0F);
		SpiCmd[2] = (unsigned char)(offset & 0xFF);
		SpiCmd[3] = 0x00;
	}
	else
	{
		SpiCmd[1] = 0x00;
		SpiCmd[2] = (unsigned char)((offset>>8) & 0xFF);
		SpiCmd[3] = (unsigned char)(offset & 0xFF); //bit0 must be "0"
	}

    if(reMapSpr)
    	spiret = spi_read(SpiCmd, 4, tmpData, gSpiNfInfo.SprSize);
	else
	{
	    if( (gSpiNfInfo.MID == 0x98) && (oriOffset != offset) )  spiret = spi_read(SpiCmd, 4, tmpData, rLen);
		else    spiret = spi_read(SpiCmd, 4, dBuf, rLen);
    }
	
	if(spiret!=SPINF_OK)
	{
		printf("send cmd 0x13 FAIL,R2=(%x,%x)\n",spiret,SPINF_OK);
		goto errRdEnd;
	}
	
	//if(gSpiNfInfo.MID==0x98) _enHighSpeedMode(1);
	
	if(reMapSpr)	ithInvalidateDCacheRange((uint32_t*)tmpData, gSpiNfInfo.SprSize);
	else
	{
	    if( (gSpiNfInfo.MID == 0x98) && (oriOffset != offset) )
	    {
	        ithInvalidateDCacheRange((uint32_t*)tmpData, rLen);
	        memcpy(&dBuf[0], &tmpData[tsbRdOffset + (oriOffset - gSpiNfInfo.PageSize)], oriRdLen);
	    }
	    else
	        ithInvalidateDCacheRange((uint32_t*)&dBuf[0], rLen);
	}
	
	if(reMapSpr)	
	{
    	int j;
    	uint8_t tmpBuf[24];
    	uint8_t *ptr;
    	
    	offset = oriOffset;

        //re-arrange the spare data
        if( rLen == 24 )	
        	ptr = (uint8_t*)dBuf;
        else
        	ptr = (uint8_t*)&tmpBuf[0];       	

		//read half data
	    memset(tmpBuf, 0xFF, 24);

	    _reMapSpare(ptr,tmpData,SPINF_REMAP_SPARE_FOR_READ);
        
        if( rLen != 24 )   
        	for(j= 0; j<rLen; j++)	dBuf[j] = ptr[offset-gSpiNfInfo.PageSize+j];
	}

	result = _checkEccStatus(blk,ppo);
    
errRdEnd:

	return result;
}

//=============================================================================
/**
* SPI NAND read page data
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_PageRead(uint32_t blk, uint32_t ppo, uint8_t *dBuf)	//(uint32_t addr, uint8_t *dBuf)
{
	unsigned char SpiCmd[8];
	unsigned char SpiData[3];
	int spiret;
	uint8_t  status=0x01;
	uint32_t addr = blk*gSpiNfInfo.PageInBlk + ppo;
	uint8_t result = 1;

	status = NF_STATUS_OIP;
	_setTimeOutT(2000);
	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))   printf(" get feature FAIL(PgRd)!!\n");
	    if(_waitTimeOut())
	    {
	    	printf("readpage command time out[1]!!\n");
	    	goto errRdEnd;
	    }
	    
	    usleep(0); 
	}

	SpiCmd[0] = 0x13;
	SpiCmd[1] = (addr>>16)&0xFF;
	SpiCmd[2] = (addr>>8)&0xFF;
	SpiCmd[3] = addr&0xFF;
	spiret = spi_write(SpiCmd, 4, &SpiData[0], 0);
	
	if(spiret!=SPINF_OK)
	{
		printf("send cmd 0x13 FAIL,R1=(%x,%x)\n",spiret,SPINF_OK);
		goto errRdEnd;
	}
	
	status = NF_STATUS_OIP;
	_setTimeOutT(2000);
	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))   printf(" get feature FAIL(PgRd)!!\n");
	    if(_waitTimeOut())
	    {
	    	printf("readpage command time out[2]!!\n");
	    	goto errRdEnd;
	    }
		usleep(0);
	}
	
	SpiCmd[0] = 0x03;
	SpiCmd[1] = 0x00;
	SpiCmd[2] = 0x00;
	SpiCmd[3] = 0x00;

	if( (gSpiNfInfo.MID==0xEF) || (gSpiNfInfo.MID==0xA1) || (gSpiNfInfo.MID==0x0B) )
	{
	    spiret = spi_read(SpiCmd, 4, &dBuf[0], gSpiNfInfo.PageSize+gSpiNfInfo.SprSize);
	    ithInvalidateDCacheRange((uint32_t*)&dBuf[0], gSpiNfInfo.PageSize+gSpiNfInfo.SprSize);
	}
	else
	{
	    spiret = spi_read(SpiCmd, 4, &dBuf[0], gSpiNfInfo.PageSize+gSpiNfInfo.FtlSprSize);
	ithInvalidateDCacheRange((uint32_t*)&dBuf[0], gSpiNfInfo.PageSize+gSpiNfInfo.FtlSprSize);
	}
	
	if( (gSpiNfInfo.MID==0xEF) || (gSpiNfInfo.MID==0xA1) || (gSpiNfInfo.MID==0x0B) )
	{
		//read half data
	    uint8_t tmpBuf[24];
	    memset(tmpBuf, 0xFF, 24);

	    _reMapSpare(&tmpBuf[0], &dBuf[gSpiNfInfo.PageSize], SPINF_REMAP_SPARE_FOR_READ);
   
	    memcpy(&dBuf[gSpiNfInfo.PageSize], &tmpBuf[0], 24);
	}
	
	if(spiret!=SPINF_OK)
	{
		printf("send cmd 0x13 FAIL,R2=(%x,%x)\n",spiret,SPINF_OK);
		goto errRdEnd;
	}
	result = _checkEccStatus(blk,ppo);
    
	if(result)	printf("chkEccErr1:(%x)\n",result);
		
errRdEnd:

	if(result)	printf("chkEccErr2:(%x)\n",result);

	return result;
}

//=============================================================================
/**
* SPI NAND program page data
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_PageProgram(uint32_t blk, uint32_t ppo, uint8_t *dBuf)
{
	unsigned char SpiCmd[4];
	int spiret;
	uint8_t  status=0x01;
	uint32_t addr = blk*gSpiNfInfo.PageInBlk + ppo;
	uint8_t  wpStatus=0x0;
	
	if(spiNf_getFeature(0xA0, &status))   printf(" get feature [0xA0] = %x!!\n",status);		
				
	if(status&0x38)
	{
	    //unlock blocks
	    printf("unlock blocks, s=%x\n",status);
	    spiNf_setFeature(0xA0, 0x00);
	}
		
	if(spiNf_getFeature(0xC0, &wpStatus))   printf(" get feature [0xC0] = %x!!\n",wpStatus);
		
	if(wpStatus & NF_STATUS_OIP)
	{
		_setTimeOutT(3000);
    	while(wpStatus & NF_STATUS_OIP)
		{
		    if(spiNf_getFeature(0xC0, &wpStatus))   printf(" get feature FAIL(wpS=%x)!!\n",wpStatus);
			if(_waitTimeOut())
			{
				printf("Program Fail, OIP still is 1, wpS=%x\n",wpStatus);
				break;
			}
			usleep(10); 
		}			
	}
		
    //06H (write enable)
    if(wpStatus & NF_STATUS_WEL)
    {
        printf("WEL=1, C0=%x, A0=%x\n",wpStatus,status);
    }
    else
    {
        //printf("WEL=0\n");
	    SpiCmd[0] = 0x06;
	    spiret = spi_write(SpiCmd, 1, &SpiCmd[0], 0);
	    if(spiret!=SPINF_OK)	spinf_error(SPINF_ERROR_CMD_WT_EN_ERR);
	}
		
    ithFlushDCacheRange((void*)&dBuf[0], gSpiNfInfo.PageSize+gSpiNfInfo.FtlSprSize);
    ithFlushMemBuffer();
	
    //02H(program load)
	SpiCmd[0] = 0x02;	
    SpiCmd[1] = 0x00;
    SpiCmd[2] = 0x00;
        
	if( (gSpiNfInfo.MID==0xEF) || (gSpiNfInfo.MID==0xA1) || (gSpiNfInfo.MID==0x0B) )
    {
        uint8_t tmpBuf[24];
        memcpy(&tmpBuf[0], &dBuf[gSpiNfInfo.PageSize+0], 24);
	    memset(&dBuf[gSpiNfInfo.PageSize], 0xFF, gSpiNfInfo.SprSize);
        
	    _reMapSpare(&dBuf[gSpiNfInfo.PageSize], &tmpBuf[0], SPINF_REMAP_SPARE_FOR_WRITE);	   
        
        ithFlushDCacheRange((void*)&dBuf[gSpiNfInfo.PageSize], gSpiNfInfo.SprSize);
        ithFlushMemBuffer();        
    }

	spiret = spi_write(SpiCmd, 3, &dBuf[0], gSpiNfInfo.PageSize+gSpiNfInfo.SprSize);
	
    //10H (program execute)
	SpiCmd[0] = 0x10;	
	SpiCmd[1] = (addr>>16)&0xFF;
	SpiCmd[2] = (addr>>8)&0xFF;
	SpiCmd[3] = addr&0xFF;
	spiret = spi_write(SpiCmd, 4, &SpiCmd[0], 0);

    //0FH (get feature) (wait busy)
	_setTimeOutT(3000);
    status=NF_STATUS_OIP;

	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))   printf(" get feature FAIL(PgPg)!!\n");
		if(_waitTimeOut())
		{
			printf("Program timeout, OIP is 1, wpS=%x\n",wpStatus);
			return (12);
		}
		usleep(0); 
	}
	
    //04H (write Disable)
	SpiCmd[0] = 0x04;	
	spiret = spi_write(SpiCmd, 1, &SpiCmd[0], 0);
	
	if(status & NF_STATUS_P_FAIL)
	{
		if(_cmdReset())	printf("[SPINF ERR] reset commnad fail:1\n",status);	
			
		printf("SPI NAND PROGRAM FAIL, status=%x\n",status);
		
		return (9);
	}
	else               return (0);
}

//=============================================================================
/**
* SPI NAND erase block data
*
* @return 0 If successful. Otherwise, return a nonzero value.
*/
//=============================================================================
uint8_t spiNf_BlockErase(uint32_t blk)
{
	unsigned char SpiCmd[4];
	int spiret;
	uint8_t  status=0x01;
	uint8_t  wpStatus=0x0;
	
	if(!spiNf_getFeature(0xC0, &status))
	{
		if(status & NF_STATUS_E_FAIL)
		{
			if(_cmdReset())	
			{
				printf("[SPINF ERR] reset commnad fail:2, status=%x\n",status);		
			}
			printf("[SPINF ERR] ERASE fail:2, status=%x\n",status);	
		}
		wpStatus = status;
	}

	if(spiNf_getFeature(0xA0, &status))   printf(" get feature [0xA0] = %x!!\n",status);		
	if(status&0x38)
	{
	    //unlock blocks
	    spiNf_setFeature(0xA0, 0x00);
	    spiNf_getFeature(0xA0, &status);
	    printf(" get [0xA0] = %x!!\n",status);
	}

	if(!spiNf_getFeature(0xC0, &wpStatus))
	{
		_setTimeOutT(3000);
    	while(wpStatus & NF_STATUS_OIP)
		{
		    if(spiNf_getFeature(0xC0, &wpStatus))   printf(" get feature FAIL(wpS=%x)!!\n",wpStatus);
			if(_waitTimeOut())
			{
				printf("Erase Fail, OIP is 1, wpS=%x\n",wpStatus);
				break;
			}
			usleep(10); 
		}	
	}

    //06H (write enable)
    if(wpStatus & NF_STATUS_WEL)
    {
        printf("WEL=1\n");
    }
    else
    {
        //printf("WEL=0\n");
	    SpiCmd[0] = 0x06;
	    spiret = spi_write(SpiCmd, 1, &SpiCmd[0], 0);
	    if(spiret!=SPINF_OK)	spinf_error(SPINF_ERROR_CMD_WT_EN_ERR);
	    //ithDelay(100);
	}

    //D8H
	SpiCmd[0] = 0xD8;	
    SpiCmd[1] = ((blk*gSpiNfInfo.PageInBlk)>>16)&0xFF;
	SpiCmd[2] = ((blk*gSpiNfInfo.PageInBlk)>>8)&0xFF;
	SpiCmd[3] = (blk*gSpiNfInfo.PageInBlk)&0xFF;
	spiret = spi_write(SpiCmd, 4, &SpiCmd[0], 0);
	if(spiret!=SPINF_OK)	spinf_error(SPINF_ERROR_CMD_ERS_ERR);
    
    //0F (get feature)
	_setTimeOutT(3000);
    status = NF_STATUS_OIP;
	while(status & NF_STATUS_OIP)
	{
	    if(spiNf_getFeature(0xC0, &status))	printf(" get feature FAIL(Ers)!!\n");
	    //else                            	printf(" status = %x\n",status);
		
		if(_waitTimeOut())
		{
		    printf("Erase timeout, OIP is 1, wpS=%x\n",wpStatus);
		    return (13);
		}
		usleep(0); 
	}
		
	//04H (write Disable)
	SpiCmd[0] = 0x04;
	spiret = spi_write(SpiCmd, 1, &SpiCmd[0], 0);
	if(spiret!=SPINF_OK)	spinf_error(SPINF_ERROR_CMD_WT_EN_ERR);
		
	//printf("Ers(%x,%x)\n",blk,status);
	
	if(status&0x04)    printf(" erase FAIL(blk=%x)!!\n",blk);
	
	if(status&0x04)    return (10);
	else               return (0);
}

void spiNf_SetSpiCsCtrl(uint32_t csCtrl)
{
	printf("spiNf_SetSpiCsCtrl(%x)\n",csCtrl);
	gEnSpiCsCtrl = csCtrl;
}

#ifdef __cplusplus
}
#endif

