/**************************************************************************//**
* @file     pan_hal_uart.h
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series UART (Universal Asynchronous Receiver-Transmitter) HAL header file.
* @note     Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __PAN_HAL_UART_H__
#define __PAN_HAL_UART_H__

#include "pan_hal_def.h"

/**
 * @brief UART HAL Interface
 * @defgroup uart_hal_interface Uart HAL Interface
 * @{
 */

typedef enum 
{
    HAL_UART_EVT_TX_FINISH       = 0UL, /**< @brief Event when the transmit operation is finished. */
    HAL_UART_EVT_RX_FINISH,             /**< @brief Event when the receive operation is finished. */
    HAL_UART_EVT_CONTI_RX_TIMEOUT,      /**< @brief Event when the continuous receive timeout. */
    HAL_UART_EVT_CONTI_RX_BUFF_FULL,    /**< @brief Event when the continuous receive buffer full. */
    HAL_UART_EVT_RX_ERR_PARITY,         /**< @brief Event when parity error occurs on the rx line. */
    HAL_UART_EVT_RX_ERR_FRAME,          /**< @brief Event when frame error occurs on the rx line. */
    HAL_UART_EVT_RX_ERR_OVERRUN,        /**< @brief Event when fifo overrun error occurs on the rx line. */
    HAL_UART_EVT_MAX                    /**< @brief Sentinel value. */
} HAL_UART_EventOpt;

typedef enum
{
    HAL_UART_INT_RECV_DATA_AVL  = 0x01, /**< @brief Receive data available interrupt. */
    HAL_UART_INT_THR_EMPTY      = 0x02, /**< @brief Transmitter holding register empty interrupt. */
    HAL_UART_INT_LINE_STATUS    = 0x04, /**< @brief Receiver line status interrupt. */
    HAL_UART_INT_MODEM_STATUS   = 0x08, /**< @brief Modem status interrupt. */
    HAL_UART_INT_ALL            = 0x0f, /**< @brief All interrupts. */
    HAL_UART_INT_DISABLE        = 0x00  /**< @brief Disable interrupts. */
} HAL_UART_IntModeOpt;

typedef enum 
{
    HAL_UART_TX_FIFO_EMPTY              = UART_TX_FIFO_EMPTY,
    HAL_UART_TX_FIFO_TWO_CHARS          = UART_TX_FIFO_TWO_CHARS,
    HAL_UART_TX_FIFO_QUARTER_FULL       = UART_TX_FIFO_QUARTER_FULL,
    HAL_UART_TX_FIFO_HALF_FULL          = UART_TX_FIFO_HALF_FULL
} HAL_UART_TxTrigOpt;

typedef enum
{
    HAL_UART_RX_FIFO_ONE_CHAR           = UART_RX_FIFO_ONE_CHAR,
    HAL_UART_RX_FIFO_QUARTER_FULL       = UART_RX_FIFO_QUARTER_FULL,
    HAL_UART_RX_FIFO_HALF_FULL          = UART_RX_FIFO_HALF_FULL,
    HAL_UART_RX_FIFO_TWO_LESS_THAN_FULL = UART_RX_FIFO_TWO_LESS_THAN_FULL
} HAL_UART_RxTrigOpt;

typedef enum
{
    HAL_UART_DMA_TX,                    /**< @brief DMA Direction for UART Tx. */
    HAL_UART_DMA_RX                     /**< @brief DMA Direction for UART Rx. */
} HAL_UART_DmaDirOpt;

/**
 * @brief UART Format Options Enum Definition.
 *
 * This enumeration lists the possible combinations of data bits, parity bits,
 * and stop bits for UART communication.
 */
