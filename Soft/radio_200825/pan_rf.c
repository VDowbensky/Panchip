/**
 * @file      pan_rf.c
 *
 * @brief     PAN3029/PAN3060 driver implementation
 *
 * @copyright Revised BSD License, see section \ref LICENSE.
 *
 * @code
 *              ____              ____ _     _
 *             |  _ \ __ _ _ __  / ___| |__ (_)_ __
 *             | |_) / _` | '_ \| |   | '_ \| | '_ \
 *             |  __/ (_| | | | | |___| | | | | |_) |
 *             |_|   \__,_|_| |_|\____|_| |_|_| .__/
 *                                            |_|
 *              (C)2009-2025 PanChip
 *
 * @endcode
 *
 * @author    PanChip
 */
#include <stdio.h>
#include <string.h>
#include "hardware.h"
#include "pan_params.h"
#include "pan_rf.h"

static uint8_t g_CurrSF = 0;
volatile int g_RfDevIndex = 0;  // 3029模块索引,0-2,可索引3个模块
RF_RxPkt_t g_RfRecvPkt;

/**
 * @brief Holds the internal operating mode of the radio
 */
static volatile RadioOperatingModes_t OperatingMode;

/**
 * @brief Get the number of trailing zeros in a byte
 * @param[in] Value The value to check
 * @return The number of trailing zeros
 */
uint8_t __ctz(uint8_t Value)
{
    int i;

    for (i = 0; i < 8; ++i)
    {
        if ((Value >> i) & 1)
            return (uint8_t)i;
    }
    
    return 0;
}

/**
 * @brief Writes a single byte to the specified register
 * @param Addr Register address to be written
 * @param Value Single byte of data to be written to the register
 * @return RF_Err_t Returns the operation result
 *         - RF_OK: peration successful
 *         - RF_FAIL: Operation failed
 */
RF_Err_t RF_WriteReg(uint8_t Addr, uint8_t Value)
{
    spi_cs_low();
    spi_writereadbyte((Addr << 1) | 0x01);
    spi_writereadbyte(Value);
    spi_cs_high();

#if USE_RF_REG_CHECK
    uint8_t Temp = RF_ReadReg(Addr);
    if (Temp == Value)
    {
        return RF_OK;
    }
    else
    {
        printf("write reg fail: 0x%02x, 0x%02x, 0x%02x\n", Addr, Value, Temp);
        return RF_FAIL;
    }
#endif

    return RF_OK;
}

/**
 * @brief Read a single byte from the specified register
 * @param Addr Register address to read
 * @return unsigned char read from the register
 */
uint8_t RF_ReadReg(uint8_t Addr)
{
    uint8_t Temp;

    spi_cs_low();
    spi_writereadbyte((Addr << 1) & 0xFE);
    Temp = spi_writereadbyte(0xFF);
    spi_cs_high();

    return Temp;
}

/**
 * @brief Select register page
 * @param Page Register page to select
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @return RF_Err_t returns the operation result
 *         - RF_OK: operation successful
 *         - RF_FAIL: operation failed
 * @note If the current page is already the required page, there is no need to configure the register
 */
RF_Err_t RF_SetPage(uint8_t Page)
{
    // The gateway has three RF chips and needs three page variables
    volatile static uint8_t gCurrPage[3] = {0xFF, 0xFF, 0xFF};

    if(gCurrPage[g_RfDevIndex] == Page)
    {
        //printf("Page %d already selected\n", Page);
        return RF_OK;
    }

    gCurrPage[g_RfDevIndex] = Page;

    RF_WriteReg(REG_SYS_CTL, gCurrPage[g_RfDevIndex]);

#if USE_RF_REG_CHECK
    if ((RF_ReadReg(REG_SYS_CTL) & 0x03) != gCurrPage[g_RfDevIndex])
    {
        printf("Switch Page fail\n");
        return RF_FAIL;
    }
#endif
    
    return RF_OK;
}

/**
 * @brief Writes a single byte to a register in a specified page.
 * @param Page Page Register page to be written.
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr Register address to be written.
 * @param Value Single byte of data to be written to the register.
 * @return RF_Err_t Returns the result of the operation.
 *         - RF_OK: Operation successful.
 *         - RF_FAIL: Operation failed. 
 */
RF_Err_t RF_WritePageReg(uint8_t Page, uint8_t Addr, uint8_t Value)
{
    RF_SetPage(Page);
    RF_WriteReg(Addr, Value);

    return RF_OK;
}

RF_Err_t RF_WritePageRegUncheck(uint8_t Page, uint8_t Addr, uint8_t Value)
{
    // Switch to the specified page
    spi_cs_low();
    spi_writereadbyte((REG_SYS_CTL << 1) | 0x01);
    spi_writereadbyte(Page);
    spi_cs_high();

    // Write the register value
    spi_cs_low();
    spi_writereadbyte((Addr << 1) | 0x01);
    spi_writereadbyte(Value);
    spi_cs_high();

    return RF_OK;
}

/**
 * @brief Write multiple bytes to the register range of the specified page
 * @param Page Register page to be written
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr Register address to be written
 * @param Buffer Buffer pointer to the register to be written
 * @param Len Number of bytes to be written
 * @return RF_Err_t Returns the operation result
 *         - RF_OK: Operation successful
 *         - RF_FAIL: Operation failed
 * @note The chip automatically increments the register address after each byte is written
 */
RF_Err_t RF_WritePageRegs(uint8_t Page, uint8_t Addr, uint8_t *Buffer, uint8_t Len)
{
    RF_SetPage(Page);

    spi_cs_low();
    spi_writereadbyte((Addr << 1) | 0x01);// Bit 7:1 is the address, Bit 0 is the read/write bit
    for(int i = 0; i < Len; i++)
    {
        spi_writereadbyte(Buffer[i]);
    }
    spi_cs_high();

    return RF_OK;
}

/**
 * @brief Read a single byte from the register in the specified page
 * @param Page Register page to read
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr Register address to read
 * @return uint8_t Value read from the register
 */
uint8_t RF_ReadPageReg(uint8_t Page, uint8_t Addr)
{
    RF_SetPage(Page);
    return RF_ReadReg(Addr);
}

/**
 * @brief Reads multiple bytes from the register range of the specified page.
 * @param Page Register page to be read
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr Register address to be read
 * @param Buffer Buffer pointer to store read data
 * @param Len Number of bytes to be read
 * @return RF_Err_t Returns the operation result
 *         - RF_OK: Operation successful
 *         - RF_FAIL: Operation failed
 */
RF_Err_t RF_ReadPageRegs(uint8_t Page, uint8_t Addr, uint8_t *Buffer, uint8_t Len)
{
    RF_SetPage(Page);
    
    spi_cs_low();
    spi_writereadbyte((Addr << 1) & 0xFE);
    for(int i = 0; i < Len; i++)
    {
        Buffer[i] = spi_writereadbyte(0xFF);
    }
    spi_cs_high();

    return RF_OK;
}

/**
 * @brief Sets the register bit of the specified page
 * @param Page Register page to set
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr Register address to set
 * @param Mask Bit mask to set
 * @return RF_Err_t Returns the operation result
 *         - RF_OK: Operation successful
 *         - RF_FAIL: Operation failed
 */
RF_Err_t RF_SetPageRegBits(uint8_t Page, uint8_t Addr, uint8_t Mask)
{
    uint8_t Temp;

    RF_SetPage(Page);
    Temp = RF_ReadReg(Addr);
    RF_WriteReg(Addr, Temp | Mask);

    return RF_OK;
}

