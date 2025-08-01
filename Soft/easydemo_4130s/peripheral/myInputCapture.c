#include "myInputCapture.h"
#include "board.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_it.h" 

uint16_t captureStartValue = 0, captureEndValue = 0;
uint16_t CaptureNumber = 0;
uint32_t Capture = 0;
uint32_t capturePLuseFrq = 0;
CaptureCallback inputCaptureCb;

irqCallback_ts irqCallback_tim2ch2;
irqCallback_ts irqCallback_tim2ch3;
irqCallback_ts irqCallback_tim3ch4;

void tim2ch2_callback(uint8_t status, uint32_t param)
{
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      captureStartValue = TIM_GetCapture2(TIM2);
      CaptureNumber = 1;
    }
    else if(CaptureNumber == 1)
    {
      /* Get the Input Capture value */
      captureEndValue = TIM_GetCapture2(TIM2); 
      
      /* Capture computation */
      if (captureEndValue > captureStartValue)
      {
        Capture = (captureEndValue - captureStartValue); 
      }
      else
      {
        Capture = ((0xFFFF - captureStartValue) + captureEndValue); 
      }
      /* Frequency computation */ 
      capturePLuseFrq = (uint32_t) SystemCoreClock / Capture;
      CaptureNumber = 0;

      inputCaptureCb(captureStartValue, captureEndValue, capturePLuseFrq);
    }
}
/**
 * @brief tim2的通道2捕捉功能初始化
 *      定时器的分频数为@TIM_ICPSC_DIV1时，捕捉的最小频率的脉冲为1098Hz
 * 
 * @param cb 将捕捉的数据反馈回应用层
 */
void myInputCaptureTIM2_CH2_init(CaptureCallback cb)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;

    irqCallback_tim2ch2.thisCb = tim2ch2_callback;
    TIM2CC2_callbackRegiste(&irqCallback_tim2ch2);
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    /* Enable the TIM3 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    
    /* TIM enable counter */
    TIM_Cmd(TIM2, ENABLE);

    /* Enable the CC2 Interrupt Request */
    TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);

    inputCaptureCb = cb;

}
/**
 * @brief tim2的通道3中断函数
 *      
 * 
 * @param status 
 * @param param 
 */
void tim2ch3_callback(uint8_t status, uint32_t param)
{
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      captureStartValue = TIM_GetCapture3(TIM2);
      CaptureNumber = 1;
    }
    else if(CaptureNumber == 1)
    {
      /* Get the Input Capture value */
      captureEndValue = TIM_GetCapture3(TIM2); 
      
      /* Capture computation */
      if (captureEndValue > captureStartValue)
      {
        Capture = (captureEndValue - captureStartValue); 
      }
      else
      {
        Capture = ((0xFFFF - captureStartValue) + captureEndValue); 
      }
      /* Frequency computation */ 
      capturePLuseFrq = (uint32_t) SystemCoreClock / Capture;
      CaptureNumber = 0;
    
      inputCaptureCb(captureStartValue, captureEndValue, capturePLuseFrq);
    }
}
/**
 * @brief 初始化定时器2的通道3的输入脉冲捕捉功能
 *       定时器的分频数为@TIM_ICPSC_DIV1时，捕捉的最小频率的脉冲为1098Hz
 *      映射到PA2脚，对应转接板的BOARD_GPIO_SPI_GPIOD
 *      捕捉两个边沿之间的频率
 * @param cb 将捕捉的数据反馈回应用层
 */
void myInputCaptureTIM2_CH3_init(CaptureCallback cb)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    irqCallback_tim2ch3.thisCb = tim2ch3_callback;
    TIM2CC3_callbackRegiste(&irqCallback_tim2ch3);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    /* Enable the TIM3 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    
    /* TIM enable counter */
    TIM_Cmd(TIM2, ENABLE);

    /* Enable the CC2 Interrupt Request */
    TIM_ITConfig(TIM2, TIM_IT_CC3, ENABLE);

    inputCaptureCb = cb;

}

/**
 * @brief tim3的通道4中断函数
 *      
 * 
 * @param status 
 * @param param 
 */
void tim3ch4_callback(uint8_t status, uint32_t param)
{ 
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      captureStartValue = TIM_GetCapture4(TIM3);
      CaptureNumber = 1;
    }
    else if(CaptureNumber == 1)
    {
      /* Get the Input Capture value */
      captureEndValue = TIM_GetCapture4(TIM3); 
      
      /* Capture computation */
      if (captureEndValue > captureStartValue)
      {
        Capture = (captureEndValue - captureStartValue); 
      }
      else
      {
        Capture = ((0xFFFF - captureStartValue) + captureEndValue); 
      }
      /* Frequency computation */ 
      capturePLuseFrq = (uint32_t) SystemCoreClock / Capture;
      CaptureNumber = 0;

      inputCaptureCb(captureStartValue, captureEndValue, capturePLuseFrq);
    }
}
/**
 * @brief 初始化定时器3的通道4的输入脉冲捕捉功能
 *       定时器的分频数为@TIM_ICPSC_DIV1时，捕捉的最小频率的脉冲为1098Hz
 *      映射到PB1脚，对应转接板的BOARD_GPIO_SPI_GPIOC
 *      捕捉两个边沿之间的频率
 * @param cb 将捕捉的数据反馈回应用层
 */
void myInputCaptureTIM3_CH4_init(CaptureCallback cb)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    irqCallback_tim3ch4.thisCb = tim3ch4_callback;
    TIM3CC4_callbackRegiste(&irqCallback_tim3ch4);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    /* Enable the TIM3 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    
    /* TIM enable counter */
    TIM_Cmd(TIM3, ENABLE);

    /* Enable the CC2 Interrupt Request */
    TIM_ITConfig(TIM3, TIM_IT_CC4, ENABLE);

    inputCaptureCb = cb;

}

