/**************************************************************************//**
 * @file     uart_data_len_select_and_stop_bit_test.c
 * @version  V1.0
 * $Date:    19/09/26 14:30 $
 * @brief    UART test case 1, test different data length config (LCR[1:0]) with
 *           corresponding stop bit (LCR[2]) being configured to 1.5 or 2 bits.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"


static uint8_t TestDataBuf[8];

static const uint8_t TestValid5BitData[] = {0x00, 0x01, 0x02, 0x03, 0x1C, 0x1D, 0x1E, 0x1F};
static const uint8_t TestInvalid5BitData[] = {0x20, 0x21, 0x22, 0x23, 0xCC, 0xDD, 0xEE, 0xFF};

static const uint8_t TestValid6BitData[] = {0x00, 0x01, 0x02, 0x03, 0x3C, 0x3D, 0x3E, 0x3F};
static const uint8_t TestInvalid6BitData[] = {0x40, 0x41, 0x42, 0x43, 0xCC, 0xDD, 0xEE, 0xFF};

static const uint8_t TestValid7BitData[] = {0x00, 0x01, 0x40, 0x44, 0x68, 0x6C, 0x7E, 0x7F};
static const uint8_t TestInvalid7BitData[] = {0x80, 0x81, 0xAA, 0xBB, 0xCC, 0xDD, 0xFE, 0xFF};

static const uint8_t TestValid8BitData[] = {0x00, 0x01, 0x02, 0x03, 0xCC, 0xDD, 0xEE, 0xFF};

static void UART_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function (Baudrate = 115200):     | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    5-bit data with 1.5 stop bits.                  | \n");
    SYS_TEST("|    Input 'B'    6-bit data with 2   stop bits.                  | \n");
    SYS_TEST("|    Input 'C'    7-bit data with 2   stop bits.                  | \n");
    SYS_TEST("|    Input 'D'    8-bit data with 2   stop bits.                  | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void MultiDataSendReceiveTest(const uint8_t* valid_data_to_send, size_t valid_data_size,
                                     const uint8_t* invalid_data_to_send, size_t invalid_data_size,
                                     uint8_t* buf_to_recv, uint8_t size_to_recv)
{
    char hexstr[50];

    SYS_TEST("\nSend data:\n\t%s (should be valid)", Hexlify(valid_data_to_send, valid_data_size, hexstr));
    if (invalid_data_to_send != NULL)
        SYS_TEST("\n\t%s (should be invalid)\n", Hexlify(invalid_data_to_send, invalid_data_size, hexstr));
    else
        SYS_TEST("\n");

    if (valid_data_to_send != NULL)
        TGT_SendMultiData(valid_data_to_send, valid_data_size);
        SYS_delay_10nop(1000000);   //Delay to make PC UartAssist tool display data in separate two lines
    if (invalid_data_to_send != NULL)
        TGT_SendMultiData(invalid_data_to_send, invalid_data_size);

    SYS_TEST("Data sent successfully.\n");

    SYS_TEST("\nTry to receive %d bytes of data...\n", size_to_recv);

    if (buf_to_recv != NULL)
        TGT_ReceiveMultiData(buf_to_recv, size_to_recv);

    SYS_TEST("Data received: %s\n", Hexlify(buf_to_recv, size_to_recv, hexstr));
}

T_UART_TEST_RESULT UART_DataLenSelectAndStopBitTestCase1(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_BaudRate = 115200;

    while(1)
    {
        UART_PrintInfoCase1();
        switch(c = getchar())
        {
        /* A. 5-bit data with 1.5 stop bits. */
        case 'A':
        case 'a':
            Init_Struct.UART_LineCtrl = Uart_Line_5n1_5;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(TestValid5BitData, sizeof(TestValid5BitData), TestInvalid5BitData, sizeof(TestInvalid5BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* B. 6-bit data with 2 stop bits. */
        case 'B':
        case 'b':
            Init_Struct.UART_LineCtrl = Uart_Line_6n2;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(TestValid6BitData, sizeof(TestValid6BitData), TestInvalid6BitData, sizeof(TestInvalid6BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* C. 7-bit data with 2 stop bits. */
        case 'C':
        case 'c':
            Init_Struct.UART_LineCtrl = Uart_Line_7n2;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(TestValid7BitData, sizeof(TestValid7BitData), TestInvalid7BitData, sizeof(TestInvalid7BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* D. 8-bit data with 2 stop bits. */
        case 'D':
        case 'd':
            Init_Struct.UART_LineCtrl = Uart_Line_8n2;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(TestValid8BitData, sizeof(TestValid8BitData), NULL, 0, TestDataBuf, sizeof(TestDataBuf));
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