typedef enum 
{
    HAL_UART_FMT_5_N_1       = 0x00,     // 5 data bits, no parity, 1 stop bit
    HAL_UART_FMT_5_N_1_5     = 0x04,     // 5 data bits, no parity, 1.5 stop bits
    HAL_UART_FMT_5_E_1       = 0x18,     // 5 data bits, even parity, 1 stop bit
    HAL_UART_FMT_5_E_1_5     = 0x1c,     // 5 data bits, even parity, 1.5 stop bits
    HAL_UART_FMT_5_O_1       = 0x08,     // 5 data bits, odd parity, 1 stop bit
    HAL_UART_FMT_5_O_1_5     = 0x0c,     // 5 data bits, odd parity, 1.5 stop bits
    HAL_UART_FMT_5_S_1       = 0x38,     // 5 data bits, space parity, 1 stop bit
    HAL_UART_FMT_5_S_1_5     = 0x3c,     // 5 data bits, space parity, 1.5 stop bits
    HAL_UART_FMT_5_M_1       = 0x28,     // 5 data bits, mark parity, 1 stop bit
    HAL_UART_FMT_5_M_1_5     = 0x2c,     // 5 data bits, mark parity, 1.5 stop bits
    HAL_UART_FMT_6_N_1       = 0x01,     // 6 data bits, no parity, 1 stop bit
    HAL_UART_FMT_6_N_2       = 0x05,     // 6 data bits, no parity, 2 stop bits
    HAL_UART_FMT_6_E_1       = 0x19,     // 6 data bits, even parity, 1 stop bit
    HAL_UART_FMT_6_E_2       = 0x1d,     // 6 data bits, even parity, 2 stop bits
    HAL_UART_FMT_6_O_1       = 0x09,     // 6 data bits, odd parity, 1 stop bit
    HAL_UART_FMT_6_O_2       = 0x0d,     // 6 data bits, odd parity, 2 stop bits
    HAL_UART_FMT_6_S_1       = 0x39,     // 6 data bits, space parity, 1 stop bit
    HAL_UART_FMT_6_S_2       = 0x3d,     // 6 data bits, space parity, 2 stop bits
    HAL_UART_FMT_6_M_1       = 0x29,     // 6 data bits, mark parity, 1 stop bit
    HAL_UART_FMT_6_M_2       = 0x2d,     // 6 data bits, mark parity, 2 stop bits
    HAL_UART_FMT_7_N_1       = 0x02,     // 7 data bits, no parity, 1 stop bit
    HAL_UART_FMT_7_N_2       = 0x06,     // 7 data bits, no parity, 2 stop bits
    HAL_UART_FMT_7_E_1       = 0x1a,     // 7 data bits, even parity, 1 stop bit
    HAL_UART_FMT_7_E_2       = 0x1e,     // 7 data bits, even parity, 2 stop bits
    HAL_UART_FMT_7_O_1       = 0x0a,     // 7 data bits, odd parity, 1 stop bit
    HAL_UART_FMT_7_O_2       = 0x0e,     // 7 data bits, odd parity, 2 stop bits
    HAL_UART_FMT_7_S_1       = 0x3a,     // 7 data bits, space parity, 1 stop bit
    HAL_UART_FMT_7_S_2       = 0x3e,     // 7 data bits, space parity, 2 stop bits
    HAL_UART_FMT_7_M_1       = 0x2a,     // 7 data bits, mark parity, 1 stop bit
    HAL_UART_FMT_7_M_2       = 0x2e,     // 7 data bits, mark parity, 2 stop bits
    HAL_UART_FMT_8_N_1       = 0x03,     // 8 data bits, no parity, 1 stop bit
    HAL_UART_FMT_8_N_2       = 0x07,     // 8 data bits, no parity, 2 stop bits
    HAL_UART_FMT_8_E_1       = 0x1b,     // 8 data bits, even parity, 1 stop bit
    HAL_UART_FMT_8_E_2       = 0x1f,     // 8 data bits, even parity, 2 stop bits
    HAL_UART_FMT_8_O_1       = 0x0b,     // 8 data bits, odd parity, 1 stop bit
    HAL_UART_FMT_8_O_2       = 0x0f,     // 8 data bits, odd parity, 2 stop bits
    HAL_UART_FMT_8_S_1       = 0x3b,     // 8 data bits, space parity, 1 stop bit
    HAL_UART_FMT_8_S_2       = 0x3f,     // 8 data bits, space parity, 2 stop bits
    HAL_UART_FMT_8_M_1       = 0x2b,     // 8 data bits, mark parity, 1 stop bit
    HAL_UART_FMT_8_M_2       = 0x2f,     // 8 data bits, mark parity, 2 stop bits
} HAL_UART_FormatOpt;

///**
//  * @brief HAL UART Event Control type definition
//  * @note  HAL UART Event Control type value aims to identify which type of Event
//  *        should be generated leading to call of the Event Callback.
//  *        This parameter can be a value of @ref HAL_UART_EvtCtrlOpt_Values :
//  *           HAL_UART_TX_FINISH_EVT_EN                 = BIT0,
//  *           HAL_UART_RX_FINISH_EVT_EN                 = BIT1,
//  *           HAL_UART_RX_ERROR_EVT_EN                  = BIT2,
//  */
//typedef uint8_t HAL_UART_EvtCtrlOpt;

