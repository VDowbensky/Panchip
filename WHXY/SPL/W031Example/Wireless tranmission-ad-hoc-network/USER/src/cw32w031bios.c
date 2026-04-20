#include "cw32w031bios.h"
#include "LED_light.h"
#define DEBUG_USART_BaudRate   115200
#define	RESET   0
#define	SET     1
Fifo_t uart_rx_fifo;
volatile uint8_t u8RxData;
uint32_t tx_mode = 0;
uint32_t tx_power = 0;
uint32_t freq_mode = 0;
uint32_t tx_pkt_max_num = 1;
uint32_t tx_pkt_cnt = 0;
uint32_t rx_pkt_cnt = 0; 
uint8_t tx_buf[256];
uint32_t tx_len = 10;
uint32_t tx_time;
uint32_t tx_ing = 0;
uint32_t keys_pressed = 0;
uint32_t key_second_pressed = 0;
uint32_t sys_tick = 0;
uint32_t Period = 500;    // 周期，单位us
uint32_t PosWidth = 0;    // ch2正脉宽，单位us
uint32_t PosWidth2 = 0;    // ch3正脉宽，单位us
uint8_t Dir = 1;    // 计数方向 1增加，0 减少

extern uint32_t app_tx_mode;
extern uint32_t cad_tx_timeout_flag;

Fifo_t uart_rx_fifo;
uint8_t uart_rx_buf[256];

void  GPIO_Configuration(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	__RCC_GPIOC_CLK_ENABLE();
	__RCC_GPIOF_CLK_ENABLE();
																																																				
	//uart的初始化
	PA08_AFx_UART1TXD();
	PA09_AFx_UART1RXD();
	
	 GPIO_InitStructure.Pins = GPIO_PIN_8;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.Pins = GPIO_PIN_9;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure); 
	
	//发送接收模式选择引脚PB11初始化   DCDC/LDO模式选择引脚PB00初始化
	GPIO_InitStructure.Pins = GPIO_PIN_11 | GPIO_PIN_0;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure); 
	
	//按键初始化
	GPIO_InitStructure.Pins = GPIO_PIN_11 | GPIO_PIN_12 ;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pins = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15 ;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure);

  //KEY1配置成下降沿中断
  GPIO_InitStructure.Pins = GPIO_PIN_10;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.IT = GPIO_IT_FALLING;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure);

  PA06_PUSHPULL_ENABLE();
 	PA06_AFx_GTIM1CH1();
	PA06_DIGTAL_ENABLE();
  PA06_DIR_OUTPUT();
		
	PB10_AFx_GTIM2CH3();
	PB10_DIGTAL_ENABLE();
  PB10_DIR_OUTPUT();
	PB10_PUSHPULL_ENABLE();

 	PA06_SETLOW();
	PB10_SETLOW();
}
int uartFifoRead(uint8_t *buf, int bufLen)
{
    int len = 0;
    int i;
    for(i=0; i<bufLen; i++)
    {
        if(!IsFifoEmpty(&uart_rx_fifo))
        {
            buf[i] = FifoPop(&uart_rx_fifo);
            len++;
        }
        else
        {
            break;
        }
    }

    return len;
}
void  UART_Init(void)
{
	    __RCC_UART1_CLK_ENABLE();
	    USART_InitTypeDef USART_InitStructure = {0};

      USART_InitStructure.USART_BaudRate = DEBUG_USART_BaudRate;
      USART_InitStructure.USART_Over = USART_Over_16;
      USART_InitStructure.USART_Source = USART_Source_PCLK;
      USART_InitStructure.USART_UclkFreq = 48000000;
      USART_InitStructure.USART_StartBit = USART_StartBit_FE;
      USART_InitStructure.USART_StopBits = USART_StopBits_1;
      USART_InitStructure.USART_Parity = USART_Parity_No ;
      USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
      USART_Init(CW_UART1, &USART_InitStructure);
		  USART_ITConfig(CW_UART1, USART_IT_RC, ENABLE);
	
		  FifoInit(&uart_rx_fifo, uart_rx_buf, 256);
}

void  SPI_config(void)
{
	   GPIO_InitTypeDef GPIO_InitStructure = {0};
	 
	   PB05_AFx_SPI1MOSI();
	   PB04_AFx_SPI1MISO();
	   PB13_AFx_SPI1SCK();
	 
	   GPIO_InitStructure.Pins = GPIO_PIN_5 | GPIO_PIN_3 | GPIO_PIN_13;
     GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	 
	   GPIO_InitStructure.Pins = GPIO_PIN_4;
     GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
     GPIO_Init(CW_GPIOB, &GPIO_InitStructure); 
	 
	   //pan3028中断引脚配置
	   GPIO_InitStructure.IT=GPIO_IT_RISING;
	   GPIO_InitStructure.Pins = GPIO_PIN_6| GPIO_PIN_7;
     GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
     GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	 
	   PB03_SETHIGH();
	 
	   __RCC_SPI1_CLK_ENABLE();
     SPI_InitTypeDef SPI_InitStructure;

     SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    //双向全双工
     SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                         // 主机模式
     SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                     // 帧数据长度为8bit
     SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                            // 时钟空闲电平为低
     SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                          // 第一个边沿采样
     SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                             // 片选信号由SSI寄存器控制
     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;    // 波特率为PCLK的8分频 6MHz
     SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                    // 最高有效位 MSB 收发在前
     SPI_InitStructure.SPI_Speed = SPI_Speed_High;                          // 高速SPI
  
     SPI_Init(CW_SPI1, &SPI_InitStructure);

     SPI_Cmd(CW_SPI1, ENABLE); 

	
}