/**
 * @brief Resets the register bits of the specified page
 * @param Page The register page to be reset
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr The register address to be reset
 * @param Mask The bit mask to be reset
 * @return RF_Err_t Returns the result of the operation
 *         - RF_OK: Operation successful
 *         - RF_FAIL: Operation failed
 */
RF_Err_t RF_ResetPageRegBits(uint8_t Page, uint8_t Addr, uint8_t Mask)
{
    uint8_t Temp;

    RF_SetPage(Page);
    Temp = RF_ReadReg(Addr);
    RF_WriteReg(Addr, Temp & (~Mask));

    return RF_OK;
}

/**
 * @brief Writes the register bits of the specified page.
 * @param Page Register page to be written
 *        - RF_PAGE0
 *        - RF_PAGE1
 *        - RF_PAGE2
 *        - RF_PAGE3
 * @param Addr Register address to be written
 * @param Value Value to be written
 * @param Mask Bit mask to be written
 * @return RF_Err_t Returns the result of the operation
 *         - RF_OK: Operation successful
 *         - RF_FAIL: Operation failed
 * @note This function first clears the register bits corresponding to the mask and then sets the new value.
 * @note For example, to set bits 2 and 3 of register 0x08 on page 1 to 0b10, leaving all other bits unchanged, you can call:
 *       RF_WritePageRegBits(RF_PAGE1, 0x08, 0x02, 0x0C);
 *       where Value = 0x02, Mask = 0x0C. Value does not need to be left-shifted because the mask already specifies the bits to be set.
 */
RF_Err_t RF_WritePageRegBits(uint8_t Page, uint8_t Addr, uint8_t Value, uint8_t Mask)
{
    uint8_t Temp;
    uint8_t shift = __ctz(Mask); // get shift Value

    Value <<= shift;
    Value &= Mask;

    RF_SetPage(Page);
    Temp = RF_ReadReg(Addr);
    RF_WriteReg(Addr, (Temp & (~Mask)) | Value);

    return RF_OK;
}

/**
 * @brief Write data to the RF TX FIFO
 * @param Buffer Data buffer to be written to the FIFO
 * @param Size Number of bytes to be written
 */
void RF_WriteFifo(uint8_t *Buffer, int Size)
{
    RF_WritePageRegs(0, REG_FIFO_ACC_ADDR, Buffer, Size);
}

/**
 * @brief Read data from the RF RX FIFO
 * @param Buffer Buffer to store data read from the FIFO
 * @param Size Number of bytes to be read
 */
void RF_ReadFifo(uint8_t *Buffer, int Size)
{
    RF_ReadPageRegs(0, REG_FIFO_ACC_ADDR, Buffer, Size);
}

/**
 * @brief Initialize the RF transceiver to STB3 state after power-up
 * @return Whether the initialization is successful, 0 indicates success, 1 indicates failure
 * @note Before calling this function, you need to configure the MCU's SPI and related GPIO pins
 */
RF_Err_t RF_Init(void)
{
#if USE_RF_RST_GPIO == 1
    RF_RESET_LOW;
    RF_DelayUs(100);   // Ensure the actual delay is above 100us
    RF_RESET_HIGH;
    RF_DelayUs(100);   // Ensure the actual delay is above 100us
#else
    // por reset
    RF_ASSERT(RF_WriteReg(0x04, 0x06));
    RF_ASSERT(RF_WriteReg(0x04, 0x16));
    RF_DelayUs(100);  // Ensure the actual delay is above 100us
    RF_ASSERT(RF_WriteReg(0x04, 0x06));
    RF_DelayUs(100);  // Ensure the actual delay is above 100us
#endif

    RF_ASSERT(RF_SoftReset());

    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_DEEP_SLEEP));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us

    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_SLEEP));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us
    
    RF_SetPage(RF_PAGE0);   // sync Page register to page0

    RF_ASSERT(RF_SetPageRegBits(RF_PAGE3, 0x06, 0x20));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us

    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB1));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us

    RF_ASSERT(RF_WritePageReg(RF_PAGE3, 0x26, 0x2F));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us

    RF_ASSERT(RF_WriteReg(0x04, 0x36));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us

    RF_InitTcxoGpio();
    RF_TurnonTcxo();

    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB2));
    RF_DelayUs(200);  // Ensure the actual delay is above 200us

    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB3));
    RF_DelayUs(10);  // Ensure the actual delay is above 10us
    
    /* Start: Set the frequency band to 490M band */
    RF_ASSERT(RF_SetLoFreq(g_RfFreqTable[0].lo_param));
    #define MIDDLE_FREQ   490000000
    uint8_t reg_freq[4] = 
    {
        (MIDDLE_FREQ & 0xFF), 
        ((MIDDLE_FREQ >> 8) & 0xFF), 
        ((MIDDLE_FREQ >> 16) & 0xFF), 
        ((MIDDLE_FREQ >> 24) & 0xFF)
    };
    RF_ASSERT(RF_WritePageRegs(RF_PAGE3, 0x09, reg_freq, 4));
    /*End: Set the frequency band to 490M frequency band */ 

    RF_ASSERT(RF_Calibrate());
    RF_ASSERT(RF_WriteDefaultParams());
    RF_ASSERT(RF_ConfigAgcParams(AGC_ON));

    RF_InitAntGpio();

    RF_SetModemMode(MODEM_MODE_NORMAL);
    
    return RF_OK;
}

RadioOperatingModes_t RF_GetOperatingMode(void)
{
    return OperatingMode;
}

void RF_SetOperatingMode(RadioOperatingModes_t mode)
{
    OperatingMode = mode;
}

void RF_CheckDeviceReady(void)
{
    if (RF_GetOperatingMode() == MODE_SLEEP)
    {
        RF_ExitSleepState();
        RF_SetOperatingMode(MODE_SLEEP);
    }
}

/**
 * @brief Function used to obtain the length and content of received data
 * @param *Buffer Pointer address of the data area to be received
 * @param Size Maximum receive length of the receive data area
 * @return Received data length
 * @note This function must be called before the receive interrupt is cleared, as clearing the receive interrupt will clear the receive length register.
 */
uint8_t RF_GetRecvPayload(uint8_t *Buffer, uint8_t Size)
{
    uint8_t RxLen;
    RxLen = RF_GetRecvPayloadLen();
    RF_ReadFifo(Buffer, RxLen);
    return RxLen;
}

void RF_SetSyncWord(uint8_t syncWord)
{
    RF_WritePageReg(RF_PAGE3, 0x0F, syncWord);
}

void RF_SetStandby(void)
{
    RF_SetRfState(RF_MODE_STB3);
    OperatingMode = MODE_STDBY;
}

void RF_SetSleep(void)
{
    RF_EnterSleepState();
    //printf("[S]");
    OperatingMode = MODE_SLEEP;
}

/**
 * @brief Set the transmit mode
 * @param Buffer Buffer Data buffer to be sent
 * @param Size Number of data bytes to be sent
 * @return None
 * @note Ensure that the RF is in standby (STB3) mode before calling this function.
 * @note This function is in single-shot transmit mode and will automatically enter standby (STB3) mode after the transmit is complete.
 * @note The TX_DONE interrupt will be triggered after the transmit is complete.
 */
void RF_SetTx(uint8_t *Buffer, uint8_t Size)
{
    RF_TxSinglePkt(Buffer, Size);
    OperatingMode = MODE_TX;
}

