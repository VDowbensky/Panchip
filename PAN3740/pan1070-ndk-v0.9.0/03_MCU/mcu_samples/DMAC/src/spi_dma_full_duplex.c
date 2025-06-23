/********************************************************* * 
@file		:soi_dma_full_duplex.c 
@brief  	:spi dma test case, transfer form memory to peripheral spi and receive from peripheral spi to memory
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
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
    spiInitStruct.SPI_baudRateDiv = 19;//SPI_BaudRateDiv_96;
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;
    spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
    spiInitStruct.SPI_role = SPI_RoleMaster;
    
//    SPI0->CPSR  = 2;
    SPI_Init(SPI0, &spiInitStruct);
    //SPI Interrput
    SPI_DisableIrq(SPI0, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI0);

    /* Init SPI1 Slave*/  
    spiInitStruct.SPI_role = SPI_RoleSlave;
//    SPI1->CPSR  = 2;
    SPI_Init(SPI1, &spiInitStruct);
    SPI_DisableIrq(SPI1, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI1);

    NVIC_EnableIRQ(DMA_IRQn);
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t SPI_DmaFullDuplexTranferCase28(void)
{
    #define TfrDataLen  (DATA_LENTH_TRANSFER_0/4)
    #define RcvDataLen  (DATA_LENTH_TRANSFER_0/8)
    
    uint32_t s_tx_i = 0;
    uint32_t s_rx_i = 0;
    volatile uint32_t tmpv;
    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};
    static uint8_t TxStartFlag = 0;
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    SPI_SysInit();
    /*dma init*/
    DMAC_Init(DMA);

    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);
    FillBuffer(gDmaSendData1,DATA_LENTH_TRANSFER_0);

    /*set spi tx dma channel control&config register*/
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_16;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = DMAC_Peripheral_SPI0_Tx;
    TxConfigTmp.IntEnable       = ENABLE;    
    
    /*set spi rx dma channel control&config register*/
    RxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Increment;
    RxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    RxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    RxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    RxConfigTmp.DataWidthDst    = DMAC_DataWidth_32;
    RxConfigTmp.DataWidthSrc    = DMAC_DataWidth_16;
    RxConfigTmp.TransferType    = DMAC_TransferType_Per2Mem;
    RxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    RxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    RxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    RxConfigTmp.PeripheralSrc   = DMAC_Peripheral_SPI0_Rx;
    RxConfigTmp.IntEnable       = ENABLE;
  
	
    
    while(TestCntValue--)
    {
        /*get free dma channel;*/
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gTxChNum,gDmaSendData,(void*)&(SPI0->DR),TfrDataLen);
        
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(SPI0->DR),gDmaRcvData,RcvDataLen);
               
        /*enable spi tx_rx dma mode */
//        SPI_SetDmaRxTxEnabled(SPI0, SPI_TXRX_DMAC_SELECT,1); 
        while(s_tx_i < RcvDataLen || s_rx_i < RcvDataLen
            || (gTfrFlag == false) || (gRxTfrFlag == false))
        {
            while(s_tx_i < RcvDataLen && !SPI_IsTxFifoFull(SPI1)){
                SPI_SendData(SPI1, (uint32_t)gDmaSendData1[s_tx_i+64]);
                s_tx_i++;
            }
            while(s_rx_i < RcvDataLen && !SPI_IsRxFifoEmpty(SPI1)){
                tmpv = SPI1->DR;
                s_rx_i++;
            }
            if(!TxStartFlag){
                SPI_SetDmaRxTxEnabled(SPI0, SPI_TXRX_DMAC_SELECT,1); 
                TxStartFlag = 1;
            }
        }
//        while(!(gTfrFlag && gRxTfrFlag)){}
        /*disable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_TXRX_DMAC_SELECT,0);
            
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        DMAC_ReleaseChannel(DMA,gTxChNum);    
        gTfrFlag = false;
        gRxTfrFlag = false;    
        s_tx_i = 0;
        s_rx_i = 0;
        TxStartFlag = 0;
        SYS_delay_10nop(0x1000);
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    DMAC_ReleaseChannel(DMA,gTxChNum);  
    return 0;
}

