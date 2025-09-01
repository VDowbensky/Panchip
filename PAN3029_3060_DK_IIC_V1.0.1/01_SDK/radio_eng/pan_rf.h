/**
 * @file      pan_rf.h
 * @brief     PAN3029/PAN3060 driver implementation
 * @version   V1.0.1
 * @date      2025-08-18
 * @copyright Panchip Microelectronics Co., Ltd. All rights reserved.
 * @code
 *              ____              ____ _     _
 *             |  _ \ __ _ _ __  / ___| |__ (_)_ __
 *             | |_) / _` | '_ \| |   | '_ \| | '_ \
 *             |  __/ (_| | | | | |___| | | | | |_) |
 *             |_|   \__,_|_| |_|\____|_| |_|_| .__/
 *                                            |_|
 *              (C)2009-2025 PanChip
 * @endcode
 * @author    PanChip
 * @addtogroup ChirpIOT
 * @{
 * @defgroup  PAN3029/3060 Radio Driver API
 * @{
 */
#ifndef __PAN_RF_H__
#define __PAN_RF_H__

#include <stdint.h>
#include <math.h>
#include "bsp.h"

#include "hal_gpio.h"
/* I2C bus state definitions */
typedef enum
{
  SI2C_READY = 0,
  SI2C_BUS_BUSY = 1,
  SI2C_BUS_ERROR = 2,
  SI2C_NACK = 3,
  SI2C_ACK = 4,
} I2C_Bus_State_t;

/* PAN3029 I2C device address macro definition */
#define I2C_DEV_ADDR 0x72

/* I2C ACK Maximum timeout count macro definition. The specific setting should be based on your hardware. */
#define ACK_TIMEOUT_DEF 1000

/* TODO: Implement according to your hardware.
* - Configure the I2C_SCK pin as a push-pull output.
* - Configure the I2C_SDA pin as a push-pull output or an input with a pull-up resistor.
* Note: When using I2C, the CS pin must be externally pulled high or left floating.
*/
#define I2C_SCK_LOW PORT_ResetBits(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN) /* Pull the I2C_SCK pin low. */
#define I2C_SCK_HIGH PORT_SetBits(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN) /* Pull the I2C_SCK pin high. */
#define I2C_SDA_LOW PORT_ResetBits(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN) /* Pull the I2C_SDA pin low */
#define I2C_SDA_HIGH PORT_SetBits(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN) /* Pull the I2C_SDA pin high */
#define I2C_SDA_STATUS PORT_GetBit(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN) /* Get the I2C_SDA level status */
#define I2C_SDA_OUTPUT gpioe_set_out(gpioe_get_ctrl_base(SOFT_I2C_SDA_PORT), get_bit_index(SOFT_I2C_SDA_PIN)) /* Set the I2C_SDA pin to output mode */
#define I2C_SDA_INPUT gpioe_set_in(gpioe_get_ctrl_base(SOFT_I2C_SDA_PORT), get_bit_index(SOFT_I2C_SDA_PIN)) /* Set the I2C_SDA pin to input mode */

/* Used for soft I2C delay implementation (adjust according to the hardware platform) and prevent compiler optimization */
static volatile uint32_t i2c_delay_counter = 0;
#define I2C_DELAY() do { \
i2c_delay_counter = 3; \
while(i2c_delay_counter--) { \
__NOP(); \
} \
} while(0) /* Use volatile variables to prevent compiler optimization. Changing the value of i2c_delay_counter can adjust the I2C rate */

/**TODO: Adjust according to your hardware:
* @brief Defines a macro to read the PAN3029/3060 interrupt pin level.
* @note This macro is used to read the interrupt pin level status of the PAN3029/3060. A high level indicates the interrupt is triggered, and a low level indicates the interrupt is not triggered.
*/
#define CHECK_RF_IRQ() PORT_GetBit(GPIO_PORT_RF_IRQ, GPIO_PIN_RF_IRQ)

/**TODO: Adjust according to your hardware:
* @brief Defines a macro to read the PAN3029/3060 CAD pin level.
* @note This macro is used to read the PAN3029/3060 CAD pin level status. A high level indicates the channel is busy, and a low level indicates the channel is idle.
* @note If the CAD function is not used, this macro does not apply. Simply leave the PAN3029/3060 CAD (GPIO11) pin pulled up or floating.
*/
#define CHECK_RF_CAD() PORT_GetBit(GPIO_PORT_RF_CAD, GPIO_PIN_RF_CAD)

/**TODO: Adjust according to your hardware:
* @brief Define the PAN3029/3060 reset pin macro
* @note RF_RESET_PIN_LOW() is used to set the reset pin low, resetting the chip
* @note RF_RESET_PIN_High() is used to release the reset control signal
* @note If a hardware reset is not required, this macro may not be used; simply pull up the PAN3029/3060 reset pin or leave it floating.
*/
#define RF_RESET_PIN_LOW() PORT_ResetBits(GPIO_PORT_RF_RST, GPIO_PIN_RF_RST) /* Set the reset pin to low */
#define RF_RESET_PIN_High() PORT_SetBits(GPIO_PORT_RF_RST, GPIO_PIN_RF_RST) /* Set the reset pin to high */

