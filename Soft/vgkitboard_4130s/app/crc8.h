#ifndef crc8_H_
#define crc8_H_

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
                    unsigned char polynomial );
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
                 unsigned char targetCRC);
int crc8_sht2x( unsigned char *pData, 
                 unsigned int dataLen,
                 unsigned char targetCRC);

unsigned char crc8_ger( unsigned char *pData, 
                 unsigned int dataLen);
int crc8_gernCheckT( unsigned char *pData, 
                 unsigned int dataLen,
                 unsigned char targetCRC);
#endif /* SCEADC_H_ */
