/********************************************************* * 
@file		:spi_dma_p2m.c 
@brief  	:spi dma test case,dma receive form peripheral spi to memory with normal dma channel config
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
uint32_t SPI_DmaPeripheral2MemTranferCase13(void)
{
#if 1
    SYS_TEST("Currently DMAC doesn't support this feature!\n");
#else
    uint32_t s_tx_i = 0;
    uint32_t s_rx_i = 0;
    uint32_t m_tx_i = 0;
    uint32_t m_rx_i = 0;
    volatile uint32_t tmpv;
    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    SPI_SysInit();
    /*dma init*/
    DMAC_Init(DMA);

    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);

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
    RxConfigTmp.PeripheralDst   = DMAC_Peripheral_SPI0_Rx;
    RxConfigTmp.ScatterEnDst    = ENABLE;
    RxConfigTmp.IntEnable       = ENABLE;
  
	
    
    while(TestCntValue--)
    {
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
        /*set gather count & interval*/
        DMA->CH[gRxChNum].SGR_L = ((0x4<<20)|(0x4<<0));
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(SPI0->DR),gDmaRcvData,DATA_LENTH_TRANSFER_0);
        
        /*enable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_RX_DMAC_SELECT,1); 
        while(s_tx_i < DATA_LENTH_TRANSFER_0 || s_rx_i < DATA_LENTH_TRANSFER_0 
           || m_tx_i < DATA_LENTH_TRANSFER_0 || m_rx_i < DATA_LENTH_TRANSFER_0){
            while(s_tx_i < DATA_LENTH_TRANSFER_0 && !SPI_IsTxFifoFull(SPI1)){
                SPI_SendData(SPI1, (uint32_t)gDmaSendData[s_tx_i++]);
            }
            while(s_rx_i < DATA_LENTH_TRANSFER_0 && !SPI_IsRxFifoEmpty(SPI1)){
                tmpv = SPI1->DR;
                s_rx_i++;
            }
            while(m_tx_i < DATA_LENTH_TRANSFER_0 && !SPI_IsTxFifoFull(SPI0)){
                SPI_SendData(SPI0, (uint32_t)gDmaSendData[m_tx_i++]);
            }
            while(m_rx_i < DATA_LENTH_TRANSFER_0 && !SPI_IsRxFifoEmpty(SPI0)){
                m_rx_i++;
            }
        }            
        /*disable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_RX_DMAC_SELECT,0);
            
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gRxTfrFlag = false;
        s_tx_i = 0;
        s_rx_i = 0;
        m_tx_i = 0;
        m_rx_i = 0;
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
#endif
    return 0;
}