/**TODO: Adjust according to your hardware:
* @brief PAN3029/3060 GPIO Pin Definitions
*/
#define MODULE_GPIO_TX 0 /* PAN3029/3060 antenna transmit control pin. A high level turns the transmit antenna on, a low level turns it off. */
#define MODULE_GPIO_RX 10 /* PAN3029/3060 antenna receive control pin. A high level turns the receive antenna on, a low level turns it off. */
#define MODULE_GPIO_TCXO 3 /* PAN3029/3060 TCXO control pin, used to turn the TCXO power on and off. */
#define MODULE_GPIO_CAD_IRQ 11 /* PAN3029/3060 CAD interrupt pin. Outputs a high level when detecting a CAD signal, otherwise outputs a low level. */

/**
* @brief PAN3029/3060 frequency band definitions
*/
#define REGION_CN470_510 0x00 /* China 470-510MHz frequency band */
#define REGION_EU_863_870 0x01 /* Europe 863-870MHz frequency band */
#define REGION_US_902_928 0x02 /* US 902-928MHz frequency band */

#define REGION_DEFAULT REGION_CN470_510 /* Default frequency band is China 470-510MHz */

/**
* @brief Defines the default PAN3029/3060 parameters. Users can modify these RF parameters as needed.
*/
#define RF_FREQ_DEFAULT 490000000 /* Default frequency configuration */
#define RF_SF_DEFAULT RF_SF7 /* Default spreading factor configuration */
#define RF_BW_DEFAULT RF_BW_500K /* Default bandwidth configuration */
#define RF_CR_DEFAULT RF_CR_4_5 /* Default channel coding rate configuration */
#define RF_CRC_DEFAULT RF_CRC_ON /* Default CRC checksum configuration */
#define RF_LDR_DEFAULT RF_LDR_OFF /* Default low-rate optimization configuration */
#define RF_PREAMBLE_DEFAULT 8 /* Default preamble length configuration */
#define RF_IQ_INVERT_DEFAULT FALSE /* Default IQ modulation configuration */

/**
* @brief PAN3029/3060 external crystal and reset pin configuration
*/
#define USE_ACTIVE_CRYSTAL 0 /* Whether to use an external active crystal oscillator (0: not used, 1: used) */
#define USE_RF_RST_GPIO 0 /* Whether to use the MCU pin to control the RF reset function (0: not used, 1: used) */

/**
* @brief PAN3029/3060 register readback confirmation function switch
* @note This function is used to confirm the success of the write by reading the register value after writing to the register.
*/
#define USE_RF_REG_CHECK 0 /* Whether to use the register readback confirmation function (0: not used, 1: used) */
#if USE_RF_REG_CHECK
#define RF_ASSERT(fn) \
do \
{ \
if (RF_OK != fn) \
{\
return RF_FAIL; \
} \
} while (0);
#else
#define RF_ASSERT(fn) fn
#endif

/**
* @brief PAN3029/3060 interrupt flag definition
*/
#define RF_IRQ_TX_DONE 0x01 /* Transmit completion interrupt flag */
#define RF_IRQ_RX_TIMEOUT 0x02 /* Single receive timeout interrupt flag */
#define RF_IRQ_CRC_ERR 0x04 /* CRC error interrupt flag */
#define RF_IRQ_RX_DONE 0x08 /* Receive completion interrupt flag */
#define RF_IRQ_MAPM_DONE 0x40 /* MAPM interrupt flag */

/**
* @brief PAN3029/3060 modulation mode definition
*/
#define MODEM_MODE_NORMAL 0x00 /* Normal modulation mode */
#define MODEM_MODE_MULTI_SECTOR 0x01 /* Multi-segment modulation mode */

/**
* @brief PAN3029/3060 Sync Word Definitions
* @note: The PAN3029/3060 sync word is a single byte, ranging from 0x00 to 0xFF
*/
#define RF_MAC_PRIVATE_SYNCWORD 0x12 /* Private network sync word, default value is 0x12 */
#define RF_MAC_PUBLIC_SYNCWORD 0x34 /* Public network sync word */

/**
* @brief PAN3029/3060 Power Range Definitions
*/
#define RF_MIN_RAMP 1 /* Minimum power range */
#define RF_MAX_RAMP 22 /* Maximum power level */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/**
* @brief RF-related operation return value definitions
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
typedef enum
{
  RF_OK, //!< Operate ok
  RF_FAIL, //!< Operate fail
} RF_Err_t;

/**
* @brief PAN3029/3060 operating state definitions
*/
typedef enum
{
  RF_STATE_DEEPSLEEP = 0x00, //!< The radio is in deep sleep mode
  RF_STATE_SLEEP = 0x01, //!< The radio is in sleep mode
  RF_STATE_STB1 = 0x02, //!< The radio is in standby mode 1 
  RF_STATE_STB2 = 0x03, //!< The radio is in standby mode 2 
  RF_STATE_STB3 = 0x04, //!< The radio is in standby mode 3 
  RF_STATE_TX = 0x05, //!< The radio is in transmit mode 
  RF_STATE_RX = 0x06, //!< The radio is in receive mode
} RfOpState_t;

