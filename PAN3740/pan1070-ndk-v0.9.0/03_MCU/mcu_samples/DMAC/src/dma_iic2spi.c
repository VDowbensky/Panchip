/********************************************************* * 
@file		:dma_i2c2spi.c 
@brief  	:i2c dma test case,dma transmit form memory to peripheral i2c
@author 	:zhongfeng
@version	:v1.0 
@date 		:10/09/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "dma_common.h"


/*
  * @brief	   :spi config init  & interrupt enable
  * @retval    :none
*/
static void SPI_SysInit(void)
{
	SPI_InitTypeDef spiInitStruct;

	/* Init SPI0 Master*/
    spiInitStruct.SPI_format = SPI_FormatMotorola;
    spiInitStruct.SPI_baudRateDiv = 20;//SPI_BaudRateDiv_96;
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;
    spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_8b;
    spiInitStruct.SPI_role = SPI_RoleMaster;
    
//    SPI0->CPSR  = 6;
    SPI_Init(SPI0, &spiInitStruct);
    //SPI Interrput
    SPI_DisableIrq(SPI0, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI0);
}

/*
  * @brief	    :i2c config init  & DMA enable
  * param I2Cx  :base address of the I2C peripheral.
  * param role  :master or slave mode select.
  * param u8Adr10BitEn  :10bit address enable or not
  * @retval     :none
*/
static void I2C_SysInit(I2C_T * I2Cx,uint8_t DmaEn,uint8_t u8Adr10BitEn)
{
		
    I2C_InitRole(I2Cx,I2C_MODE_SLAVE,u8Adr10BitEn);
    I2C_SetRxTirggerLevel(I2Cx, I2C_RX_TL_3);
    
    if(DmaEn == DMA_ENABLE){
        I2C_DMAReceiveDataLevel(I2Cx, I2C_RX_TL_3);
    }
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t DMAC_Iic2SpiTranferCase15(void)
{
//    uint32_t TestCntValue = TestCnt;
//    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    DMAC_ChannelConfigTypeDef DmaConfigTmp = {0,};
    
	// Initialize the DMA controller
	DMAC_Init(DMA);
	NVIC_EnableIRQ(DMA_IRQn);
    I2C_SysInit(I2C0,DMA_ENABLE,ADR10BIT_DISABLE);
    I2C_DMACmd(I2C0, ENABLE);
    SPI_SysInit();
    /*enable spi tx_rx dma mode */
    SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,1); 
    
    /*set tx dma channel control&config register*/
    DmaConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    DmaConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    DmaConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    DmaConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    DmaConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    DmaConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    DmaConfigTmp.TransferType    = DMAC_TransferType_Per2Per;
    DmaConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    DmaConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    DmaConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    DmaConfigTmp.PeripheralDst   = DMAC_Peripheral_SPI0_Tx;
    DmaConfigTmp.PeripheralSrc   = DMAC_Peripheral_I2C0_Rx;
    DmaConfigTmp.IntEnable       = ENABLE;


//    while(TestCnt--)
    {        
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&DmaConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(I2C0->DATACMD),(void*)&(SPI0->DR),DATA_LENTH_TRANSFER_0);
        
        while(!gTfrFlag){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gTfrFlag = false;
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

