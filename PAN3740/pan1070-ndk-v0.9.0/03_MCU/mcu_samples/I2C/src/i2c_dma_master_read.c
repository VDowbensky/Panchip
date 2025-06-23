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
    I2C_SetRxTirggerLevel(I2Cx, I2C_RX_TL_3);
    
    if(DmaEn == DMA_ENABLE){
        I2C_DMATransferDataLevel(I2Cx, I2C_TX_TL_3);
        I2C_DMAReceiveDataLevel(I2Cx, I2C_RX_TL_3);
//        I2C_DMACmd(I2Cx, ENABLE);
        // Initialize the DMA controller
        DMAC_Init(DMA);
        NVIC_EnableIRQ(DMA_IRQn);
    }
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t I2C_DmaMasterReadCase13(void)
{
    uint16_t TmpBuf[__BYTE_LEN] = {0};
    uint8_t RcvDataBuf[128] = {0};
    uint32_t i = 0;
//    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};
    
    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    I2C_SysInit(I2C0,DMA_ENABLE,ADR10BIT_DISABLE);

    for(i=0;i<__BYTE_LEN-1;i++){
        TmpBuf[i] = (I2C_CMD_RD << 8);
    }
    TmpBuf[i] = ((I2C_CMD_RD|I2C_CMD_STOP) << 8);
    
//    FillBuffer(TmpBuf,DATA_LENTH_TRANSFER_0);

    /*set tx dma channel control&config register*/
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
    TxConfigTmp.PeripheralDst   = DMAC_Peripheral_I2C0_Tx;
    TxConfigTmp.IntEnable       = ENABLE;

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
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gTxChNum,TmpBuf,(void*)&(I2C0->DATACMD),__BYTE_LEN/2);

        /*get free dma channel;*/
        gRxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
        /*start dma tx channel*/
        DMAC_StartChannel(DMA, gRxChNum,(void*)&(I2C0->DATACMD),RcvDataBuf,__BYTE_LEN);
        I2C_DMACmd(I2C0, ENABLE);
        while(!(gTfrFlag && gRxTfrFlag)){}
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gTxChNum);
        DMAC_ReleaseChannel(DMA,gRxChNum);
        gTfrFlag = false;
    }
    for(i=0;i<__BYTE_LEN;i++)
    {
        SYS_TEST("%02x ",RcvDataBuf[i]);
    }   
    SYS_TEST("\r\n");
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
    DMAC_ReleaseChannel(DMA,gRxChNum);
    return 0;
}

