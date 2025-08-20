/**
 * @file      radio.h
 *
 * @brief     The header file of ChirpIot radio
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
 *
 * @addtogroup LORA
 *
 * @{
 *
 * @defgroup  LORA_RADIO
 *
 * @{
 */
#ifndef __RADIO_H__
#define __RADIO_H__

#include <stdint.h>
#include <stdbool.h>
#include "pan_rf.h"
/**
 * Radio driver internal state machine states definition
 */
typedef enum
{
    RF_IDLE = 0,   //!< The radio is idle
    RF_RX_RUNNING, //!< The radio is in reception state
    RF_TX_RUNNING, //!< The radio is in transmission state
    RF_CAD,        //!< The radio is doing channel activity detection
}RadioState_t;

/**
 * @brief Radio driver callback functions
 */
typedef struct
{
    /**
     * @brief  Tx Done callback prototype.
     */
    void (*TxDone)(void);
    /**
     * @brief  Tx Timeout callback prototype.
     */
    void (*TxTimeout)(void);
    /**
     * @brief Rx Done callback prototype.
     *
     * @param [IN] payload Received buffer pointer
     * @param [IN] size    Received buffer size
     * @param [IN] rssi    RSSI value computed while receiving the frame [dBm]
     * @param [IN] snr     Raw SNR value given by the radio hardware
     *                     FSK : N/A ( set to 0 )
     *                     ChirpIot: SNR value in dB
     */
    void (*RxDone)(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    /**
     * @brief  Rx Timeout callback prototype.
     */
    void (*RxTimeout)(void);
    /**
     * @brief Rx Error callback prototype.
     */
    void (*RxError)(void);
    /**
     * @brief  FHSS Change Channel callback prototype.
     *
     * @param [IN] currentChannel   Index number of the current channel
     */
    void (*FhssChangeChannel)(uint8_t currentChannel);

    /**
     * @brief CAD Done callback prototype.
     *
     * @param [IN] channelDetected    Channel Activity detected during the CAD
     */
    void (*CadDone)(bool channelActivityDetected);
} RadioEvents_t;

/**
 * @brief Radio driver definition
 */
struct Radio_s
{
    /**
     * @brief Initializes the radio
     *
     * @param [IN] events Structure containing the driver callback functions
     */
    int (*Init)(RadioEvents_t *events);
    /**
     * Return current radio status
     *
     * @param status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
     */
    RadioState_t (*GetStatus)(void);
    /**
     * @brief Configures the radio with the given modem
     *
     * @param [IN] modem Modem to be used [0: FSK, 1: ChirpIot]
     */
    void (*SetChannel)(uint32_t freq);
    /**
     * @brief Checks if the channel is free for the given time
     *
     * @param [IN] freq       Channel RF frequency
     * @param [IN] rssiThresh RSSI threshold
     * @param [IN] maxCarrierSenseTime Max time while the RSSI is measured
     *
     * @retval isFree         [true: Channel is free, false: Channel is not free]
     */
    bool (*IsChannelFree)(uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime);
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
    uint32_t (*Random)(void);
    /**
     * @brief Sets the reception parameters
     *
     * @param [IN] bw           Sets the bandwidth
     *                          [6: 62.5kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz]
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
     *                          [false: single mode with timeout, true: continuous mode]
     * @param [IN] Timeout      Sets the RxSingle timeout [ms]
     */
    void (*SetRxConfig)(uint8_t bw, uint8_t sf,
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
    void (*SetTxConfig)(int8_t txPower, uint8_t bw, 
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
    bool (*CheckRfFrequency)(uint32_t frequency);
    /**
     * @brief Computes the packet time on air in ms for the given payload
     *
     * @Remark Can only be called once SetRxConfig or SetTxConfig have been called
     *
     * @param [IN] pktLen     Packet payload length
     *
     * @retval airTime        Computed airTime (ms) for the given packet payload length
     */
    uint32_t (*TimeOnAir)(uint8_t pktLen);
    /**
     * @brief Sends the buffer of size. Prepares the packet to be sent and sets
     *        the radio in transmission
     *
     * @param [IN]: buffer     Buffer pointer
     * @param [IN]: size       Buffer size
     */
    void (*Send)(uint8_t *buffer, uint8_t size);
    /**
     * @brief Sets the radio in sleep mode
     */
    void (*Sleep)(void);
    /**
     * @brief Sets the radio in standby mode
     */
    void (*Standby)(void);
    /**
     * @brief Sets the radio in reception mode for the given time
     * @param [IN] timeout Reception timeout [ms]
     *                     [0: continuous, others timeout]
     */
    void (*Rx)(uint32_t timeout);
    /**
     * @brief Start a Channel Activity Detection
     */
    void (*StartCad)(uint8_t symbols);
    /**
     * @brief Sets the radio in continuous wave transmission mode
     *
     * @param [IN]: freq       Channel RF frequency
     * @param [IN]: power      Sets the output power [dBm]
     * @param [IN]: time       Transmission mode timeout [s]
     */
    void (*SetTxContinuousWave)(uint32_t freq, int8_t power, uint16_t time);
    /**
     * @brief Reads the current RSSI value
     *
     * @retval rssiValue Current RSSI value in [dBm]
     */
    int8_t (*Rssi)(void);
    /**
     * @brief Writes the radio register at the specified address
     *
     * @param [IN]: addr Register address
     * @param [IN]: data New register value
     */
    void (*Write)(uint8_t addr, uint8_t data);
    /**
     * @brief Reads the radio register at the specified address
     *
     * @param [IN]: addr Register address
     * @retval data Register value
     */
    uint8_t (*Read)(uint8_t addr);
    /**
     * @brief Writes multiple radio registers starting at address
     *
     * @param [IN] addr   First Radio register address
     * @param [IN] buffer Buffer containing the new register's values
     * @param [IN] size   Number of registers to be written
     */
    void (*WriteBuffer)(uint8_t addr, uint8_t *buffer, uint8_t size);
    /**
     * @brief Reads multiple radio registers starting at address
     *
     * @param [IN] addr First Radio register address
     * @param [OUT] buffer Buffer where to copy the registers data
     * @param [IN] size Number of registers to be read
     */
    void (*ReadBuffer)(uint8_t addr, uint8_t *buffer, uint8_t size);
    void (*SyncWord)(uint8_t SyncWord);
    /**
     * @brief Sets the network to public or private. Updates the sync byte.
     *
     * @remark Applies to ChirpIot modem only
     *
     * @param [IN] enable if true, it enables a public network
     */
    void (*SetPublicNetwork)(bool enable);
    /**
     * @brief Gets the time required for the board plus radio to get out of sleep.[ms]
     *
     * @retval time Radio plus board wakeup time in ms.
     */
    uint32_t (*GetWakeupTime)(void);
    /**
     * @brief Process radio irq
     */
    void (*IrqProcess)(void);
    /*
     * The next functions are available only on gPanRf radios.
     */
    /**
     * @brief Sets the radio in reception mode with Max LNA gain for the given time
     *
     * @remark Available on gPanRf radios only.
     *
     * @param [IN] timeout Reception timeout [ms]
     *                     [0: continuous, others timeout]
     */
    void (*RxBoosted)(uint32_t timeout);
    /**
     * @brief Sets the Rx duty cycle management parameters
     *
     * @remark Available on gPanRf radios only.
     *
     * @param [in]  rxTime        Structure describing reception timeout value
     * @param [in]  sleepTime     Structure describing sleep timeout value
     */
    void (*SetRxDutyCycle)(uint32_t rxTime, uint32_t sleepTime);
};

uint32_t RadioGetOneChirpTime(uint32_t bw, uint32_t sf);

/**
 * @brief Radio driver
 *
 * @remark This variable is defined and initialized in radio.c
 */
extern const struct Radio_s Radio;
extern volatile uint8_t g_RfIrqFlag;

/** \} defgroup CHIRPIOT_RADIO */
/** \} addtogroup CHIRPIOT */

#endif // __RADIO_H__
