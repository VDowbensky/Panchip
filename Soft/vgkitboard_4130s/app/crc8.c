#include "crc8.h"

/**
 * @funtion:crc8多项式冗余校验
 * @param 1:pData,计算数据源地址
 * @param 2:dataLen,计算数据源长度
 * @param 3:initialValue,crc结果初值
 * @param 4:polynomial,多项式
 * 
 * @return :校验结果
*/
unsigned char crc8( unsigned char *pData, 
                    unsigned int dataLen,
                    unsigned char initialValue, 
                    unsigned char polynomial )
{
    unsigned char i;
    unsigned char crc;

    crc = initialValue;
    while (dataLen --)
    {
        crc ^= *pData ++;
        for( i = 0; i < 8; i++ )
        {
            if(crc & 0x80)
            {
                crc <<= 1; // shift left once
                crc ^= polynomial; // XOR with polynomial
            }
            else
            { 
                crc <<= 1; // shift left once
            }
        }
    }

    return crc;
}
/**
 * @funtion :针对温湿度传感器sht3X系列的crc8校验
 * @param 1:pData,计算数据源地址
 * @param 2:dataLen,计算数据源长度
 * @param 3:targetCRC,对比结果CRC
 * 
 * @return :对比校验结果，=1校验成功，=0校验失败
*/
unsigned char crc8_ger( unsigned char *pData, 
                 unsigned int dataLen)
{
    return crc8(pData, dataLen, 0xff, 0x31);
}
/**
 * @funtion :针对温湿度传感器sht3X系列的crc8校验
 * @param 1:pData,计算数据源地址
 * @param 2:dataLen,计算数据源长度
 * @param 3:targetCRC,对比结果CRC
 * 
 * @return :对比校验结果，=1校验成功，=0校验失败
*/
int crc8_gernCheckT( unsigned char *pData, 
                 unsigned int dataLen,
                 unsigned char targetCRC)
{
    if (crc8(pData, dataLen, 0xff, 0x31) == targetCRC)
    {
        return 1;
    }
    return 0;
}
/**
 * @funtion :针对温湿度传感器sht3X系列的crc8校验
 * @param 1:pData,计算数据源地址
 * @param 2:dataLen,计算数据源长度
 * @param 3:targetCRC,对比结果CRC
 * 
 * @return :对比校验结果，=1校验成功，=0校验失败
*/
int crc8_sht3x( unsigned char *pData, 
                 unsigned int dataLen,
                 unsigned char targetCRC)
{
    if (crc8(pData, dataLen, 0xff, 0x31) == targetCRC)
    {
        return 1;
    }
    return 0;
}
/**
 * @funtion :针对温湿度传感器sht2X系列的crc8校验
 * @param 1:pData,计算数据源地址
 * @param 2:dataLen,计算数据源长度
 * @param 3:targetCRC,对比结果CRC
 * 
 * @return :对比校验结果，=1校验成功，=0校验失败
*/
int crc8_sht2x( unsigned char *pData, 
                 unsigned int dataLen,
                 unsigned char targetCRC)
{
    if (crc8(pData, dataLen, 0x00, 0x31) == targetCRC)
    {
        return 1;
    }
    return 0;
}
