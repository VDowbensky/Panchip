/**
 * @file      pan_rf.h
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
 *
 * @addtogroup CHIRP
 *
 * @{
 *
 * @defgroup  CHIRP_RF_API Radio Driver API
 *
 * @{
 *
 */
#ifndef __PAN_RF_H__
#define __PAN_RF_H__

#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"

#define USE_RF_REG_CHECK     0
#define USE_RF_RST_GPIO      0

#if USE_RF_REG_CHECK
#define RF_ASSERT(fn)        \
    do                       \
    {                        \
        if (RF_OK != fn)     \
        {                    \
            return RF_FAIL;  \
        }                    \
    } while (0);
#else
#define RF_ASSERT(fn)  fn
#endif

#ifdef USB_DONGLE
#define MODULE_GPIO_TX          0
#define MODULE_GPIO_RX          10
#endif

#ifdef BLACK_NODE
#define MODULE_GPIO_TX          1
#define MODULE_GPIO_RX          3
#endif

#ifdef BLUE_NODE
#define MODULE_GPIO_TX          3
#define MODULE_GPIO_RX          1
#endif

#define MODULE_GPIO_TCXO        3
#define MODULE_GPIO_CAD_IRQ     11

/**
 * Radio complete Wake-up Time with margin for temperature compensation
 */
#define RADIO_WAKEUP_TIME               3 // [ms]

/**
 * Syncword for Private ChirpIot networks
 */
#define CHIRP_MAC_PRIVATE_SYNCWORD      0x12

/**
 * Syncword for Public ChirpIot networks
 */
#define CHIRP_MAC_PUBLIC_SYNCWORD       0x34

/* RF state */
#define RF_MODE_DEEP_SLEEP              0x00
#define RF_MODE_SLEEP                   0x01
#define RF_MODE_STB1                    0x02
#define RF_MODE_STB2                    0x03
#define RF_MODE_STB3                    0x04
#define RF_MODE_TX                      0x05
#define RF_MODE_RX                      0x06

/*IRQ BIT MASK*/
#define RF_IRQ_MAPM_DONE                0x40
#define RF_IRQ_RX_PLHD_DONE             0x10
#define RF_IRQ_RX_DONE                  0x08
#define RF_IRQ_CRC_ERR                  0x04
#define RF_IRQ_RX_TIMEOUT               0x02
#define RF_IRQ_TX_DONE                  0x01

/* RF Tx mode */
#define RF_TX_SINGLE                    0x00
#define RF_TX_CONTINOUS                 0x01

/* RF Rx mode */
#define RF_RX_SINGLE                    0x00
#define RF_RX_SINGLE_TIMEOUT            0x01
#define RF_RX_CONTINOUS                 0x02

/* RF power maximum ramp */
#define RF_MIN_RAMP                     1
#define RF_MAX_RAMP                     23

/* System control register */
#define REG_SYS_CTL                     0x00
#define REG_FIFO_ACC_ADDR               0x01

/* dcdc calibration select */
#define CALIBR_REF_CMP                  0x01
#define CALIBR_ZERO_CMP                 0x02
#define CALIBR_IMAX_CMP                 0x03

#define MODEM_MODE_NORMAL               0x00
#define MODEM_MODE_MULTI_SECTOR         0x01

#define LO_400M                         0x00
#define LO_800M                         0x01

#define AGC_ON                          0x01
#define AGC_OFF                         0x00

#define SPI_READ                        0x00
#define SPI_WRITE                       0x01

#define CRC_OFF                         0x00
#define CRC_ON                          0x01

#define LDR_OFF                         0x00
#define LDR_ON                          0x01

#define MAPM_ON                         0x00
#define MAPM_OFF                        0x01

#define CAD_DETECT_THRESHOLD_0A         0x0A
#define CAD_DETECT_THRESHOLD_10         0x10
#define CAD_DETECT_THRESHOLD_15         0x15
#define CAD_DETECT_THRESHOLD_20         0x20

#define CAD_DETECT_NUMBER_1             0x01
#define CAD_DETECT_NUMBER_2             0x02
#define CAD_DETECT_NUMBER_3             0x03

