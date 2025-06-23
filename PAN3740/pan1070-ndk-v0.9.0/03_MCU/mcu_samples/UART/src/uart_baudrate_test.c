/**************************************************************************//**
 * @file     uart_baudrate_test.c
 * @version  V1.0
 * $Date:    19/09/26 17:30 $
 * @brief    UART test case 3, test send/recv data under different baudrate.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"


static uint8_t TestDataBuf[8];

static const uint8_t Const8BitData[] = {0x00, 0x01, 0x02, 0x03, 0xCC, 0xDD, 0xEE, 0xFF};

static void UART_PrintInfoCase3(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    8-bit data under 9600       baudrate.           | \n");
    SYS_TEST("|    Input 'B'    8-bit data under 115200     baudrate.           | \n");
    SYS_TEST("|    Input 'C'    8-bit data under 345600     baudrate.           | \n");
    SYS_TEST("|    Input 'D'    8-bit data under 1M         baudrate.           | \n");
    SYS_TEST("|    Input 'E'    8-bit data under 2M         baudrate.           | \n");
    SYS_TEST("|    Input 'F'    8-bit data under customized baudrate.           | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void MultiDataSendReceiveTest(const uint8_t* data_to_send, size_t send_size,
                                     uint8_t* buf_to_recv, uint8_t size_to_recv)
{
    char hexstr[50];

    SYS_TEST("\nSend data:\n\t%s\n", Hexlify(data_to_send, send_size, hexstr));

    if (data_to_send != NULL)
        TGT_SendMultiData(data_to_send, send_size);

    SYS_TEST("Data sent successfully.\n");

    SYS_TEST("\nTry to receive %d bytes of data...\n", size_to_recv);

    if (buf_to_recv != NULL)
        TGT_ReceiveMultiData(buf_to_recv, size_to_recv);

    SYS_TEST("Data received: %s\n", Hexlify(buf_to_recv, size_to_recv, hexstr));
}

uint32_t GetInputNumberFromDebugPort(void)
{
    char in_str[10];
    size_t i = 0;

    while((in_str[i] = getchar()) != '\n')
    {
        if (i >= sizeof(in_str) - 1)    //Truncated if input string is too long
        {
            break;
        }
        i++;
    }
    in_str[i] = '\0';

    return atoi(in_str);
}

T_UART_TEST_RESULT UART_BaudrateTestCase3(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;

    while(1)
    {
        UART_PrintInfoCase3();
        switch(c = getchar())
        {
        /* A. 8-bit data under 9600 baudrate. */
        case 'A':
        case 'a':
            Init_Struct.UART_BaudRate = 9600;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* B. 8-bit data under 115200 baudrate. */
        case 'B':
        case 'b':
            Init_Struct.UART_BaudRate = 115200;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* C. 8-bit data under 345600 baudrate. */
        case 'C':
        case 'c':
            Init_Struct.UART_BaudRate = 345600;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* D. 8-bit data under 1M baudrate. */
        case 'D':
        case 'd':
            Init_Struct.UART_BaudRate = 1000000;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* E. 8-bit data under 2M baudrate. */
        case 'E':
        case 'e':
            Init_Struct.UART_BaudRate = 2000000;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), TestDataBuf, sizeof(TestDataBuf));
            break;
        /* F. 8-bit data under customized baudrate. */
        case 'F':
        case 'f':
            SYS_TEST("\nPlease input new baudrate (Press <Enter> to confirm):\n");
            while(1)
            {
                Init_Struct.UART_BaudRate = GetInputNumberFromDebugPort();
                if (Init_Struct.UART_BaudRate == 0)
                    SYS_TEST("Invalid, baudrate is 0, please input a valid baudrate:\n");
                else if (UART_Init(TGT_UART, &Init_Struct) == false)
                    SYS_TEST("Invalid, baudrate is too large, please input a valid baudrate:\n");
                else
                    break;
            }
            SYS_TEST("Baudrate changed, new value: %d\n", Init_Struct.UART_BaudRate);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), TestDataBuf, sizeof(TestDataBuf));
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