/** 
* @brief GPIO mode definition of PAN3029/3060 
* - GPIO_MODE_INPUT: GPIO input mode 
* - GPIO_MODE_OUTPUT: GPIO output mode 
*/
typedef enum
{ 
  GPIO_MODE_INPUT = 0x00, //!< GPIO input mode 
  GPIO_MODE_OUTPUT = 0x01, //!< GPIO output mode
} RfGpioMode_t;

/**
* @brief PAN3029/3060 Transmit Mode Definitions
* - RF_TX_MODE_SINGLE: Single transmit mode, used in most cases
* - RF_TX_MODE_CONTINOUS: Continuous transmit mode, commonly used for carrier signal transmission
*/
typedef enum
{
  RF_TX_MODE_SINGLE = 0x00, //!< Single transmission mode
  RF_TX_MODE_CONTINOUS = 0x01, //!< Continuous transmission mode
} RfTxMode_t;

/**
* @brief PAN3029/3060 Receive Mode Definitions
* - RF_RX_MODE_SINGLE: Single receive mode, automatically enters standby mode after receiving a packet of data. This mode is generally not used.
* - RF_RX_MODE_SINGLE_TIMEOUT: Single reception mode with timeout, automatically enters standby mode after timeout.
* - RF_RX_MODE_CONTINOUS: Continuous reception mode, continues receiving after receiving a packet.
*/
typedef enum
{
  RF_RX_MODE_SINGLE = 0x00, //!< Single reception mode
  RF_RX_MODE_SINGLE_TIMEOUT = 0x01, //!< Single reception mode with timeout
  RF_RX_MODE_CONTINOUS = 0x02, //!< Continuous reception mode
} RfRxMode_t;

/**
* @brief PAN3029/3060 power supply mode definition
* - USE_LDO: Use LDO for power supply; current is slightly higher, but the receiver sensitivity is slightly better.
* - USE_DCDC: Use DCDC for power supply; current is slightly lower, but the receiver sensitivity is slightly worse by 1-2dB.
*/
typedef enum
{
  USE_LDO = 0x00, //!< Use LDO for power regulation
  USE_DCDC = 0x01, //!< Use DCDC for power regulation
} RfRegulatorMode_t;

/**
* @brief PAN3029/3060 chip mode definitions
* - CHIPMODE_MODE0
* - CHIPMODE_MODE1
*/
typedef enum
{
  CHIPMODE_MODE0 = 0, //!< Mode 0
  CHIPMODE_MODE1 = 1, //!< Mode 1
} RfChipMode_t;

/**
* @brief CAD detection threshold configuration definitions
* - A smaller threshold value allows detection of weaker signals, but the false alarm rate increases.
* - A larger threshold value reduces the false alarm rate, but the detectable signal strength range decreases.
*/
typedef enum
{
  RF_CAD_THRESHOLD_0A = 0x0A,
  RF_CAD_THRESHOLD_10 = 0x10,
  RF_CAD_THRESHOLD_15 = 0x15,
  RF_CAD_THRESHOLD_20 = 0x20,
} RfCadThreshold_t;

/**
* @brief CAD detection symbol count configuration definition
*/
typedef enum
{
  RF_CAD_01_SYMBOL = 0x01, //!< Actual detection time may be greater than 1 symbol
  RF_CAD_02_SYMBOL = 0x02, //!< Actual detection time may be greater than 2 symbols
  RF_CAD_03_SYMBOL = 0x03, //!< Actual detection time may be greater than 3 symbols
  RF_CAD_04_SYMBOL = 0x04, //!< Actual detection time may be greater than 4 symbols
} RfCadSymbols_t;

/**
* @brief Represents the possible spreading factor values ​​in ChirpIot packet types 
*/
typedef enum
{ 
  RF_SF5 = 0x05, //!< 5 spreading factor 
  RF_SF6 = 0x06, //!< 6 spreading factor 
  RF_SF7 = 0x07, //!< 7 spreading factor 
  RF_SF8 = 0x08, //!< 8 spreading factor 
  RF_SF9 = 0x09, //!< 9 spreading factor 
  RF_SF10 = 0x0A, //!< 10 spreading factor 
  RF_SF11 = 0x0B, //!< 11 spreading factor 
  RF_SF12 = 0x0C, //!< 12 spreading factor
} RfSpreadFactor_t;

/** 
* @brief Represents the bandwidth values ​​for ChirpIot packet type 
*/
typedef enum
{ 
  RF_BW_062K = 6, //!< 62.5KHz bandwidth 
  RF_BW_125K = 7, //!< 125KHz bandwidth 
  RF_BW_250K = 8, //!< 250KHz bandwidth 
  RF_BW_500K = 9, //!< 500KHz bandwidth
} RfBandwidths_t;

/** 
* @brief Represents the coding rate values ​​for ChirpIot packet type 
* @note The coding rate is expressed as 4/x where x is the value below 
*/
typedef enum
{ 
  RF_CR_4_5 = 0x01, //!< 4/5 coding rate 
  RF_CR_4_6 = 0x02, //!< 4/6 coding rate 
  RF_CR_4_7 = 0x03, //!< 4/7 coding rate 
  RF_CR_4_8 = 0x04, //!< 4/8 coding rate
} RfCodingRates_t;

