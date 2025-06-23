/**************************************************************************//**
 * @file     spi_data_szie_select_test.c
 * @version  V1.0
 * $Date:    19/10/23 15:30 $
 * @brief    SPI test case 1, test different data size config (CR0[3:0]) from
 *           4-bit to 16-bit.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"


static uint16_t TestDataBuf[8];

static const uint16_t TestValid4BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x000C, 0x000D, 0x000E, 0x000F};
static const uint16_t TestInvalid4BitData[] = {0x0010, 0x0011, 0x0012, 0x0013, 0x00CC, 0x00DD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid5BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x001C, 0x001D, 0x001E, 0x001F};
static const uint16_t TestInvalid5BitData[] = {0x0020, 0x0021, 0x0022, 0x0023, 0x00CC, 0x00DD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid6BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x003C, 0x003D, 0x003E, 0x003F};
static const uint16_t TestInvalid6BitData[] = {0x0040, 0x0041, 0x0042, 0x0043, 0x00CC, 0x00DD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid7BitData[] = {0x0000, 0x0001, 0x0040, 0x0044, 0x0068, 0x006C, 0x007E, 0x007F};
static const uint16_t TestInvalid7BitData[] = {0x0080, 0x0081, 0x0082, 0x0083, 0x00CC, 0x00DD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid8BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x00CC, 0x00DD, 0x00EE, 0x00FF};
static const uint16_t TestInvalid8BitData[] = {0x0100, 0x0101, 0x0102, 0x0103, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid9BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x01CC, 0x01DD, 0x01EE, 0x01FF};
static const uint16_t TestInvalid9BitData[] = {0x0200, 0x0201, 0x0202, 0x0203, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid10BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x03CC, 0x03DD, 0x03EE, 0x03FF};
static const uint16_t TestInvalid10BitData[] = {0x0400, 0x0401, 0x0402, 0x0403, 0xCCCC, 0xEDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid11BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x07CC, 0x07DD, 0x07EE, 0x07FF};
static const uint16_t TestInvalid11BitData[] = {0x0800, 0x0801, 0x0802, 0x0803, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid12BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0FCC, 0x0FDD, 0x0FEE, 0x0FFF};
static const uint16_t TestInvalid12BitData[] = {0x1000, 0x1001, 0x1002, 0x1003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid13BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x1FCC, 0x1FDD, 0x1FEE, 0x1FFF};
static const uint16_t TestInvalid13BitData[] = {0x2000, 0x2001, 0x2002, 0x2003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid14BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x3FCC, 0x3FDD, 0x3FEE, 0x3FFF};
static const uint16_t TestInvalid14BitData[] = {0x4000, 0x4001, 0x4002, 0x4003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid15BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x7FCC, 0x7FDD, 0x7FEE, 0x7FFF};
static const uint16_t TestInvalid15BitData[] = {0x8000, 0x8001, 0x8002, 0x8003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid16BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static void SPI_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Transfer  4-bit data.                           | \n");
    SYS_TEST("|    Input 'B'    Transfer  5-bit data.                           | \n");
    SYS_TEST("|    Input 'C'    Transfer  6-bit data.                           | \n");
    SYS_TEST("|    Input 'D'    Transfer  7-bit data.                           | \n");
    SYS_TEST("|    Input 'E'    Transfer  8-bit data.                           | \n");
    SYS_TEST("|    Input 'F'    Transfer  9-bit data.                           | \n");
    SYS_TEST("|    Input 'G'    Transfer 10-bit data.                           | \n");
    SYS_TEST("|    Input 'H'    Transfer 11-bit data.                           | \n");
    SYS_TEST("|    Input 'I'    Transfer 12-bit data.                           | \n");
    SYS_TEST("|    Input 'J'    Transfer 13-bit data.                           | \n");
    SYS_TEST("|    Input 'K'    Transfer 14-bit data.                           | \n");
    SYS_TEST("|    Input 'L'    Transfer 15-bit data.                           | \n");
    SYS_TEST("|    Input 'M'    Transfer 16-bit data.                           | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void MultiDataSendReceiveSpi(const uint16_t* valid_data_to_send, size_t valid_data_size,
                                     const uint16_t* invalid_data_to_send, size_t invalid_data_size,
                                     uint16_t* buf_to_recv, size_t recv_buf_size)
{
    size_t actual_sent_size = 0;
    size_t actual_recv_size = 0;

    if (valid_data_size > recv_buf_size || invalid_data_size > recv_buf_size)
    {
        SYS_TEST("\nError, recv buffer is not enough!\n");
        return;
    }

    //1. Send/Recv valid data
    if (valid_data_to_send != NULL)
    {
        SYS_TEST("\nSend data by Target SPI (should be valid):");
        for (size_t i = 0; i < valid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", valid_data_to_send[i]);
        }
        SYS_TEST("\n");

        while (!(actual_sent_size == valid_data_size && actual_recv_size == valid_data_size))
        {
            actual_sent_size += SPI_SendMultiDataAsync(TGT_SPI, valid_data_to_send + actual_sent_size, valid_data_size - actual_sent_size);
            actual_recv_size += SPI_RecvMultiDataAsync(AUX_SPI, buf_to_recv + actual_recv_size, valid_data_size - actual_recv_size);
        }

        SYS_TEST("Receive data by Auxiliary SPI:");
        for (size_t i = 0; i < valid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", buf_to_recv[i]);
        }
        SYS_TEST("\n");
    }

    //2. Send/Recv invalid data
    if (invalid_data_to_send != NULL)
    {
        actual_sent_size = 0;
        actual_recv_size = 0;
        SYS_TEST("\nSend data by Target SPI (should be invalid):");
        for (size_t i = 0; i < invalid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", invalid_data_to_send[i]);
        }
        SYS_TEST("\n");

        while (!(actual_sent_size == valid_data_size && actual_recv_size == valid_data_size))
        {
            actual_sent_size += SPI_SendMultiDataAsync(TGT_SPI, invalid_data_to_send + actual_sent_size, invalid_data_size - actual_sent_size);
            actual_recv_size += SPI_RecvMultiDataAsync(AUX_SPI, buf_to_recv + actual_recv_size, invalid_data_size - actual_recv_size);
        }

        SYS_TEST("Receive data by Auxiliary SPI:");
        for (size_t i = 0; i < invalid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", buf_to_recv[i]);
        }
        SYS_TEST("\n");
    }
}

T_SPI_TEST_RESULT SPI_DataSizeSelectTestCase1(void)
{
    char c;
    SPI_InitTypeDef spiInitStruct;

    spiInitStruct.SPI_format = SPI_FormatMotorola;
    spiInitStruct.SPI_baudRateDiv = SPI_BAUD_RATE_DIV(240);//SPI_BaudRateDiv_96;
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;

    while(1)
    {
        SPI_PrintInfoCase1();
        switch(c = getchar())
        {
        /* A. Transfer 4-bit data. */
        case 'A':
        case 'a':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_4b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_4b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid4BitData, sizeof(TestValid4BitData)/sizeof(uint16_t), TestInvalid4BitData, sizeof(TestInvalid4BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* B. Transfer 5-bit data. */
        case 'B':
        case 'b':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_5b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_5b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid5BitData, sizeof(TestValid5BitData)/sizeof(uint16_t), TestInvalid5BitData, sizeof(TestInvalid5BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* C. Transfer 6-bit data. */
        case 'C':
        case 'c':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_6b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_6b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid6BitData, sizeof(TestValid6BitData)/sizeof(uint16_t), TestInvalid6BitData, sizeof(TestInvalid6BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* D. Transfer 7-bit data. */
        case 'D':
        case 'd':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_7b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_7b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid7BitData, sizeof(TestValid7BitData)/sizeof(uint16_t), TestInvalid7BitData, sizeof(TestInvalid7BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* E. Transfer 8-bit data. */
        case 'E':
        case 'e':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_8b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_8b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid8BitData, sizeof(TestValid8BitData)/sizeof(uint16_t), TestInvalid8BitData, sizeof(TestInvalid8BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* F. Transfer 9-bit data. */
        case 'F':
        case 'f':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_9b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_9b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid9BitData, sizeof(TestValid9BitData)/sizeof(uint16_t), TestInvalid9BitData, sizeof(TestInvalid9BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* G. Transfer 10-bit data. */
        case 'G':
        case 'g':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_10b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_10b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid10BitData, sizeof(TestValid10BitData)/sizeof(uint16_t), TestInvalid10BitData, sizeof(TestInvalid10BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* H. Transfer 11-bit data. */
        case 'H':
        case 'h':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_11b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_11b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid11BitData, sizeof(TestValid11BitData)/sizeof(uint16_t), TestInvalid11BitData, sizeof(TestInvalid11BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* I. Transfer 12-bit data. */
        case 'I':
        case 'i':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_12b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_12b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid12BitData, sizeof(TestValid12BitData)/sizeof(uint16_t), TestInvalid12BitData, sizeof(TestInvalid12BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* J. Transfer 13-bit data. */
        case 'J':
        case 'j':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_13b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_13b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid13BitData, sizeof(TestValid13BitData)/sizeof(uint16_t), TestInvalid13BitData, sizeof(TestInvalid13BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* K. Transfer 14-bit data. */
        case 'K':
        case 'k':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_14b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_14b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid14BitData, sizeof(TestValid14BitData)/sizeof(uint16_t), TestInvalid14BitData, sizeof(TestInvalid14BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* L. Transfer 15-bit data. */
        case 'L':
        case 'l':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_15b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_15b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid15BitData, sizeof(TestValid15BitData)/sizeof(uint16_t), TestInvalid15BitData, sizeof(TestInvalid5BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* M. Transfer 16-bit data. */
        case 'M':
        case 'm':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);
            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid16BitData, sizeof(TestValid16BitData)/sizeof(uint16_t), NULL, 0, TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return SPI_TST_OK;
}