/**
 * @brief Set the receive mode
 * @param TimeoutMs Receive timeout in milliseconds
 *        - 0: Continuous receive mode
 *        - >0: Single receive mode, after which a timeout interrupt is generated and the device automatically enters standby (STB3) mode
 * @return None
 * @note Ensure that the RF is in standby (STB3) mode before calling this function.
 */
void RF_SetRx(uint32_t TimeoutMs)
{
    if (TimeoutMs == 0)
    {
        RF_EnterContinousRxState();
    }
    else
    {
        RF_EnterSingleRxWithTimeout(TimeoutMs);
    }

    OperatingMode = MODE_RX;
}

void RF_SetCad(void)
{
    OperatingMode = MODE_CAD;
}

void RF_StartTxContinuousWave(void)
{
    RF_WriteReg(0x02, RF_MODE_STB3);
    RF_EnableLdoPa();
    RF_SetTxMode(RF_TX_CONTINOUS);
    RF_SetTxPower(RF_MAX_RAMP);
    RF_SetPageRegBits(RF_PAGE1, 0x1E, RF_BIT0);
    OperatingMode = MODE_TX;
}

void RF_StopTxContinuousWave(void)
{
    RF_WriteReg(0x02, RF_MODE_STB3);
    RF_DisableLdoPa();
    RF_ResetPageRegBits(RF_PAGE1, 0x1E, RF_BIT0);
    OperatingMode = MODE_STDBY;
}

void RF_SetRegulatorMode(RadioRegulatorMode_t Mode)
{
    if (Mode == USE_LDO)
    {
        RF_EnableDCDC(0); // use ldo power
    }
    else
    {
        RF_EnableDCDC(1); // use dc-dc power
    }
}

RF_Err_t RF_SetFreq(uint32_t frequency)
{
    uint32_t fb, fc;
    uint32_t temp_val = 0;
    uint32_t integer_part;
    uint32_t table_num = (sizeof(g_RfFreqTable) / sizeof(RadioFreqTable_t));

    if (frequency < g_RfFreqTable[0].freq_low || frequency > g_RfFreqTable[table_num - 1].freq_high)
    {
        return RF_FAIL;
    }
    
    for (int i = 0; i < table_num; i++)
    {
        if (frequency > g_RfFreqTable[i].freq_low && frequency <= g_RfFreqTable[i].freq_high)
        {
            RF_WritePageReg(RF_PAGE0, 0x40, g_RfFreqTable[i].vco_param);
            temp_val = frequency * g_RfFreqTable[i].freq_factor;
            break;
        }

        // no frequency range matched
        if (i >= table_num)
        {
            return RF_FAIL;
        }
    }
    
    integer_part = temp_val / 32000000;
    fb = integer_part - 20;
    fc = (temp_val % 32000000) / 40000;

//    uint8_t reg_freq[4] = 
//    {
//        (uint8_t)(frequency),
//        (uint8_t)((frequency >> 8)),
//        (uint8_t)((frequency >> 16)),
//        (uint8_t)((frequency >> 24))
//    };
//    RF_WritePageRegs(RF_PAGE3, 0x09, reg_freq, 4);
    
    uint8_t temp_fx[3] = {(uint8_t)(fb), (uint8_t)(fc), (uint8_t)((fc >> 8) & 0x0F)};
    RF_WritePageRegs(RF_PAGE3, 0x15, temp_fx, 3);
    
    return RF_OK;
}


/* @brief CAD function disable
* @param[in] <none>
* @return  result
*/
RF_Err_t RF_StopCad(void)
{
    RF_ASSERT(RF_SetPageRegBits(0, 0x5E, RF_BIT6));
    RF_ASSERT(RF_WritePageReg(1, 0x0F, 0x0A));

    return RF_OK;
}

/**
 * @brief CAD function enable
 * @param[in] <threshold>
            CAD_DETECT_THRESHOLD_0A / CAD_DETECT_THRESHOLD_10 / CAD_DETECT_THRESHOLD_15 / CAD_DETECT_THRESHOLD_20
              <chirps>
            CAD_DETECT_NUMBER_0 / CAD_DETECT_NUMBER_1 / CAD_DETECT_NUMBER_2 / CAD_DETECT_NUMBER_3 /
 * @return  result
 */
RF_Err_t RF_StartCad(uint8_t threshold, uint8_t chirps)
{
    RF_SetGpioOutput(11);

    RF_ASSERT(RF_ResetPageRegBits(0, 0x5E, RF_BIT6));
    RF_ASSERT(RF_WritePageRegBits(1, 0x25, chirps, RF_BIT0|RF_BIT1));
    RF_ASSERT(RF_WritePageReg(1, 0x0f, threshold));

    return RF_OK;
}

void RF_SetModulationParams(ModulationParams_t *modulationParams)
{
    RF_WritePageRegBits(RF_PAGE3, 0x0E, modulationParams->Params.ChirpIot.SpreadingFactor, 0xF0);      // set spreading factor
    RF_WritePageRegBits(RF_PAGE3, 0x0D, modulationParams->Params.ChirpIot.Bandwidth, 0xF0);            // set bandwidth
    RF_WritePageRegBits(RF_PAGE3, 0x0D, modulationParams->Params.ChirpIot.CodingRate, 0x0E);           // set coding rate
    RF_WritePageRegBits(RF_PAGE3, 0x12, modulationParams->Params.ChirpIot.LowDatarateOptimize, 0x08);  // set low data rate optimize
    
    g_CurrSF = modulationParams->Params.ChirpIot.SpreadingFactor;

    // printf("sf: %d, bw: %d, cr: %d, ldr: %d\n", modulationParams->Params.ChirpIot.SpreadingFactor, 
    //                                             modulationParams->Params.ChirpIot.Bandwidth,       
    //                                             modulationParams->Params.ChirpIot.CodingRate,      
    //                                             modulationParams->Params.ChirpIot.LowDatarateOptimize);
    
    // set calibration mode
    if (modulationParams->Params.ChirpIot.Bandwidth == CHIRP_BW_500)
    {
        RF_ResetPageRegBits(RF_PAGE2, 0x3F, RF_BIT1);
    }
    else
    {
        RF_SetPageRegBits(RF_PAGE2, 0x3F, RF_BIT1);
    }
}

void RF_SetPacketParams(PacketParams_t *packetParams)
{
    RF_SetInvertIQ(packetParams->Params.ChirpIot.InvertIQ);
    RF_SetPreamLen(packetParams->Params.ChirpIot.PreambleLength);
    // RF_SetCrc(CRC_OFF);   /* Keep crc disabled */
    RF_SetCrc(packetParams->Params.ChirpIot.CrcMode);
}

void RF_SetCadParams(RadioChirpCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout)
{
#if 0
    uint8_t Buffer[7];

    Buffer[0] = ( uint8_t )cadSymbolNum;
    Buffer[1] = cadDetPeak;
    Buffer[2] = cadDetMin;
    Buffer[3] = ( uint8_t )cadExitMode;
    Buffer[4] = ( uint8_t )( ( cadTimeout >> 16 ) & 0xFF );
    Buffer[5] = ( uint8_t )( ( cadTimeout >> 8 ) & 0xFF );
    Buffer[6] = ( uint8_t )( cadTimeout & 0xFF );
    RF_WriteCommand( RADIO_SET_CADPARAMS, Buffer, 5 );
#endif

    OperatingMode = MODE_CAD;
}

