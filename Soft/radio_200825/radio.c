/**
 * @file      radio.c
 *
 * @brief     Radio driver API definition
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
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "radio.h"
#include "pan_rf.h"
#include "hardware.h"
#include "utilities.h"

/**
 * @brief Initializes the radio
 *
 * @param [IN] events Structure containing the driver callback functions
 */
int RadioInit(RadioEvents_t *events);

/**
 * Return current radio status
 *
 * @param status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
 */
RadioState_t RadioGetStatus(void);

/**
 * @brief Sets the channel frequency
 *
 * @param [IN] freq         Channel RF frequency
 */
void RadioSetChannel(uint32_t freq);

/**
 * @brief Checks if the channel is free for the given time
 *
 * @param [IN] modem      Radio modem to be used [0: FSK, 1: ChirpIot]
 * @param [IN] freq       Channel RF frequency
 * @param [IN] rssiThresh RSSI threshold
 * @param [IN] maxCarrierSenseTime Max time while the RSSI is measured
 *
 * @retval isFree         [true: Channel is free, false: Channel is not free]
 */
bool RadioIsChannelFree(uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime);

/**
 * @brief Generates a 32 bits random value based on the RSSI readings
 *
 * @remark This function sets the radio in ChirpIot modem mode and disables
 *         all interrupts.
 *         After calling this function either Radio.SetRxConfig or
 *         Radio.SetTxConfig functions must be called.
 *
 * @retval randomValue    32 bits random value
 */
uint32_t RadioRandom(void);

/**
 * @brief Sets the reception parameters
 *
 * @param [IN] bw            Sets the bandwidth
 *                          [6: 62.5kHz, 7: 125 kHz,
 *                           8: 250 kHz, 9: 500 kHz]
 * @param [IN] sf           Sets the SpreadingFactor
 *                          [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
 * @param [IN] cr           Sets the coding rate
 *                          [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * @param [IN] preambleLen  Sets the Preamble length
 *                          Length in symbols (the hardware adds 4 more symbols)

 * @param [IN] fixLen       Fixed length packets 
 *                          [0: variable, 1: fixed(not supported)]
 * @param [IN] payloadLen   Sets payload length when fixed length is used(fixLen==1)
 * @param [IN] crc          Enables/Disables the CRC 
 *                          [0: OFF, 1: ON]
 * @param [IN] iqInverted   Inverts IQ signals 
 *                          [0: not inverted, 1: inverted]
 * @param [IN] rxMode       Sets the reception  mode
 *                          [false: single mode, true: continuous mode]
 * @param [IN] Timeout      Sets the RxSingle timeout in milliseconds
 */
void RadioSetRxConfig(uint8_t bw, uint8_t sf,
                      uint8_t cr, uint16_t preambleLen,
                      bool fixLen,uint8_t payloadLen, 
                      bool crc, bool iqInverted,
                      bool rxMode, uint16_t Timeout);

/**
 * @brief Sets the transmission parameters
 *
 * @param [IN] txPower      Sets the output power [dBm]
 * @param [IN] bw           Sets the bandwidth
 *                          [6: 62.5kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz]
 * @param [IN] sf           Sets the SpreadingFactor
 *                          [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
 * @param [IN] cr           Sets the coding rate[1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * @param [IN] preambleLen  Sets the preamble length
 *                          Length in symbols (the hardware adds 4 more symbols)
 * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * @param [IN] crc          Enables disables the CRC [0: OFF, 1: ON]
 * @param [IN] iqInverted   Inverts IQ signals [0: not inverted, 1: inverted]
 * @param [IN] timeout      Transmission timeout [ms]
 */
void RadioSetTxConfig(int8_t txPower, uint8_t bw, 
                      uint8_t sf, uint8_t cr,
                      uint16_t preambleLen, bool fixLen, 
                      bool crc, bool iqInverted, 
                      uint32_t timeout);

/**
 * @brief Checks if the given RF frequency is supported by the hardware
 *
 * @param [IN] frequency RF frequency to be checked
 * @retval isSupported [true: supported, false: unsupported]
 */
bool RadioCheckRfFrequency(uint32_t frequency);

/**
 * @brief Computes the packet time on air in ms for the given payload
 *
 * @Remark Can only be called once SetRxConfig or SetTxConfig have been called
 *
 * @param [IN] modem      Radio modem to be used [0: FSK, 1: ChirpIot]
 * @param [IN] pktLen     Packet payload length
 *
 * @retval airTime        Computed airTime (ms) for the given packet payload length
 */
