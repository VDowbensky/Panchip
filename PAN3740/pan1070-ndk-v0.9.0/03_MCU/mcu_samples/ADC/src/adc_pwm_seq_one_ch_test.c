/********************************************************* * 
@file		:adc_pwm_seq_one_ch_test.c 
@brief  	:pwm sequential mode in ad one channel test
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/10/28
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "adc_common.h"


/*
  * @brief	   :iic register default value check
  * @retval    :0/1
*/
uint32_t ADC_OneAdcChSequentialTestCase7(void)
{
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};
    uint32_t RcvDataBuf[128] = {0};
	
	DMAC_Init(DMA);
	NVIC_EnableIRQ(DMA_IRQn);
	
    RxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Increment;
    RxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    RxConfigTmp.BurstLenDst     = DMAC_BurstLen_8;
    RxConfigTmp.BurstLenSrc     = DMAC_BurstLen_8;
    RxConfigTmp.DataWidthDst    = DMAC_DataWidth_16;
    RxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    RxConfigTmp.TransferType    = DMAC_TransferType_Per2Mem;
    RxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    RxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    RxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    RxConfigTmp.PeripheralSrc   = DMAC_Peripheral_ADC;
    RxConfigTmp.IntEnable       = ENABLE;
      
	/*get free dma channel;*/
	uint8_t gRxChNum = DMAC_AcquireChannel(DMA);
	/*enable dma transfer interrupt*/
	DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
	DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
	/*start dma tx channel*/
	DMAC_StartChannel(DMA, gRxChNum,(void*)&(ADC->DAT),RcvDataBuf,100);	
		
    ADC_Open(ADC,ADC_TEST_CH);
    //0.0V~VDD & 0.0V~1.2V both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_HIGH);   
    ADC_PowerOn(ADC);

	ADC_IntMask(ADC,ADC_STATUS_INTMSK_HALF_Msk,DISABLE);
	ADC_SeqOneChModeConfig(ADC,ADC_FALLING_EDGE_TRIGGER,ADC_FIFO_TRIG_LEVEL_HALF,1,1);
    // TRG1CTL  select for 1-shunt sequential mode
    ADC_Trigger2Select(ADC,DISABLE);
    // ADC PWM first Trigger Type and Source Selection, PWM0 Falling for ADC one channel;
    ADC_SeqModeTriggerSrc(ADC, ADC_SEQMODE_PWM0_PERIOD);
    // Delay ADC start conversion time after PWM trigger
    ADC_TriggerDelay(ADC, 0x0a);     
	ADC_SetExtraSampleTime(ADC,0x20);
    // Power on ADC
	
    // PWM0 frequency is 1200Hz, duty 30%,
    PWM_ConfigOutputChannel(PWM, 0, 100000, 50, OPERATION_EDGE_ALIGNED);
    // Enable output of 0 PWM channel
    PWM_EnableOutput(PWM, BIT0);
    // Start PWM
    PWM_Start(PWM, BIT0);        
    
	while(!gTfrFlag){}
	/*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gRxChNum);
	gTfrFlag = false;
	
    ADC_PowerDown(ADC);
	PWM_ForceStop(PWM,BIT0);
	CLK->IPRST1 |= (0x1<<8);
		
	for(uint8_t cnt = 0;cnt < 100;cnt++){
		SYS_TEST("%d ",RcvDataBuf[cnt]);
		if(cnt % 10 == 9)
			SYS_TEST("\r\n");
	}
	CLK->IPRST1 &= ~(0x1<<8);
    return 0;                  
}