/**
 * @brief set intermediate frequency
 * @param[in] <freq> receive req
 * @return result
 */
uint32_t RF_SetImdFreq(uint32_t freq)
{
    uint16_t mid_freq, val1, val2;

    mid_freq = 4096 - freq*4096/32000;
    val1 = mid_freq & 0xff;
    val2 = mid_freq >> 8;

#if 0
    uint8_t temp_val;
    temp_val = RF_ReadPageReg(PAGE0_SEL, 0x34);
    temp_val &= 0x7f;
    if (RF_WritePageReg(PAGE0_SEL, 0x34, temp_val) != RF_OK)
    {
        return RF_FAIL;
    }
    temp_val = RF_ReadPageReg(PAGE0_SEL, 0x34);
    printf("0x34 = %x\n",temp_val);
    temp_val = RF_ReadPageReg(PAGE0_SEL, 0x35);
    temp_val |= 0x4;
    if (RF_WritePageReg(PAGE0_SEL, 0x35, temp_val) != RF_OK)
    {
        return RF_FAIL;
    }
    temp_val = RF_ReadPageReg(PAGE0_SEL, 0x35);
    printf("0x35 = %x\n",temp_val);
    temp_val = RF_ReadPageReg(PAGE0_SEL, 0x36);
    temp_val |= 0x1c;
    if (RF_WritePageReg(PAGE0_SEL, 0x36, temp_val) != RF_OK)
    {
        return RF_FAIL;
    }
    temp_val = RF_ReadPageReg(PAGE0_SEL, 0x36);
    printf("0x36 = %x\n",temp_val);
#endif

    if(RF_WritePageReg(3, 0x10, val1) != RF_OK)
    {
        return RF_FAIL;
    }
    if(RF_WritePageReg(3, 0x11, val2) != RF_OK)
    {
        return RF_FAIL;
    }

    return RF_OK;
}

uint8_t RF_GetRecvPayloadLen(void)
{
    return RF_ReadPageReg(RF_PAGE1, 0x7D);
}

void RF_GetPacketStatus(PacketStatus_t *pktStatus)
{
    pktStatus->Params.ChirpIot.RssiPkt = RF_GetPktRssi();
    pktStatus->Params.ChirpIot.SnrPkt = RF_GetSnr();
    pktStatus->Params.ChirpIot.FreqError = 0;
}

/**
 * @brief set RF GPIO as input
 * @param[in] <GpioPin>  pin number of GPIO to be enable
 * @return result
 */
RF_Err_t RF_SetGpioInput(uint8_t GpioPin)
{
    if(GpioPin < 8)
    {
        RF_ASSERT(RF_SetPageRegBits(RF_PAGE0, 0x63, (1 << GpioPin)));
    }
    else
    {
        RF_ASSERT(RF_SetPageRegBits(RF_PAGE0, 0x64, (1 << (GpioPin - 8))));
    }

    return RF_OK;
}

/**
 * @brief set RF GPIO as output
 * @param[in] <GpioPin>  pin number of GPIO to be enable
 * @return result
 */
RF_Err_t RF_SetGpioOutput(uint8_t GpioPin)
{
    if(GpioPin < 8)
    {
        RF_SetPageRegBits(RF_PAGE0, 0x65, (1 << GpioPin));
    }
    else
    {
        RF_SetPageRegBits(RF_PAGE0, 0x66, (1 << (GpioPin - 8)));
    }

    return RF_OK;
}

/**
 * @brief set GPIO output state, SET or RESET
 * @param[in] <GpioPin>  pin number of GPIO to be opearted
 *            <state>   0  -  reset,
 *                      1  -  set
 * @return result
 */
RF_Err_t RF_WriteGpio(uint8_t GpioPin, uint8_t state)
{
    if(GpioPin < 8)
    {
        RF_WritePageRegBits(RF_PAGE0, 0x67, state, (1 << GpioPin));
    }
    else
    {
        RF_WritePageRegBits(RF_PAGE0, 0x68, state, (1 << (GpioPin - 8)));
    }

    return RF_OK;
}

/**
 * @brief get GPIO input state
 * @param[in] <GpioPin>  pin number of GPIO to be opearted
 *            <state>   0  -  low,
 *                      1  -  high
 * @return result
 */
bool RF_ReadGpio(uint8_t GpioPin)
{
    uint8_t Temp;

    if(GpioPin < 6)
    {
        Temp = RF_ReadPageReg(RF_PAGE0, 0x74);
    }
    else
    {
        Temp = RF_ReadPageReg(RF_PAGE0, 0x75);
        GpioPin -= 6;
    }

    return (bool)((Temp >> GpioPin) & 0x01);
}

/**
 * @brief set LO frequency
 * @param[in] <lo> LO frequency
 *            LO_400M / LO_800M
 * @return result
 */
RF_Err_t RF_SetLoFreq(uint32_t lo)
{
    if(lo == LO_400M)
    {
        RF_ASSERT(RF_WritePageRegBits(RF_PAGE0, 0x3D, 1, 0x70));
    }
    else if(lo == LO_800M)
    {
        RF_ASSERT(RF_WritePageRegBits(RF_PAGE0, 0x3D, 0, 0x70));
    }
    else
    {
        RF_ASSERT(RF_WritePageRegBits(RF_PAGE0, 0x3D, lo>>4, 0x70));
    }

    return RF_OK;
}

/**
 * @brief set rf tx power
 * @param[in] <TxPower> open gears (range in 1--23（405MHz-565MHz）1-22(868/915MHz))
 * @return result
 */
RF_Err_t RF_SetTxPower(uint8_t TxPower)
{
    int index;
    uint8_t tmp_value1, tmp_value2;
    static bool pa_bias_flag = false; // for read efuse only once
    static uint8_t pa_bias = 0;
    
    TxPower = (TxPower > RF_MAX_RAMP ? RF_MAX_RAMP : TxPower);
    TxPower = (TxPower < RF_MIN_RAMP ? RF_MIN_RAMP : TxPower);
    
    index = TxPower - 1;

    /* modulate wave ramp mode */
    RF_ASSERT(RF_WritePageReg(RF_PAGE3, 0x22, power_ramp_cfg[index].pa_ldo & 0x01));
    RF_ASSERT(RF_WritePageReg(RF_PAGE0, 0x1E, power_ramp_cfg[index].ramp));
    RF_ASSERT(RF_WritePageReg(RF_PAGE0, 0x4B, power_ramp_cfg[index].pa_ldo >> 4));
    
    if (power_ramp_cfg[index].pa_duty != 0x70)
    {
        RF_ASSERT(RF_WritePageReg(RF_PAGE0, 0x46, 0x06));
    }
    else
    {
        RF_ASSERT(RF_WritePageReg(RF_PAGE0, 0x46, 0x02));
    }
    
    if(!pa_bias_flag)
    {
        RF_ResetPageRegBits(RF_PAGE2, 0x3E, RF_BIT3); // RF unlock info
        pa_bias = RF_ReadInfoByte(0x3B, 0x5AA5, 0x20);
        RF_SetPageRegBits(RF_PAGE2, 0x3E, RF_BIT3);  // RF lock info

        if (pa_bias == 0)
        {
            pa_bias = 8;
        }

        pa_bias_flag = true;
    }

    tmp_value1 = pa_bias - (power_ramp_cfg[index].pa_duty & 0x0F);
    tmp_value2 = (power_ramp_cfg[index].pa_duty & 0xF0) | tmp_value1;
    RF_ASSERT(RF_WritePageReg(RF_PAGE0, 0x45, tmp_value2));

    return RF_OK;
}