uint32_t RadioTimeOnAir(uint8_t pktLen);

double RadioSymbTime(uint8_t bw, uint8_t sf);

/**
 * @brief Sends the buffer of size. Prepares the packet to be sent and sets
 *        the radio in transmission
 *
 * @param [IN]: buffer     Buffer pointer
 * @param [IN]: size       Buffer size
 */
void RadioSend(uint8_t *buffer, uint8_t size);

/**
 * @brief Sets the radio in sleep mode
 */
void RadioSleep(void);

/**
 * @brief Sets the radio in standby mode
 */
void RadioStandby(void);

/**
 * @brief Sets the radio in reception mode for the given time
 * @param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
void RadioRx(uint32_t timeout);

/**
 * @brief Start a Channel Activity Detection
 */
void RadioStartCad(uint8_t symbols);

/**
 * @brief Sets the radio in continuous wave transmission mode
 *
 * @param [IN]: freq       Channel RF frequency
 * @param [IN]: power      Sets the output power [dBm]
 * @param [IN]: time       Transmission mode timeout [s]
 */
void RadioSetTxContinuousWave(uint32_t freq, int8_t power, uint16_t time);

/**
 * @brief Reads the current RSSI value
 *
 * @retval rssiValue Current RSSI value in [dBm]
 */
int8_t RadioRssi(void);

/**
 * @brief Writes the radio register at the specified address
 *
 * @param [IN]: addr Register address
 * @param [IN]: data New register value
 */
void RadioWrite(uint8_t addr, uint8_t data);

/**
 * @brief Reads the radio register at the specified address
 *
 * @param [IN]: addr Register address
 * @retval data Register value
 */
uint8_t RadioRead(uint8_t addr);

/**
 * @brief Writes multiple radio registers starting at address
 *
 * @param [IN] addr   First Radio register address
 * @param [IN] buffer Buffer containing the new register's values
 * @param [IN] size   Number of registers to be written
 */
void RadioWriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size);

/**
 * @brief Reads multiple radio registers starting at address
 *
 * @param [IN] addr First Radio register address
 * @param [OUT] buffer Buffer where to copy the registers data
 * @param [IN] size Number of registers to be read
 */
void RadioReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size);

/**
 * @brief Sets the network to public or private. Updates the sync byte.
 *
 * @remark Applies to ChirpIot modem only
 *
 * @param [IN] enable if true, it enables a public network
 */
void RadioSetPublicNetwork(bool enable);

/**
 * @brief Gets the time required for the board plus radio to get out of sleep.[ms]
 *
 * @retval time Radio plus board wakeup time in ms.
 */
uint32_t RadioGetWakeupTime(void);

/**
 * @brief Process radio irq
 */
void RadioIrqProcess(void);

/**
 * @brief Sets the radio in reception mode with Max LNA gain for the given time
 * @param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
void RadioRxBoosted(uint32_t timeout);

/**
 * @brief Sets the Rx duty cycle management parameters
 *
 * @param [in]  rxTime        Structure describing reception timeout value
 * @param [in]  sleepTime     Structure describing sleep timeout value
 */
void RadioSetRxDutyCycle(uint32_t rxTime, uint32_t sleepTime);

/**
 * @brief Set synchro word in radio
 *
 * @param [IN] data  THe syncword
 */
void RadioSyncWord(uint8_t SyncWord);

/**
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    RadioInit,
    RadioGetStatus,
    RadioSetChannel,
    RadioIsChannelFree,
    RadioRandom,
    RadioSetRxConfig,
    RadioSetTxConfig,
    RadioCheckRfFrequency,
    RadioTimeOnAir,
    RadioSend,
    RadioSleep,
    RadioStandby,
    RadioRx,
    RadioStartCad,
    RadioSetTxContinuousWave,
    RadioRssi,
    RadioWrite,
    RadioRead,
    RadioWriteBuffer,
    RadioReadBuffer,
    RadioSyncWord,
    RadioSetPublicNetwork,
    RadioGetWakeupTime,
    RadioIrqProcess,
    // not available
    RadioRxBoosted,
    RadioSetRxDutyCycle
};

/*
 * Local types definition
 */

