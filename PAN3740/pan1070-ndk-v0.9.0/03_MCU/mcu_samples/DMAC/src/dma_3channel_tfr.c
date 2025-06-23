/********************************************************* * 
@file		:dma_3channel_tfr.c 
@brief  	:3dma channel transfer form memory to peripheral spi with normal dma channel config
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
    spiInitStruct.SPI_role = SPI_RoleMaster;
//    SPI1->CPSR  = 2;
    SPI_Init(SPI1, &spiInitStruct);
    SPI_DisableIrq(SPI1, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI1);
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @param in  :where TestCnt is count of transfer 
  * @retval    :none
*/
uint32_t DMA_ThreeChannelTranferCase19(void)
{
	if(__MaxDmaCh < 3)
		SYS_TEST("Unsupported case, There are not enough DMA channels!\n");
	else{
		uint32_t TestCntValue = TestCnt;
		DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
		DMAC_ChannelConfigTypeDef TxConfigTmp1 = {0,};
		DMAC_ChannelConfigTypeDef TxConfigTmp2 = {0,};
		
		/*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
		SPI_SysInit();
		/*dma init*/
		DMAC_Init(DMA);
		NVIC_EnableIRQ(DMA_IRQn);
		//UART_SetDmaMode(UART0);

		FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);
		FillBuffer(gDmaSendData1,DATA_LENTH_TRANSFER_0/4);

		/*set spi tx dma channel control&config register*/
		TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
		TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
		TxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
		TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_1;
		TxConfigTmp.DataWidthDst    = DMAC_DataWidth_16;
		TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
		TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
		TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
		TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
		TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
		TxConfigTmp.PeripheralDst   = DMAC_Peripheral_SPI0_Tx;
		TxConfigTmp.IntEnable       = ENABLE;
		/*spi1 master transfer*/
		TxConfigTmp1.AddrChangeDst   = DMAC_AddrChange_NoChange;
		TxConfigTmp1.AddrChangeSrc   = DMAC_AddrChange_Increment;
		TxConfigTmp1.BurstLenDst     = DMAC_BurstLen_4;
		TxConfigTmp1.BurstLenSrc     = DMAC_BurstLen_1;
		TxConfigTmp1.DataWidthDst    = DMAC_DataWidth_16;
		TxConfigTmp1.DataWidthSrc    = DMAC_DataWidth_32;
		TxConfigTmp1.TransferType    = DMAC_TransferType_Mem2Per;
		TxConfigTmp1.FlowControl     = DMAC_FlowControl_DMA;
		TxConfigTmp1.HandshakeDst    = DMAC_Handshake_Hardware;
		TxConfigTmp1.HandshakeSrc    = DMAC_Handshake_Hardware;
		TxConfigTmp1.PeripheralDst   = DMAC_Peripheral_SPI1_Tx;
		TxConfigTmp1.IntEnable       = ENABLE;
		/*uart transfer*/
		TxConfigTmp2.AddrChangeDst   = DMAC_AddrChange_NoChange;
		TxConfigTmp2.AddrChangeSrc   = DMAC_AddrChange_Increment;
		TxConfigTmp2.BurstLenDst     = DMAC_BurstLen_8;
		TxConfigTmp2.BurstLenSrc     = DMAC_BurstLen_4;
		TxConfigTmp2.DataWidthDst    = DMAC_DataWidth_8;
		TxConfigTmp2.DataWidthSrc    = DMAC_DataWidth_32;
		TxConfigTmp2.TransferType    = DMAC_TransferType_Mem2Per;
		TxConfigTmp2.FlowControl     = DMAC_FlowControl_DMA;
		TxConfigTmp2.HandshakeDst    = DMAC_Handshake_Hardware;
		TxConfigTmp2.HandshakeSrc    = DMAC_Handshake_Hardware;
		TxConfigTmp2.PeripheralDst   = DMA_UART_PERIPHERAL_DST;
		TxConfigTmp2.IntEnable       = ENABLE;
		
		while(TestCntValue--){
			/*get free dma channel;*/
			gTxChNum = DMAC_AcquireChannel(DMA);
			/*enable dma transfer interrupt*/
			DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
			DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
			/*start dma tx channel*/
			DMAC_StartChannel(DMA, gTxChNum,gDmaSendData,(void*)&(SPI0->DR),DATA_LENTH_TRANSFER_0);

			/*get free dma channel;*/
			gTxChNum1 = DMAC_AcquireChannel(DMA);
			/*enable dma transfer interrupt*/
			DMAC_ClrIntFlagMsk(DMA,gTxChNum1,DMAC_FLAG_INDEX_TFR);
			DMAC_SetChannelConfig(DMA,gTxChNum1,&TxConfigTmp1);
			/*start dma tx channel*/
			DMAC_StartChannel(DMA, gTxChNum1,gDmaSendData1,(void*)&(SPI1->DR),DATA_LENTH_TRANSFER_0/4);

			/*get free dma channel;*/
			gRxChNum = DMAC_AcquireChannel(DMA);
			/*enable dma transfer interrupt*/
			DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
			DMAC_SetChannelConfig(DMA,gRxChNum,&TxConfigTmp2);
			/*start dma tx channel*/
			DMAC_StartChannel(DMA, gRxChNum,gDmaSendData,(void*)&(DMA_UART_SELECT->RBR_THR_DLL),DATA_LENTH_TRANSFER_0/16);
			
			/*enable spi tx_rx dma mode */
			SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,1); 
			SPI_SetDmaRxTxEnabled(SPI1, SPI_TX_DMAC_SELECT,1); 
			while(!(gTfrFlag && gRxTfrFlag && gTfrFlag1)){}
			/*disable spi tx_rx dma mode */
			SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,0);
			SPI_SetDmaRxTxEnabled(SPI1, SPI_TX_DMAC_SELECT,0);
				
			/*release tx dma channel*/
			DMAC_ReleaseChannel(DMA,gTxChNum);
			DMAC_ReleaseChannel(DMA,gTxChNum1);
			DMAC_ReleaseChannel(DMA,gRxChNum);
			gTfrFlag = false;
			gRxTfrFlag = false;
			gTfrFlag1 = false;
		}
        SYS_delay_10nop(0x10000);

		/*disable dma*/
		DMAC_DeInit(DMA);
		/*release tx dma channel*/
		DMAC_ReleaseChannel(DMA,gTxChNum);
		DMAC_ReleaseChannel(DMA,gRxChNum);
		DMAC_ReleaseChannel(DMA,gTxChNum1);
	}
    return 0;
}

