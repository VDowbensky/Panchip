/********************************************************* * 
@file		:dma_common.c 
@brief  	:common interface source file,store some common interface functions 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "adc_common.h"

volatile bool compare_finish_flag = false;
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile bool gTfrFlag =false,gRxTfrFlag =false,gTfrFlag1 = false;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

uint32_t (*ADC_TestEntry[])(void) = 
{
    ADC_RegisterDefaultCheckCase1,
    ADC_ConvertTestCase2,
    ADC_ExternaltriggerTestCase3,
    ADC_CompareTestCase4,
    ADC_PwmSequentialTestCase5,
    ADC_TemperatureTestCase6,
	ADC_OneAdcChSequentialTestCase7,
	ADC_LeftShiftTestCase8,
	ADC_ConvertTestChVbgCase9,
	ADC_ConvertTestChVDD4Case10
};

void ADC_TestFunctionEnter(uint16_t Idx)
{
    (ADC_TestEntry[Idx])();
}

void DMA_IRQHandler()
{
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_TFR))
    {
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR)){
            SYS_TEST("0_TRF0 \r\n");
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR)){
//            SYS_TEST("1_TRF1 \r\n");
			gRxTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR);
        }
    }

}

void ADC_IRQHandler(void)
{
    if(ADC_IsIntOccured(ADC, ADC_ADIF_INT)) // Get ADC comparator interrupt flag
    {
        SYS_TEST("Convert end \r\n");
        ADC_ClearIntFlag(ADC,ADC_ADIF_INT);
    }
    else if(ADC_IsIntOccured(ADC, ADC_CMP0_INT)) // Get ADC comparator interrupt flag
    {
        SYS_TEST("Channel 5 input < 2000 \r\n");
        ADC_ClearIntFlag(ADC,ADC_CMP0_INT);
		compare_finish_flag = true;
    }
    else if(ADC_IsIntOccured(ADC, ADC_CMP1_INT)) // Get ADC comparator interrupt flag
    {
        SYS_TEST("Channel 5 input >= 2000 \r\n"); 
        ADC_ClearIntFlag(ADC,ADC_CMP1_INT); 
		compare_finish_flag = true;
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
        pBuffer[idx] = (uint8_t)(idx % 0xFF);
    }
     
}

void ADC_SetExternalTriggerProcess(void)
{
	SYS_SET_MFP(P0, 5, EXT_STADC);  //Set P44 as external trigger channel
    GPIO_SetMode(P0, BIT5, GPIO_MODE_INPUT);

    SYS_SET_MFP(P1, 0, GPIO);
    GPIO_SetMode(P1, BIT0, GPIO_MODE_OUTPUT);
		P10 = 1; 
		SYS_delay_10nop(1000);   
		P10 = 0;
		SYS_delay_10nop(1000);
		P10 = 1;
    SYS_delay_10nop(1000);    
}