/**
* @brief CRC mode
* @note RF_CRC_ON: Enables CRC checking
* @note RF_CRC_OFF: Disables CRC checking, requiring the user to verify data integrity
*/
typedef enum
{ 
  RF_CRC_OFF = 0x00, //!< CRC not used 
  RF_CRC_ON = 0x01, //!< CRC activated
} RfCrcModes_t;

/** 
* @brief low rate optimization mode 
* @note RF_LDR_ON: Low data rate optimization activated 
* @note RF_LDR_OFF: Low data rate optimization not used 
*/
typedef enum
{ 
  RF_LDR_OFF = 0x00, //!< Low data rate optimization not used 
  RF_LDR_ON = 0x01, //!< Low data rate optimization activated
} RfLdr_t;

/** 
* @brief Represents the IQ mode for ChirpIot packet type 
*/
typedef enum
{ 
  RF_IQ_NORMAL = 0x00, //!< Normal IQ mode,default 
  RF_IQ_INVERTED = 0x01, //!< Inverted IQ mode
} RfIQModes_t;

typedef enum
{ 
  RF_MAPM_GRP_ADDR = 0x00, //!< Address group 
  RF_MAPM_GRP_COUNTER = 0x01, //!< Counter group
} RfMapmGrpType_t;

/** 
* @brief RF configuration structure of mapm 
* @member Addr: the address of mapm, which is 4 bytes 
* @member fn: the number of field before the standard preamble 
* @member fnm: the clone number of one field, which can be 0, 1, 2, 3, 
* fnm=0 indicates clone the field 1 times 
* fnm=1 indicates clone the field 2 times 
* fnm=2 indicates clone the field 4 times 
* fnm=3 indicates clone the field 8 times 
* @member gfs: the function select of last address, which can be 0 or 1 
* 0: the last address is ordinary address 
* 1: the last address is field counter 
* @member gn: the number of group in one filed, which can be 1, 2, 3, 4 
* gn=1 indicates 1 group(when gn=1, the field has only one group, fgs must be 0) 
* gn=2 indicates 2 groups 
* gn=3 indicates 3 groups 
*gn=4 indicates 4 groups 
* @member pg1: the number of preamble in the first group, which can be 8~255 
* pg1=8 indicates 8 chirps in the first group 
* pg1=255 indicates 255 chirps in the first group 
* @member pgn: the number of preamble in the other group, which can be 0~255 
* pgn=0 indicates 0 chirp in the other group, addr2~addr3 will be combined to addr1 
* pgn=255 indicates 255 chirps in the other group 
* @member pn: the number of preamble between fields and syncword, which can be 1~65535 
* @note the total numer of chirp before syncword: Pl=(pg1+1+pgn+1)*gn*fn*Fmux+pn 
* @note The mapm frame structure is as follows: 
* 1. The number of fields is fn, so N in the following figure equals fn; 
* 2. Preamble length in the figure is pn which is equal to the preamble length of normal 
* ChirpIot packet; 
* |<---------------------------------- Mapm Frame------------------------------------------------>| 
* | Field1 | Field2 | Field3 | ... | FieldN | Preamble | SyncWord | Header | Payload | CRC | 
* |--------------------------------------------------------------------------------------------------------| 
* 
* @note The mapm field structure is as follows(gn=4 && gfs=1): 
* 1. Each field has 4 groups, the preamble len of the fist group must be bigger than 8 
* and less than 255; the preamble len of the other groups can be 0~255; 
* 2. Each group carries 1byte payload, the type of first three groups is address, and 
* the type of last(fourth) group is counter which indicates the number of left fields; 
* 3. The coding rate of the address and counter is 4/8, so the number of chirps in the 
* first group is pg1 + 2, and the number of chirps in the other groups is pgn + 2; 
* |<---------------------------------- Field -------------------------------------------------->| 
* | Group1 | Group2 | Group3 | Group4 | 
* |--------------------------------------------------------------------------------------------------------| 
* | Preamble | Address1 | Preamble | Address2 | Preamble | Address3 | Preamble | Counter | 
* |--------------------------------------------------------------------------------------------------------| 
* 
* @note The mapm field structure is as follows(gn=3 && gfs=1): 
* 1. Each field has 4 groups, the preamble len of the fist group must be bigger than 8 
* and less than 255; the preamble len of the other groups can be 0~255; 
* 2. Each group carries 1byte payload, the type of first two groups is address, and 
* the type of last(third) group is counter which indicates the number of left fields; 
* 3. The coding rate of the address and counter is 4/8, so the number of chirps in the 
* first group is pg1 + 2, and the number of chirps in the other groups is pgn + 2; 
* |<---------------------------------- Field ----------------------->| 
* | Group1 | Group2 | Group3 | 
* |------------------------------------------------------------------| 
* | Preamble | Address1 | Preamble | Address2 | Preamble | Counter | 
* |------------------------------------------------------------------| 
*/
typedef struct
{ 
  uint8_t Addr[4]; //!< the address of mapm, which is 4 bytes
  uint8_t fn; //!< the number of field before the standard preamble 
  uint8_t fnm; //!< the clone number of one field, which can be 0, 1, 2, 3 
  uint8_t gn; //!< the number of group in one filed 
  uint8_t gfs; //!< the function select of last address 
  uint8_t pg1; //!< the number of preamble in the first group 
  uint8_t pgn; //!< the number of preamble in the other group 
  uint16_t pn; //!< the number of preamble between fields and syncword
} RF_MapmCfg_t;