/**
 * @brief set bandwidth
 * @param[in] <bw_val> Value relate to bandwidth
 *            BW_62_5K / BW_125K / BW_250K / BW_500K
 * @return result
 */
RF_Err_t RF_SetBW(uint8_t bw)
{
    RF_ASSERT(RF_WritePageRegBits(RF_PAGE3, 0x0D, bw, 0xF0));

    if (bw != CHIRP_BW_500)
    {
        RF_ASSERT(RF_SetPageRegBits(RF_PAGE2, 0x3F, RF_BIT1));
    }
    else
    {
        RF_ASSERT(RF_ResetPageRegBits(RF_PAGE2, 0x3F, RF_BIT1));
    }

    return RF_OK;
}

/**
 * @brief set spread factor
 * @param[in] <sf> spread factor to set
 *            SF_5 / SF_6 /SF_7 / SF_8 / SF_9 / SF_10 / SF_11 / SF_12
 * @return result
 */
RF_Err_t RF_SetSF(uint8_t sf)
{
    RF_ASSERT(RF_WritePageRegBits(RF_PAGE3, 0x0E, sf, 0xF0));
    g_CurrSF = sf;
    return RF_OK;
}

/**
 * @brief set payload CRC
 * @param[in] <NewState> CRC to set
 *            CRC_ON / CRC_OFF
 * @return result
 */
RF_Err_t RF_SetCrc(bool NewState)
{
    RF_ASSERT(RF_WritePageRegBits(RF_PAGE3, 0x0E, NewState>0?1:0, 0x08));
    return RF_OK;
}

/**
 * @brief set code rate
 * @param[in] <CodeRate> code rate to set
 *            PAN_RF_CR_4_5 / PAN_RF_CR_4_6 / PAN_RF_CR_4_7 / PAN_RF_CR_4_8
 * @return result
 */
RF_Err_t RF_SetCR(uint8_t CodeRate)
{
    RF_ASSERT(RF_WritePageRegBits(RF_PAGE3, 0x0D, CodeRate, 0x0E));

    return RF_OK;
}

/**
 * @brief set LDR mode
 * @param[in] <mode> LDR switch
 *            LDR_ON / LDR_OFF
 * @return result
 */
RF_Err_t RF_SetLDR(uint8_t mode)
{
    RF_ASSERT(RF_WritePageRegBits(RF_PAGE3, 0x12, mode, 0x08));
    return RF_OK;
}

/**
 * @brief set preamble
 * @param[in] preamble Len
 * @return result
 */
RF_Err_t RF_SetPreamLen(uint16_t Len)
{
    uint8_t Temp[2] = {(uint8_t)(Len), (uint8_t)((Len >> 8))};
    RF_ASSERT(RF_WritePageRegs(RF_PAGE3, 0x13, Temp, 2));
    return RF_OK;
}

/**
 * @brief get rssi Value
 * @param[in] <none>
 * @return rssi
 */
int8_t RF_GetPktRssi(void)
{
    return RF_ReadPageReg(RF_PAGE1, 0x7F);
}

/**
 * @brief current channel energy detection
 * @param[in] <none>
 * @return rssi
 */
int8_t RF_GetRealTimeRssi(void)
{
    int8_t RtRssi;

    RF_ASSERT(RF_ResetPageRegBits(RF_PAGE2, 0x06, 0x04));
    RF_ASSERT(RF_SetPageRegBits(RF_PAGE2, 0x06, 0x04));

    RtRssi = RF_ReadPageReg(RF_PAGE1, 0x7E);

    return RtRssi;
}

/**
 * @brief set modem mode
 * @param[in] <modem_mode>
 *            MODEM_MODE_NORMAL / MODEM_MODE_MULTI_SECTOR
 * @return result
 */
RF_Err_t RF_SetModemMode(uint8_t modem_mode)
{
    if (modem_mode == MODEM_MODE_NORMAL)
    {
        RF_ASSERT(RF_WritePageReg(RF_PAGE1, 0x0B, 0x08));
    }
    else if (modem_mode == MODEM_MODE_MULTI_SECTOR)
    {
        RF_ASSERT(RF_WritePageReg(RF_PAGE1, 0x0B, 0x18));
        RF_ASSERT(RF_WritePageReg(RF_PAGE1, 0x2F, 0x54));
        RF_ASSERT(RF_WritePageReg(RF_PAGE1, 0x30, 0x40));
    }

    return RF_OK;
}

/**
 * @brief set timeout for Rx. It is useful in RF_RX_SINGLE_TIMEOUT mode
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
RF_Err_t RF_SetRxTimeout(uint16_t TimeoutMs)
{
    uint8_t Temp[2] = {(uint8_t)TimeoutMs, (uint8_t)(TimeoutMs >> 8)};

    RF_ASSERT(RF_WritePageRegs(RF_PAGE3, 0x07, Temp, 2));

    return RF_OK;
}

/**
 * @brief Enter Sleep Mode from Standby3 Mode
 * 
 * @return RF_Err_t 
 */
RF_Err_t RF_EnterSleepState(void)
{
    RF_ShutdownAnt();
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB3));
    RF_DelayUs(150);
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB2));
    RF_DelayUs(10);
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB1));
    RF_DelayUs(10);
    RF_TurnoffTcxo();
    RF_ASSERT(RF_WriteReg(0x04, 0x16));
    RF_DelayUs(10);
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_SLEEP));
    RF_ASSERT(RF_ResetPageRegBits(RF_PAGE3, 0x06, 0x20));
    RF_DelayUs(10);
    RF_ASSERT(RF_WritePageReg(RF_PAGE3, 0x26, 0x0F));
    
    OperatingMode = MODE_SLEEP;
    
    return RF_OK;
}