#define RF_BIT0                         0x01
#define RF_BIT1                         0x02
#define RF_BIT2                         0x04
#define RF_BIT3                         0x08
#define RF_BIT4                         0x10
#define RF_BIT5                         0x20
#define RF_BIT6                         0x40
#define RF_BIT7                         0x80

typedef enum
{
    RF_OK,   // Operate ok
    RF_FAIL, // Operate fail
} RF_Err_t;

typedef struct
{
    uint8_t ramp;    // ramp
    uint8_t pa_ldo;  // trim+ldo
    uint8_t pa_duty; // bandsel+duty+PAbias(0/1)
} power_ramp_cfg_t;

enum REF_CLK_SEL
{
    REF_CLK_32M,
    REF_CLK_16M
};

typedef enum
{
    RF_PAGE0 = 0,
    RF_PAGE1 = 1,
    RF_PAGE2 = 2,
    RF_PAGE3 = 3,
}RF_Page_t;

enum MAPM_LASTADDR_FUNC
{
    ORDINARY_ADDR,
    FIELD_COUNTER
};

#pragma pack(1)
typedef struct
{
    uint8_t page;
    uint8_t addr;
    uint8_t value;
}pan_reg_cfg_t;

typedef struct
{
    uint8_t Size;
    uint8_t Payload[255];
    uint8_t PlhdSize;
    uint8_t PlhdPayload[16];
    uint8_t MapmRxIndex;
    uint8_t MapmRxBuf[255];
    int8_t Rssi;
    float Snr;
}RF_RxPkt_t;
#pragma pack ()

/**
 * @brief Represents the operating mode the radio is actually running
 */
typedef enum
{
    MODE_SLEEP = 0x00, //! The radio is in sleep mode
    MODE_STDBY,        //! The radio is in standby mode with RC oscillator
    MODE_TX,           //! The radio is in transmit mode
    MODE_RX,           //! The radio is in receive mode
    MODE_CAD           //! The radio is in channel activity detection mode
} RadioOperatingModes_t;

/**
 * @brief Declares the power regulation used to power the device
 *
 * This command allows the user to specify if DC-DC or LDO is used for power regulation.
 * Using only LDO implies that the Rx or Tx current is doubled
 */
typedef enum
{
    USE_LDO = 0x00, // default
    USE_DCDC = 0x01,
} RadioRegulatorMode_t;

typedef struct
{
    uint32_t freq_low;
    uint32_t freq_high;
    uint8_t  vco_param;
    uint8_t  freq_factor;
    uint8_t  lo_param;
} RadioFreqTable_t;

typedef struct
{
    uint8_t vco;
    uint8_t lo;
    uint8_t fx[3];
}RadioFreqParam_t;

typedef struct
{
    uint8_t Page;
    uint8_t Addr;
    uint8_t Value;
} PAN_RegCfg_t;

typedef enum
{
    CHIPMODE_SX = 0,
    CHIPMODE_PAN = 1,
} RadioChipMode_t;

/**
 * @brief Represents the number of symbols to be used for channel activity detection operation
 */
typedef enum
{
    CHIRP_CAD_01_SYMBOL = 0x00,
    CHIRP_CAD_02_SYMBOL = 0x01,
    CHIRP_CAD_04_SYMBOL = 0x02,
    CHIRP_CAD_08_SYMBOL = 0x03,
    CHIRP_CAD_16_SYMBOL = 0x04,
} RadioChirpCadSymbols_t;

/**
 * @brief Represents the Channel Activity Detection actions after the CAD operation is finished
 */
typedef enum
{
    CHIRP_CAD_ONLY = 0x00,
    CHIRP_CAD_RX = 0x01,
    CHIRP_CAD_LBT = 0x10,
} RadioCadExitModes_t;

/**
 * @brief Represents the possible spreading factor values in ChirpIot packet types
 */
typedef enum
{
    CHIRP_SF5 = 0x05,
    CHIRP_SF6 = 0x06,
    CHIRP_SF7 = 0x07,
    CHIRP_SF8 = 0x08,
    CHIRP_SF9 = 0x09,
    CHIRP_SF10 = 0x0A,
    CHIRP_SF11 = 0x0B,
    CHIRP_SF12 = 0x0C,
} RadioChirpSpreadingFactors_t;

/**
 * @brief Represents the bandwidth values for ChirpIot packet type
 */
