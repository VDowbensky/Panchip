/**************************************************************************//**
 * @file     uart_fifo_and_line_status_interrupt_test.c
 * @version  V1.0
 * $Date:    19/10/09 15:30 $
 * @brief    UART test case 5, test UART line status interrupt.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"

static void UART_PrintInfoCase5(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    < Baudrate=115200, Parity=ODD, DataBits=8, StopBits=1 >      | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Prepare to receive data.                        | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void MultiDataReceiveTest(void)
{
    // Prepare to print buffer data out once detected first data entry in Rx FIFO
    while(!uartRxIntFlag);
    SYS_TEST("Begin to receive data...\n");
    SYS_delay_10nop(100000);

    // Print out all received data
    SYS_TEST("Data received (length=%d):", uartRxBufIdx);
    for (size_t i = 0; i < uartRxBufIdx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", uartRxBuffer[i]);
    }
    SYS_TEST("\n");
    uartRxBufIdx = 0;   //Reset Rx buf index
}

T_UART_TEST_RESULT UART_FifoAndLineStatusInterruptTestCase5(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_LineCtrl = Uart_Line_8o1;
    Init_Struct.UART_BaudRate = 115200;
    UART_Init(TGT_UART, &Init_Struct);
    UART_EnableFifo(TGT_UART);

    while(1)
    {
		uartRxIntFlag = false;
        UART_PrintInfoCase5();
        switch(c = getchar())
        {
        /* A. Trigger Rx: FIFO 1/2 full */
        case 'A':
        case 'a':
            SYS_TEST("Prepare to receive data...\n\n");
            UART_SetRxTrigger(TGT_UART, UART_RX_FIFO_HALF_FULL);
            UART_EnableIrq(TGT_UART, UART_IRQ_RECV_DATA_AVL);   //Enable RDA Interrupt
            UART_EnableIrq(TGT_UART, UART_IRQ_LINE_STATUS);    //Enable RLS Interrupt
            NVIC_EnableIRQ(TGT_UART_IRQn);              //Enable target UART INT in NVIC
            MultiDataReceiveTest();
            NVIC_DisableIRQ(TGT_UART_IRQn);
            UART_DisableIrq(TGT_UART, UART_IRQ_RECV_DATA_AVL);
            UART_DisableIrq(TGT_UART, UART_IRQ_LINE_STATUS);
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return UART_TST_OK;
}
