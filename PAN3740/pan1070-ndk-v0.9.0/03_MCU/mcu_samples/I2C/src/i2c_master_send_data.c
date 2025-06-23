/********************************************************* * 
@file		:i2c_master_send_data.c 
@brief  	:i2c master send data case
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/10/16
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
    
//    if(I2C0 == I2Cx)
//        NVIC_EnableIRQ(I2C0_IRQn);
//    else if(I2C1 == I2Cx)
//        NVIC_EnableIRQ(I2C1_IRQn);
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t I2C_MasterSendDataCase5(void)
{
    uint8_t WriteBuf[__BYTE_LEN+1] = {0};

    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    I2C_SysInit(I2C0,DMA_DISABLE,ADR10BIT_DISABLE);
    SYS_TEST("I2C_MasterSendDataCase5 \r\n");

    FillBuffer(WriteBuf,__BYTE_LEN);
    I2C_Write(I2C0,WriteBuf,__BYTE_LEN);

    return 0;
}