typedef enum
{
    CHIRP_BW_062 = 6,
    CHIRP_BW_125 = 7,
    CHIRP_BW_250 = 8,
    CHIRP_BW_500 = 9,
} RadioChirpBandwidths_t;

/**
 * @brief Represents the coding rate values for ChirpIot packet type
 */
typedef enum
{
    CHIRP_CR_4_5 = 0x01,
    CHIRP_CR_4_6 = 0x02,
    CHIRP_CR_4_7 = 0x03,
    CHIRP_CR_4_8 = 0x04,
} RadioChirpCodingRates_t;

/**
 *  \brief Radio GFSK packet length mode
 */
typedef enum
{
    RADIO_PACKET_FIXED_LENGTH = 0x00,    //!< The packet is known on both sides, no header included in the packet
    RADIO_PACKET_VARIABLE_LENGTH = 0x01, //!< The packet is on variable size, header included
} RadioPacketLengthModes_t;

/**
 * @brief Radio whitening mode activated or deactivated
 */
typedef enum
{
    RADIO_DC_FREE_OFF = 0x00,
    RADIO_DC_FREEWHITENING = 0x01,
} RadioDcFree_t;

/**
 * @brief Holds the Radio lengths mode for the ChirpIot packet type
 */
typedef enum
{
    CHIRP_PACKET_VARIABLE_LENGTH = 0x00, //!< The packet is on variable size, header included
    CHIRP_PACKET_FIXED_LENGTH = 0x01,    //!< The packet is known on both sides, no header included in the packet
    CHIRP_PACKET_EXPLICIT = CHIRP_PACKET_VARIABLE_LENGTH,
    CHIRP_PACKET_IMPLICIT = CHIRP_PACKET_FIXED_LENGTH,
} RadioChirpPacketLengthsMode_t;

/**
 * @brief Represents the CRC mode for ChirpIot packet type
 */
typedef enum
{
    CHIRP_CRC_ON = 0x01,  //!< CRC activated
    CHIRP_CRC_OFF = 0x00, //!< CRC not used
} RadioChirpCrcModes_t;

/**
 * @brief Represents the IQ mode for ChirpIot packet type
 */
typedef enum
{
    CHIRP_IQ_NORMAL = 0x00,
    CHIRP_IQ_INVERTED = 0x01,
} RadioChirpIQModes_t;

/**
 * @brief Represents the interruption masks available for the radio
 *
 * @remark Note that not all these interruptions are available for all packet types
 */
typedef enum
{
    IRQ_RADIO_NONE = 0x00,
    IRQ_TX_DONE = 0x01,
    IRQ_RX_TIMEOUT = 0x02,
    IRQ_CRC_ERROR = 0x04,
    IRQ_RX_DONE = 0x08,
    IRQ_RADIO_ALL = 0x7F,
} RadioIrqMasks_t;

/**
 * @brief The type describing the modulation parameters for every packet types
 */
typedef struct
{
    struct
    {
        struct
        {
            RadioChirpSpreadingFactors_t SpreadingFactor; //!< Spreading Factor for the ChirpIot modulation
            RadioChirpBandwidths_t Bandwidth;             //!< Bandwidth for the ChirpIot modulation
            RadioChirpCodingRates_t CodingRate;           //!< Coding rate for the ChirpIot modulation
            uint8_t LowDatarateOptimize;                  //!< Indicates if the modem uses the low datarate optimization
        } ChirpIot;
    } Params; //!< Holds the modulation parameters structure
} ModulationParams_t;

/**
 * @brief The type describing the packet parameters for every packet types
 */
typedef struct
{
    struct
    {
        struct
        {
            uint16_t PreambleLength;                 //!< The preamble length is the number of ChirpIot symbols in the preamble
            RadioChirpPacketLengthsMode_t HeaderType; //!< If the header is explicit, it will be transmitted in the ChirpIot packet. If the header is implicit, it will not be transmitted
            uint8_t PayloadLength;                   //!< Size of the payload in the ChirpIot packet
            RadioChirpCrcModes_t CrcMode;             //!< Size of CRC block in ChirpIot packet
            RadioChirpIQModes_t InvertIQ;             //!< Allows to swap IQ for ChirpIot packet
        } ChirpIot;
    } Params; //!< Holds the packet parameters structure
} PacketParams_t;

