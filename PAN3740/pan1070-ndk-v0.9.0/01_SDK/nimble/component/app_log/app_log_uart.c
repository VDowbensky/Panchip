/**
 *******************************************************************************
 * @file     log_uart.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app_log_uart.h"
#include "PanSeries.h"

#if PAN_LOG_ENABLE && CONFIG_UART_LOG_ENABLE
void debug_uart_init(void)
{
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = CONFIG_LOG_UART_BAUDRATE,
        .UART_LineCtrl = Uart_Line_8n1,
    };

#if (CONFIG_LOG_UART_PIN == 0)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P0, 5, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_PIN == 1)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P1, 1, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_PIN == 2)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P1, 6, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_PIN == 3)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P0, 1, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 4)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P1, 0, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 5)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P1, 2, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 6)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P2, 5, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 7)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P3, 1, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
}
#elif PAN_LOG_ENABLE && CONFIG_RTT_LOG_ENABLE
// No nothing here
#else
void SendChar(int ch)
{
}
#endif