///** @defgroup HAL_UART_EvtCtrlOpt_Values  UART Event Control Values
//  * @{
//  */
//#define HAL_UART_TX_FINISH_EVT_EN               BIT0             /*!< Specifies this control flag to enable UART Tx Finish Event */
//#define HAL_UART_RX_FINISH_EVT_EN               BIT1             /*!< Specifies this control flag to enable UART Rx Finish Event */
//#define HAL_UART_RX_ERROR_EVT_EN                BIT2             /*!< Specifies this control flag to enable UART Rx Error Event */
///**
//  * @}
//  */

//struct _HAL_UART_HandleTypeDef;

typedef struct _HAL_UART_HandleTypeDef HAL_UART_HandleTypeDef;

typedef void (*HAL_UART_CallbackFunc)(HAL_UART_HandleTypeDef*, HAL_UART_EventOpt, uint8_t*, uint16_t);

/**
 * @brief  UART Init Structure definition
 */
typedef struct
{
  uint32_t baudRate;        /**< @brief Set the initial baud rate. */
  HAL_UART_FormatOpt format;    /**< @brief Set the initial line control parameters. */
} HAL_UART_InitTypeDef;

/**
 * @brief UART Interrupt Options Structure Definition.
 *
 * This structure contains configuration and control information for
 * managing the interrupt behavior of a UART peripheral.
 */
typedef struct
{
//    HAL_UART_EvtCtrlOpt eventCtrl;      /**< @brief Specifies which interrupt event(s) should be generated for related callback function. */
    HAL_UART_TxTrigOpt txTrigLevel;     /**< @brief Specifies the Tx interrupt trigger level. */
    HAL_UART_RxTrigOpt rxTrigLevel;     /**< @brief Specifies the Rx interrupt trigger level. */
    IRQn_Type IRQn;                     /**< @brief Specifies the IRQ number associated with current UART module. */
    uint8_t IrqPriority;                /**< @brief Specifies priority of current UART IRQ, value can be 0 ~ 3. */
    __IO bool continuousRxMode;         /*!< Indicates UART Rx is in continuous interrupt transfer mode. */
    HAL_UART_CallbackFunc callbackFunc; /**< @brief Pointer to the callback function to be executed when the specified interrupt event occurs. */
} HAL_UART_IntTypeDef;

/**
 * @brief  UART handle Structure definition
 */
struct _HAL_UART_HandleTypeDef
{
    UART_T *pUartx;                     /*!< UART registers base address. */

    HAL_UART_InitTypeDef initObj;       /*!< UART communication parameters. */

    HAL_UART_IntTypeDef interruptObj;   /*!< UART interrupt communication parameters. */

    uint8_t *pTxBuffPtr;                /*!< Pointer to UART Tx transfer Buffer. */
    uint16_t txXferSize;                /*!< UART Tx Transfer size. */
    __IO uint16_t txXferCount;          /*!< UART Tx Transfer Counter. */

    uint8_t *pRxBuffPtr;                /*!< Pointer to UART Rx transfer Buffer. */
    uint16_t rxXferSize;                /*!< UART Rx Transfer size. */
    __IO uint16_t rxXferCount;          /*!< UART Rx Transfer Counter. */

    __IO bool isTxBusy;                 /*!< Indicates UART is busy transmitting data. */
    __IO bool isRxBusy;                 /*!< Indicates UART is busy receiving data. */

    uint8_t txDmaCh;                    /*!< DMA Channel for UART Tx */
    uint8_t rxDmaCh;                    /*!< DMA Channel for UART Rx */

    uint32_t dmaSrc;                    /**< DMA source channel for UART data transfer. */
    uint32_t dmaDst;                    /**< DMA destination channel for UART data transfer. */

//    __IO uint32_t errorCode;            /**< UART error code, indicating any encountered errors. */
};

#define PAN_HAL_UART_INST_COUNT     2
#define UART0_OBJ UART_Handle_Array[0]    /**< UART handle for UART0. */
#define UART1_OBJ UART_Handle_Array[1]    /**< UART handle for UART1. */

extern HAL_UART_HandleTypeDef UART_Handle_Array[PAN_HAL_UART_INST_COUNT];  /**< Array of UART handles for multiple UART instances. */

/**
 * @brief Initialize the UART peripheral.
 *
 * This function initializes the UART peripheral with the specified baud rate and format.
 * It configures the UART's line control parameters, including data bits, stop bits, and parity.
 *
 * @param[in] pUart Pointer to the UART handle structure.
 *
 * @return HAL Status
 */
HAL_Status HAL_UART_Init(HAL_UART_HandleTypeDef *pUart);