/** 
* @brief Represents the possible operating states of the radio 
*/
typedef struct
{ 
  uint8_t RxLen; //!< Size of the payload in the ChirpIot packet 
  uint8_t RxBuf[255]; //!< Buffer to store the received payload
  uint8_t MapmRxIndex; //!< Index of the received payload in the ChirpIot packet
  uint8_t MapmRxBuf[16]; //!< Buffer to store the received payload in mapm mode
  int8_t Rssi; //!< The RSSI of the received packet
  int8_t Snr; //!< The SNR of the received packet
} RfRxPkt_t;

/**
* @brief PAN3029/3060 configuration parameter structure
* @note Convenient for users to quickly obtain previously set parameters
*/
typedef struct
{
  uint8_t TxPower; //!< The power level to be used for transmission
  uint32_t Frequency; //!< The frequency to be used for transmission and reception
  RfSpreadFactor_t SpreadingFactor; //!< Spreading Factor for the ChirpIot modulation 
  RfBandwidths_t Bandwidth; //!< Bandwidth for the ChirpIot modulation 
  RfCodingRates_t CodingRate; //!< Coding rate for the ChirpIot modulation 
  RfCrcModes_t CrcMode; //!< Size of CRC block in ChirpIot packet 
  RfIQModes_t InvertIQ; //!< Allows to swap IQ for ChirpIot packet 
  uint8_t SyncWord; //!< Sync word byte 
  uint8_t LowDatarateOptimize; //!< Indicates if the modem uses the low datarate optimization 
  uint16_t PreambleLen; //!< The preamble length is the number of ChirpIot symbols in the preamble 
  RfChipMode_t ChipMode; //!< The Chip for Communication with This Device 
  RfRegulatorMode_t RegulatorMode; //!< The regulator mode for the radio
} RfConfig_t;

/**
* @brief PAN3029/3060 receive data packet structure
* @note This structure is used to store received data packets, including data length, data buffer, SNR, and RSSI information.
*/
extern volatile RfRxPkt_t g_RfRxPkt;

/**
* RF definitions
*/

/**
* ===================================================================================
* Public functions prototypes
* ===============================================================================
*/

/**
* @brief I2C reset, used to clear error states on the I2C bus.
* @note The MCU may reset or experience an error during I2C bus operations, causing the I2C bus to enter an error state.
* @note This function resets the I2C bus to a normal state to prevent subsequent operation failures.
*/
void I2C_Reset(void);

/**
* @brief Microsecond delay function
* @param us Delay in microseconds
*/
void RF_DelayUs(uint32_t us);

/**
* @brief Millisecond delay function
* @param ms Delay in milliseconds
*/
void RF_DelayMs(uint32_t ms);

/**
* @brief Write a single byte to the specified register
* @param Addr Register address to be written
* @param Value Single byte of data to be written to the register
* @return RF_Err_t Returns the result of the operation
*/
RF_Err_t RF_WriteReg(uint8_t Addr, uint8_t Value);

/**
* @brief Read a single byte from the specified register
* @param Addr Register address to be read
* @return uint8_t Value read from the register
*/
uint8_t RF_ReadReg(uint8_t Addr);

/**
* @brief Continuously write multiple bytes to the specified register area.
* @param Addr Starting address of the register area to be written.
* @param Buffer Buffer pointer to the register to be written.
* @param Size Number of bytes to be written.
*/
void RF_WriteRegs(uint8_t Addr, uint8_t *Buffer, uint8_t Size);

/**
* @brief Continuously read multiple bytes from the specified register.
* @param Addr Register address to be read.
* @param Buffer Buffer pointer to store the read data.
* @param Size Number of bytes to be read.
*/
void RF_ReadRegs(uint8_t Addr, uint8_t *Buffer, uint8_t Size);

/**
* @brief Select a register page.
* @param Page Register page to be selected.
* @return RF_Err_t Returns the result of the operation.
*/
RF_Err_t RF_SetPage(uint8_t Page);

/**
* @brief Writes a single byte to a register on a specified page.
* @param Page Register page to be written
* @param Addr Register address to be written
* @param Value Single byte of data to be written to the register
* @return RF_Err_t Returns the result of the operation
*/
RF_Err_t RF_WritePageReg(uint8_t Page, uint8_t Addr, uint8_t Value);

/**
* @brief Writes multiple bytes to a register range on a specified page.
* @param Page Register page to be written
* @param Addr Register address to be written
* @param Buffer Buffer pointer to the register to be written
* @param Size Number of bytes to be written
*/
void RF_WritePageRegs(uint8_t Page, uint8_t Addr, uint8_t *Buffer, uint8_t Size);

/**
* @brief Reads a single byte from a register in the specified page.
* @param Page Register page to read
* @param Addr Register address to read
* @return uint8_t Value read from register
*/
uint8_t RF_ReadPageReg(uint8_t Page, uint8_t Addr);