const RadioChirpBandwidths_t Bandwidths[] = { CHIRP_BW_125, CHIRP_BW_250, CHIRP_BW_500, CHIRP_BW_062 };

// /*                                        SF12    SF11    SF10    SF9    SF8    SF7 */
// static float RadioLoRaSymbTime[3][6] = {{ 32.768, 16.384, 8.192, 4.096, 2.048, 1.024 },  // 125 KHz
//                                         { 16.384, 8.192,  4.096, 2.048, 1.024, 0.512 },  // 250 KHz
//                                         { 8.192,  4.096,  2.048, 1.024, 0.512, 0.256 }}; // 500 KHz

uint8_t MaxPayloadLength = 0xFF;

uint32_t TxTimeout = 0;
uint32_t RxTimeout = 0;

bool RxContinuous = false;

PacketStatus_t RadioPktStatus;
uint8_t RadioRxPayload[255];

volatile uint8_t g_RfIrqFlag = 0;

/*
 * PAN3029/PAN3060 DIO IRQ callback functions prototype
 */

/**
 * @brief DIO 0 IRQ callback
 */
void RadioOnDioIrq( void );

/**
 * @brief Tx timeout timer callback
 */
void RadioOnTxTimeoutIrq( void );

/**
 * @brief Rx timeout timer callback
 */
void RadioOnRxTimeoutIrq( void );

/**
 * @brief Cad timeout timer callback
 */
void RadioOnCadTimeoutIrq( void );
/*
 * Private global variables
 */

/**
 * Holds the current network type for the radio
 */
typedef struct
{
    bool Previous;
    bool Current;
}RadioPublicNetwork_t;

static RadioPublicNetwork_t RadioPublicNetwork = { false };

/**
 * Radio callbacks variable
 */
static RadioEvents_t* RadioEvents;

/**
 * Public global variables
 */

/**
 * Radio hardware and global parameters
 */
PAN_RF_t gPanRf;

int RadioInit( RadioEvents_t *events )
{
    RadioEvents = events;

    RF_Err_t ret = RF_Init();
    if (ret != RF_OK)
    {
        // printf("rf init fail.\r\n");
        goto exit;
    }
    else
    {
        // printf("rf init ok.\r\n");
    }

    RF_SetStandby();
    RF_SetTxPower(18);
    RF_SetRegulatorMode(USE_LDO);
    RF_SetCrc(CRC_OFF);
    RF_SetSF(CHIRP_SF5);
    RF_SetCR(CHIRP_CR_4_5);
    RF_SetBW(CHIRP_BW_500);
    RF_SetTxMode(RF_TX_SINGLE);
    // RF_SetChipMode(CHIPMODE_SX);
    RF_SetChipMode(CHIPMODE_PAN);

    g_RfIrqFlag = false;

exit:
    return 0;
}

RadioState_t RadioGetStatus(void)
{
    switch (RF_GetOperatingMode())
    {
    case MODE_TX:
        return RF_TX_RUNNING;
    case MODE_RX:
        return RF_RX_RUNNING;
    case MODE_CAD:
        return RF_CAD;
    default:
        return RF_IDLE;
    }
}

/**
 * @brief Wakeup the radio if it is in Sleep mode
 * @param none
 * @return none
 */
void RadioCheckRFState(void)
{
    if (RF_GetOperatingMode() == MODE_SLEEP)
    {
        //RFBoardDisableIrq();
        RF_ExitSleepState();
        //RFBoardEnableIrq();
        RF_DelayMs(2);
        printf("[W]");
    }
}

void RadioSetChannel(uint32_t freq)
{
    RadioCheckRFState();  /* Wakeup the radio if it is in Sleep mode */

    RF_SetFreq(freq);
}

bool RadioIsChannelFree(uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime)
{
#if 0
    int16_t rssi = 0;
    bool status = true;
    uint32_t carrierSenseTime = 0;

    RadioSetChannel(freq);

    RadioRx(0);

    RF_DelayMs(1);

    carrierSenseTime = TimerGetCurrentTime();

    // Perform carrier sense for maxCarrierSenseTime
    while (TimerGetElapsedTime(carrierSenseTime) < maxCarrierSenseTime)
    {
        rssi = RadioRssi(modem);

        if (rssi > rssiThresh)
        {
            status = false;
            break;
        }
    }
    RadioSleep();

    return status;
#else
    return true;
#endif
}

