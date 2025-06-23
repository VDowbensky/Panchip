/********************************************************* * 
@file		:dma_common.c 
@brief  	:common interface source file,store some common interface functions 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "dma_common.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile int gTxChNum = 0xff,gTxChNum1 = 0xff;
volatile int gRxChNum = 0xff;

volatile bool gTfrFlag =false,gRxTfrFlag =false,gTfrFlag1 = false;
volatile bool gErrFlag;

uint32_t gDmaSendData[BUFSIZE]    = {0};
uint32_t gDmaSendData1[BUFSIZE]   = {0};
uint32_t gDmaRcvData[BUFSIZE]     = {0};

static uint32_t RxCnt =0;
volatile int I2cStopFlag = 0;

uint32_t TestCnt = TEST_CNT_VALUE;
//extern __IO uint32_t DMAEndOfTransfer;

__DMA_Config DmaConf;
uint8_t __MaxDmaCh = DMAC_CHANNEL_NUMBER;   // 2 channels for PAN107x


/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t SDIO_DmaTranferCase7(void)
{
    SYS_TEST("PAN107x does not have SDIO!\n");
    return 0;
}


void DMA_TestModuleInit(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();
	switch(DmaConf.idx){
		case 0xff:break;
		default:
			/*SPI0 Master*/
			SYS_SET_MFP(P1, 1, SPI0_MOSI);
			SYS_SET_MFP(P1, 2, SPI0_MISO);
			SYS_SET_MFP(P0, 3, SPI0_CS);
			SYS_SET_MFP(P0, 4, SPI0_CLK);
			GPIO_EnableDigitalPath(P0, BIT4);   //SPI CLK is input pin while act as slave
			GPIO_EnableDigitalPath(P0, BIT3);   //SPI CS is input pin while act as slave
			GPIO_EnableDigitalPath(P1, BIT1);   //SPI MOSI is input pin while act as slave
			GPIO_EnableDigitalPath(P1, BIT2);   //SPI MISO is input pin while act as master
			/*SPI1 Slave*/
			SYS_SET_MFP(P2, 1, SPI1_MOSI);
			SYS_SET_MFP(P2, 4, SPI1_MISO);
			SYS_SET_MFP(P2, 2, SPI1_CLK);
			SYS_SET_MFP(P2, 3, SPI1_CS);
			GPIO_EnableDigitalPath(P2, BIT2);   //SPI CLK is input pin while act as slave
			GPIO_EnableDigitalPath(P2, BIT3);   //SPI CS is input pin while act as slave
			GPIO_EnableDigitalPath(P2, BIT1);   //SPI MOSI is input pin while act as slave
			GPIO_EnableDigitalPath(P2, BIT4);   //SPI MISO is input pin while act as master

			GPIO_SetMode(P0, BIT7, GPIO_MODE_QUASI);
			GPIO_SetMode(P1, BIT0, GPIO_MODE_QUASI);
			GPIO_EnableDigitalPath(P0, BIT7);
			GPIO_EnablePullupPath(P0, BIT7);
			GPIO_EnableDigitalPath(P1, BIT0); 
			GPIO_EnablePullupPath(P1, BIT0); 
			/* Set P1.2 and P1.3 for I2C0 SCL and SDA */
			SYS_SET_MFP(P0, 7, I2C0_SCL);
			SYS_SET_MFP(P1, 0, I2C0_SDA);
		break;
	}

    SYS_LockReg();
}


uint32_t (*DMA_TestEntry[])(void) = 
{
    DMA_DefaultValueCheckCase0,
    SPI_DmaMem2PeripheralTranferCase1,
    SPI_DmaMem2PeripheralTranferCase2,
    SPI_DmaMem2PeripheralTranferCase3,
    SPI_DmaPeripheral2MemTranferCase4,
    SPI_DmaPeripheral2MemTranferCase5,
    SPI_DmaPeripheral2MemTranferCase6,
    SDIO_DmaTranferCase7,
    I2C_DmaMem2PeripheralTranferCase8,
    I2C_DmaPeripheral2MemTranferCase9,
    UART_DmaMem2PeripheralTranferCase10,
    UART_DmaPeripheral2MemTranferCase11,
    SPI_DmaMem2PeripheralTranferCase12,
    SPI_DmaPeripheral2MemTranferCase13,
    DMAC_Uart2IicTranferCase14,
    DMAC_Iic2SpiTranferCase15,
    DMAC_Spi2UartTranferCase16,
    DMA_MultiBlockTranferCase17,
    DMA_TwoChannelTranferCase18,
    DMA_ThreeChannelTranferCase19,
    DMA_BusLockedTranferCase20,
    DMA_ChannelLockedTranferCase21,
    DMA_ChannelBusLockedTranferCase22,
    DMA_PriorityTranferCase23,
    DMA_MultiChannelBlockTranferCase24,
    DMA_Mem2MemTranferCase25,
    DMA_OneByteTranferCase26,
    DMA_4092ByteTranferCase27,
    SPI_DmaFullDuplexTranferCase28,
	DMA_HardwareMultiBlockTestCase29,
	FLASH_DmaFlash2PeripheralTranferCase30
};

