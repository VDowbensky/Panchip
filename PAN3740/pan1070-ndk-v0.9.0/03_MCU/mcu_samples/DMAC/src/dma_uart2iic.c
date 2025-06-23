/********************************************************* * 
@file		:dma_uart2iic.c 
@brief  	:i2c dma test case,dma transmit from i2c to uart
@author 	:zhongfeng
@version	:v1.0 
@date 		:10/09/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "dma_common.h"

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
uint32_t DMAC_Uart2IicTranferCase14(void)
{
//    uint32_t TestCntValue = TestCnt;
//    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    DMAC_ChannelConfigTypeDef DmaConfigTmp = {0,};
    
    //UART_SetDmaMode(UART0);
	// Initialize the DMA controller
	DMAC_Init(DMA);
	NVIC_EnableIRQ(DMA_IRQn);
    I2C_SysInit(I2C0,DMA_ENABLE,ADR10BIT_DISABLE);
    I2C_DMACmd(I2C0, ENABLE);


    FillBuffer(gDmaSendData,DATA_LENTH_TRANSFER_0);
             
    /*set rx dma channel control&config register*/
    DmaConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    DmaConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    DmaConfigTmp.BurstLenDst     = DMAC_BurstLen_8;
    DmaConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    DmaConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    DmaConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    DmaConfigTmp.TransferType    = DMAC_TransferType_Per2Per;
    DmaConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    DmaConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    DmaConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    DmaConfigTmp.PeripheralDst   = DMA_UART_PERIPHERAL_DST;
    DmaConfigTmp.PeripheralSrc   = DMAC_Peripheral_I2C0_Rx;
    DmaConfigTmp.IntEnable       = ENABLE;


//    while(TestCnt--)
    {        
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&DmaConfigTmp);
        /*start dma rx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(I2C0->DATACMD),(void*)&(DMA_UART_SELECT->RBR_THR_DLL),DATA_LENTH_TRANSFER_0);
        
        while(!gTfrFlag){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gTfrFlag = false;
        SYS_delay_10nop(0x10000);
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