uint32_t RadioRandom(void)
{
    uint8_t i;
    uint32_t rnd = 0;

    /*
     * Radio setup for random number generation
     */
    // Set radio in continuous reception
    RF_SetRx(0);

    RF_DelayMs(5);

    for (i = 0; i < 32; i++)
    {
        RF_DelayMs(2);

        uint8_t noise_rssi = RF_GetRealTimeRssi();
        // Unfiltered RSSI value reading. Only takes the LSB value
        rnd |= ((uint32_t)noise_rssi & 0x01) << i;
        //printf("%d ", noise_rssi);
    }

    rnd += rand1();

    // printf("\n");
    // printf("rnd: %x\n", (unsigned int)rnd);
    RadioSleep();

    return rnd;
}

/**
 * @brief Sets the reception parameters
 *
 * @param [IN] bw            Sets the bandwidth
 *                          [6: 62.5kHz, 7: 125 kHz,
 *                           8: 250 kHz, 9: 500 kHz]
 * @param [IN] sf           Sets the SpreadingFactor
 *                          [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
 * @param [IN] cr           Sets the coding rate
 *                          [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * @param [IN] preambleLen  Sets the Preamble length
 *                          Length in symbols (the hardware adds 4 more symbols)

 * @param [IN] fixLen       Fixed length packets 
 *                          [0: variable, 1: fixed(not supported)]
 * @param [IN] payloadLen   Sets payload length when fixed length is used(fixLen==1)
 * @param [IN] crc          Enables/Disables the CRC 
 *                          [0: OFF, 1: ON]
 * @param [IN] iqInverted   Inverts IQ signals 
 *                          [0: not inverted, 1: inverted]
 * @param [IN] rxMode       Sets the reception  mode
 *                          [false: single mode, true: continuous mode]
 * @param [IN] Timeout      Sets the RxSingle timeout in milliseconds
 */
void RadioSetRxConfig(uint8_t bw, uint8_t sf,
                      uint8_t cr, uint16_t preambleLen,
                      bool fixLen,uint8_t payloadLen, 
                      bool crc, bool iqInverted,
                      bool rxMode, uint16_t Timeout)
{
    RxContinuous = rxMode;

    if (rxMode == true)
    {
        Timeout = 0;
    }

    gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor = (RadioChirpSpreadingFactors_t)sf;
    gPanRf.ModulationParams.Params.ChirpIot.Bandwidth = (RadioChirpBandwidths_t)bw;
    gPanRf.ModulationParams.Params.ChirpIot.CodingRate = (RadioChirpCodingRates_t)cr;

    if (((bw == CHIRP_BW_062) && ((sf == CHIRP_SF11) || (sf == CHIRP_SF12))) ||
        ((bw == CHIRP_BW_125) && (sf == CHIRP_SF12)) || (RadioSymbTime(bw, sf) >= 16.38))
    {
        gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize = 0x01;
    }
    else
    {
        gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize = 0x00;
    }

    if ((gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor == CHIRP_SF5) ||
        (gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor == CHIRP_SF6))
    {
        if (preambleLen < 12)
        {
            gPanRf.PacketParams.Params.ChirpIot.PreambleLength = 12;
        }
        else
        {
            gPanRf.PacketParams.Params.ChirpIot.PreambleLength = preambleLen;
        }
    }
    else
    {
        gPanRf.PacketParams.Params.ChirpIot.PreambleLength = preambleLen;
    }

    gPanRf.PacketParams.Params.ChirpIot.HeaderType = (RadioChirpPacketLengthsMode_t)fixLen;
    gPanRf.PacketParams.Params.ChirpIot.PayloadLength = MaxPayloadLength;
    gPanRf.PacketParams.Params.ChirpIot.CrcMode = (RadioChirpCrcModes_t)crc;
    gPanRf.PacketParams.Params.ChirpIot.InvertIQ = (RadioChirpIQModes_t)iqInverted;
    
    RadioCheckRFState();  // Wakeup the radio if it is in Sleep mode
    RadioStandby();
    RF_SetModulationParams(&gPanRf.ModulationParams);
    RF_SetPacketParams(&gPanRf.PacketParams);

    // printf("bw:%d,sf:%d,cr:%d,ldr:%d\r\n", (int)bw, (int)sf, (int)cr, (int)gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize);

    // Timeout Max, Timeout handled directly in SetRx function
    RxTimeout = Timeout;
}