/**
 * @brief Exit Sleep Mode and enter Standby3 Mode
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_ExitSleepState(void)
{
    RF_ASSERT(RF_SetPageRegBits(RF_PAGE3, 0x06, RF_BIT5));
    RF_DelayUs(10);
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB1));
    RF_DelayUs(10);
    RF_ASSERT(RF_WritePageReg(RF_PAGE3, 0x26, 0x2F));
    RF_DelayUs(10);
    RF_ASSERT(RF_WriteReg(0x04, 0x36));
    RF_DelayUs(10);
    RF_TurnonTcxo();    // turn on TCXO
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB2));
    RF_DelayUs(150);
    RF_ASSERT(RF_WriteReg(0x02, RF_MODE_STB3));
    RF_DelayUs(10);
    
    OperatingMode = MODE_STDBY;
    
    return RF_OK;
}

RF_Err_t RF_SoftReset(void)
{
    RF_ASSERT(RF_WriteReg(REG_SYS_CTL, 0x80));
    RF_DelayUs(2);
    RF_ASSERT(RF_WriteReg(REG_SYS_CTL, 0x00));
    RF_DelayUs(2);

    RF_ReadReg(REG_SYS_CTL);

    return RF_OK;
}

void RF_InitAntGpio(void)
{
    RF_SetGpioOutput(MODULE_GPIO_RX);
    RF_SetGpioOutput(MODULE_GPIO_TX);
    RF_SetGpioInput(MODULE_GPIO_CAD_IRQ);

    RF_WriteGpio(MODULE_GPIO_RX, 0);
    RF_WriteGpio(MODULE_GPIO_TX, 0);
    RF_WriteGpio(MODULE_GPIO_CAD_IRQ, 0);
}

void RF_TurnonTxAnt(void)
{
    RF_WriteGpio(MODULE_GPIO_RX, 0);
    RF_WriteGpio(MODULE_GPIO_TX, 1);
}

void RF_TurnonRxAnt(void)
{
    RF_WriteGpio(MODULE_GPIO_TX, 0);
    RF_WriteGpio(MODULE_GPIO_RX, 1);
}

void RF_ShutdownAnt(void)
{
    RF_WriteGpio(MODULE_GPIO_RX, 0);
    RF_WriteGpio(MODULE_GPIO_TX, 0);
}

void RF_InitTcxoGpio(void)
{
    //not used in this 
    // RF_SetGpioOutput(MODULE_GPIO_TCXO);
    // RF_WriteGpio(MODULE_GPIO_TCXO, 1);
}

void RF_TurnonTcxo(void)
{
    // RF_WriteGpio(MODULE_GPIO_TCXO, 1);
}

void RF_TurnoffTcxo(void)
{
    // RF_WriteGpio(MODULE_GPIO_TCXO, 0);
}

RF_Err_t RF_EnableAgc(bool NewState)
{
    if (NewState == AGC_OFF)
    {
        RF_ASSERT(RF_SetPageRegBits(RF_PAGE2, 0x06, RF_BIT0));
    }
    else
    {
        RF_ASSERT(RF_ResetPageRegBits(RF_PAGE2, 0x06, RF_BIT0));
    }

    return RF_OK;
}

/**
 * @brief configure AGC function
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_ConfigAgcParams(bool NewState)
{
    RF_ASSERT(RF_EnableAgc(NewState));

#ifdef REGION_CN470
    RF_ASSERT(RF_WritePageRegs(RF_PAGE2, 0x0A, (uint8_t *)g_low_freq_agc_cfg, 40));
#else
    RF_ASSERT(RF_WritePageRegs(RF_PAGE2, 0x0A, (uint8_t *)g_high_freq_agc_cfg, 40));
#endif

    RF_ASSERT(RF_WritePageReg(RF_PAGE2, 0x34, 0xEF));

    return RF_OK;
}

/**
 * @brief Set filtering bandwidth
 * @param[in] <lpf> filtering bandwidth
 * @return result
 */
RF_Err_t RF_SetLPF(uint32_t lpf)
{
    uint8_t val1, val2;
    val1 = RF_ReadPageReg(1, 0x43)&0x80;
    val2 = val1 + lpf;
    RF_WritePageReg(1, 0x43, val2);
    return RF_OK;
}

/**
 * @brief open all sf auto-search mode
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_SetAllSfSearchOn(void)
{
    uint8_t Temp;

    Temp = (RF_ReadPageReg(3, 0x12) | 0x01);
    RF_WritePageReg(3, 0x12, Temp);
    RF_WritePageReg(1, 0x25, 0x04);
    RF_WritePageReg(1, 0x2d, 0xff);

    return RF_OK;
}

/**
 * @brief close all sf auto-search mode
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_SetAllSfSearchOff(void)
{
    uint8_t Temp;

    Temp = (RF_ReadPageReg(3, 0x12) & 0xFE);

    RF_WritePageReg(3, 0x12, Temp);
    RF_WritePageReg(3, 0x14, 0);
    RF_WritePageReg(3, 0x13, 8);

    return RF_OK;
}

RF_Err_t RF_EnableDCDC(bool NewState)
{
    RF_ASSERT(RF_WritePageRegBits(RF_PAGE3, 0x24, NewState, RF_BIT3));
    return RF_OK;
}

/**
 * @brief Read info from info area
 * @param[in] <Addr>  register address
 *            <Pattern>  pattern to be written
 *            <InfoAddr> info address which to be read
 * @return Value read from info area
 */
uint8_t RF_ReadInfoByte(uint8_t Addr, uint16_t Pattern, uint8_t InfoAddr)
{
    uint8_t Value;
    uint8_t Buffer[3];
    uint16_t Timeout = 100;

    Buffer[0] = Pattern >> 8;
    Buffer[1] = Pattern & 0xFF;
    Buffer[2] = InfoAddr << 1;

    RF_WritePageRegs(RF_PAGE2, Addr, Buffer, sizeof(Buffer));
    do
    {
        if (RF_ReadPageReg(RF_PAGE0, 0x6C) & 0x80)
        {
            break;
        }
    } while (Timeout--); // wait for efuse register ready

    Value = RF_ReadPageReg(RF_PAGE2, Addr);

    return Value;
}

RF_Err_t RF_Calibrate(void)
{
    int i;
    uint8_t Temp[3] = {0};

    /* Temp[0]: efuse[0x1E] - DCDCIMAX
       Temp[1]: efuse[0x1F] - DCDCREF
       Temp[2]: efuse[0x20] - PABIAS */

    RF_ResetPageRegBits(RF_PAGE2, 0x3E, RF_BIT3); // Unlock info
    for (i = 0; i < sizeof(Temp); i++)
    {
        Temp[i] = RF_ReadInfoByte(0x3B, 0x5AA5, 0x1E + i);
    }

    if (RF_ReadInfoByte(0x3B, 0x5AA5, 0x1C) == 0x5A)
    {
        RF_WritePageReg(RF_PAGE2, 0x3D, 0xFD);

        if (Temp[2] != 0)
        {
            /* Write PABIAS */
            RF_WritePageReg(RF_PAGE0, 0x45, Temp[2]);
        }

        /* Write DCDCIMAX */
        RF_WritePageReg(RF_PAGE3, 0x1C, (0xC0 | (Temp[0] & 0x1F)));

        /* Write DCDCREF */
        RF_WritePageReg(RF_PAGE3, 0x1D, Temp[1]);
    }
    RF_ASSERT(RF_SetPageRegBits(RF_PAGE2, 0x3E, RF_BIT3)); // Lock info

    return RF_OK;
}

/**
 * @brief rf register configuration
 * @param[in] <none>
 * @return none
 */
RF_Err_t RF_WriteDefaultParams(void)
{
    for(unsigned int i = 0; i < sizeof(g_RfDefaultConfig)/sizeof(PAN_RegCfg_t); i++)
    {
        RF_WritePageReg(g_RfDefaultConfig[i].Page, g_RfDefaultConfig[i].Addr, g_RfDefaultConfig[i].Value);
    }

    return RF_OK;
}

/**
 * @brief set rf state
 * @param[in] <State>
 *            - RF_MODE_DEEP_SLEEP
 *            - RF_MODE_SLEEP
 *            - RF_MODE_STB1
 *            - RF_MODE_STB2
 *            - RF_MODE_STB3
 *            - RF_MODE_TX
 *            - RF_MODE_RX
 * @return result
 */
RF_Err_t RF_SetRfState(uint8_t State)
{
    RF_WriteReg(0x02, State);
    return RF_OK;
}

/**
 * @brief enable LDO PA
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_EnableLdoPa(void)
{    
    RF_SetPageRegBits(RF_PAGE0, 0x4F, RF_BIT3);
    return RF_OK;
}

/*
 * @brief disable LDO PA
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_DisableLdoPa(void)
{
    RF_ResetPageRegBits(RF_PAGE0, 0x4F, RF_BIT3);
    return RF_OK;
}

/**
 * @brief set rf tx mode
 * @param[in] <TxMode>
 *            - RF_TX_SINGLE
 *            - RF_TX_CONTINOUS
 * @return result
 */
