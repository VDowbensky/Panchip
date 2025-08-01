#include "myFlashData.h"
#include "stmflash.h"

/**
 * 将数据写入flash
 * @pBuffer，写入数据地址指针
 * @writeLen，写入数据长度
    备注：
        写入数据以BYTE为单位，保存flash地址：@FLASH_APP1_PARAMS_ADDR
*/
void myFlash_write(uint8_t *pBuffer,uint16_t writeLen)
{
    STMFLASH_Write(FLASH_APP1_PARAMS_ADDR, (uint16_t *)pBuffer, (writeLen % 2) ? (writeLen + 1) : writeLen);
}
/**
 * 读取flash中的数据
 * @pBuffer，读取数据缓存地址指针
 * @writeLen，读取数据的长度
    备注：
        读取的数据以BYTE为单位，读取flash的初始地址：@FLASH_APP1_PARAMS_ADDR
*/
void myFlash_read(uint8_t *pBuffer,uint16_t readLen)
{
    uint16_t i;
    uint32_t readAddr = FLASH_APP1_PARAMS_ADDR;
    uint16_t *pBufferT = (uint16_t *)pBuffer;

    readLen = (readLen % 2) ? (readLen + 1) : readLen;

    for(i = 0; i < readLen; i++)
    {
        pBufferT[i] = STMFLASH_ReadHalfWord(readAddr);//读取2个字节.
        readAddr += 2;//偏移2个字节.	
    }
}