/**
 * @brief Sets the transmission parameters
 *
 * @param [IN] txPower      Sets the output power [dBm]
 * @param [IN] bw           Sets the bandwidth
 *                          [6: 62.5kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz]
 * @param [IN] sf           Sets the SpreadingFactor
 *                          [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
 * @param [IN] cr           Sets the coding rate[1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * @param [IN] preambleLen  Sets the preamble length
 *                          Length in symbols (the hardware adds 4 more symbols)
 * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * @param [IN] crc          Enables disables the CRC [0: OFF, 1: ON]
 * @param [IN] iqInverted   Inverts IQ signals [0: not inverted, 1: inverted]
 * @param [IN] timeout      Transmission timeout [ms]
 */
void RadioSetTxConfig(int8_t txPower, uint8_t bw, 
                      uint8_t sf, uint8_t cr,
                      uint16_t preambleLen, bool fixLen, 
                      bool crc, bool iqInverted, 
                      uint32_t timeout)
{
    gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor = (RadioChirpSpreadingFactors_t)sf;
    gPanRf.ModulationParams.Params.ChirpIot.Bandwidth = (RadioChirpBandwidths_t)bw;
    gPanRf.ModulationParams.Params.ChirpIot.CodingRate = (RadioChirpCodingRates_t)cr;

    if (((bw == CHIRP_BW_062) && ((sf == CHIRP_SF11) || (sf == CHIRP_SF12))) ||
        ((bw == CHIRP_BW_125) && (sf == CHIRP_SF12)) || (RadioSymbTime(bw, sf) >= 16.38))
    {
        gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize = 0x01;
    }
    else
    {
        gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize = 0x00;
    }

    if ((gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor == CHIRP_SF5) ||
        (gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor == CHIRP_SF6))
    {
        if (preambleLen < 12)
        {
            gPanRf.PacketParams.Params.ChirpIot.PreambleLength = 12;
        }
        else
        {
            gPanRf.PacketParams.Params.ChirpIot.PreambleLength = preambleLen;
        }
    }
    else
    {
        gPanRf.PacketParams.Params.ChirpIot.PreambleLength = preambleLen;
    }
    
    gPanRf.PacketParams.Params.ChirpIot.HeaderType = (RadioChirpPacketLengthsMode_t)fixLen;
    gPanRf.PacketParams.Params.ChirpIot.PayloadLength = MaxPayloadLength;
    gPanRf.PacketParams.Params.ChirpIot.CrcMode = (RadioChirpCrcModes_t)crc;
    gPanRf.PacketParams.Params.ChirpIot.InvertIQ = (RadioChirpIQModes_t)iqInverted;
    
    // printf("bw: %d, sf:%d, cr:%d, ldr:%d\r\n", (int)bw, (int)sf, (int)cr, (int)gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize);
    
    
    RadioCheckRFState();  // Wakeup the radio if it is in Sleep mode
    RadioStandby();
    RF_SetModulationParams(&gPanRf.ModulationParams);
    RF_SetPacketParams(&gPanRf.PacketParams);
    // printf("tx power:%d\r\n", power);

    RF_SetTxPower(txPower);
    
    TxTimeout = timeout; // unit is ms
}

bool RadioCheckRfFrequency(uint32_t frequency)
{
    return true;
}

double RadioSymbTime(uint8_t bw, uint8_t sf)
{
    double bw_khz = 0;
    switch (bw)
    {
    case CHIRP_BW_062:
        bw_khz = 62.5;
        break;
    case CHIRP_BW_125:
        bw_khz = 125;
        break;
    case CHIRP_BW_250:
        bw_khz = 250;
        break;
    case CHIRP_BW_500:
        bw_khz = 500;
        break;
    default:
        break;
    }

    return (1 << sf) / bw_khz;
}

/**
 * * @brief Calculate the time on air for a given packet length
 * @param pktLen: Packet payload length
 * @return airTime: Computed airTime (ms) for the given packet payload length
 */
