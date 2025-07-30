#include "myTim.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_it.h" 

TIM_CALLBACK timCallBack;

static irqCallback_ts myIrqCallback_tim1;
static irqCallback_ts myIrqCallback_tim3;

/**
 * @brief tim3中断函数
 *  通过API@TIM3_callbackRegiste注册到@stm32f10x_it.c中的真正的中断函数
 * 
 * @param status 
 * @param param 
 */
void tim3_callback(uint8_t status, uint32_t param)   //TIM3中断
{
    if(timCallBack)
    {
        timCallBack();
    }
}
/**
 * tim3定时器初始化
 * @period_us：定时器定时周期，以微秒为单位
 * @cb：定时器回调函数，定时器计时溢出时调用该函数，
        相当于向外抛出中断，方便主函数调用
*/
void myTim3_init(uint32_t period_us, TIM_CALLBACK cb)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    myIrqCallback_tim3.thisCb = tim3_callback;
    TIM3_callbackRegiste(&myIrqCallback_tim3);       

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
    //中断优先级NVIC设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
        
    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = period_us; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值  
    TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1); //系统主时钟72MHz，TIM_ClockDivision = TIM_CKD_DIV1
                                                    //此处再72分频得到1MHz的计数时钟（即1MHz = 1us周期）
                                                    //需要多少微妙的周期，只需通过TIM_Period赋值既可得到相应
                                                    //的周期定时器
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

    TIM_Cmd(TIM3, ENABLE);  //使能TIMx 
    timCallBack = cb;    

}
/**
 * @brief tim1中断函数
 *  通过API@TIM1_callbackRegiste注册到@stm32f10x_it.c中的真正的中断函数
 * 
 * @param status 
 * @param param 
 */
void tim1_callback(uint8_t status, uint32_t param)   //TIM1中断
{
    if(timCallBack)
    {
        timCallBack();
    }
}
/**
 * tim1定时器初始化
 * @period_us：定时器定时周期，以微秒为单位
 * @cb：定时器回调函数，定时器计时溢出时调用该函数，
        相当于向外抛出中断，方便主函数调用
*/
void myTim1_init(uint32_t period_us, TIM_CALLBACK cb)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    myIrqCallback_tim1.thisCb = tim1_callback;
    TIM1_callbackRegiste(&myIrqCallback_tim1);  

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //时钟使能
    //中断优先级NVIC设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
        
    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = period_us; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值  
    TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1); //系统主时钟72MHz，TIM_ClockDivision = TIM_CKD_DIV1
                                                    //此处再72分频得到1MHz的计数时钟（即1MHz = 1us周期）
                                                    //需要多少微妙的周期，只需通过TIM_Period赋值既可得到相应
                                                    //的周期定时器
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE ); //使能指定的TIM3中断,允许更新中断

    TIM_Cmd(TIM1, ENABLE);  //使能TIMx    
    timCallBack = cb;    

}

