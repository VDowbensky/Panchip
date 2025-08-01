#include "myUart3.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_it.h" 
#include "misc.h"
#include <stdarg.h>
#include <string.h>

static uint16_t USART_RX_STA;       //接收状态标记    
static UART_CALLBACK uartRxCallBack;
static uint8_t USART3_RX_BUF[USART_REC_LEN];
static irqCallback_ts myIrqCallback_uart3;

/**
 * @brief uart3中断函数
 *  通过API@USART1_callbackRegiste注册到@stm32f10x_it.c中的真正的中断函数
 * 
 * @param status 串口接收状态，
 *                  =0，接收超时完成
 *                  =1，接收到一个字节数据
 * @param param 
 */
void uart3_callback(uint8_t status, uint32_t param)
{
    uint8_t Res;

    switch (status)
    {
    case 0:
    {
        uartRxCallBack(USART3_RX_BUF, USART_RX_STA);
        memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));
        USART_RX_STA=0;
    }
        break;
    case 1:
    {
        //读取接收到的数据
        USART3_RX_BUF[USART_RX_STA] = param;
        USART_RX_STA++;
        if(USART_RX_STA > (USART_REC_LEN - 1))
        {
            USART_RX_STA=0;//接收数据错误,重新开始接收    
        }
    }
        break;
    
    default:
        break;
    }
}

void myUart3_init(uint32_t baudrate, UART_CALLBACK cb)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    myIrqCallback_uart3.thisCb = uart3_callback;
    USART3_callbackRegiste(&myIrqCallback_uart3);  

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  
    //USART3_TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

    //USART3_RX初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

    //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;      //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = baudrate;//串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式

    USART_Init(USART3, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启串口接受中断
    // USART_ITConfig(USART3, USART_IT_TC, ENABLE);// 使能串口发送完成中断
    uartRxCallBack = cb;
    USART_Cmd(USART3, ENABLE);                    //使能串口

}

static void myUart3_sendByte(uint8_t sendByte)
{
    while(!USART_GetFlagStatus(USART3, USART_FLAG_TC))
    {
        ;
    }
    USART_SendData(USART3, sendByte);
}
static void myUart3_sendArray(uint8_t *buffer, uint16_t sendLen)
{
	int i = 0;
    for (i = 0; i < sendLen; i++)
    {
        myUart3_sendByte(*buffer);
        buffer ++;
    }
    
}
void myUart3_printf(uint16_t sendLen, char *fmt, ...)
{
    va_list va;
    char tempTab[256];
    int len;
    if (sendLen)
    {
        myUart3_sendArray((uint8_t *)fmt, sendLen);
    }
    else
    {
        va_start(va, fmt);
        memset(tempTab, 0, sizeof(tempTab));
        vsnprintf(tempTab, (int)sizeof(tempTab), fmt, va);
        len = strlen(tempTab);
        myUart3_sendArray((uint8_t *)tempTab, len);
        va_end(va);
    }
}