/**
* @brief Reads multiple bytes from a register range in the specified page.
* @param Page Register page to read
* @param Addr Register address to read
* @param Buffer Buffer pointer to store read data
* @param Size Number of bytes to read
*/
void RF_ReadPageRegs(uint8_t Page, uint8_t Addr, uint8_t *Buffer, uint8_t Size);

/**
* @brief Sets a register bit in the specified page.
* @param Page Register page to set
* @param Addr Register address to set
* @param Mask Bit mask to set
* @return RF_Err_t Returns the operation result
*/
RF_Err_t RF_SetPageRegBits(uint8_t Page, uint8_t Addr, uint8_t Mask);

/**
* @brief Resets the register bits of the specified page
* @param Page Register page to be reset
* @param Addr Register address to be reset
* @param Mask Bit mask to be reset
* @return RF_Err_t Returns the operation result
*/
RF_Err_t RF_ResetPageRegBits(uint8_t Page, uint8_t Addr, uint8_t Mask);

/**
* @brief Writes to the register bits of the specified page
* @param Page Register page to be written
* @param Addr Register address to be written
* @param Value Value to be written
* @param Mask Bit mask to be written
* @return RF_Err_t Returns the operation result
*/
RF_Err_t RF_WritePageRegBits(uint8_t Page, uint8_t Addr, uint8_t Value, uint8_t Mask);

/**
* @brief Configure GPIO mode
* @param[in] <GpioPin> Pin number
* @param[in] <GpioMode> GPIO mode
* @return RF_Err_t Returns the operation result
*/
RF_Err_t RF_ConfigGpio(uint8_t GpioPin, uint8_t GpioMode);

/**
* @brief Control GPIO output level
* @param[in] <GpioPin> Pin number
* @param[in] <Level> GPIO level
* @return RF_Err_t Returns the operation result
*/
RF_Err_t RF_WriteGpioLevel(uint8_t GpioPin, uint8_t Level);

/**
* @brief Read the GPIO level
* @param[in] <GpioPin> Pin number
* @return Read GPIO level
*/
uint8_t RF_ReadGpioLevel(uint8_t GpioPin);

/**
* @brief Initialize the PAN3029/3060 antenna control GPIO
*/
void RF_InitAntGpio(void);

/**
* @brief Turn on the PAN3029/3060 transmitting antenna
*/
void RF_TurnonTxAnt(void);

/**
* @brief Turn on the PAN3029/3060 receiving antenna
*/
void RF_TurnonRxAnt(void);

/**
* @brief Turn off the PAN3029/3060 antenna
*/
void RF_ShutdownAnt(void);

/**
* @brief Initialize TCXO control GPIO
*/
void RF_InitTcxoGpio(void);

/**
* @brief Turn on TCXO power supply
*/
void RF_TurnonTcxo(void);

/**
* @brief Turn off TCXO power supply
*/
void RF_TurnoffTcxo(void);

/**
* @brief Turn on LDO PA
*/
void RF_TurnonLdoPA(void);

/**
* @brief Turn off LDO PA
*/
void RF_TurnoffLdoPA(void);

/**
* @brief Turn on internal and external PA
*/
void RF_TurnonPA(void);

/**
* @brief Turn off internal and external PA
*/
void RF_TurnoffPA(void);

/**
* @brief Set the chip mode
* @param[in] <ChipMode> Chip mode
*/
void RF_SetChipMode(RfChipMode_t ChipMode);

/**
* @brief Get the chip mode
* @return RfChipMode_t Current chip mode
*/
RfChipMode_t RF_GetChipMode(void);

/**
* @brief Initialize the RF transceiver to STB3 state after power-on
* @return Whether initialization was successful (0 for success, 1 for failure)
*/
RF_Err_t RF_Init(void);

/**
* @brief Configure the user parameters of the RF chip
*/
void RF_ConfigUserParams(void);

/**
* @brief Software reset of the RF chip control logic
*/
void RF_ResetLogic(void);

/**
* @brief Get the RF chip's operating state
* @return RfOpState_t Current operating state
*/
RfOpState_t RF_GetOperateState(void);

/**
* @brief Set the RF chip's operating state
* @param[in] <RfState> Operating state
*/
void RF_SetOperateState(RfOpState_t RfState);

/**
* @brief Set the RF chip's operating state
* @param[in] <RfState> Operating state
*/
void RF_SetRfState(uint8_t RfState);

/**
* @brief Enter deep sleep mode
* @note This function puts the RF chip into deep sleep mode, turning off the antenna power supply and TCXO power supply.
* @note This function sets the chip's operating state to MODE_DEEPSLEEP.
* @note After executing this function, if you need to wake up the RF chip, call RF_Init().
*/
void RF_EnterDeepsleepState(void);

/**
* @brief Enters sleep mode
* @note This function is used to put the RF chip into sleep mode, turning off the antenna power supply and TCXO power supply.
* @note This function sets the chip's operating state to MODE_SLEEP.
* @note After executing this function, to wake up the RF chip, you need to call the RF_ExitSleepState() function.
*/
void RF_EnterSleepState(void);

/**
* @brief Exits sleep mode
*/
void RF_ExitSleepState(void);

