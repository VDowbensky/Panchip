/********************************************************* * 
@file		:i2c_dma_m2p.c 
@brief  	:i2c dma test case,dma transmit form memory to peripheral i2c
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
		
    I2C_InitRole(I2Cx,I2C_MODE_MASTER,u8Adr10BitEn);
    I2C_SetTxTirggerLevel(I2Cx, I2C_TX_TL_3);
    
    if(DmaEn == DMA_ENABLE){
        I2C_DMATransferDataLevel(I2Cx, I2C_TX_TL_3);
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
uint32_t I2C_DmaMem2PeripheralTranferCase8(void)
{
    uint8_t TmpBuf[DATA_LENTH_TRANSFER_0] = {0};
//    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    I2C_SysInit(I2C0,DMA_ENABLE,ADR10BIT_DISABLE);


    FillBuffer(TmpBuf,DATA_LENTH_TRANSFER_0);

    /*set tx dma channel control&config register*/
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = DMAC_Peripheral_I2C0_Tx;
    TxConfigTmp.IntEnable       = ENABLE;
      
//    while(TestCnt--)
    {
        /*get free dma channel;*/
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gTxChNum,TmpBuf,(void*)&(I2C0->DATACMD),DATA_LENTH_TRANSFER_0/4);
        while(!gTfrFlag){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gTxChNum);
        gTfrFlag = false;
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

