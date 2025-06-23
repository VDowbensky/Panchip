/**************************************************************************//**
 * @file     uart_parity_check_test.c
 * @version  V1.0
 * $Date:    19/09/29 10:30 $
 * @brief    UART test case 2, test parity check patterns include the following
 *           4 ways: EVEN, ODD, MARK (ParityBit=1), and SPACE (ParityBit=0).
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"


#define RECV_DATA_COUNT     8

static uint32_t RecvBuffer[RECV_DATA_COUNT][2] = {0, 0};    /* Data, Line Status */

static const uint8_t Const8BitData[] = {0x00, 0x01, 0x02, 0x03, 0xCC, 0xDD, 0xEE, 0xFF};

static void UART_PrintInfoCase2(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function (Baudrate = 115200):     | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    8-bit data with ODD   parity check.             | \n");
    SYS_TEST("|    Input 'B'    8-bit data with EVEN  parity check.             | \n");
    SYS_TEST("|    Input 'C'    8-bit data with MARK  parity check.             | \n");
    SYS_TEST("|    Input 'D'    8-bit data with SPACE parity check.             | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}


void ReceiveMultiDataWithLineStatus(uint32_t recv_buffer[][2], size_t expect_data_count)
{
    uint32_t status;
    size_t i = 0;

    TGT_ClrAllReadyData();

    while (i < expect_data_count)
    {
        recv_buffer[i][0] = TGT_GetChar(&status);
        recv_buffer[i][1] = status;
        i++;
    }
}

static void MultiDataSendReceiveTest(const uint8_t* data_to_send, size_t send_size,
                                     uint32_t recv_buffer[][2], uint8_t size_to_recv)
{
    char str[80];

    SYS_TEST("\nSend data:\n\t%s\n", Hexlify(data_to_send, send_size, str));

    if (data_to_send != NULL)
        TGT_SendMultiData(data_to_send, send_size);

    SYS_TEST("Data sent successfully.\n");

    SYS_TEST("\nTry to receive %d bytes of data...\n", size_to_recv);

    ReceiveMultiDataWithLineStatus(recv_buffer, size_to_recv);

    for (size_t i = 0; i < size_to_recv; i++)
    {
        SYS_TEST("Data: 0x%02x, Error Flag: %s\n", recv_buffer[i][0], ConvLsrErrFlagToStr(recv_buffer[i][1], str, sizeof(str)));
    }
}

T_UART_TEST_RESULT UART_ParityCheckTestCase2(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_BaudRate = 115200;

    while(1)
    {
        UART_PrintInfoCase2();
        switch(c = getchar())
        {
        /* A. 8-bit data with ODD parity check. */
        case 'A':
        case 'a':
            Init_Struct.UART_LineCtrl = Uart_Line_8o1;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), RecvBuffer, RECV_DATA_COUNT);
            break;
        /* B. 8-bit data with EVEN parity check. */
        case 'B':
        case 'b':
            Init_Struct.UART_LineCtrl = Uart_Line_8e1;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), RecvBuffer, RECV_DATA_COUNT);
            break;
        /* C. 8-bit data with MARK parity check. */
        case 'C':
        case 'c':
            Init_Struct.UART_LineCtrl = Uart_Line_8m1;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), RecvBuffer, RECV_DATA_COUNT);
            break;
        /* D. 8-bit data with SPACE parity check. */
        case 'D':
        case 'd':
            Init_Struct.UART_LineCtrl = Uart_Line_8s1;
            UART_Init(TGT_UART, &Init_Struct);
            MultiDataSendReceiveTest(Const8BitData, sizeof(Const8BitData), RecvBuffer, RECV_DATA_COUNT);
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
