/********************************************************* * 
@file		:dma_mem2mem.c 
@brief  	:dma channel transfer form memory to memory with normal dma channel config,dma priority test
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "dma_common.h"

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @param in  :where TestCnt is count of transfer 
  * @retval    :none
*/
uint32_t DMA_Mem2MemTranferCase25(void)
{
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    
    /*dma init*/
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);

    /*set spi tx dma channel control&config register*/
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Decrement;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_32;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Mem;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.ChannelPriority = DMAC_ChannelPriority_7;
    TxConfigTmp.IntEnable       = ENABLE;


//    while(TestCnt--)
    {
        /*get free dma channel;*/
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
//        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gTxChNum,gDmaSendData,&gDmaRcvData[63],DATA_LENTH_TRANSFER_0/4);
        
        while(!gTfrFlag){}            
        gTfrFlag = false;
    }

    SYS_TEST("Data Sent:\n");
    for(uint8_t i=0;i<DATA_LENTH_TRANSFER_0/4;i++){
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST("%08x ",gDmaSendData[i]);
    }
    SYS_TEST("\n\n");

    SYS_TEST("Data Rcvd:\n");
    for(uint8_t i=0;i<DATA_LENTH_TRANSFER_0/4;i++){
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST("%08x ",gDmaRcvData[i]);
    }
    SYS_TEST("\n\n");

    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gTxChNum);
    return 0;
}

