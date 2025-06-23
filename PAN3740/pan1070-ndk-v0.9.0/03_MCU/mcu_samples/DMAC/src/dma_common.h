/********************************************************* * 
@file		:dma_common.c 
@brief  	:common interface header file 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#ifndef __DMA_COMMON_H__
#define __DMA_COMMON_H__

typedef struct{
	uint8_t idx;
	uint32_t (*func)(void);
}__DMA_Config;


#define BUFSIZE  (256*4)

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
#define I2C_SLAVE_OWN_ADDRESS7          (0x50)
#define I2C_SLAVE_OWN_ADDRESS10         (0x350)
#define I2C_MASTER_TAR_ADDRESS7         (0xA0)//0xA8

#define I2C_WR	                        (0)
#define I2C_RD	                        (1)

#define ADR10BIT_ENABLE	                (1)
#define ADR10BIT_DISABLE	            (0)

#define DMA_ENABLE	                    (1)
#define DMA_DISABLE	                    (0)

#define DMA_UART_SELECT					(UART0)
#define DMA_UART_PERIPHERAL_DST			(DMAC_Peripheral_UART0_Tx)
#define DMA_UART_PERIPHERAL_SRC			(DMAC_Peripheral_UART0_Rx)

extern volatile int gTxChNum,gTxChNum1;
extern volatile int gRxChNum;
extern volatile bool gTfrFlag,gRxTfrFlag,gTfrFlag1;
extern volatile bool gErrFlag;
extern volatile int I2cStopFlag;
extern uint32_t gDmaSendData[BUFSIZE];
extern uint32_t gDmaSendData1[BUFSIZE];
extern uint32_t gDmaRcvData[BUFSIZE];
extern uint32_t TestCnt;
extern uint8_t __MaxDmaCh;

void FillBuffer(uint32_t *pBuffer, uint32_t size);
void delay(uint32_t t);
void SPI_SetDmaRxTxEnabled(SPI_T* SPIx, uint8_t RxTXSel,uint8_t Enable);
void I2C_InitRole(I2C_T * I2Cx, uint16_t role,uint8_t Adr10BitEn);

uint32_t DMA_DefaultValueCheckCase0(void);
uint32_t SPI_DmaMem2PeripheralTranferCase1(void);
uint32_t SPI_DmaMem2PeripheralTranferCase2(void);
uint32_t SPI_DmaMem2PeripheralTranferCase3(void);
uint32_t SPI_DmaPeripheral2MemTranferCase4(void);
uint32_t SPI_DmaPeripheral2MemTranferCase5(void);
uint32_t SPI_DmaPeripheral2MemTranferCase6(void);
uint32_t SDIO_DmaTranferCase7(void);
uint32_t I2C_DmaMem2PeripheralTranferCase8(void);
uint32_t I2C_DmaPeripheral2MemTranferCase9(void);
uint32_t UART_DmaMem2PeripheralTranferCase10(void);
uint32_t UART_DmaPeripheral2MemTranferCase11(void);
uint32_t SPI_DmaMem2PeripheralTranferCase12(void);
uint32_t SPI_DmaPeripheral2MemTranferCase13(void);
uint32_t DMAC_Uart2IicTranferCase14(void);
uint32_t DMAC_Iic2SpiTranferCase15(void);
uint32_t DMAC_Spi2UartTranferCase16(void);
uint32_t DMA_MultiBlockTranferCase17(void);
uint32_t DMA_TwoChannelTranferCase18(void);
uint32_t DMA_ThreeChannelTranferCase19(void);
uint32_t DMA_BusLockedTranferCase20(void);
uint32_t DMA_ChannelLockedTranferCase21(void);
uint32_t DMA_ChannelBusLockedTranferCase22(void);
uint32_t DMA_PriorityTranferCase23(void);
uint32_t DMA_MultiChannelBlockTranferCase24(void);
uint32_t DMA_Mem2MemTranferCase25(void);
uint32_t DMA_OneByteTranferCase26(void);
uint32_t DMA_4092ByteTranferCase27(void);
uint32_t SPI_DmaFullDuplexTranferCase28(void);
uint32_t DMA_HardwareMultiBlockTestCase29(void);
uint32_t FLASH_DmaFlash2PeripheralTranferCase30(void);

void DMA_TestFunctionEnter(uint16_t Idx);

#endif //__DMA_COMMON_H__