uint32_t RadioTimeOnAir(uint8_t pktLen)
{
    uint32_t airTime;

    // double ts = RadioLoRaSymbTime[gPanRf.ModulationParams.Params.ChirpIot.Bandwidth - 4][12 - gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor];
    double ts = RadioSymbTime(gPanRf.ModulationParams.Params.ChirpIot.Bandwidth, gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor);
    // time of preamble
    double tPreamble = (gPanRf.PacketParams.Params.ChirpIot.PreambleLength + 4.25) * ts;
    // Symbol length of payload and time
    double tmp = ceil((8 * pktLen - 4 * gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor +
                       28 + 16 * gPanRf.PacketParams.Params.ChirpIot.CrcMode -
                       ((gPanRf.PacketParams.Params.ChirpIot.HeaderType == CHIRP_PACKET_FIXED_LENGTH) ? 20 : 0)) /
                      (double)(4 * (gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor -
                                    ((gPanRf.ModulationParams.Params.ChirpIot.LowDatarateOptimize > 0) ? 2 : 0)))) *
                      ((gPanRf.ModulationParams.Params.ChirpIot.CodingRate % 4) + 4);
    double nPayload = 8 + ((tmp > 0) ? tmp : 0);
    double tPayload = nPayload * ts;
    // Time on air
    double tOnAir = tPreamble + tPayload;
    // return milli seconds
    airTime = floor(tOnAir + 0.999);

    return airTime;
}

void RadioSend(uint8_t *buffer, uint8_t size)
{
    gPanRf.PacketParams.Params.ChirpIot.PayloadLength = size;

    RadioCheckRFState();  // Wakeup the radio if it is in Sleep mode

    RF_SetPacketParams(&gPanRf.PacketParams);
    RF_SetTx(buffer, size);
    
    // uint32_t timeOnAir = RadioTimeOnAir(size);
    // if(TxTimeout < timeOnAir)
    // {
    //     TxTimeout = timeOnAir + 100;
    // }
    
    // print_hex(buffer, size);
    
//    uint8_t rf_page_data[4][128];
//    for (int page = 0; page < 4; page++)
//    {
//        RF_ReadPageRegs((RF_Page_t)page, 0, &rf_page_data[page][0], 128);

//        printf("RF_PAGE%d data:\r\n", page);
//        for (int reg = 0; reg < 128; reg++)
//        {
//            printf("0x%02X ", rf_page_data[page][reg]);
//            if ((reg + 1) % 16 == 0)
//            {
//                printf("\r\n");
//            }
//        }
//        //printf("\r\n");
//    }
    
    // TimerSetValue(&TxTimeoutTimer, TxTimeout);
    // TimerStart(&TxTimeoutTimer);
}

void RadioSleep(void)
{
    if(g_RfIrqFlag != 0)
    {
        RF_ClearIRQFlag(g_RfIrqFlag); // clear irq flag if irq is pending
    }
    
    if(RF_GetOperatingMode() != MODE_SLEEP)
    {
        RF_SetSleep();
        RF_DelayMs(1);
    }
}

void RadioStandby(void)
{
    RF_SetStandby();
}

/**
 * @brief Sets the radio in rx mode for the given time
 * @param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
void RadioRx(uint32_t timeout)
{
    if (timeout != 0)
    {
        // TimerSetValue(&RxTimeoutTimer, timeout);
        // TimerStart(&RxTimeoutTimer);
    }
    
    RadioCheckRFState();  // Wakeup the radio if it is in Sleep mode

    if (RxContinuous == true)
    {
        RF_SetRx(0);
        printf("continuous rx\n");
    }
    else
    {
        RF_SetRx(RxTimeout);
        printf("single rx\n");
    }
}

void RadioRxBoosted(uint32_t timeout)
{
    if (timeout != 0)
    {
        // TimerSetValue(&RxTimeoutTimer, timeout);
        // TimerStart(&RxTimeoutTimer);
    }

    RadioCheckRFState();  // Wakeup the radio if it is in Sleep mode

    if (RxContinuous == true)
    {
        RF_EnterContinousRxState();
    }
    else
    {
        RF_EnterSingleRxWithTimeout(RxTimeout);
    }
}

void RadioSetRxDutyCycle(uint32_t rxTime, uint32_t sleepTime)
{
    // RF_SetRxDutyCycle( rxTime, sleepTime );
    /* pan3029 not support */
}

