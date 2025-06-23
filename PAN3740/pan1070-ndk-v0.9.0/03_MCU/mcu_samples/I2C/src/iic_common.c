/********************************************************* * 
@file		:iic_common.c 
@brief  	:common interface source file,store some common interface functions 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#include "PanSeries.h"
#include "iic_common.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile int gTxChNum = 0xff,gTxChNum1 = 0xff;
volatile int gRxChNum = 0xff;

volatile bool gTfrFlag =false,gRxTfrFlag =false,gTfrFlag1 = false;
volatile bool gErrFlag;

uint32_t RxCnt =0;
volatile int I2cStopFlag = 0;

uint32_t TestCnt = TEST_CNT_VALUE;

uint8_t RxBuf[__BYTE_LEN+1] = {0};

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

uint32_t (*I2C_TestEntry[])(void) = 
{
    I2C_RegisterDefaultCheckCase1,
    I2C_SlaveReceiveDataCase2,
    I2C_SlaveSendDataCase3,
    I2C_MasterReceiveDataCase4,
    I2C_MasterSendDataCase5,
    I2C_10BitMasterSendDataCase6,
    I2C_10BitSlaveReceiveDataCase7,
    I2C_DmaMem2PeripheralTranferCase8,
    I2C_DmaPeripheral2MemTranferCase9,
    I2C_MasterGeneralCallCase10,
    I2C_MasterInterruptCase11,
    I2C_SlaveInterruptCase12,
    I2C_DmaMasterReadCase13,
    I2C_DmaSlaveWriteCase14,
};

void I2C_TestFunctionEnter(uint16_t Idx)
{
    (I2C_TestEntry[Idx])();
}

void DMA_IRQHandler()
{
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_TFR))
    {
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR)){
//            SYS_TEST("0_TRF0 \r\n");
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR)){
//            SYS_TEST("1_TRF1 \r\n");
			gRxTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR)){
//            SYS_TEST("2_TRF2 \r\n");
			gTfrFlag1 = true;
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR);          
        }           
    }

}

void I2C0_IRQHandler()
{
	static uint8_t txCnt =0;

    if(I2C_GetITStatus(I2C0, I2C_IT_START_DET))
	{
//        SYS_TEST("I2C_IT_START_DET \r\n");
        I2C_ClearITPendingBit(I2C0, I2C_IT_START_DET);
	}    
    if(I2C_GetITStatus(I2C0, I2C_IT_TX_ABORT))
	{
//        SYS_TEST("I2C_IT_TX_ABORT \r\n");
        I2C_ClearITPendingBit(I2C0, I2C_IT_TX_ABORT);
	}
	if(I2C_GetITStatus(I2C0, I2C_IT_RX_FULL))
	{
//        SYS_TEST("I2C_IT_RX_FULL \r\n");
        while(I2C_GetFlagStatus(I2C0, I2C_FLAG_RFNE))
        {
            RxBuf[RxCnt]= I2C_ReceiveData(I2C0);
            RxCnt++;
        }
        I2C_ClearITPendingBit(I2C0, I2C_IT_RX_FULL);
	}

	if(I2C_GetITStatus(I2C0,I2C_IT_STOP_DET))
	{
//        SYS_TEST("I2C_IT_STOP_DET \r\n");
		txCnt = 0;
		while(I2C_GetFlagStatus(I2C0, I2C_FLAG_RFNE))
		{
            RxBuf[RxCnt]= I2C_ReceiveData(I2C0);
		}
		if(RxCnt>0)
			I2cStopFlag = 1;
		I2C_ClearITPendingBit(I2C0, I2C_IT_STOP_DET);
	}
		
	if(I2C_GetITStatus(I2C0, I2C_IT_RD_REQ))
	{
//        SYS_TEST("RD \r\n");
        while(I2C_GetFlagStatus(I2C0, I2C_FLAG_TFNF)==0);//wait tx fifo no full
        I2C_SendDataCmd(I2C0, 0xa0+txCnt, I2C_CMD_WR);
        txCnt++;
		I2C_ClearITPendingBit(I2C0, I2C_IT_RD_REQ);
	}
}

void delay(uint32_t t)
{
    while(t){t--;};
}


void FillBuffer(uint8_t *pBuffer, uint32_t size)
{
    uint32_t idx = 0;
    
    for(idx = 0; idx < size; idx++)
    {
        pBuffer[idx] = (uint8_t)(idx % 0x100);
    }
}

/*
  * @brief	    :i2c config init  & interrupt enable
  * param I2Cx  :base address of the I2C peripheral.
  * param role  :master or slave mode select.
  * param u8Adr10BitEn  :10bit address enable or not
  * @retval     :none
*/
void I2C_InitRole(I2C_T * I2Cx, uint16_t role,uint8_t Adr10BitEn)
 {
	 I2C_InitTypeDef  I2C_InitStructure;
 
	 /* I2C configuration */
	 I2C_InitStructure.I2C_DutyCycle = 50;
	 //The IC_SAR holds the slave address when the I2C is operating as a slave
	 if(Adr10BitEn){
		I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS10;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
	 }
	 else{
		I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS7;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	 }
	 I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED_100K;
 
	 I2C_Cmd(I2Cx, DISABLE);
	 bool ret = I2C_Init(I2Cx, &I2C_InitStructure);
	 
	 if(!ret){
		SYS_TEST("apb clk can not support the iic speed %d \n",I2C_InitStructure.I2C_ClockSpeed);
	 }
	 if(role == I2C_MODE_MASTER)
	 {
		 //1. set master mode
		 I2C_SetMode(I2Cx, I2C_MODE_MASTER);
		 //2. set remote slave addr
		 if(Adr10BitEn){
			I2C_Set10bitAddress(I2Cx, I2C_SLAVE_OWN_ADDRESS10);
         }
		 else
			I2C_Set7bitAddress(I2Cx, I2C_SLAVE_OWN_ADDRESS7);         
		 I2Cx->SDA_HOLD = ((0x3<<16) | 0x3);
	 }
	 else
	 {
		 I2C_SetMode(I2Cx, I2C_MODE_SLAVE);
	 }
	 I2C_DisableAllIT(I2Cx);	 
	 I2C_ITConfig(I2Cx, I2C_IT_RX_FULL|I2C_IT_RD_REQ|I2C_IT_STOP_DET, ENABLE);

	  /* I2C Peripheral Enable */
	 I2C_Cmd(I2Cx, ENABLE);
 }
