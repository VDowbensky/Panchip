/********************************************************* * 
@file		:uart_dma_p2m.c 
@brief  	:i2c dma test case,dma transmit form peripheral i2c to memory 
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
uint32_t UART_DmaPeripheral2MemTranferCase11(void)
{
    // Reset UART Rx FIFO to avoid DMA malfunction
    UART_ResetRxFifo(DMA_UART_SELECT);

//    UART_SetRxTrigger(UART0, UART_RX_FIFO_HALF_FULL);

    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};
    uint8_t RcvDataBuf[512] = {0};

	//UART_SetDmaMode(UART0);
	// Initialize the DMA controller
	DMAC_Init(DMA);
	NVIC_EnableIRQ(DMA_IRQn);

    /*set tx dma channel control&config register*/
    RxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Increment;
    RxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    RxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    RxConfigTmp.BurstLenSrc     = DMAC_BurstLen_8;
    RxConfigTmp.DataWidthDst    = DMAC_DataWidth_32;
    RxConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    RxConfigTmp.TransferType    = DMAC_TransferType_Per2Mem;
    RxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    RxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    RxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    RxConfigTmp.PeripheralSrc   = DMA_UART_PERIPHERAL_SRC;
    RxConfigTmp.IntEnable       = ENABLE;
      
//    while(TestCnt--)
    {
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
        /*start dma rx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(DMA_UART_SELECT->RBR_THR_DLL),RcvDataBuf,DATA_LENTH_TRANSFER_0/4);
//        while(!I2cStopFlag){}
        SYS_TEST("Please input data sending to UART...\r\n");
        while(!gTfrFlag){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gTfrFlag = false;
    }
    SYS_TEST("\r\n");
    for(uint8_t i=0;i<DATA_LENTH_TRANSFER_0/4;i++){
        SYS_TEST("0x%02x ",RcvDataBuf[i]);
    }
    SYS_TEST("\r\n");
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