void GTIM_Config(void)
{
     GTIM_InitTypeDef GTIM_InitStruct = {0};

     __RCC_GTIM1_CLK_ENABLE();
     GTIM_InitStruct.Mode = GTIM_MODE_TIME;
     GTIM_InitStruct.OneShotMode = GTIM_COUNT_CONTINUE;
     GTIM_InitStruct.Prescaler = 11;    // 计数时钟TCLKD = TCLK/(PSC+1)
     GTIM_InitStruct.ReloadValue = 2000 - 1;
     GTIM_InitStruct.ToggleOutState = DISABLE;
     GTIM_TimeBaseInit(CW_GTIM1, &GTIM_InitStruct);
     GTIM_OCInit(CW_GTIM1, GTIM_CHANNEL1, GTIM_OC_OUTPUT_PWM_HIGH);
     GTIM_SetCompare1(CW_GTIM1, 2000);
     GTIM_ITConfig(CW_GTIM1, GTIM_IT_OV, ENABLE);
				
		 __RCC_GTIM2_CLK_ENABLE();
     GTIM_InitStruct.Mode = GTIM_MODE_TIME;
     GTIM_InitStruct.OneShotMode = GTIM_COUNT_CONTINUE;
     GTIM_InitStruct.Prescaler = 11;    // 计数时钟TCLKD = TCLK/(PSC+1)
     GTIM_InitStruct.ReloadValue = 2000 - 1;
     GTIM_InitStruct.ToggleOutState = DISABLE;
     GTIM_TimeBaseInit(CW_GTIM2, &GTIM_InitStruct);
     GTIM_OCInit(CW_GTIM2, GTIM_CHANNEL3, GTIM_OC_OUTPUT_PWM_HIGH);
     GTIM_SetCompare3(CW_GTIM2, 2000);
     GTIM_ITConfig(CW_GTIM2, GTIM_IT_OV, ENABLE);
}

uint16_t time_ms = 0;
void GTIM1_MS(uint16_t ms)
{
    time_ms = ms;
	GTIM_Cmd(CW_GTIM1,ENABLE);
}

uint16_t gtim1_ms = 0;
void GTIM1_CallBack(void)
{
	gtim1_ms++;

	if(gtim1_ms > time_ms)
	{
		gtim1_ms = 0;
		cad_tx_timeout_flag |= MAC_EVT_TX_CAD_TIMEOUT;
		GTIM_Cmd(CW_GTIM1,DISABLE);;
	}
}

extern uint16_t led3_value1;
extern uint16_t led3_value2;
extern uint16_t led4_value1;
extern uint16_t led4_value2;
void GTIM1_IRQHandler(void)
{
    /* USER CODE BEGIN */
    // 中断每500us进入一次，每50ms改变一次PosWidth
    static uint16_t TimeCnt = 0;
    GTIM_ClearITPendingBit(CW_GTIM1, GTIM_IT_OV);
    if (TimeCnt++ >= 50)    // 50ms
    {
        TimeCnt = 0;
        if (Dir)
        {
            PosWidth += led3_value1;    
												PosWidth2+= led4_value1;
        }
        else
        {
            PosWidth -= led3_value2;
									   PosWidth2-= led4_value2;
        }
        if (PosWidth >= CW_GTIM1->ARR)
        {
            Dir = 0;
        }
        if (0 == PosWidth)
        {
            Dir = 1;
        }
        GTIM_SetCompare1(CW_GTIM1, PosWidth);
							 GTIM_SetCompare3(CW_GTIM2, PosWidth2);	
    }
    /* USER CODE END */
}

void ADC_Config(void)
{
      //打开ADC时钟
    REGBITS_SET( CW_SYSCTRL->APBEN2, SYSCTRL_APBEN2_ADC_Msk );
   
    PB02_ANALOG_ENABLE();

    ADC_InitTypeDef ADC_InitStructure;
	  ADC_WdtTypeDef ADC_WdtStructure;
	  ADC_SingleChTypeDef ADC_SingleChStructure;
	  //ADC默认值初始化
	  ADC_StructInit(&ADC_InitStructure);
	  //ADC模拟看门狗通道初始化
	  ADC_WdtInit(&ADC_WdtStructure);

	  ADC_InitStructure.ADC_OpMode=ADC_SingleChOneMode;
	  ADC_InitStructure.ADC_ClkDiv = ADC_Clk_Div16;    //ADCCLK:500KHz.
	  ADC_InitStructure.ADC_InBufEn = ADC_BufEnable;
	  ADC_InitStructure.ADC_SampleTime = ADC_SampTime10Clk;

	  //配置单通道转换模式
	  ADC_SingleChStructure.ADC_DiscardEn = ADC_DiscardNull;
	  ADC_SingleChStructure.ADC_Chmux = ADC_ExInputCH10; //选择ADC转换通道
	  ADC_SingleChStructure.ADC_InitStruct = ADC_InitStructure;
	  ADC_SingleChStructure.ADC_WdtStruct = ADC_WdtStructure;

	  ADC_SingleChOneModeCfg(&ADC_SingleChStructure);
	  REGBITS_SET( CW_ADC->START_f.START, ADC_START_START_Msk );
	  ADC_ITConfig(ADC_IT_EOC, ENABLE);
    ADC_EnableIrq(ADC_INT_PRIORITY);
	  ADC_ClearITPendingAll();
	  //ADC_ClearITPendingAll();
 
	  //ADC使能
	  ADC_Enable();
}


void BSP_LED_Toggle(void)
{
	  PB10_TOG();
}