/**
 * @brief Represents the packet status for every packet type
 */
typedef struct
{
    struct
    {
        struct
        {
            int8_t RssiPkt; //!< The RSSI of the last packet
            int8_t SnrPkt;  //!< The SNR of the last packet
            int8_t SignalRssiPkt;
            uint32_t FreqError;
        } ChirpIot;
    } Params;
} PacketStatus_t;

/**
 * @brief Represents the Rx internal counters values when GFSK or ChirpIot packet type is used
 */
typedef struct
{
    uint16_t PacketReceived;
    uint16_t CrcOk;
    uint16_t LengthError;
} RxCounter_t;

/**
 * @brief Represents a sleep mode configuration
 */
typedef union
{
    struct
    {
        uint8_t WakeUpRTC : 1; //!< Get out of sleep mode if wakeup signal received from RTC
        uint8_t Reset : 1;
        uint8_t WarmStart : 1;
        uint8_t Reserved : 5;
    } Fields;
    uint8_t Value;
} SleepParams_t;

/**
 * Radio hardware and global parameters
 */
typedef struct RF_s
{
    PacketParams_t PacketParams;
    PacketStatus_t PacketStatus;
    ModulationParams_t ModulationParams;
}PAN_RF_t;

typedef struct
{
    uint8_t mapm_addr[4];
    uint8_t fn;
    uint8_t fnm;
    uint8_t gfs;
    uint8_t gn;
    uint8_t pg1;
    uint8_t pgn;
    uint16_t pn;
} stc_mapm_cfg_t;


extern volatile int g_RfDevIndex;
extern RF_RxPkt_t g_RfRecvPkt;

/**
 * Hardware IO IRQ callback function definition
 */
typedef void(DioIrqHandler)(void);

/**
 * RF_ definitions
 */

/**
 * ============================================================================
 * Public functions prototypes
 * ============================================================================
 */
 
/**
 * @brief Initializes the radio driver
 */
RF_Err_t RF_Init(void);

/**
 * @brief Gets the current Operation Mode of the Radio
 *
 * @retval      RadioOperatingModes_t last operating mode
 */
RadioOperatingModes_t RF_GetOperatingMode(void);

/**
 * @brief Sets the current Operation Mode of the Radio
 * 
 */
void RF_SetOperatingMode(RadioOperatingModes_t mode);

/**
 * @brief Wakeup the radio if it is in Sleep mode and check that Busy is low
 */
void RF_CheckDeviceReady(void);

/**
 * @brief Reads the payload received. If the received payload is longer
 * than maxSize, then the method returns 1 and do not set size and payload.
 *
 * @param [out] Buffer       A pointer to a buffer into which the payload will be copied
 * @param [out] Size         The size of the payload to be copied into the buffer. 
 * @retval  The size of received datas in rx fifo
 */
uint8_t RF_GetRecvPayload(uint8_t *Buffer, uint8_t Size);

/**
 * @brief Sets the Sync Word in the radio
 *
 * @param [in]  syncWord      SyncWord byte
 *
 * @retval      status        [0: OK, 1: NOK]
 */
void RF_SetSyncWord(uint8_t syncWord);

/**
 * @brief Sets the radio in sleep mode
 *
 * @param None
 */
void RF_SetSleep(void);

/**
 * @brief Sets the radio in configuration mode
 *
 * @param [in]  mode          The standby mode to put the radio into
 */
void RF_SetStandby(void);

/**
 * @brief Sets the radio in transmission mode
 *
 * @param [in]  timeout       Structure describing the transmission timeout value
 */
void RF_SetTx(uint8_t *buffer, uint8_t size);

/**
 * @brief Sets the radio in reception mode
 *
 * @param [in]  timeout       Structure describing the reception timeout value
 */
void RF_SetRx(uint32_t timeout);

/**
 * @brief Sets the radio in CAD mode
 */
void RF_SetCad(void);

/**
 * @brief Sets the radio in continuous wave transmission mode
 */
void RF_StartTxContinuousWave(void);

/**
 * @brief Sets the power regulators operating mode
 *
 * @param [in]  mode          [0: LDO, 1:DC_DC]
 */
void RF_SetRegulatorMode( RadioRegulatorMode_t mode );