void RadioStartCad(uint8_t symbols)
{
#if 0
    uint8_t cadDetPeak = gPanRf.ModulationParams.Params.ChirpIot.SpreadingFactor + 13;
    uint8_t cadDetMin = 10;
    RadioChirpCadSymbols_t cadSymbolNum = CHIRP_CAD_16_SYMBOL;
    
    if(symbols>=16)
        cadSymbolNum = CHIRP_CAD_16_SYMBOL;
    else if(symbols>=8)
        cadSymbolNum = CHIRP_CAD_08_SYMBOL;
    else if(symbols>=4)
        cadSymbolNum = CHIRP_CAD_04_SYMBOL;
    else if(symbols>=2)
        cadSymbolNum = CHIRP_CAD_02_SYMBOL;
    else
        cadSymbolNum = CHIRP_CAD_01_SYMBOL;
    
    RF_SetDioIrqParams( IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
                           IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE );
    RF_SetCadParams( cadSymbolNum, cadDetPeak, cadDetMin, CHIRP_CAD_ONLY, 0 );
    
    RF_SetCad();
    
    TimerSetValue( &CadTimeoutTimer, 2000 );
    TimerStart( &CadTimeoutTimer );
#endif
}

#if 0
// todo : need to implement
void RadioTx(uint8_t *Buffer, uint8_t Size)
{
    RF_SetTx(Buffer, Size);
}
#endif

void RadioSetTxContinuousWave(uint32_t freq, int8_t power, uint16_t time)
{
    RadioCheckRFState();  // Wakeup the radio if it is in Sleep mode

    RF_SetFreq(freq);
    RF_SetTxPower(power);
    RF_StartTxContinuousWave();

    // TimerSetValue(&RxTimeoutTimer, time * 1e3);
    // TimerStart(&RxTimeoutTimer);
}

int8_t RadioRssi(void)
{
    return RF_GetRealTimeRssi();
}

void RadioWrite(uint8_t addr, uint8_t data)
{
    RF_WriteReg(addr, data);
}

uint8_t RadioRead(uint8_t addr)
{
    return RF_ReadReg(addr);
}

void RadioWriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    //RF_WriteRegs(addr, buffer, size);
}

void RadioReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    //RF_ReadRegs(addr, buffer, size);
}

void RadioSyncWord(uint8_t SyncWord)
{
    RF_SetSyncWord(SyncWord);
}

void RadioWriteFifo(uint8_t *buffer, uint8_t size)
{
    //RFBoardWriteBuffer(buffer, size);
}

void RadioReadFifo(uint8_t *buffer, uint8_t size)
{
    //RFBoardReadBuffer(buffer, size);
}

void RadioSetPublicNetwork(bool enable)
{
    if (enable == true)
    {
        // Change ChirpIot modem SyncWord
        RF_SetSyncWord(CHIRP_MAC_PUBLIC_SYNCWORD);
    }
    else
    {
        // Change ChirpIot modem SyncWord
        RF_SetSyncWord(CHIRP_MAC_PRIVATE_SYNCWORD);
    }
}

uint32_t RadioGetWakeupTime(void)
{
    // return RFBoardGetTcxoWakeupTime() + RADIO_WAKEUP_TIME;
    return RADIO_WAKEUP_TIME;
}

void RadioOnTxTimeoutIrq(void)
{
    if ((RadioEvents != NULL) && (RadioEvents->TxTimeout != NULL))
    {
        RadioEvents->TxTimeout();
    }
}

void RadioOnRxTimeoutIrq(void)
{
    if ((RadioEvents != NULL) && (RadioEvents->RxTimeout != NULL))
    {
        RadioEvents->RxTimeout();
    }
}

void RadioOnCadTimeoutIrq(void)
{
    RF_SetOperatingMode(MODE_SLEEP);
    if ((RadioEvents != NULL) && (RadioEvents->CadDone != NULL))
    {
        RadioEvents->CadDone(0);
    }
}

uint32_t RadioGetOneChirpTime(uint32_t bw, uint32_t sf)
{
    switch (bw)
    {
    case 6:
        bw = 62500;
        break;
    case 7:
        bw = 125000;
        break;
    case 8:
        bw = 250000;
        break;
    case 9:
        bw = 500000;
        break;
    default:
        return FAIL;
    }

    return (1000000.0f / bw) * (1 << sf);/*return us*/
}