/**
* @brief Enters standby mode
*/
void RF_EnterStandbyState(void);

/**
* @brief Checks whether the RF chip is in sleep mode.
*/
void RF_CheckDeviceReady(void);

/**
* @brief Sets the chip's power mode.
* @param[in] <RegulatorMode> Power mode
*/
void RF_SetRegulatorMode(RfRegulatorMode_t RegulatorMode);

/**
* @brief Set the RF chip frequency
* @param[in] <Frequency> Frequency value
*/
RF_Err_t RF_SetFreq(uint32_t Frequency);

/**
* @brief Set IQ inversion
* @param[in] <enable> Enable or disable IQ inversion
*/
void RF_SetInvertIQ(bool NewState);

/**
* @brief Set the preamble length
* @param[in] <PreamLen> Preamble length value
*/
void RF_SetPreamLen(uint16_t PreamLen);

/**
* @brief Set the synchronization word
* @param[in] <syncWord> Synchronization word value
*/
void RF_SetSyncWord(uint8_t SyncWord);

/**
* @brief Sets transmit power
* @param[in] <TxPower> Transmit power level
*/
void RF_SetTxPower(uint8_t TxPower);

/**
* @brief Sets modulation bandwidth
* @param[in] <BandWidth> Modulation bandwidth value
* - RF_BW_062K / RF_BW_125K / RF_BW_250K / RF_BW_500K
* @note A larger modulation bandwidth increases the data rate but shortens the transmission distance.
* @note The modulation bandwidth range for the PAN3029 chip is RF_BW_062K - RF_BW_500K
* @note The modulation bandwidth range for the PAN3060 chip is RF_BW_125K - RF_BW_500K
*/
void RF_SetBW(uint8_t BandWidth);

/**
* @brief Set the spreading factor
* @param[in] <SpreadFactor> Spreading factor value
* - RF_SF5 / RF_SF6 / RF_SF7 / RF_SF8 / RF_SF9 / RF_SF10 / RF_SF11 / RF_SF12
* @note A larger spreading factor increases the transmission distance, but also reduces the data rate.
* @note The spreading factor range for the PAN3029 chip is RF_SF5 - RF_SF12
* @note The spreading factor range for the PAN3060 chip is RF_SF5 - RF_SF9
*/
void RF_SetSF(uint8_t SpreadFactor);

/**
* @brief Set the channel coding rate
* @param[in] <CodingRate> Channel coding rate value
*/
void RF_SetCR(uint8_t CodingRate);

/**
* @brief Set CRC checksum
* @param[in] <NewState> Enable or disable CRC checksum
*/
void RF_SetCRC(uint8_t CrcMode);

/**
* @brief Set low-rate mode
* @param[in] <LdrMode> Low-rate mode value
*/
void RF_SetLDR(uint8_t LdrMode);

/**
* @brief Set modem mode
* @param[in] <modem_mode>
*/
void RF_SetModemMode(uint8_t ModemMode);

/**
* @brief Set transmit mode
* @param Buffer Data buffer to be sent
* @param Size Number of data bytes to be sent
*/
void RF_SetTx(uint8_t *Buffer, uint8_t Size);

/**
* @brief Set receive mode
* @param TimeoutMs Receive timeout
*/
void RF_SetRx(uint32_t TimeoutMs);

/**
* @brief CAD function enable
* @param[in] <Threshold>
*/
void RF_StartCad(uint8_t Threshold, uint8_t Chirps);

/**
* @brief Set CAD detection threshold
* @param[in] <Threshold> CAD detection threshold
* - RF_CAD_THRESHOLD_0A
* - RF_CAD_THRESHOLD_10
* - RF_CAD_THRESHOLD_15
* - RF_CAD_THRESHOLD_20
*/
void RF_SetCadThreshold(uint8_t Threshold);

/**
* @brief Set the number of CAD detection symbols
* @param[in] <Chirps> CAD detection symbol number
* - RF_CAD_01_SYMBOL
* - RF_CAD_02_SYMBOL
* - RF_CAD_03_SYMBOL
* - RF_CAD_04_SYMBOL
*/
void RF_SetCadChirps(uint8_t Chirps);

/**
* @brief CAD function disable
*/
void RF_StopCad(void);

/**
* @brief Set transmit mode
* @param[in] <TxMode>
*/
void RF_SetTxMode(uint8_t TxMode);

/**
* @brief Send a single data packet
* @param[in] <Buffer> Data buffer to be sent
* @param[in] <Size> Number of data bytes to be sent
*/
void RF_TxSinglePkt(uint8_t *Buffer, uint8_t Size);

/**
* @brief Set the receive mode
* @param[in] <RxMode> Receive mode
*/
void RF_SetRxMode(uint8_t RxMode);

/**
* @brief Put the chip into continuous receive state
*/
void RF_EnterContinousRxState(void);

/**
* @brief Set the receive timeout
* @param[in] <TimeoutMs> Timeout period
*/
void RF_SetRxTimeout(uint16_t TimeoutMs);

/**
* @brief Enters single receive mode with timeout.
* @param[in] <TimeoutMs> Timeout duration
*/
void RF_EnterSingleRxWithTimeout(uint16_t TimeoutMs);

