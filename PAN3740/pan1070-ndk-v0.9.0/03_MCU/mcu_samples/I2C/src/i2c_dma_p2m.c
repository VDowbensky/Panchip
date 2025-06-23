/********************************************************* * 
@file		:i2c_dma_p2m.c 
@brief  	:i2c dma test case,dma transmit form peripheral i2c to memory 
@author 	:zhongfeng
@version	:v1.0 
@date 		:10/09/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "iic_common.h"


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
//        I2C_DMATransferDataLevel(I2Cx, I2C_TX_TL_3);
        I2C_DMAReceiveDataLevel(I2Cx, I2C_RX_TL_3);
        I2C_DMACmd(I2Cx, ENABLE);
        // Initialize the DMA controller
        DMAC_Init(DMA);
        NVIC_EnableIRQ(DMA_IRQn);
    }

}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t I2C_DmaPeripheral2MemTranferCase9(void)
{
//    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};
    uint8_t RcvDataBuf[DATA_LENTH_TRANSFER_0] = {0};
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    I2C_SysInit(I2C0,DMA_ENABLE,ADR10BIT_DISABLE);
    
    SYS_TEST("I2C_DmaPeripheral2MemTranferCase9\r\n");

    /*set tx dma channel control&config register*/
    RxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Increment;
    RxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    RxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    RxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    RxConfigTmp.DataWidthDst    = DMAC_DataWidth_32;
    RxConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    RxConfigTmp.TransferType    = DMAC_TransferType_Per2Mem;
    RxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    RxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    RxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    RxConfigTmp.PeripheralSrc   = DMAC_Peripheral_I2C0_Rx;
    RxConfigTmp.IntEnable       = ENABLE;
      
//    while(TestCnt--)
    {
        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(I2C0->DATACMD),RcvDataBuf,DATA_LENTH_TRANSFER_0);
//        while(!I2cStopFlag){}
        while(!gTfrFlag){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gTfrFlag = false;
        I2cStopFlag = 0;
    }
    for(uint32_t i=0;i<DATA_LENTH_TRANSFER_0;i++){
        SYS_TEST("%02x ",RcvDataBuf[i]);
    }
    SYS_TEST("\r\n");
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

