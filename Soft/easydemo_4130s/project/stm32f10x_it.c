/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h" 

volatile uint8_t rfIntRequest = 0;
irqCallback_ts *irqCallback_tim1;
irqCallback_ts *firstIrqCallback_tim1;
irqCallback_ts *irqCallback_tim2cc2;
irqCallback_ts *firstIrqCallback_tim2cc2;
irqCallback_ts *irqCallback_tim2cc3;
irqCallback_ts *firstIrqCallback_tim2cc3;
irqCallback_ts *irqCallback_tim3cc4;
irqCallback_ts *firstIrqCallback_tim3cc4;
irqCallback_ts *irqCallback_tim3;
irqCallback_ts *firstIrqCallback_tim3;
irqCallback_ts *irqCallback_uart1;
irqCallback_ts *firstIrqCallback_uart1;
irqCallback_ts *irqCallback_uart3;
irqCallback_ts *firstIrqCallback_uart3;
irqCallback_ts *irqCallback_extiLine1;
irqCallback_ts *firstIrqCallback_extiLine1;

void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

// void EXTI3_IRQHandler(void)
// {

// 	//rfIntRequest=1;
// 	EXTI_ClearITPendingBit(EXTI_Line3);  //清除EXTI0线路挂起位
// }
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

void EXTILINE1_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_extiLine1 == 0)
    {
        irqCallback_extiLine1 = cbStruct;
        firstIrqCallback_extiLine1 = irqCallback_extiLine1;
    }
    else
    {
        irqCallback_extiLine1->nextStruct = cbStruct;
        irqCallback_extiLine1 = cbStruct;
    }
}
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        /* Clear the EXTI line 1 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line1);
        irqCallback_ts *irqCallbackTemp;
        irqCallbackTemp = firstIrqCallback_extiLine1;
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, 0);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
}
void USART1_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_uart1 == 0)
    {
        irqCallback_uart1 = cbStruct;
        firstIrqCallback_uart1 = irqCallback_uart1;
    }
    else
    {
        irqCallback_uart1->nextStruct = cbStruct;
        irqCallback_uart1 = cbStruct;
    }
}
void USART1_IRQHandler(void)                    //串口1中断服务程序
{
    uint16_t recvData;
    irqCallback_ts *irqCallbackTemp;
    irqCallbackTemp = firstIrqCallback_uart1;
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        recvData = USART_ReceiveData(USART1);
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, recvData);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //
    {
        recvData = USART_ReceiveData(USART1);
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(1, recvData);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }            
} 
void USART3_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_uart3 == 0)
    {
        irqCallback_uart3 = cbStruct;
        firstIrqCallback_uart3 = irqCallback_uart3;
    }
    else
    {
        irqCallback_uart3->nextStruct = cbStruct;
        irqCallback_uart3 = cbStruct;
    }
}
void USART3_IRQHandler(void)                    //串口1中断服务程序
{
    uint16_t recvData;
    irqCallback_ts *irqCallbackTemp;
    irqCallbackTemp = firstIrqCallback_uart3;
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {
        recvData = USART_ReceiveData(USART3);
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, recvData);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        recvData = USART_ReceiveData(USART3);
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(1, recvData);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }            
}
void TIM1_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_tim1 == 0)
    {
        irqCallback_tim1 = cbStruct;
        firstIrqCallback_tim1 = irqCallback_tim1;
    }
    else
    {
        irqCallback_tim1->nextStruct = cbStruct;
        irqCallback_tim1 = cbStruct;
    }
}

void TIM1_UP_IRQHandler(void)   //TIM1中断
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {   
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update  );  //清除TIMx更新中断标志 
        irqCallback_ts *irqCallbackTemp;
        irqCallbackTemp = firstIrqCallback_tim1;
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, 0);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
}
void TIM2CC2_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_tim2cc2 == 0)
    {
        irqCallback_tim2cc2 = cbStruct;
        firstIrqCallback_tim2cc2 = irqCallback_tim2cc2;
    }
    else
    {
        irqCallback_tim2cc2->nextStruct = cbStruct;
        irqCallback_tim2cc2 = cbStruct;
    }
}
void TIM2CC3_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_tim2cc3 == 0)
    {
        irqCallback_tim2cc3 = cbStruct;
        firstIrqCallback_tim2cc3 = irqCallback_tim2cc3;
    }
    else
    {
        irqCallback_tim2cc3->nextStruct = cbStruct;
        irqCallback_tim2cc3 = cbStruct;
    }
}
void TIM2_IRQHandler(void)
{ 
    if(TIM_GetITStatus(TIM2, TIM_IT_CC3) == SET) 
    {
        /* Clear TIM2 Capture compare interrupt pending bit */
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);
        irqCallback_ts *irqCallbackTemp;
        irqCallbackTemp = firstIrqCallback_tim2cc3;
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, 0);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
    if(TIM_GetITStatus(TIM2, TIM_IT_CC2) == SET) 
    {
        /* Clear TIM2 Capture compare interrupt pending bit */
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        irqCallback_ts *irqCallbackTemp;
        irqCallbackTemp = firstIrqCallback_tim2cc2;
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, 0);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
}
void TIM3_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_tim3 == 0)
    {
        irqCallback_tim3 = cbStruct;
        firstIrqCallback_tim3 = irqCallback_tim3;
    }
    else
    {
        irqCallback_tim3->nextStruct = cbStruct;
        irqCallback_tim3 = cbStruct;
    }
}
void TIM3CC4_callbackRegiste(irqCallback_ts *cbStruct)
{
    if (irqCallback_tim3cc4 == 0)
    {
        irqCallback_tim3cc4 = cbStruct;
        firstIrqCallback_tim3cc4 = irqCallback_tim3cc4;
    }
    else
    {
        irqCallback_tim3cc4->nextStruct = cbStruct;
        irqCallback_tim3cc4 = cbStruct;
    }
}
void TIM3_IRQHandler(void)   //TIM3中断
{
    if(TIM_GetITStatus(TIM3, TIM_IT_CC4) == SET) 
    {
        /* Clear TIM3 Capture compare interrupt pending bit */
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
        irqCallback_ts *irqCallbackTemp;
        irqCallbackTemp = firstIrqCallback_tim3cc4;
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, 0);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {    
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
        irqCallback_ts *irqCallbackTemp;
        irqCallbackTemp = firstIrqCallback_tim3;
        while (irqCallbackTemp)
        {
            irqCallbackTemp->thisCb(0, 0);
            irqCallbackTemp = irqCallbackTemp->nextStruct;
        }
    }
}
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