const uint32_t IicDefaultValue[]=
{
    0x0000007f,        // I2Cx->CON             
    0x00001055,        // I2Cx->TAR             
    0x00000055,        // I2Cx->SAR              
    0x00000001,        // I2Cx->HS_MADDR         
    0x00000000,        // I2Cx->DATACMD          
    0x00000190,        // I2Cx->SS_SCL_HCNT      
    0x000001d6,        // I2Cx->SS_SCL_LCNT      
    0x0000003c,        // I2Cx->FS_SCL_HCNT      
    0x00000082,        // I2Cx->FS_SCL_LCNT      
    0x00000006,        // I2Cx->HS_SCL_HCNT      
    0x00000010,        // I2Cx->HS_SCL_LCNT      
    0x00000000,        // I2Cx->INTR_STAT        
    0x000008ff,        // I2Cx->INTR_MASK        
    0x00000000,        // I2Cx->RAW_INTR_STAT    
    0x00000000,        // I2Cx->RX_TL            
    0x00000000,        // I2Cx->TX_TL            
    0x00000000,        // I2Cx->CLR_INTR         
    0x00000000,        // I2Cx->CLR_RX_UND       
    0x00000000,        // I2Cx->CLR_RX_OVR       
    0x00000000,        // I2Cx->CLR_TX_OVR       
    0x00000000,        // I2Cx->CLR_RD_REQ       
    0x00000000,        // I2Cx->CLR_TX_ABRT      
    0x00000000,        // I2Cx->CLR_RX_DONE      
    0x00000000,        // I2Cx->CLR_ACTIVITY     
    0x00000000,        // I2Cx->CLR_STOP_DET     
    0x00000000,        // I2Cx->CLR_START_DET    
    0x00000000,        // I2Cx->CLR_GEN_CALL     
    0x00000000,        // I2Cx->IC_ENABLE        
    0x00000006,        // I2Cx->STATUS           
    0x00000000,        // I2Cx->TXFLR            
    0x00000000,        // I2Cx->RXFLR            
    0x00000001,        // I2Cx->SDA_HOLD         
    0x00000000,        // I2Cx->TX_ABRT_SRC      
    0x00000000,        // I2Cx->SLV_DATA_NACK    
    0x00000000,        // I2Cx->DMA_CR           
    0x00000000,        // I2Cx->DMA_TDLR         
    0x00000000,        // I2Cx->DMA_RDLR         
    0x00000064,        // I2Cx->SDA_SETUP        
    0x00000001,        // I2Cx->ACK_GENERAL_CALL 
    0x00000000,        // I2Cx->ENABLE_STATUS    
    0x00000005,        // I2Cx->FS_SPKLEN        
    0x00000001,        // I2Cx->HS_SPKLEN        
    0x00000000         // I2Cx->CLR_RESTART_DET               
};

/*
  * @brief	   :iic register default value check
  * @retval    :0/1
*/
uint32_t I2C_RegisterDefaultCheckCase1(void)
{
    uint8_t i = 0,different = 0;
    I2C_T * I2Cx;    
    
    I2Cx = I2C0;
    while(i<43){
        if(((__IO uint32_t *)(&I2Cx->CON))[i] != IicDefaultValue[i]){
            SYS_TEST("iic0,iic addr offset:%x,expect:%08x,current:%08x \r\n",i*4,IicDefaultValue[i],((__IO uint32_t *)(&I2Cx->CON))[i]);
            different++;
        }
        i++;
    }
    if(!different){
       SYS_TEST("iic default value check ok \r\n"); 
    }
    return 0;
}
