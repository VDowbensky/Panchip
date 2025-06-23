/********************************************************* * 
@file		:dma_spi2uart.c 
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
    spiInitStruct.SPI_baudRateDiv = SPI_BaudRateDiv_240;
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;
    spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
    spiInitStruct.SPI_role = SPI_RoleMaster;

    SPI_Init(SPI0, &spiInitStruct);

    //Further slow down SPI clock to make it slower than UART
//    SPI0->CR0 = (SPI0->CR0 & ~SPI_CR0_SCR_Msk) | (0x2 << SPI_CR0_SCR_Pos);

    //SPI Interrput
    SPI_DisableIrq(SPI0, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI0);
    
    /* Init SPI1 Slave*/  
    spiInitStruct.SPI_role = SPI_RoleSlave;
    SPI_Init(SPI1, &spiInitStruct);

    //Further slow down SPI clock to make it slower than UART
    SPI1->CR0 = (SPI1->CR0 & ~SPI_CR0_SCR_Msk) | (0x9 << SPI_CR0_SCR_Pos);

    SPI_DisableIrq(SPI1, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI1);    
}


/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t DMAC_Spi2UartTranferCase16(void)
{
//    uint32_t s_tx_i = 0;
//    uint32_t s_rx_i = 0;
    uint32_t m_tx_i = 0;
//    uint32_t m_rx_i = 0;
    volatile uint32_t tmpv;
//    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef DmaConfigTmp = {0,};
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    SPI_SysInit();
    /*dma init*/
    DMAC_Init(DMA);
	  NVIC_EnableIRQ(DMA_IRQn);
    
    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);

    /*set spi rx dma channel control&config register*/
    DmaConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    DmaConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    DmaConfigTmp.BurstLenDst     = DMAC_BurstLen_8;
    DmaConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    DmaConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    DmaConfigTmp.DataWidthSrc    = DMAC_DataWidth_16;
    DmaConfigTmp.TransferType    = DMAC_TransferType_Per2Per;
    DmaConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    DmaConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    DmaConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    DmaConfigTmp.PeripheralSrc   = DMAC_Peripheral_SPI1_Rx;
    DmaConfigTmp.PeripheralDst   = DMA_UART_PERIPHERAL_DST;
    DmaConfigTmp.IntEnable       = ENABLE;
  
	
    
//    while(TestCnt--)
    {
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&DmaConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(SPI1->DR),(void*)&(DMA_UART_SELECT->RBR_THR_DLL),DATA_LENTH_TRANSFER_0);
        
        /*enable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI1, SPI_RX_DMAC_SELECT,1); 
        while(m_tx_i < DATA_LENTH_TRANSFER_0)
        {
            while(m_tx_i < DATA_LENTH_TRANSFER_0 && !SPI_IsTxFifoFull(SPI0)){
                SPI_SendData(SPI0, (uint32_t)gDmaSendData[m_tx_i++]);
            }
        }
        while(!gTfrFlag){}       
        /*disable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI1, SPI_RX_DMAC_SELECT,0);
            
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gRxTfrFlag = false;
//        s_tx_i = 0;
//        s_rx_i = 0;
        m_tx_i = 0;
//        m_rx_i = 0;
        SYS_delay_10nop(0x10000);
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