RF_Err_t RF_SetTxMode(uint8_t TxMode)
{
    RF_WritePageRegBits(RF_PAGE3, 0x06, TxMode, RF_BIT2);
    return RF_OK;
}

/**
 * @brief rf enter single tx mode and send packet
 * @param[in] <Buffer> Buffer contain data to send
 * @param[in] <Size> the length of data to send
 * @return result
 */
RF_Err_t RF_TxSinglePkt(uint8_t *Buffer, uint8_t Size)
{
    RF_WriteReg(0x02, RF_MODE_STB3);
    RF_SetTxMode(RF_TX_SINGLE);
    RF_EnableLdoPa();
    RF_TurnonTxAnt();
    RF_WriteReg(0x02, RF_MODE_TX);
    RF_WritePageReg(RF_PAGE1, 0x0C, Size);
    RF_WritePageRegs(0, REG_FIFO_ACC_ADDR, Buffer, Size);
    return RF_OK;
}

/**
 * @brief set rf rx mode
 * @param[in] <RxMode>
 *            - RF_RX_SINGLE
 *            - RF_RX_SINGLE_TIMEOUT
 *            - RF_RX_CONTINOUS
 * @return result
 */
RF_Err_t RF_SetRxMode(uint8_t RxMode)
{
    RF_WritePageRegBits(RF_PAGE3, 0x06, RxMode, 0x03);
    return RF_OK;
}

/**
 * @brief rf enter rx continous mode to receive packet
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_EnterContinousRxState(void)
{
    RF_SetRfState(RF_MODE_STB3);
    RF_TurnonRxAnt();
    RF_DisableLdoPa();
    RF_SetRxMode(RF_RX_CONTINOUS);
    RF_SetRfState(RF_MODE_RX);
    return RF_OK;
}

/**
 * @brief rf enter rx single timeout mode to receive packet
 * @param [in] <TimeoutMs> timeout time in ms
 * @return result
 */
RF_Err_t RF_EnterSingleRxWithTimeout(uint16_t TimeoutMs)
{
    RF_SetRfState(RF_MODE_STB3);
    RF_TurnonRxAnt();
    RF_DisableLdoPa();
    RF_SetRxMode(RF_RX_SINGLE_TIMEOUT);
    RF_SetRxTimeout(TimeoutMs);
    RF_SetRfState(RF_MODE_RX);
    return RF_OK;
}

/**
 * @brief rf clear all irq
 * @param[in] <none>
 * @return result
 */
uint8_t RF_ClearIRQFlag(uint8_t flag)
{
    RF_WritePageRegUncheck(RF_PAGE0, 0x6C, flag);
    RF_WritePageRegUncheck(RF_PAGE0, 0x6C, flag);

    return RF_OK;
}

/**
 * @brief get irq status
 * @param[in] <none>
 * @return ira status
 */
uint8_t RF_GetIRQFlag(void)
{
    return (RF_ReadPageReg(RF_PAGE0, 0x6C) & 0x7F);
}

static const float SnrLog10Talbe[31] = 
{
    0.010f, 0.012f, 0.015f, 0.019f, 0.025f, 0.031f, 0.039f, 0.050f, 0.063f, 0.079f,
    0.100f, 0.125f, 0.158f, 0.199f, 0.251f, 0.316f, 0.398f, 0.501f, 0.630f, 0.794f,
    1.000f, 1.258f, 1.584f, 1.995f, 2.511f, 3.162f, 3.981f, 5.011f, 6.309f, 7.943f,
    10.00f
};

/**
 * @brief get snr Value
 * @param[in] <none>
 * @return snr
 */
float RF_GetSnr(void)
{
    float Snr = 0;
    uint8_t Temp[6];
    uint32_t NoiseStrength;
    uint32_t SingalStrength;

    RF_ReadPageRegs(RF_PAGE2, 0x71, &Temp[0], 3);   // NoiseStrength
    RF_ReadPageRegs(RF_PAGE1, 0x74, &Temp[3], 3);   // SingalStrength

    SingalStrength = (((uint32_t)Temp[5] << 16) | ((uint32_t)Temp[4] << 8) | Temp[3]);
    NoiseStrength = (((uint32_t)Temp[2] << 16) | ((uint32_t)Temp[1] << 8) | Temp[0]);

    if (NoiseStrength == 0)
    {
        NoiseStrength = 1;
    }

    float snr_val = ((float)SingalStrength / (float)((uint32_t)2 << g_CurrSF)) / (float)NoiseStrength;

    for (int i = 0; i < 31; i++)
    {
        if (snr_val <= SnrLog10Talbe[i])
        {
            Snr = (int)i - (int)20;
            break;
        }
    }

    return Snr;
}

/* when enable, the IQ data will be inverted */
void RF_SetInvertIQ(bool enable)
{
    if (enable)
    {
        /*
         * BIT6=0: invert rx IQ
         * BIT5=1: invert tx IQ
         */
        RF_WritePageRegBits(RF_PAGE1, 0x0E, 0x01, RF_BIT6 | RF_BIT5);
    }
    else
    {
        /*
         * BIT6=1: non-invert rx IQ
         * BIT5=0: non-invert tx IQ
         */
        RF_WritePageRegBits(RF_PAGE1, 0x0E, 0x02, RF_BIT6 | RF_BIT5);
    }
}

void RF_SetChipMode(RadioChipMode_t chip_mode)
{
    if(chip_mode == CHIPMODE_SX)
    {
        /* semtech mode */
        RF_WritePageRegBits(RF_PAGE1, 0x25, 4, 0xF0);    // 0: adapt to pan mode, 4: adapt to sx mode
        RF_WritePageRegBits(RF_PAGE1, 0x25, 1, RF_BIT3); // 0: adapt to pan mode, 1: adapt to sx mode
        RF_WritePageRegBits(RF_PAGE3, 0x12, 0, RF_BIT2); // 0: sx_mode, 1: pan_mode
        RF_WritePageRegBits(RF_PAGE3, 0x12, 0, RF_BIT4); // 0: sx_scr_mode, 1: pan_scr_mode
        RF_WritePageRegBits(RF_PAGE0, 0x58, 0, RF_BIT2); // disable crc interrupt for using sniffer mode
    }
    else
    {
        /* panchip mode */
        RF_WritePageRegBits(RF_PAGE1, 0x25, 0, 0xF0);    // 0: adapt to pan mode, 4: adapt to sx mode
        RF_WritePageRegBits(RF_PAGE1, 0x25, 0, RF_BIT3); // 0: adapt to pan mode, 1: adapt to sx mode
        RF_WritePageRegBits(RF_PAGE3, 0x12, 1, RF_BIT2); // 0: sx_mode, 1: pan_mode
        RF_WritePageRegBits(RF_PAGE3, 0x12, 1, RF_BIT4); // 0: sx_scr_mode, 1: pan_scr_mode
        RF_WritePageRegBits(RF_PAGE0, 0x58, 1, RF_BIT2); // enable crc interrupt
    }
}

/**
 * @brief set mapm mode enable
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_MapmEnable(void)
{
    RF_ASSERT(RF_SetPageRegBits(1, 0x38, RF_BIT0));

    return RF_OK;
}

/**
 * @brief set mapm mode disable
 * @param[in] <none>
 * @return result
 */