void DMA_TestFunctionEnter(uint16_t Idx)
{
	DmaConf.idx = Idx;
	DmaConf.func = DMA_TestEntry[Idx];
	DMA_TestModuleInit();
	(DmaConf.func)();
	
//    (DMA_TestEntry[Idx])();
	
}

static uint32_t ch0_block_cnt = 0,ch1_block_cnt = 0,ch2_block_cnt = 0;
void DMA_IRQHandler()
{
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_TFR))
    {
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR)){
			gTfrFlag = true;
			ch0_block_cnt = 0;
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR)){
			gRxTfrFlag = true;
			ch1_block_cnt = 0;
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR)){
			gTfrFlag1 = true;
			ch2_block_cnt = 0;
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR);          
        }           
    }
	if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_BLK))
	{
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_BLK)){
			ch0_block_cnt++;
			SYS_TEST("ch0_block_cnt:%d\n",ch0_block_cnt);
			if(ch0_block_cnt >= DATA_LENTH_TRANSFER_0/16 - 1){
				DMA->CH[0].CFG_L &= ~(0x3ul<<30);
			}
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_BLK);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_BLK)){
			if(ch1_block_cnt >= DATA_LENTH_TRANSFER_0/16 - 1){
				DMA->CH[0].CFG_L &= ~(0x3ul<<30);
			}
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_BLK);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_BLK)){
			if(ch2_block_cnt >= DATA_LENTH_TRANSFER_0/16 - 1){
				DMA->CH[0].CFG_L &= ~(0x3ul<<30);
			}
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_BLK);          
        } 
	}
}

void I2C0_IRQHandler()
{
	static uint8_t txCnt =0;
    volatile uint8_t tmp;
    
	if(I2C_GetITStatus(I2C0, I2C_IT_RX_FULL))
	{
        while(I2C_GetFlagStatus(I2C0, I2C_FLAG_RFNE))
        {
            tmp= I2C_ReceiveData(I2C0);
            RxCnt++;
        }
        I2C_ClearITPendingBit(I2C0, I2C_IT_RX_FULL);
	}
		
	if(I2C_GetITStatus(I2C0,I2C_IT_STOP_DET))
	{
		txCnt = 0;
		while(I2C_GetFlagStatus(I2C0, I2C_FLAG_RFNE))
		{
            tmp= I2C_ReceiveData(I2C0);
		}
		if(RxCnt>0)
			I2cStopFlag = 1;
		I2C_ClearITPendingBit(I2C0, I2C_IT_STOP_DET);
	}
		
	if(I2C_GetITStatus(I2C0, I2C_IT_RD_REQ))
	{
        while(I2C_GetFlagStatus(I2C0, I2C_FLAG_TFNF)==0);//waite tx fifo no full
        I2C_SendDataCmd(I2C0, 0xaa, I2C_CMD_WR);
        txCnt++;
		I2C_ClearITPendingBit(I2C0, I2C_IT_RD_REQ);
	}	
	
}

void delay(uint32_t t)
{
    while(t){t--;};
}


void FillBuffer(uint32_t *pBuffer, uint32_t size)
{
    uint32_t idx = 0;
    
    for(idx = 0; idx < size; idx++)
    {
        pBuffer[idx] = 0x55aa0000|idx;//(uint32_t)((0xaa00|idx) % 0x10000);
    }
     
}

/*
  * @brief	   :spi dma mode enable
  * @param in  :where SPIx is the base address of SPI peripheral,it could be 0,1
  * @param in  :where RxTXSel is spi tx rx select
  * @param in  :where Enable is used to judge whether enabled
  * @retval    :none
*/


