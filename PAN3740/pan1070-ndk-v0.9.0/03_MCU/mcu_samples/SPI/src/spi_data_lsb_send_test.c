/**************************************************************************//**
 * @file     spi_frame_format_test.c
 * @version  V1.0
 * $Date:    19/10/24 18:30 $
 * @brief    SPI test case 3, test different frame format (Motorola/TI).
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"

static uint16_t TestDataBuf[8];

static const uint16_t TestValid16BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static void SPI_PrintInfoCase7(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Motorola SPI Format with SPO=0, SPH=0.          | \n");
    SYS_TEST("|    Input 'B'    Motorola SPI Format with SPO=0, SPH=1.          | \n");
    SYS_TEST("|    Input 'C'    Motorola SPI Format with SPO=1, SPH=0.          | \n");
    SYS_TEST("|    Input 'D'    Motorola SPI Format with SPO=1, SPH=1.          | \n");
    SYS_TEST("|    Input 'E'    TI Synchronous Serial Frame Format.             | \n");
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

static void BaudrateTestProc(SPI_InitTypeDef spiInitStruct)
{
    /* Init Target SPI */
    spiInitStruct.SPI_role = SPI_RoleMaster;
    SPI_Init(TGT_SPI, &spiInitStruct);

    /* Init Auxiliary SPI */
    spiInitStruct.SPI_role = SPI_RoleSlave;
    SPI_Init(AUX_SPI, &spiInitStruct);

    /* Enable SPI */
    SPI_EnableSpi(TGT_SPI);
    SPI_EnableSpi(AUX_SPI);

    /* Transmit and reseive data */
    MultiDataSendReceiveSpi(TestValid16BitData, sizeof(TestValid16BitData)/sizeof(uint16_t), NULL, 0, TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));

    /* Disable SPI after use */
    SPI_DisableSpi(TGT_SPI);
    SPI_DisableSpi(AUX_SPI);
}

T_SPI_TEST_RESULT SPI_DataLsbSendRcvTestCase7(void)
{
    char c;
	bool tlsbEn = false,alsbEn =false;
    SPI_InitTypeDef spiInitStruct =
    {
        .SPI_dataFrameSize = SPI_DataFrame_16b,
        .SPI_baudRateDiv = SPI_BaudRateDiv_250,
    };

    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to enable lsb send or not:                         | \n");
    SYS_TEST("|    Input '1'    target lsb enable,aux disable.                  | \n");
	SYS_TEST("|    Input '2'    target lsb disable,aux enable.                  | \n");
    SYS_TEST("|    Input '3'    target and aux lsb send enable.                 | \n");

	switch(c= getchar())
	{
		case '1':tlsbEn = true;alsbEn = false;break;
		case '2':tlsbEn = false;alsbEn = true;break;
		case '3':tlsbEn = true;alsbEn = true;break;
		default:tlsbEn = false;alsbEn = false;break;
	}
	SPI_TxLsbEnable(TGT_SPI,tlsbEn);
	SPI_RxLsbEnable(AUX_SPI,alsbEn);

    while(1)
    {
        SPI_PrintInfoCase7();
        switch(c = getchar())
        {
        /* A. Motorola SPI Format with SPO=0, SPH=0. */
        case 'A':
        case 'a':
            spiInitStruct.SPI_format = SPI_FormatMotorola;
            spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;
            spiInitStruct.SPI_CPHA = SPI_ClockPhaseFirstEdge;
            BaudrateTestProc(spiInitStruct);
            break;
        /* B. Motorola SPI Format with SPO=0, SPH=1. */
        case 'B':
        case 'b':
            spiInitStruct.SPI_format = SPI_FormatMotorola;
            spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;
            spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
            BaudrateTestProc(spiInitStruct);
            break;
        /* C. Motorola SPI Format with SPO=1, SPH=0. */
        case 'C':
        case 'c':
            spiInitStruct.SPI_format = SPI_FormatMotorola;
            spiInitStruct.SPI_CPOL = SPI_ClockPolarityHigh;
            spiInitStruct.SPI_CPHA = SPI_ClockPhaseFirstEdge;
            BaudrateTestProc(spiInitStruct);
            break;
        /* D. Motorola SPI Format with SPO=1, SPH=1. */
        case 'D':
        case 'd':
            spiInitStruct.SPI_format = SPI_FormatMotorola;
            spiInitStruct.SPI_CPOL = SPI_ClockPolarityHigh;
            spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
            BaudrateTestProc(spiInitStruct);
            break;
        /* E. TI Synchronous Serial Frame Format. */
        case 'E':
        case 'e':
            spiInitStruct.SPI_format = SPI_FormatTi;
            BaudrateTestProc(spiInitStruct);
            break;
        case 0x1B:  // Keyboard code <ESC>
			SPI_TxLsbEnable(TGT_SPI,false);
			SPI_RxLsbEnable(AUX_SPI,false);
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return SPI_TST_OK;
}
