/**************************************************************************//**
 * @file     spi_simple_transmission_demo.c
 * @version  V1.0
 * $Date:    20/03/03 16:50 $
 * @brief    SPI test case 6, simple transmission demo.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"

static uint16_t TestDataBuf[8];

static const uint16_t TestValid8BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x00CC, 0x00DD, 0x00EE, 0x00FF};
static const uint16_t TestInvalid8BitData[] = {0x0100, 0x0101, 0x0102, 0x0103, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid15BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x7FCC, 0x7FDD, 0x7FEE, 0x7FFF};
static const uint16_t TestInvalid15BitData[] = {0x8000, 0x8001, 0x8002, 0x8003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static const uint16_t TestValid16BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static void SPI_PrintInfoCase(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Transfer  8-bit data.                           | \n");
    SYS_TEST("|    Input 'B'    Transfer  15-bit data.                          | \n");
    SYS_TEST("|    Input 'C'    Transfer  16-bit data.                          | \n");
    SYS_TEST("|    Input 'D'    Reveive  8-bit data,master receive              | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void MultiDataSendReceiveSpi(const uint16_t* valid_data_to_send, size_t valid_data_size,
                                     uint16_t* buf_to_recv, size_t recv_buf_size)
{
    size_t actual_sent_size = 0;
    size_t actual_recv_size = 0;

    if (valid_data_size > recv_buf_size)
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

        while (!(actual_sent_size == valid_data_size && actual_recv_size == valid_data_size + 1))
        {
            actual_sent_size += SPI_SendMultiDataAsync(TGT_SPI, valid_data_to_send + actual_sent_size, valid_data_size - actual_sent_size);
            actual_recv_size += SPI_RecvMultiDataAsync(AUX_SPI, buf_to_recv + actual_recv_size, valid_data_size - actual_recv_size + 1);
        }

        SYS_TEST("Receive data by Auxiliary SPI:");
        for (size_t i = 0; i < valid_data_size + 1; i++)
        {
            if (i % 9 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", buf_to_recv[i]);
        }
        SYS_TEST("\n");
    }
}

static void MultiDataSendReceiveSpi1(const uint16_t* valid_data_to_send, size_t valid_data_size,
                                     uint16_t* buf_to_recv, size_t recv_buf_size)
{
    size_t slv_actual_sent_size = 0;
	size_t mst_actual_sent_size = 0;
    size_t actual_recv_size = 0;

    if (valid_data_size > recv_buf_size)
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


        while (!(slv_actual_sent_size == valid_data_size && actual_recv_size == valid_data_size + 1))
        {
            slv_actual_sent_size += SPI_SendMultiDataAsync(AUX_SPI, valid_data_to_send + slv_actual_sent_size, valid_data_size - slv_actual_sent_size);
			mst_actual_sent_size += SPI_SendMultiDataAsync(TGT_SPI, valid_data_to_send + mst_actual_sent_size, valid_data_size - mst_actual_sent_size);
			actual_recv_size += SPI_RecvMultiDataAsync(TGT_SPI, buf_to_recv + actual_recv_size, valid_data_size - actual_recv_size + 1);
		}

        SYS_TEST("Receive data by Auxiliary SPI:");
        for (size_t i = 0; i < valid_data_size + 1; i++)
        {
            if (i % 9 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", buf_to_recv[i]);
        }
        SYS_TEST("\n");
    }
}

T_SPI_TEST_RESULT SPI_ThreeWireSimpleTransmissionCase9(void)
{
    char c;
    SPI_InitTypeDef spiInitStruct;

    spiInitStruct.SPI_format = SPI_FormatMotorola;
    spiInitStruct.SPI_baudRateDiv = SPI_BAUD_RATE_DIV(240);
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseFirstEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;

    while(1)
    {
        SPI_PrintInfoCase();
		c = getchar();
        switch(c)
        {
        /* E. Transfer 8-bit data. */
        case 'a':
        case 'A':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_8b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_9b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);

			SPI_SetWireNum(TGT_SPI,SPI_WireNumThree);
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_CommandSelect);
			TGT_SPI->DR = 0xa; //must set a invalid data
			SYS_delay_10nop(20);
			/*send data in 3-wire mode*/
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_DataSelect);

            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid8BitData, sizeof(TestValid8BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* L. Transfer 15-bit data. */
        case 'B':
        case 'b':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_15b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);

			SPI_SetWireNum(TGT_SPI,SPI_WireNumThree);
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_CommandSelect);
			TGT_SPI->DR = 0xa; //must set a invalid data
			SYS_delay_10nop(20);
			/*send data in 3-wire mode*/
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_DataSelect);

            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid15BitData, sizeof(TestValid15BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;
        /* M. Transfer 16-bit data. */
        case 'C':
        case 'c':
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

			SPI_SetWireNum(TGT_SPI,SPI_WireNumThree);
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_CommandSelect);
			TGT_SPI->DR = 0xa; //must set a invalid data
			SYS_delay_10nop(20);
			/*send data in 3-wire mode*/
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_DataSelect);

            /* Transmit and reseive data */
            MultiDataSendReceiveSpi(TestValid16BitData, sizeof(TestValid16BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
            /* Disable SPI after use */
            SPI_DisableSpi(TGT_SPI);
            SPI_DisableSpi(AUX_SPI);
            break;

        /* D. Transfer 8-bit data receive. */
        case 'd':
        case 'D':
            /* Init Target SPI */
            spiInitStruct.SPI_role = SPI_RoleMaster;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_8b;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Init Auxiliary SPI */
            spiInitStruct.SPI_role = SPI_RoleSlave;
            spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_9b;
            SPI_Init(AUX_SPI, &spiInitStruct);
            SPI_EnableSpi(AUX_SPI);

			SPI_SetWireNum(TGT_SPI,SPI_WireNumThree);
//			SPI_Set3WireConfig(TGT_SPI,SPI_3WireRead,SPI_DataSelect);
			TGT_SPI->DR = 0xa; //must set a invalid data
			SYS_delay_10nop(20);
			/*send data in 3-wire mode*/
//			SPI_DataCmdLineEnable(TGT_SPI,ENABLE);
			SPI_Set3WireConfig(TGT_SPI,SPI_3WireRead,SPI_DataSelect);

            /* Transmit and reseive data */
            MultiDataSendReceiveSpi1(TestValid8BitData, sizeof(TestValid8BitData)/sizeof(uint16_t), TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
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
