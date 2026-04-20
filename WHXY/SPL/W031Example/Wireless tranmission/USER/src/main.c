/******************************************************************************/
/** \file main.c
 **
 ** A detailed description is available at
 ** @link Sample Group Some description @endlink
 **
 **   - 2021-03-12  1.0  xiebin First version for Device Driver Library of Module.
 **
 ******************************************************************************/
/*******************************************************************************
*
* 代码许可和免责信息
* 武汉芯源半导体有限公司授予您使用所有编程代码示例的非专属的版权许可，您可以由此
* 生成根据您的特定需要而定制的相似功能。根据不能被排除的任何法定保证，武汉芯源半
* 导体有限公司及其程序开发商和供应商对程序或技术支持（如果有）不提供任何明示或暗
* 含的保证或条件，包括但不限于暗含的有关适销性、适用于某种特定用途和非侵权的保证
* 或条件。
* 无论何种情形，武汉芯源半导体有限公司及其程序开发商或供应商均不对下列各项负责，
* 即使被告知其发生的可能性时，也是如此：数据的丢失或损坏；直接的、特别的、附带的
* 或间接的损害，或任何后果性经济损害；或利润、业务、收入、商誉或预期可节省金额的
* 损失。
* 某些司法辖区不允许对直接的、附带的或后果性的损害有任何的排除或限制，因此某些或
* 全部上述排除或限制可能并不适用于您。
*
*******************************************************************************/
/******************************************************************************
 * Include files
 ******************************************************************************/
#include "../inc/main.h"
#include "radio.h"
#include "pan3028.h" 
#include "oled.h"
#include "display.h"
#include "cw32w031bios.h"
/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')                                      *
 ******************************************************************************/

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
void RCC_Configuration(void);
void NVIC_Config(void);
//extern struct RxDoneMsg RxDoneParams;
uint32_t app_tx_mode = 0;
//static uint16_t m_au16Adc1Value[17];

#ifdef __GNUC__
    /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
    set to 'Yes') calls __io_putchar() */
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ** LED1, LED2闪烁
 **
 ******************************************************************************/
volatile uint8_t gFlagIrq;
uint16_t valueAdc;
double   temp;
		
int32_t main(void)
{
	  uint32_t ret = 0;
	  //float bat_value = 0;
	  //开启时钟，选择HSI为时钟源，HCLK=PCLK=48MHZ
	  RCC_Configuration();

	  //GPIO初始化，uart和控制发送接受模式的引脚配置，key按键配置,LED配置
    GPIO_Configuration();
	
	  //初始化串口
	  UART_Init();
	
	  //OLED的引脚配置以及初始化OLED屏幕
	  OLED_Init();

	  //SPI的引脚配置以及初始化,IRQ也在当中
	  SPI_config();
	
	  //ADC初始化
	  ADC_Config();
	  REGBITS_SET( CW_ADC->START_f.START, ADC_START_START_Msk );

	  NVIC_Config();
	  InitTick(48000000); 
    printf("\r\ncw32w031 TEST BEGIN\r\n");
	  
		
	  //根据PB11选择接收发送模式
	  if(TX_RX_SELECT == TX_SELECT)
	  {
		  app_tx_mode = 1;
	  }
		
	  ret = rf_init();

	  if(ret != OK)
	  {
		dis_err(" RF Init Fail");
		while(1);
	  }
		
	  rf_set_default_para();
	  rf_set_tx_mode(PAN3028_TX_SINGLE);
	  rf_set_rx_mode(PAN3028_RX_CONTINOUS);
	  show_menu();
	  dis_init();
	  OLED_Refresh_Gram();
		
	  //根据PB00选择DCDC/LDO模式,如若开启DCDC模式，还需将开发板上的JP5短接换为JP6
//	  if(DCDC_LDO_SELECT == DCDC_SELECT)
//	  {
//		  rf_set_dcdc_mode(DCDC_ON);
//	  }
//	  else
//	  { 
//		  rf_set_dcdc_mode(DCDC_OFF);
//	  }
		
	  Rf_Irq_Callback();
	  if(app_tx_mode)
	  {
		    printf("tx\r\n");
		    rf_port.set_tx();
		    PAN3028_set_vco(PAN3028_MODE_TX);
	  }
	  else
	  {
		    printf("rx\r\n");
		    rf_port.set_rx();
		    rf_set_mode(PAN3028_MODE_RX);
	  } 
		
	  while(1)
	  {
	      rf_irq_process();
		    key_scan();
		    key_event_process();
		    process_rf_events();
			
	  }

}


void RCC_Configuration(void)
{
    ///< 当使用的时钟源HCLK大于24M,小于等于48MHz：设置FLASH 读等待周期为2 cycle
    
    __RCC_FLASH_CLK_ENABLE();
    FLASH_SetLatency(FLASH_Latency_2);
    
    /* 0. HSI使能并校准 */
    RCC_HSI_Enable(RCC_HSIOSC_DIV1);

    /* 1. 设置HCLK和PCLK的分频系数　*/
    RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
    RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
  
    RCC_SystemCoreClockUpdate(48000000);
}

void NVIC_Config(void)
{
	__disable_irq();
	NVIC_EnableIRQ(GPIOB_IRQn);
  NVIC_EnableIRQ(GTIM1_IRQn);
	__enable_irq();
}

PUTCHAR_PROTOTYPE
{
    USART_SendData_8bit(CW_UART1, (uint8_t)ch);

    while (USART_GetFlagStatus(CW_UART1, USART_FLAG_TXE) == RESET);

    return ch;
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
#ifdef  USE_FULL_ASSERT
 /**
   * @brief  Reports the name of the source file and the source line number
   *         where the assert_param error has occurred.
   * @param  file: pointer to the source file name
   * @param  line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
       /* USER CODE END 6 */
}
#endif
