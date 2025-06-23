/**************************************************************************//**
 * @file     uart_auto_flow_control_test.c
 * @version  V1.0
 * $Date:    19/10/09 15:30 $
 * @brief    UART test case 6, test UART auto flow control.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"

static const uint8_t Const8BitData[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
};

static void UART_PrintInfoCase6(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    < Baudrate=115200, Parity=NONE, DataBits=8, StopBits=1 >     | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Work as receiving end.                          | \n");
    SYS_TEST("|    Input 'B'    Work as transmitting end.                       | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void MultiDataReceiveTest(void)
{
    // Enable Auto Flow Control
    UART_EnableAfc(TGT_UART);
    UART_EnableRts(TGT_UART);
    // Enable Interrupts
    UART_EnableIrq(TGT_UART, UART_IRQ_RECV_DATA_AVL);   //Enable Recv Data Available Interrupt
    UART_EnableIrq(TGT_UART, UART_IRQ_LINE_STATUS);    //Enable Recv Line Status Interrupt
    UART_EnableIrq(TGT_UART, UART_IRQ_MODEM_STATUS);   //Enable Modem Status Interrupt

    SYS_TEST("Prepare to receive data...\n");
    SYS_delay_10nop(1000);
    NVIC_EnableIRQ(TGT_UART_IRQn);              //Enable target UART INT in NVIC

    // Prepare to print buffer data out once detected first data entry in Rx FIFO
    while(!uartRxIntFlag);
    SYS_TEST("Begin to receive data...\n");

    // Wait until all data receiving done
    while (uartRxBufIdx < sizeof(Const8BitData));

    // Print out all received data
    SYS_TEST("Data received done (length=%d):", uartRxBufIdx);
    for (size_t i = 0; i < uartRxBufIdx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", uartRxBuffer[i]);
    }
    SYS_TEST("\n");
    uartRxBufIdx = 0;   //Reset Rx buf index

    // Disable interrupts after use
    NVIC_DisableIRQ(TGT_UART_IRQn);
    UART_DisableIrq(TGT_UART, UART_IRQ_RECV_DATA_AVL);
    UART_DisableIrq(TGT_UART, UART_IRQ_LINE_STATUS);
    UART_DisableIrq(TGT_UART, UART_IRQ_MODEM_STATUS);
    // Disable Auto Flow Control
    UART_DisableRts(TGT_UART);
    UART_DisableAfc(TGT_UART);
}

static void MultiDataSendTest(void)
{
    // Copy data to send to tx buffer
    memcpy(uartTxBuffer, Const8BitData, sizeof(Const8BitData));
    uartTxBufIdx = sizeof(Const8BitData);

    // Enable Auto Flow Control
    UART_EnableAfc(TGT_UART);
    UART_EnableRts(TGT_UART);
    // Enable Interrupts
    UART_EnablePtime(TGT_UART);                 //Enable Programmable THRE Interrupt Mode
    UART_EnableIrq(TGT_UART, UART_IRQ_THR_EMPTY);   //Enable Transmit Holding Register Empty Interrupt
    UART_EnableIrq(TGT_UART, UART_IRQ_MODEM_STATUS);   //Enable Modem Status Interrupt

    SYS_TEST("Prepare to transmit data (%d Bytes)...\n", uartTxBufIdx);
    SYS_delay_10nop(1000);
    NVIC_EnableIRQ(TGT_UART_IRQn);              //Enable target UART INT in NVIC

    // Wait until data transmitting done
    while (!uartTxDone);

    uartTxDone = false;
    SYS_TEST("Data transmitting done, size: %d\n", uartTxBufIdx);

    // Disable interrupts after use
    NVIC_DisableIRQ(TGT_UART_IRQn);
    UART_DisablePtime(TGT_UART);
    UART_DisableIrq(TGT_UART, UART_IRQ_THR_EMPTY);
    UART_DisableIrq(TGT_UART, UART_IRQ_MODEM_STATUS);
    // Disable Auto Flow Control
    UART_DisableAfc(TGT_UART);
}

T_UART_TEST_RESULT UART_AutoFlowControlTestCase6(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;
    Init_Struct.UART_BaudRate = 115200;
    UART_Init(TGT_UART, &Init_Struct);
    UART_EnableFifo(TGT_UART);

    while(1)
    {
		uartRxIntFlag = false;
        UART_PrintInfoCase6();
        switch(c = getchar())
        {
        /* A. Work as receiving end */
        case 'A':
        case 'a':
            UART_SetRxTrigger(TGT_UART, UART_RX_FIFO_TWO_LESS_THAN_FULL);
            MultiDataReceiveTest();
            break;
        /* B. Work as transmitting end */
        case 'B':
        case 'b':
            UART_SetTxTrigger(TGT_UART, UART_TX_FIFO_HALF_FULL);
            MultiDataSendTest();
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