/**
 * @brief Sets the RF frequency
 *
 * @param [in]  frequency     RF frequency [Hz]
 */
RF_Err_t RF_SetFreq(uint32_t frequency);

/**
 * @brief Set the modulation parameters
 *
 * @param [in]  modParams     A structure describing the modulation parameters
 */
void RF_SetModulationParams(ModulationParams_t *modParams);

/**
 * @brief Sets the packet parameters
 *
 * @param [in]  packetParams  A structure describing the packet parameters
 */
void RF_SetPacketParams(PacketParams_t *packetParams);

/**
 * @brief Sets the Channel Activity Detection (CAD) parameters
 *
 * @param [in]  cadSymbolNum   The number of symbol to use for CAD operations
 *                             [CHIRP_CAD_01_SYMBOL, CHIRP_CAD_02_SYMBOL,
 *                              CHIRP_CAD_04_SYMBOL, CHIRP_CAD_08_SYMBOL,
 *                              CHIRP_CAD_16_SYMBOL]
 * @param [in]  cadDetPeak     Limit for detection of SNR peak used in the CAD
 * @param [in]  cadDetMin      Set the minimum symbol recognition for CAD
 * @param [in]  cadExitMode    Operation to be done at the end of CAD action
 *                             [CHIRP_CAD_ONLY, CHIRP_CAD_RX, CHIRP_CAD_LBT]
 * @param [in]  cadTimeout     Defines the timeout value to abort the CAD activity
 */
void RF_SetCadParams( RadioChirpCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout );

/**
 * @brief Returns the instantaneous RSSI value for the last packet received
 *
 * @retval      rssiInst      Instantaneous RSSI
 */
int8_t RF_GetPktRssi(void);

/**
 * @brief Gets the last received packet buffer status
 *
 * @param None
 * @return payloadLength Last received packet payload length
 */
uint8_t RF_GetRecvPayloadLen(void);

/**
 * @brief Gets the last received packet payload length
 *
 * @param [out] pktStatus     A structure of packet status
 */
void RF_GetPacketStatus(PacketStatus_t *pktStatus);

/**
 * @brief Clears the IRQs
 *
 * @param [in]  irq           IRQ(s) to be cleared
 */
// void RF_ClearIrqStatus(uint8_t irq);

uint8_t __ctz(uint8_t val);
RF_Err_t RF_SetPage(uint8_t page);
RF_Err_t RF_WriteReg(uint8_t addr, uint8_t value);
uint8_t RF_ReadReg(uint8_t addr);
RF_Err_t RF_WritePageReg(uint8_t page, uint8_t addr, uint8_t value);
RF_Err_t RF_WritePageRegs(uint8_t page, uint8_t addr, uint8_t *buffer, uint8_t len);
uint8_t RF_ReadPageReg(uint8_t page, uint8_t addr);
RF_Err_t RF_ReadPageRegs(uint8_t page, uint8_t addr, uint8_t *buffer, uint8_t len);
RF_Err_t RF_SetPageRegBits(uint8_t page, uint8_t addr, uint8_t mask);
RF_Err_t RF_ResetPageRegBits(uint8_t page, uint8_t addr, uint8_t mask);
void RF_ReadFifo(uint8_t *buffer, int size);
void RF_WriteFifo(uint8_t *buffer, int size);
RF_Err_t RF_WritePageRegBits(uint8_t page, uint8_t addr, uint8_t val, uint8_t mask);
RF_Err_t RF_WritePageRegUncheck(uint8_t Page, uint8_t Addr, uint8_t Value);