/**
* @brief Gets the received data length
* @return Received data length
*/
uint8_t RF_GetRxPayloadLen(void);

/**
* @brief Function used to get the received data length and content
* @param *Buffer Pointer to the data area to be received
* @return Received data length
*/
uint8_t RF_GetRecvPayload(uint8_t *Buffer);

/**
* @brief Gets the RSSI value of the received data packet
* @return RSSI value
*/
int8_t RF_GetPktRssi(void);

/**
* @brief Gets the real-time RSSI value
* @return RSSI value
*/
int8_t RF_GetRealTimeRssi(void);

/**
* @brief Get the SNR value of the received packet
* @return the SNR value
*/
int32_t RF_GetPktSnr(void);

/**
* @brief Get the interrupt flag
* @return the IRQ flag
*/
uint8_t RF_GetIRQFlag(void);

/**
* @brief Clear the interrupt flag
* @param[in] <IRQFlag> Interrupt flag
*/
void RF_ClrIRQFlag(uint8_t IRQFlag);

/**
* @brief Get the current frequency setting
*/
uint32_t RF_GetFreq(void);

/**
* @brief Get the IQ inversion value
*/
RfIQModes_t RF_GetInvertIQ(void);

/**
* @brief Get the current preamble length setting
*/
uint16_t RF_GetPreamLen(void);

/**
* @brief Get the current transmit power setting
*/
uint8_t RF_GetTxPower(void);

/**
* @brief Get the current modulation bandwidth setting
*/
uint8_t RF_GetBandWidth(void);

/**
* @brief Get the current spreading factor setting
*/
uint8_t RF_GetSF(void);

/**
* @brief Get the current CRC checksum setting
*/
uint8_t RF_GetCRC(void);

/**
* @brief Get the current coding rate setting
*/
uint8_t RF_GetCR(void);

/**
* @brief Get the current sync word setting
*/
uint8_t RF_GetSyncWord(void);

/**
* @brief Get the current transmit mode settings
*/
uint8_t RF_GetLDR(void);

/**
* @brief Get the time for a single symbol
* @param[in] <bw> Bandwidth
* - RF_BW_062K / RF_BW_125K / RF_BW_250K / RF_BW_500K
* @param[in] <sf> Spreading factor
* - RF_SF5 / RF_SF6 / RF_SF7 / RF_SF8 / RF_SF9 / RF_SF10 / RF_SF11 / RF_SF12
* @return Single symbol time, in µs
* @note This function is used to calculate the time for a single symbol
*/
uint32_t RF_GetOneSymbolTime(uint8_t bw, uint8_t sf);

/**
* @brief Calculate the time to send a packet
* @param[in] <Size> The size of the packet to be sent, in bytes
* @return The time to send the packet, in milliseconds
*/
uint32_t RF_GetTxTimeMs(uint8_t Size);

/**
* @brief Enable MPM mode
*/
void RF_EnableMapm(void);

/**
* @brief Disable MPM mode
*/
void RF_DisableMapm(void);

/**
* @brief Configure MPM-related parameters
* @param[in] <pMapmCfg>
*/
void RF_ConfigMapm(RF_MapmCfg_t *pMapmCfg);

/**
* @brief Set the group address in MPM mode
* @param[in] <MapmAddr> The MPM group address
* <AddrWidth> Address width, range 1~4
* @note The MapmAddr[0] of the receiver must be consistent with the MapmAddr[0] of the transmitter.
* Otherwise, the receiver will not trigger the mapm interrupt.
*/
void RF_SetMapmAddr(uint8_t *MapmAddr, uint8_t AddrWidth);

/**
* @brief Time (ms) to calculate a field
* @param[in] <pMapmCfg> mapm configuration parameter
* <SymbolTime> Single symbol (chirp) time
* @note The number of chirps in Group 1 is (pg1 + 2), where pg1 is the number of preambles in Group 1 and 2 is the number of chirps occupied by the address in Group 1.
* @note The number of chirps in other groups is (pgn + 2) * (gn - 1), where pgn is the number of preambles in the other single group,
* 2 is the number of chirps occupied by the address (or count value) in the other single group, and (gn - 1) is the number of groups remaining after removing Group 1.
*/
uint32_t RF_GetMapmOneFieldTime(RF_MapmCfg_t *pMapmCfg, uint32_t SymbolTime);

/**
* @brief Get the remaining Mapm time in Mapm mode
* @param[in] <pMapmCfg> Mapm configuration parameters
* @param[in] <SynbolTime> Single chirp time
* @return Remaining Mapm time, in milliseconds
*/
uint32_t RF_GetLeftMapmTime(RF_MapmCfg_t *pMapmCfg, uint32_t SynbolTime);

/**
* @brief Start transmitting continuous carrier
*/
void RF_StartTxContinuousWave(void);

/**
* @brief Stop transmitting continuous carrier
*/
void RF_StopTxContinuousWave(void);

/**
* @brief Handle RF interrupts
* @note This function is called from the interrupt service routine.
*/
void RF_IRQ_Process(void);

/** \} defgroup PAN3029/3060 */
/** \} addtogroup ChirpIOT */

#endif // __PAN_RF_H__
