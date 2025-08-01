#include "myADC.h"
#include "board.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
ADC_InitTypeDef ADC_InitStructure;
__IO uint16_t VREFINT_CAL;
uint16_t getRefAdcValue;
void myADC_delay(void)
{
  uint16_t i = 0;
  for (i = 0; i < 1000; i++)
  {
    ;
  }
  
}

void myADC_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
	/* Enable ADC1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_CURRENT_AD;				
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	 
    GPIO_Init(BOARD_PORT_CURRENT_AD, &GPIO_InitStructure);
  /* DMA1 channel1 configuration ----------------------------------------------*/
//   DMA_DeInit(DMA1_Channel1);
//   DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
//   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_RegularConvertedValueTab;
//   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//   DMA_InitStructure.DMA_BufferSize = 64;
//   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
//   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//   DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
//   /* Enable DMA1 channel1 */
//   DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* ADC1 configuration ------------------------------------------------------*/
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channel1 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 2, ADC_SampleTime_55Cycles5);

  /* Enable ADC1 DMA */
//   ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE); 
    ADC_TempSensorVrefintCmd(ENABLE);
  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  
}
uint16_t myADC_getValue(void)
{
	uint16_t getAdcValue;
    getAdcValue = ADC_GetConversionValue(ADC1);
	return getAdcValue;
}
uint16_t myADC_getADC(uint8_t chl)
{
	uint16_t getAdcValue;
	
	// getAdcValue = ADC_GetConversionValue(ADC1);
    ADC_RegularChannelConfig(ADC1, chl, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    getAdcValue = ADC_GetConversionValue(ADC1);
	return getAdcValue;
}
float myADC_getVoltageValue(void)
{
	uint16_t getAdcValue;
	uint16_t getRefAdcValue;
    float voltage;
    getRefAdcValue = myADC_getADC(ADC_Channel_Vrefint);
    getAdcValue = myADC_getADC(ADC_Channel_1);
    voltage = (float)getAdcValue * 1.2 / getRefAdcValue;
	return voltage;
}