void SPI_SetDmaRxTxEnabled(SPI_T* SPIx, uint8_t RxTXSel,uint8_t Enable)
{
    uint32_t tmpreg = 0x00;
    const uint32_t SpiDmaSel[3]={SPI_DMACR_RDMAE_Msk,SPI_DMACR_TDMAE_Msk,SPI_DMACR_RDMAE_Msk | SPI_DMACR_TDMAE_Msk};

    tmpreg = SPIx->DMACR;
    
    if(Enable) tmpreg |= SpiDmaSel[RxTXSel];
//    tmpreg |= SPI_DMACR_RDMAE_Msk | SPI_DMACR_TDMAE_Msk;
    else  tmpreg &= ~SpiDmaSel[RxTXSel];
//        tmpreg &= ~(SPI_DMACR_RDMAE_Msk | SPI_DMACR_TDMAE_Msk);

    SPIx->DMACR = tmpreg;
}

/*
  * @brief	    :i2c config init  & interrupt enable
  * param I2Cx  :base address of the I2C peripheral.
  * param role  :master or slave mode select.
  * param u8Adr10BitEn  :10bit address enable or not
  * @retval     :none
*/
void I2C_InitRole(I2C_T * I2Cx, uint16_t role,uint8_t Adr10BitEn)
 {
	 I2C_InitTypeDef  I2C_InitStructure;
 
	 /* I2C configuration */
	 I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	 //The IC_SAR holds the slave address when the I2C is operating as a slave
	 if(Adr10BitEn){
		I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS10;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
	 }
	 else{
		I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS7;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	 }
	 I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED_100K;
 
	 I2C_Cmd(I2Cx, DISABLE);
	 I2C_Init(I2Cx, &I2C_InitStructure);
 
	 if(role == I2C_MODE_MASTER)
	 {
		 //1. set master mode
		 I2C_SetMode(I2Cx, I2C_MODE_MASTER);
		 //2. set remote slave addr
		 if(Adr10BitEn)
			I2C_Set10bitAddress(I2Cx, I2C_SLAVE_OWN_ADDRESS10);
		 else
			I2C_Set7bitAddress(I2Cx, I2C_SLAVE_OWN_ADDRESS7);
		 I2Cx->SDA_HOLD = ((0x3<<16) | 0x3);
	 }
	 else
	 {
		 I2C_SetMode(I2Cx, I2C_MODE_SLAVE);
	 }
	 I2C_DisableAllIT(I2Cx);	 
	 I2C_ITConfig(I2Cx, I2C_IT_RX_FULL|I2C_IT_RD_REQ|I2C_IT_STOP_DET, ENABLE);

	  /* I2C Peripheral Enable */
	 I2C_Cmd(I2Cx, ENABLE);
 }