RF_Err_t RF_SetGpioInput(uint8_t gpio_pin);
RF_Err_t RF_SetGpioOutput(uint8_t gpio_pin);
RF_Err_t RF_WriteGpio(uint8_t gpio_pin, uint8_t state);
bool RF_ReadGpio(uint8_t gpio_pin);
RF_Err_t RF_SetTxPower(uint8_t tx_power);
RF_Err_t RF_SetLoFreq(uint32_t lo);
bool RF_GetLDR(void);
RF_Err_t RF_SetLDR(uint8_t mode);
RF_Err_t RF_SetBW(uint8_t bw);
uint8_t RF_GetBandWidth(void);
RF_Err_t RF_SetSF(uint8_t sf);
uint8_t RF_getSF(void);
RF_Err_t RF_SetCrc(bool NewState);
uint8_t RF_GetCRC(void);
RF_Err_t RF_SetCR(uint8_t CodeRate);
uint8_t RF_GetCodeRate(void);
uint8_t RF_GetSyncWord(void);
RF_Err_t RF_SetPreamLen(uint16_t len);
uint16_t RF_GetPreamble(void);
int8_t RF_GetPktRssi(void);
int8_t RF_GetRealTimeRssi(void);
RF_Err_t RF_SetModemMode(uint8_t modem_mode);
RF_Err_t RF_SetRxTimeout(uint16_t timeout_ms);
RF_Err_t RF_ExitSleepState(void);
RF_Err_t RF_EnterSleepState(void);
RF_Err_t RF_SoftReset(void);
void RF_InitAntGpio(void);
void RF_TurnonTxAnt(void);
void RF_TurnonRxAnt(void);
void RF_ShutdownAnt(void);
void RF_InitTcxoGpio(void);
void RF_TurnonTcxo(void);
void RF_TurnoffTcxo(void);
RF_Err_t RF_EnableAgc(bool NewState);
RF_Err_t RF_ConfigAgcParams(bool NewState);
RF_Err_t RF_EnableDCDC(bool NewState);
RF_Err_t RF_SetAllSfSearchOn(void);
RF_Err_t RF_SetAllSfSearchOff(void);
uint8_t RF_ReadInfoByte(uint8_t reg_addr, uint16_t pattern, uint8_t efuse_addr);
RF_Err_t RF_Calibrate(void);
RF_Err_t RF_WriteDefaultParams(void);
RF_Err_t RF_SetRfState(uint8_t State);
RF_Err_t RF_EnableLdoPa(void);
RF_Err_t RF_DisableLdoPa(void);
RF_Err_t RF_SetTxMode(uint8_t TxMode);
RF_Err_t RF_TxSinglePkt(uint8_t *buf, uint8_t size);
RF_Err_t RF_SetRxMode(uint8_t RxMode);
RF_Err_t RF_EnterContinousRxState(void);
RF_Err_t RF_EnterSingleRxWithTimeout(uint16_t TimeoutMs);
uint8_t RF_ClearIRQFlag(uint8_t flags);
uint8_t RF_GetIRQFlag(void);
void RF_SetInvertIQ(bool enable);
void RF_StartTxContinuousWave( void );
void RF_StopTxContinuousWave( void );
void RF_SetChipMode(RadioChipMode_t chip_mode);
void RF_ReadSnrRegs(void);
void RF_CalculateSnr(void);
float RF_GetSnr(void);

RF_Err_t RF_SetLPF(uint32_t lpf);
uint32_t RF_SetImdFreq(uint32_t freq);

RF_Err_t RF_StartCad(uint8_t threshold, uint8_t chirps);
RF_Err_t RF_StopCad(void);

RF_Err_t RF_MapmEnable(void);
RF_Err_t RF_MapmDisable(void);
RF_Err_t RF_SetMapmMask(uint8_t mapm_val);
RF_Err_t RF_SetMapmAddr(uint8_t addr_no, uint8_t addr);
uint32_t RF_GetMapmLeftTime(stc_mapm_cfg_t *mapm_cfg, uint32_t one_chirp_time);
uint8_t RF_GetMapmFieldNum(void);
RF_Err_t RF_SetMapmFieldNum(uint8_t);
RF_Err_t RF_SetMapmFieldCloneNum(uint8_t fnm);
RF_Err_t RF_SetMapmLastAddrType(uint8_t gfs);
RF_Err_t RF_SetMapmGroupNum(uint8_t gn);
RF_Err_t RF_SetMapmGroup1PreamLen(uint8_t pgl);
RF_Err_t RF_SetMapmGroupNPreamLen(uint8_t pgn);
RF_Err_t RF_SetMapmNormalPreamLen(uint16_t pn);
void RF_SetMapmOn(void);
void RF_SetMapmOff(void);
void RF_SetMapmParams(stc_mapm_cfg_t *p_mapm_cfg);

void RF_DelayUs(uint32_t us);
void RF_DelayMs(uint32_t ms);

/** \} defgroup CHIRP_RF_ */
/** \} addtogroup LORA */

#endif // __RF__H__
