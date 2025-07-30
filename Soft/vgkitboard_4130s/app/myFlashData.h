#ifndef __MYFLASHDATA_H__
#define __MYFLASHDATA_H__
#include <stdint.h>
#include <string.h>

/**
 * 	参数本地保存地址
 * 预留4Kflash空间，芯片总的flash空间256K（0X08040000）
 * 
*/
#define FLASH_APP1_PARAMS_ADDR    0X0803e000

/**
 * 将数据写入flash
 * @pBuffer，写入数据地址指针
 * @writeLen，写入数据长度
    备注：
        写入数据以BYTE为单位，保存flash地址：@FLASH_APP1_PARAMS_ADDR
*/
void myFlash_write(uint8_t *pBuffer,uint16_t writeLen);
/**
 * 读取flash中的数据
 * @pBuffer，读取数据缓存地址指针
 * @writeLen，读取数据的长度
    备注：
        读取的数据以BYTE为单位，读取flash的初始地址：@FLASH_APP1_PARAMS_ADDR
*/
void myFlash_read(uint8_t *pBuffer,uint16_t readLen);
#endif
