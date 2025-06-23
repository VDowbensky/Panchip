/********************************************************* * 
@file		:i2c_interrupt_test.c 
@brief  	:i2c interrupt test case
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/10/16
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "iic_common.h"


static void I2C_InitRoleStatic(I2C_T * I2Cx, uint16_t role,uint8_t Adr10BitEn)
 {
	 I2C_InitTypeDef  I2C_InitStructure;
 
	 /* I2C configuration */
	 I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	 //The IC_SAR holds the slave address when the I2C is operating as a slave
	 if(Adr10BitEn){
		I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS10;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
	 }
	 else{
		I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS7;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	 }
	 I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED_400K;
 
	 I2C_Cmd(I2Cx, DISABLE);
	 I2C_Init(I2Cx, &I2C_InitStructure);
 
	 if(role == I2C_MODE_MASTER)
	 {
		 //1. set master mode
		 I2C_SetMode(I2Cx, I2C_MODE_MASTER);
		 //2. set remote slave addr
		 if(Adr10BitEn)
			I2C_Set10bitAddress(I2Cx, I2C_SLAVE_OWN_ADDRESS10);
		 else
			I2C_Set7bitAddress(I2Cx, I2C_SLAVE_OWN_ADDRESS7);
		 I2Cx->SDA_HOLD = ((0x3<<16) | 0x3);
	 }
	 else
	 {
		 I2C_SetMode(I2Cx, I2C_MODE_SLAVE);
	 }
	 I2C_DisableAllIT(I2Cx);	 
	 I2C_ITConfig(I2Cx, I2C_IT_START_DET|I2C_IT_RX_FULL|I2C_IT_RD_REQ|I2C_IT_STOP_DET, ENABLE);

	  /* I2C Peripheral Enable */
	 I2C_Cmd(I2Cx, ENABLE);
 }

/*
  * @brief	    :i2c config init  & DMA enable
  * param I2Cx  :base address of the I2C peripheral.
  * param role  :master or slave mode select.
  * param u8Adr10BitEn  :10bit address enable or not
  * @retval     :none
*/
static void I2C_SysInit(I2C_T * I2Cx,uint8_t DmaEn,uint8_t u8Adr10BitEn)
{
		
    I2C_InitRoleStatic(I2Cx,I2C_MODE_MASTER,u8Adr10BitEn);
    I2C_SetTxTirggerLevel(I2Cx, I2C_TX_TL_0);  
    I2C_SetRxTirggerLevel(I2Cx, I2C_RX_TL_0);   
    
    if(I2C0 == I2Cx)
        NVIC_EnableIRQ(I2C0_IRQn);
//    else if(I2C1 == I2Cx)
//        NVIC_EnableIRQ(I2C1_IRQn);
}

/*
  * @brief	   :spi dma transfer,data from memory to peripheral
  * @retval    :0/1
*/
uint32_t I2C_MasterInterruptCase11(void)
{ 
    uint8_t i = 0,ReadSize = __BYTE_LEN;
    uint8_t WriteBuf[__BYTE_LEN+1] = {0};

    /*spi clk\frame_mode\data_format\mode\POL\PHA\interrupt mask */
    I2C_SysInit(I2C0,DMA_DISABLE,ADR10BIT_DISABLE);
    SYS_TEST("I2C_InterruptCase11 \r\n");

    FillBuffer(WriteBuf,__BYTE_LEN);
    I2C_Write(I2C0,WriteBuf,__BYTE_LEN);
    
        
//    delay(100000000);
    while(ReadSize--){
        while(!I2C_GetFlagStatus(I2C0,I2C_FLAG_TFNF));
        I2C_SendCmd(I2C0,I2C_CMD_RD);
    }
    while(!I2C_GetFlagStatus(I2C0,I2C_FLAG_TFNF));
    I2C_SendCmd(I2C0,I2C_CMD_STOP);
    while(!I2cStopFlag){}
    for(i=0;i<__BYTE_LEN;i++){SYS_TEST("%02X ",RxBuf[i]);}SYS_TEST("\r\n");
    return 0;
}

