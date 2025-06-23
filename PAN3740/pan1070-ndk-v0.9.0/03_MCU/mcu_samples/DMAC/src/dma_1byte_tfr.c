/********************************************************* * 
@file		:dma_1byte_tfr.c 
@brief  	:spi dma test case, transfer 1 byte form memory to peripheral spi with normal dma channel config
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
    spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_8b;
    spiInitStruct.SPI_role = SPI_RoleMaster;
    
//    SPI0->CPSR  = 2;
    SPI_Init(SPI0, &spiInitStruct);
    //SPI Interrput
    SPI_DisableIrq(SPI0, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI0);

    NVIC_EnableIRQ(DMA_IRQn);
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @param in  :where TestCnt is count of transfer 
  * @retval    :none
*/
uint32_t DMA_OneByteTranferCase26(void)
{
    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    uint8_t SendData = 0xaa;
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    SPI_SysInit();
    /*dma init*/
    DMAC_Init(DMA);

//    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);

    /*set spi tx dma channel control&config register*/
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_1;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_1;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = DMAC_Peripheral_SPI0_Tx;
    TxConfigTmp.IntEnable       = ENABLE;
  
    
    while(TestCntValue--){
        /*get free dma channel;*/
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gTxChNum,&SendData,(void*)&(SPI0->DR),1);
        
        /*enable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,1); 
        while(!gTfrFlag){}
        /*disable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,0);
            
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gTxChNum);
        gTfrFlag = false;
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gTxChNum);
    return 0;
}

