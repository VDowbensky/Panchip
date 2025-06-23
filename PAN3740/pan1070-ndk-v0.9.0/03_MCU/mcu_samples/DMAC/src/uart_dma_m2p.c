/********************************************************* * 
@file		:uart_dma_m2p.c 
@brief  	:i2c dma test case,dma transmit form memory to peripheral i2c
@author 	:zhongfeng
@version	:v1.0 
@date 		:10/09/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "dma_common.h"


/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t UART_DmaMem2PeripheralTranferCase10(void)
{
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};

//	//UART_SetDmaMode(UART0);
	// Initialize the DMA controller
	DMAC_Init(DMA);
	NVIC_EnableIRQ(DMA_IRQn);

    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);

    /*set tx dma channel control&config register*/
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_8;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = DMA_UART_PERIPHERAL_DST;
    TxConfigTmp.IntEnable       = ENABLE;
      
//    while(TestCnt--)
    {
        /*get free dma channel;*/
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gTxChNum,gDmaSendData,(void*)&(DMA_UART_SELECT->RBR_THR_DLL),DATA_LENTH_TRANSFER_0/4);
        while(!gTfrFlag){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gTxChNum);
        gTfrFlag = false;
        SYS_delay_10nop(0x10000);
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