void RadioIrqProcess( void )
{
#if 0
    if (CHECK_RF_IRQ())
    {
        //RFBoardDisableIrq();
        g_RfIrqFlag = RF_GetIRQFlag();
        // if (g_RfIrqFlag)
        // {
        //     printf("[0x%02X]",g_RfIrqFlag);
        // }
                
        if ((g_RfIrqFlag & IRQ_TX_DONE) == IRQ_TX_DONE)
        {
            TimerStop(&TxTimeoutTimer);
            RF_ShutdownAnt();
            RF_DisableLdoPa();
            RF_SetOperatingMode(MODE_STDBY);
            RF_SetRfState(RF_MODE_STB3);
            
            if(g_RfIrqFlag & IRQ_TX_DONE)
            {
                RF_ClearIRQFlag(IRQ_TX_DONE);
                g_RfIrqFlag &= ~IRQ_TX_DONE;
            }

            if ((RadioEvents != NULL) && (RadioEvents->TxDone != NULL))
            {
                RadioEvents->TxDone(); // OnRadioTxDone
            }

            printf("[T]");
        }

        if ((g_RfIrqFlag & IRQ_RX_DONE) == IRQ_RX_DONE)
        {
            uint8_t size = 0;

            TimerStop(&RxTimeoutTimer);

            RF_GetRecvPayload(RadioRxPayload, &size);
            RF_GetPacketStatus(&RadioPktStatus);

            if(g_RfIrqFlag & IRQ_RX_DONE)
            {
                RF_ClearIRQFlag(IRQ_RX_DONE);
                g_RfIrqFlag &= ~IRQ_RX_DONE;
            }
            
            RF_ShutdownAnt();
            RF_SetOperatingMode(MODE_STDBY);
            RF_SetRfState(RF_MODE_STB3);

            if ((RadioEvents != NULL) && (RadioEvents->RxDone != NULL) && ((g_RfIrqFlag & IRQ_CRC_ERROR) != IRQ_CRC_ERROR))
            {
                /* OnRadioRxDone */
                RadioEvents->RxDone(RadioRxPayload, size, RadioPktStatus.Params.ChirpIot.RssiPkt + RadioPktStatus.Params.ChirpIot.SnrPkt, RadioPktStatus.Params.ChirpIot.SnrPkt);
            }
            
            printf("size:%d,",size);
            print_hex(RadioRxPayload, size);
            printf("[R]");
        }

        if ((g_RfIrqFlag & IRQ_CRC_ERROR) == IRQ_CRC_ERROR)
        {
            if(g_RfIrqFlag & IRQ_RX_TIMEOUT)
            {
                RF_ClearIRQFlag(IRQ_CRC_ERROR);
                g_RfIrqFlag &= ~IRQ_CRC_ERROR;
            }
            
            RF_ShutdownAnt();
            RF_SetOperatingMode(MODE_STDBY);
            RF_SetRfState(RF_MODE_STB3);
            
            if ((RadioEvents != NULL) && (RadioEvents->RxError))
            {
                RadioEvents->RxError();
            }

            printf("[C]");
        }

        if ((g_RfIrqFlag & IRQ_RX_TIMEOUT) == IRQ_RX_TIMEOUT)
        {
            if (RF_GetOperatingMode() == MODE_RX)
            {
                TimerStop(&RxTimeoutTimer);

                if(g_RfIrqFlag & IRQ_RX_TIMEOUT)
                {
                    RF_ClearIRQFlag(IRQ_RX_TIMEOUT);
                    g_RfIrqFlag &= ~IRQ_RX_TIMEOUT;
                }
                
                RF_ShutdownAnt();
                RF_SetOperatingMode(MODE_STDBY);
                RF_SetRfState(RF_MODE_STB3);
                
                if ((RadioEvents != NULL) && (RadioEvents->RxTimeout != NULL))
                {
                    RadioEvents->RxTimeout(); // OnRadioRxTimeout
                }
            }

            printf("[O]");
        }

#if 0   // todo : need to implement
        if( ( g_RfIrqFlag & IRQ_CAD_DONE ) == IRQ_CAD_DONE )
        {
            TimerStop( &CadTimeoutTimer );
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            RF_SetOperatingMode( MODE_STDBY );
            if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
            {
                RadioEvents->CadDone( ( ( g_RfIrqFlag & IRQ_CAD_ACTIVITY_DETECTED ) == IRQ_CAD_ACTIVITY_DETECTED ) );
            }
        }
#endif

        if(g_RfIrqFlag)
        {
            RF_ClearIRQFlag(g_RfIrqFlag);
        }

        //RFBoardEnableIrq();
    }
    #endif
}
