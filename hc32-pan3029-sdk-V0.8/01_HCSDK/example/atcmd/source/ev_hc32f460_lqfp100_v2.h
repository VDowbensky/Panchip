/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file ev_hc32f460_lqfp100_v2.h
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#ifndef __EV_HC32F460_LQFP100_V2_H__
#define __EV_HC32F460_LQFP100_V2_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_common.h"
#include "hc32f460_utility.h"
#include "hc32f460_clk.h"
#include "hc32f460_efm.h"
#include "hc32f460_gpio.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "hc32f460_pwc.h"
#include "hc32f460_sram.h"
#include "hc32f460_keyscan.h"


#if (BSP_EV_HC32F460_LQFP100_V2 == BSP_EV_HC32F460)

#define BSP_PRINTF_DEVICE       (M4_USART1)
#define BSP_PRINTF_BAUDRATE     (115200)

#define BSP_PRINTF_PORT         (PortA)
#define BSP_PRINTF_PIN          (Pin12)
#define BSP_PRINTF_PORT_FUNC    (Func_Usart1_Tx)

#define TX_RX_SELECT            PORT_GetBit(PortB, Pin10)
#define TX_SELECT               GPIO_PIN_SET
#define RX_SELECT               GPIO_PIN_RESET

#define CARRIER_TEST_SELECT     PORT_GetBit(PortB, Pin02)
#define TESTD_ENABLE            GPIO_PIN_RESET
#define TESTD_DISBLE            GPIO_PIN_SET

void BSP_CLK_Init(void);
void BSP_KEY_Init(void);
void BSP_A15B03_Init(void);
void Spi_Config(void);
void BSP_LED_Init(void);
void RF_IRQ_Init(void);
void BSP_GPIO_Init(void);
void BSP_LED_Toggle(void);
void UsartRxErrProcess(void);
void Uart_Config(void);
void BSP_PRINTF_PortInit(void);
void Timer4_Init(void);
void timer6_open_ms(uint32_t ms);
void Timer6_Cfg(void);
void uart_rx_fifo_flush(void);
int uart_fifo_read(uint8_t *buf, int bufLen);
#endif /* BSP_EV_HC32F460_LQFP100_V2 */

#ifdef __cplusplus
}
#endif

#endif /* __EV_HC32F460_LQFP100_V2_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