RF_Err_t RF_MapmDisable(void)
{
    RF_ASSERT(RF_ResetPageRegBits(1, 0x38, RF_BIT0));

    return RF_OK;
}

/**
 * @brief set mapm mask
 * @param[in] <mapm_val> mapm mask to set
 *			  MAPM_ON / MAPM_OFF
 * @return result
 */
RF_Err_t RF_SetMapmMask(uint8_t mapm_val)
{
    RF_ASSERT(RF_WritePageRegBits(0, 0x58, mapm_val, !RF_BIT6));

    return RF_OK;
}

/**
 * @brief get the number of fields
 * @param[in] <none>

 * @return <fn>
 */
uint8_t RF_GetMapmFieldNum(void)
{
    uint8_t reg_fn, fn_h, fn_l, fn;

    reg_fn = RF_ReadPageReg(1, 0x3d);
    fn_h = ((reg_fn >> 4) - 1) * 15;
    fn_l = (reg_fn & 0x0f) - 1;
    fn = fn_h + fn_l;

    return fn;
}

/**
 * @brief set the number of fields(range in 0x01~0xe0)
 * @param[in] <fn> the number of fields you want to set

 * @return result
 */
RF_Err_t RF_SetMapmFieldNum(uint8_t fn)
{
    uint8_t reg_fn, fn_h, fn_l;

    fn_h = fn / 15 + 1;
    fn_l = fn % 15 + 1;
    reg_fn = (fn_h << 4) + fn_l;
    RF_ASSERT(RF_WritePageReg(1, 0x3d, reg_fn));

    return RF_OK;
}

/**
 * @brief set the unit code word of the field counter represents several fields
 * @param[in] <fnm> the represents number you want to set
              0-- don't clone the field，every field is different, the total number of fields is fn*1
              1-- clone 1 field，every 2 fields are the same, the total number of fields is fn*2
              2-- clone 3 fields, every 4 fields are the same, the total number of fields is fn*4
              3-- clone 7 fields, every 8 fields are the same, the total number of fields is fn*8
 * @return result
 */
RF_Err_t RF_SetMapmFieldCloneNum(uint8_t fnm)
{
    RF_ASSERT(RF_WritePageRegBits(1, 0x37, fnm, RF_BIT7|RF_BIT6));

    return RF_OK;
}

/**
 * @brief set the last group function selection
 * @param[in] <group_fun_sel> The last group in the Field, its ADDR position function selection
 *             0:ordinary address      1:Field counter
 * @return result
 */
RF_Err_t RF_SetMapmLastAddrType(uint8_t gfs)
{
    RF_ASSERT(RF_WritePageRegBits(1, 0x38, gfs, RF_BIT1));

    return RF_OK;
}

/**
 * @brief set the number of groups in Field
 * @param[in] <gn> the number of groups

 * @return result
 */
RF_Err_t RF_SetMapmGroupNum(uint8_t gn)
{
    RF_ASSERT(RF_WritePageRegBits(1, 0x38, gn, RF_BIT3|RF_BIT2));

    return RF_OK;
}

/**
 * @brief set the number of Preambles in first groups
 * @param[in] <pgl> The numbers want set to Preambles in first groups(at least 10)

 * @return result
 */
RF_Err_t RF_SetMapmGroup1PreamLen(uint8_t pgl)
{
    RF_ASSERT(RF_WritePageReg(1, 0x3b, pgl));

    return RF_OK;
}

/**
 * @brief set the number of preambles for groups other than the first group
 * @param[in] <pgn>  the number of Preambles in other groups
 * @return result
 */
RF_Err_t RF_SetMapmGroupNPreamLen(uint8_t pgn)
{
    RF_ASSERT(RF_WritePageReg(1, 0x3c, pgn));

    return RF_OK;
}

/**
 * @brief set group address1 of mapm mode
 * @param[in] <addr> The value of group address1 you want to set

 * @return result
 */
RF_Err_t RF_SetMapmNormalPreamLen(uint16_t pn)
{
    RF_ASSERT(RF_WritePageRegBits(1, 0x39, (uint8_t)(pn >> 8), 0x0F));
    RF_ASSERT(RF_WritePageReg(1, 0x3A, (uint8_t)(pn)));

    return RF_OK;
}

/**
 * @brief set group address4 of mapm mode
 * @param[in] <addr> The value of group address4 you want to set

 * @return result
 */
RF_Err_t RF_SetMapmAddr(uint8_t addr_no, uint8_t addr)
{
    RF_ASSERT(RF_WritePageReg(1, 0x3e + addr_no, addr));

    return RF_OK;
}

/**
 * @brief calculate mapm preamble can sleep time
 * @param[in] <none>
 * @return The left time of preamble can sleep in ms
 */
uint32_t RF_GetMapmLeftTime(stc_mapm_cfg_t *mapm_cfg, uint32_t one_chirp_time)
{
    uint8_t fnm, gn, pgn, pg1, fn, pn;
    uint16_t one_field_chirp, chirp_num;
    uint32_t preamble_time;

    pn = mapm_cfg->pn;
    pgn = mapm_cfg->pgn;
    pg1 = mapm_cfg->pg1;
    gn = mapm_cfg->gn;
    fnm = mapm_cfg->fnm;
    fn = mapm_cfg->fn;
    one_field_chirp = pg1 + 2 + (pgn + 2) * gn;
    chirp_num = (1 << fnm) * fn * one_field_chirp + pn - one_field_chirp;
    preamble_time = one_chirp_time * chirp_num;

    return preamble_time / 1000;
}

/**
 * @brief set rf mapm mode on , rf will use mapm interruption
 * @param[in] <none>
 * @return result
 */
void RF_SetMapmOn(void)
{
    RF_MapmEnable();
    RF_SetMapmMask(MAPM_ON);
}

/**
 * @brief set mapm mode off
 * @param[in] <none>
 * @return result
 */
void RF_SetMapmOff(void)
{
    RF_MapmDisable();
    RF_SetMapmMask(MAPM_OFF);
}

/**
 * @brief configure relevant parameters used in mapm mode
 * @param[in] <p_mapm_cfg>
              <fn>set the number of fields(range in 0x01~0xe0)
              <field_num_mux> The unit code word of the Field counter represents several Fields
              <group_fun_sel> The last group in the Field, its ADDR position function selection
              0:ordinary address      1:Field counter
              <gn> register for configuring the number of groups in a Field
              0 1group\1 2group\2 3group\3 4group
              <pgl>set the number of Preambles in first groups>
              <pgn> the number of Preambles in other groups
              <pn> the number of chirps before syncword after all fields have been sent
 * @return result
 */
void RF_SetMapmParams(stc_mapm_cfg_t *p_mapm_cfg)
{
    RF_SetMapmFieldNum(p_mapm_cfg->fn);
    RF_SetMapmFieldCloneNum(p_mapm_cfg->fnm);
    RF_SetMapmLastAddrType(p_mapm_cfg->gfs);
    RF_SetMapmGroupNum(p_mapm_cfg->gn);
    RF_SetMapmGroup1PreamLen(p_mapm_cfg->pg1);
    RF_SetMapmGroupNPreamLen(p_mapm_cfg->pgn);
    RF_SetMapmNormalPreamLen(p_mapm_cfg->pn);
}

void RF_DelayUs(uint32_t us)
{
    delay_us(us);
}

void RF_DelayMs(uint32_t ms)
{
    delay1ms(ms);
}
