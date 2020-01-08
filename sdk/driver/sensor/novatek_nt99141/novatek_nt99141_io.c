#include "ite/ith.h"
#include "ite/mmp_types.h"
#include "iic/iic.h"
#include "iic/mmp_iic.h"
#include "sensor/novatek_nt99141_io.h"    

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static uint8_t   NOVATEK_IICADDR = 0x54 >> 1;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
uint8_t 
NOVATEK_ReadI2C_8Bit(
    uint16_t RegAddr)
{
    int result;
    uint8_t  dbuf[256];
    uint8_t*  pdbuf = dbuf;
    uint8_t value;

    *pdbuf++ = (uint8_t)((RegAddr&0xff00)>>8);
    *pdbuf++ = (uint8_t)(RegAddr&0x00ff);

    mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, NOVATEK_IICADDR, dbuf, 2, pdbuf, 1);
    
    value = (dbuf[2] & 0xFF); 
    
    printf("0x%04x = 0x%02x %d\n", RegAddr, value, value);           
      
    return value;
}

int 
NOVATEK_WriteI2C_8Bit(
    uint16_t RegAddr,
    uint8_t  data)
{
    int result;
    uint8_t  dbuf[256];
    uint8_t*  pdbuf = dbuf;
    
    *pdbuf++ = (uint8_t)((RegAddr&0xff00)>>8);
    *pdbuf++ = (uint8_t)(RegAddr&0x00ff);
    *pdbuf = (uint8_t)(data);
    
    if(0 != (result = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, NOVATEK_IICADDR, dbuf, 3)))
    {
        printf("NOVATEK_WriteI2C_8Bit I2c Write Error, reg=%04x val=%02x\n", RegAddr, data);
    }        
    
    return result;
}