/**
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t DMA_DefaultValueCheckCase0(void)
{
    SYS_TEST("DMA->CH[0].SAR_L     :%08X\r\n",DMA->CH[0].SAR_L     );
    SYS_TEST("DMA->CH[0].SAR_H     :%08X\r\n",DMA->CH[0].SAR_H     );    
    SYS_TEST("DMA->CH[0].DAR_L     :%08X\r\n",DMA->CH[0].DAR_L     );    
    SYS_TEST("DMA->CH[0].DAR_H     :%08X\r\n",DMA->CH[0].DAR_H     );    
    SYS_TEST("DMA->CH[0].CTL_L     :%08X\r\n",DMA->CH[0].CTL_L     );    
    SYS_TEST("DMA->CH[0].CTL_H     :%08X\r\n",DMA->CH[0].CTL_H     );    
    SYS_TEST("DMA->CH[0].CFG_L     :%08X\r\n",DMA->CH[0].CFG_L     );    
    SYS_TEST("DMA->CH[0].CFG_H     :%08X\r\n",DMA->CH[0].CFG_H     );    
//    SYS_TEST("DMA->CH[0].SGR_L     :%08X\r\n",DMA->CH[0].SGR_L     );    
//    SYS_TEST("DMA->CH[0].SGR_H     :%08X\r\n",DMA->CH[0].SGR_H     );    
//    SYS_TEST("DMA->CH[0].DSR_L     :%08X\r\n",DMA->CH[0].DSR_L     );    
//    SYS_TEST("DMA->CH[0].DSR_H     :%08X\r\n",DMA->CH[0].DSR_H     );    
                                                                   
    SYS_TEST("DMA->CH[1].SAR_L     :%08X\r\n",DMA->CH[1].SAR_L     );    
    SYS_TEST("DMA->CH[1].SAR_H     :%08X\r\n",DMA->CH[1].SAR_H     );    
    SYS_TEST("DMA->CH[1].DAR_L     :%08X\r\n",DMA->CH[1].DAR_L     );    
    SYS_TEST("DMA->CH[1].DAR_H     :%08X\r\n",DMA->CH[1].DAR_H     );    
    SYS_TEST("DMA->CH[1].CTL_L     :%08X\r\n",DMA->CH[1].CTL_L     );    
    SYS_TEST("DMA->CH[1].CTL_H     :%08X\r\n",DMA->CH[1].CTL_H     );    
    SYS_TEST("DMA->CH[1].CFG_L     :%08X\r\n",DMA->CH[1].CFG_L     );    
    SYS_TEST("DMA->CH[1].CFG_H     :%08X\r\n",DMA->CH[1].CFG_H     );    
//    SYS_TEST("DMA->CH[1].SGR_L     :%08X\r\n",DMA->CH[1].SGR_L     );    
//    SYS_TEST("DMA->CH[1].SGR_H     :%08X\r\n",DMA->CH[1].SGR_H     );    
//    SYS_TEST("DMA->CH[1].DSR_L     :%08X\r\n",DMA->CH[1].DSR_L     );    
//    SYS_TEST("DMA->CH[1].DSR_H     :%08X\r\n",DMA->CH[1].DSR_H     );    

//    SYS_TEST("DMA->CH[2].SAR_L     :%08X\r\n",DMA->CH[2].SAR_L     );    
//    SYS_TEST("DMA->CH[2].SAR_H     :%08X\r\n",DMA->CH[2].SAR_H     );    
//    SYS_TEST("DMA->CH[2].DAR_L     :%08X\r\n",DMA->CH[2].DAR_L     );    
//    SYS_TEST("DMA->CH[2].DAR_H     :%08X\r\n",DMA->CH[2].DAR_H     );    
//    SYS_TEST("DMA->CH[2].CTL_L     :%08X\r\n",DMA->CH[2].CTL_L     );    
//    SYS_TEST("DMA->CH[2].CTL_H     :%08X\r\n",DMA->CH[2].CTL_H     );    
//    SYS_TEST("DMA->CH[2].CFG_L     :%08X\r\n",DMA->CH[2].CFG_L     );    
//    SYS_TEST("DMA->CH[2].CFG_H     :%08X\r\n",DMA->CH[2].CFG_H     );    
//    SYS_TEST("DMA->CH[2].SGR_L     :%08X\r\n",DMA->CH[2].SGR_L     );    
//    SYS_TEST("DMA->CH[2].SGR_H     :%08X\r\n",DMA->CH[2].SGR_H     );    
//    SYS_TEST("DMA->CH[2].DSR_L     :%08X\r\n",DMA->CH[2].DSR_L     );    
//    SYS_TEST("DMA->CH[2].DSR_H     :%08X\r\n",DMA->CH[2].DSR_H     );  
   
    SYS_TEST("DMA->RAW_TFR_L       :%08X\r\n",DMA->RAW_TFR_L       );    
    SYS_TEST("DMA->RAW_TFR_H       :%08X\r\n",DMA->RAW_TFR_H       );    
    SYS_TEST("DMA->RAW_BLOCK_L     :%08X\r\n",DMA->RAW_BLOCK_L     );    
    SYS_TEST("DMA->RAW_BLOCK_H     :%08X\r\n",DMA->RAW_BLOCK_H     );    
    SYS_TEST("DMA->RAW_SRCTRAN_L   :%08X\r\n",DMA->RAW_SRCTRAN_L   );    
    SYS_TEST("DMA->RAW_SRCTRAN_H   :%08X\r\n",DMA->RAW_SRCTRAN_H   );    
    SYS_TEST("DMA->RAW_DSTTRAN_L   :%08X\r\n",DMA->RAW_DSTTRAN_L   );    
    SYS_TEST("DMA->RAW_DSTTRAN_H   :%08X\r\n",DMA->RAW_DSTTRAN_H   );    
    SYS_TEST("DMA->RAW_ERR_L       :%08X\r\n",DMA->RAW_ERR_L       );    
    SYS_TEST("DMA->RAW_ERR_H       :%08X\r\n",DMA->RAW_ERR_H       );    
    SYS_TEST("DMA->STATUS_TFR_L    :%08X\r\n",DMA->STATUS_TFR_L    );    
    SYS_TEST("DMA->STATUS_TFR_H    :%08X\r\n",DMA->STATUS_TFR_H    );    
    SYS_TEST("DMA->STATUS_BLOCK_L  :%08X\r\n",DMA->STATUS_BLOCK_L  );    
    SYS_TEST("DMA->STATUS_BLOCK_H  :%08X\r\n",DMA->STATUS_BLOCK_H  );    
    SYS_TEST("DMA->STATUS_SRCTRAN_L:%08X\r\n",DMA->STATUS_SRCTRAN_L);    
    SYS_TEST("DMA->STATUS_SRCTRAN_H:%08X\r\n",DMA->STATUS_SRCTRAN_H);    
    SYS_TEST("DMA->STATUS_DSTTRAN_L:%08X\r\n",DMA->STATUS_DSTTRAN_L);    
    SYS_TEST("DMA->STATUS_DSTTRAN_H:%08X\r\n",DMA->STATUS_DSTTRAN_H);    
    SYS_TEST("DMA->STATUS_ERR_L    :%08X\r\n",DMA->STATUS_ERR_L    );    
    SYS_TEST("DMA->STATUS_ERR_H    :%08X\r\n",DMA->STATUS_ERR_H    );    
    SYS_TEST("DMA->MSK_TFR_L       :%08X\r\n",DMA->MSK_TFR_L       );    
    SYS_TEST("DMA->MSK_TFR_H       :%08X\r\n",DMA->MSK_TFR_H       );    
    SYS_TEST("DMA->MSK_BLOCK_L     :%08X\r\n",DMA->MSK_BLOCK_L     );    
    SYS_TEST("DMA->MSK_BLOCK_H     :%08X\r\n",DMA->MSK_BLOCK_H     );    
    SYS_TEST("DMA->MSK_SRCTRAN_L   :%08X\r\n",DMA->MSK_SRCTRAN_L   );    
    SYS_TEST("DMA->MSK_SRCTRAN_H   :%08X\r\n",DMA->MSK_SRCTRAN_H   );    
    SYS_TEST("DMA->MSK_DSTTRAN_L   :%08X\r\n",DMA->MSK_DSTTRAN_L   );    
    SYS_TEST("DMA->MSK_DSTTRAN_H   :%08X\r\n",DMA->MSK_DSTTRAN_H   );    
    SYS_TEST("DMA->MSK_ERR_L       :%08X\r\n",DMA->MSK_ERR_L       );    
    SYS_TEST("DMA->MSK_ERR_H       :%08X\r\n",DMA->MSK_ERR_H       ); 

//    SYS_TEST("DMA->MSK_ERR_H       :%p\r\n",&DMA->CLEAR_TFR_L       ); 
//    SYS_TEST("DMA->CLEAR_TFR_L     :%08X\r\n",DMA->CLEAR_TFR_L     );    
//    SYS_TEST("DMA->CLEAR_TFR_H     :%08X\r\n",DMA->CLEAR_TFR_H     );    
//    SYS_TEST("DMA->CLEAR_BLOCK_L   :%08X\r\n",DMA->CLEAR_BLOCK_L   );    
//    SYS_TEST("DMA->CLEAR_BLOCK_H   :%08X\r\n",DMA->CLEAR_BLOCK_H   );    
//    SYS_TEST("DMA->CLEAR_SRCTRAN_L :%08X\r\n",DMA->CLEAR_SRCTRAN_L );    
//    SYS_TEST("DMA->CLEAR_SRCTRAN_H :%08X\r\n",DMA->CLEAR_SRCTRAN_H );    
//    SYS_TEST("DMA->CLEAR_DSTTRAN_L :%08X\r\n",DMA->CLEAR_DSTTRAN_L );    
//    SYS_TEST("DMA->CLEAR_DSTTRAN_H :%08X\r\n",DMA->CLEAR_DSTTRAN_H );    
//    SYS_TEST("DMA->CLEAR_ERR_L     :%08X\r\n",DMA->CLEAR_ERR_L     );    
//    SYS_TEST("DMA->CLEAR_ERR_H     :%08X\r\n",DMA->CLEAR_ERR_H     );    
    SYS_TEST("DMA->STATUS_INT_L    :%08X\r\n",DMA->STATUS_INT_L    );    
    SYS_TEST("DMA->STATUS_INT_H    :%08X\r\n",DMA->STATUS_INT_H    );    
    SYS_TEST("DMA->DMA_CFG_REG_L   :%08X\r\n",DMA->DMA_CFG_REG_L   );    
    SYS_TEST("DMA->DMA_CFG_REG_H   :%08X\r\n",DMA->DMA_CFG_REG_H   );    
    SYS_TEST("DMA->CH_EN_REG_L     :%08X\r\n",DMA->CH_EN_REG_L     );    
    SYS_TEST("DMA->CH_EN_REG_H     :%08X\r\n",DMA->CH_EN_REG_H     );    
                        
    return 0;
} 
