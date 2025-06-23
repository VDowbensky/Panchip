/********************************************************* * 
@file		:iic_common.h
@brief  	:common interface header file 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#ifndef __IIC_COMMON_H__
#define __IIC_COMMON_H__

#define BUFSIZE                         (256)
#define __BYTE_LEN	                    (32)

#define SPI_RX_DMAC_SELECT              (0X0)
#define SPI_TX_DMAC_SELECT              (0X1)
#define SPI_TXRX_DMAC_SELECT            (0X2)
        
#define DATA_LENTH_TRANSFER_0           (64*4)
#define DATA_LENTH_TRANSFER_1           (63*4)
#define DATA_LENTH_TRANSFER_2           (62*4)
#define DATA_LENTH_TRANSFER_3           (61*4)
        
#define TEST_CNT_VALUE                  (10)

#define I2C_SPEED_100K                  (100000)
#define I2C_SPEED_200K                  (200000)
#define I2C_SPEED_400K                  (400000)
#define I2C_SPEED_500K                  (500000)
#define I2C_SPEED_1M                    (1000000)
#define I2C_SPEED_2M                    (2000000)
#define I2C_SLAVE_OWN_ADDRESS7          (0x50)
#define I2C_SLAVE_OWN_ADDRESS10         (0x350)
#define I2C_MASTER_TAR_ADDRESS7         (0xA0)//0xA8

#define I2C_WR	                        (0)
#define I2C_RD	                        (1)

#define ADR10BIT_ENABLE	                (1)
#define ADR10BIT_DISABLE	            (0)

#define DMA_ENABLE	                    (1)
#define DMA_DISABLE	                    (0)

extern volatile int gTxChNum,gTxChNum1;
extern volatile int gRxChNum;
extern volatile bool gTfrFlag,gRxTfrFlag,gTfrFlag1;
extern volatile bool gErrFlag;
extern volatile int I2cStopFlag;
extern uint32_t TestCnt;
extern uint32_t RxCnt;
extern uint8_t RxBuf[__BYTE_LEN+1];

void FillBuffer(uint8_t *pBuffer, uint32_t size);
void delay(uint32_t t);
void I2C_InitRole(I2C_T * I2Cx, uint16_t role,uint8_t Adr10BitEn);


uint32_t I2C_RegisterDefaultCheckCase1(void);
uint32_t I2C_MasterSendDataCase1(void);
uint32_t I2C_SlaveReceiveDataCase2(void);
uint32_t I2C_SlaveSendDataCase3(void);
uint32_t I2C_MasterReceiveDataCase4(void);
uint32_t I2C_MasterSendDataCase5(void);
uint32_t I2C_10BitMasterSendDataCase6(void);
uint32_t I2C_10BitSlaveReceiveDataCase7(void);
uint32_t I2C_DmaMem2PeripheralTranferCase8(void);
uint32_t I2C_DmaPeripheral2MemTranferCase9(void);
uint32_t I2C_MasterGeneralCallCase10(void);
uint32_t I2C_MasterInterruptCase11(void);
uint32_t I2C_SlaveInterruptCase12(void);
uint32_t I2C_DmaMasterReadCase13(void);
uint32_t I2C_DmaSlaveWriteCase14(void);
void I2C_TestFunctionEnter(uint16_t Idx);

#endif /* __IIC_COMMON_H__ */

