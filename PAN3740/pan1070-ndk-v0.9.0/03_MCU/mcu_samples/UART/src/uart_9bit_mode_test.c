/**************************************************************************//**
 * @file     uart_9bit_mode_test.c
 * @version  V1.0
 * $Date:    19/10/16 19:00 $
 * @brief    UART test case 8, test UART 9-bit mode.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"

#define NINE_BIT_MODE_ADDR1     0xB4
#define NINE_BIT_MODE_ADDR2     0x87

static const uint8_t Const8BitData[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static const uint8_t Const8BitData2[] =
{
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
};

static void UART_PrintInfoCase8(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    < Baudrate=115200, Parity=NONE, DataBits=9, StopBits=1 >     | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Work as receiving end.                          | \n");
    SYS_TEST("|    Input 'B'    Work as transmitting end.                       | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

// Try to receive multi-data in 9-bit mode until getting expected number of bytes
static bool TGT_ReceiveMultiData9Bit(uint8_t* data, size_t expect_size, uint8_t recv_addr, volatile bool* addr_rcvd_flag)
{
    size_t data_cnt = 0;

    TGT_ClrAllReadyData();
    while (1)
    {
        while (UART_IsRxFifoEmpty(TGT_UART));
        if (*addr_rcvd_flag == true)
        {
            if (UART_ReceiveData(TGT_UART) == recv_addr)
            {
                *addr_rcvd_flag = false;
            }
            else
            {
                *addr_rcvd_flag = false;
                return false;
            }
        }
        else
        {
            data[data_cnt] = UART_ReceiveData(TGT_UART);
            if (++data_cnt >= expect_size)
            {
                return true;
            }
        }
    }
}

static void MultiDataReceive9BitMode(void)
{
    uint8_t buf_to_recv[256] = {0,};
    size_t recv_size = sizeof(Const8BitData);

    // Set receive address
    UART_SetReceiveAddress(TGT_UART, NINE_BIT_MODE_ADDR1);
    // Set address match mode
    UART_EnableAddrMatchMode(TGT_UART);

    // Enable Interrupts
    UART_EnableIrq(TGT_UART, UART_IRQ_LINE_STATUS);    //Enable Recv Line Status Interrupt
    NVIC_EnableIRQ(TGT_UART_IRQn);              //Enable target UART INT in NVIC

    SYS_TEST("Prepare to receive data (Addr: 0x%x)...\n", NINE_BIT_MODE_ADDR1);

    // Try to receive data until expected size get
    if (TGT_ReceiveMultiData9Bit(buf_to_recv, recv_size, NINE_BIT_MODE_ADDR1, &uartAddrRcvd) == false)
    {
        SYS_TEST("Receive 9-bit data error!\n");
        goto OUT;
    }

    // Print out all received data
    SYS_TEST("Data received done (length=%d):", recv_size);
    for (size_t i = 0; i < recv_size; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", buf_to_recv[i]);
    }
    SYS_TEST("\n");
OUT:
    // Disable interrupts after use
    NVIC_DisableIRQ(TGT_UART_IRQn);
    UART_DisableIrq(TGT_UART, UART_IRQ_LINE_STATUS);
}

static void MultiDataSend9BitMode(void)
{
    size_t send_size1 = sizeof(Const8BitData) / 2;
    size_t send_size2 = sizeof(Const8BitData2);

    /*----------- 1. Transmit the first half part of data1 sequence ------------*/
    // Set transmit address
    UART_SetTransmitAddress(TGT_UART, NINE_BIT_MODE_ADDR1);

    SYS_TEST("Begin to transmit address1 (0x%x) ...\n", NINE_BIT_MODE_ADDR1);
    // Send transmit address
    UART_SendAddr(TGT_UART);

    SYS_TEST("Prepare to transmit the first half of data1 (%d Bytes)...\n", send_size1);
    // Send data
    TGT_SendMultiData(Const8BitData, send_size1);
    SYS_TEST("Transmitting done.\n");

    /*----------- 2. Transmit data2 sequence -----------------------------------*/
    // Set transmit address
    UART_SetTransmitAddress(TGT_UART, NINE_BIT_MODE_ADDR2);

    SYS_TEST("Begin to transmit address2 (0x%x) ...\n", NINE_BIT_MODE_ADDR2);
    // Send transmit address
    UART_SendAddr(TGT_UART);

    SYS_TEST("Prepare to transmit data2 (%d Bytes)...\n", send_size2);
    // Send data
    TGT_SendMultiData(Const8BitData2, send_size2);
    SYS_TEST("Transmitting done.\n");

    /*----------- 3. Transmit the second half part of data1 sequence -----------*/
    // Set transmit address
    UART_SetTransmitAddress(TGT_UART, NINE_BIT_MODE_ADDR1);

    SYS_TEST("Begin to transmit address1 (0x%x) ...\n", NINE_BIT_MODE_ADDR1);
    // Send transmit address
    UART_SendAddr(TGT_UART);

    SYS_TEST("Prepare to transmit the second half of data1 (%d Bytes)...\n", send_size1);
    // Send data
    TGT_SendMultiData(Const8BitData + send_size1, send_size1);
    SYS_TEST("Transmitting done.\n");
}

T_UART_TEST_RESULT UART_9BitModeTestCase8(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;
    Init_Struct.UART_BaudRate = 115200;
    UART_Init(TGT_UART, &Init_Struct);
    UART_EnableFifo(TGT_UART);

    while(1)
    {
        UART_PrintInfoCase8();
        switch(c = getchar())
        {
        /* A. Work as receiving end */
        case 'A':
        case 'a':
            UART_Enable9BitData(TGT_UART);  //Enable 9bit mode
            MultiDataReceive9BitMode();
            UART_Disable9BitData(TGT_UART); //Disable 9bit mode
            break;
        /* B. Work as transmitting end */
        case 'B':
        case 'b':
            UART_Enable9BitData(TGT_UART);  //Enable 9bit mode
            MultiDataSend9BitMode();
            UART_Disable9BitData(TGT_UART); //Disable 9bit mode
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