/**
 * @brief Initialize UART interrupt settings.
 *
 * This function configures the UART interrupt settings based on the provided
 * UART handle and the settings specified in the InterruptObj structure.
 *
 * @param[in] pUart Pointer to the UART handle structure.
 * @return HAL Status
 */
HAL_Status HAL_UART_Init_INT(HAL_UART_HandleTypeDef *pUart);

/**
 * @brief Initialize UART interrupt settings.
 *
 * This function configures the UART interrupt settings based on the provided
 * UART handle and the settings specified in the InterruptObj structure.
 *
 * @param[in] pUart Pointer to the UART handle structure.
 * @param[in] dmaDir DMA Direction for UART Tx or Rx.
 * @param[in] callback Pointer to the UART DMA Tx/Rx handle due to dmaDir.
 * @return HAL Status
 */
HAL_Status HAL_UART_Init_DMA(HAL_UART_HandleTypeDef *pUart, HAL_UART_DmaDirOpt dmaDir, UART_CallbackFunc callback);

/**
 * @brief Send data using UART.
 *
 * This function sends data over UART without interrupts. It waits until the UART's
 * transmitter holding register is empty before sending each byte.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to be transmitted.
 * @param[in] size      Size of the data to be transmitted.
 * @param[in] timeout   Timeout for sending data in millisecond, can be a number or HAL_TIME_FOREVER.
 * @return HAL Status
 */
HAL_Status HAL_UART_SendData(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size, uint32_t timeout);

/**
 * @brief Receive data using UART.
 *
 * This function receives data over UART without interrupts. It continuously polls
 * the UART's data register and stores received bytes in the specified buffer.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to store received data.
 * @param[in] size      Size of the data to be received.
 * @param[in] timeout   Timeout for receiving data in millisecond, can be a number or HAL_TIME_FOREVER.
 * @return HAL Status
 */
HAL_Status HAL_UART_ReceiveData(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size, uint32_t timeout);

/**
 * @brief Transmit data using UART with interrupt.
 *
 * This function initializes the UART transmission with interrupt and sets up the required parameters.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to be transmitted.
 * @param[in] size      Size of the data to be transmitted.
 * @return HAL Status
 */
HAL_Status HAL_UART_SendData_INT(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size);

/**
 * @brief Receive data using UART with interrupt.
 *
 * This function initializes the UART reception with interrupt and sets up the required parameters.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to store received data.
 * @param[in] size      Size of the data to be received.
 * @return HAL Status
 */
HAL_Status HAL_UART_ReceiveData_INT(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size);

/**
 * @brief Receive data continuously using UART with interrupt.
 *
 * This function initializes the UART continous reception with interrupt and sets up the required parameters.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to store received data.
 * @param[in] bufSize   Size of the data buffer for receiving.
 * @return HAL Status
 */
HAL_Status HAL_UART_ReceiveDataContinuously_INT(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t bufSize);

/**
 * @brief Abort transmitting data using UART with interrupt.
 *
 * This function aborts the UART transmission with interrupt.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @return HAL Status
 */
HAL_Status HAL_UART_SendDataAbort_INT(HAL_UART_HandleTypeDef *pUart);

/**
 * @brief Abort receiving data using UART with interrupt.
 *
 * This function aborts the UART receiving with interrupt.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @return HAL Status
 */
HAL_Status HAL_UART_ReceiveDataAbort_INT(HAL_UART_HandleTypeDef *pUart);

/**
 * @brief Transmit data using UART with DMA.
 *
 * This function initializes UART transmission with DMA and sets up the required parameters.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to be transmitted.
 * @param[in] size      Size of the data to be transmitted.
 * @return HAL Status
 */
HAL_Status HAL_UART_SendData_DMA(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size);

/**
 * @brief Receive data using UART with DMA.
 *
 * This function initializes UART reception with DMA and sets up the required parameters.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @param[in] pBuf      Pointer to the data buffer to store received data.
 * @param[in] size      Size of the data to be received.
 * @return HAL Status
 */
HAL_Status HAL_UART_ReceiveData_DMA(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size);

/**
 * @brief Abort transmitting data using UART with DMA.
 *
 * This function aborts the UART transmission with DMA.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @return HAL Status
 */
HAL_Status HAL_UART_SendDataAbort_DMA(HAL_UART_HandleTypeDef *pUart);

/**
 * @brief Abort receiving data using UART with DMA.
 *
 * This function aborts the UART receiving with DMA.
 *
 * @param[in] pUart     Pointer to the UART handle structure.
 * @return HAL Status
 */
HAL_Status HAL_UART_ReceiveDataAbort_DMA(HAL_UART_HandleTypeDef *pUart);

/** @} */ // end of group 
#endif
