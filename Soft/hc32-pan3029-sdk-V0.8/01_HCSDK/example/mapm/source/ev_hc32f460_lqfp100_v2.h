/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.c
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

#define BSP_KEY_ROW_NUM         (6U)

#define BSP_KEYIN0_PORT         (PortB)
#define BSP_KEYIN0_PIN          (Pin08)

#define BSP_KEYIN1_PORT         (PortB)
#define BSP_KEYIN1_PIN          (Pin09)

#define BSP_KEYIN2_PORT         (PortB)
#define BSP_KEYIN2_PIN          (Pin07)

#define BSP_KEYIN3_PORT         (PortB)
#define BSP_KEYIN3_PIN          (Pin06)

#define BSP_KEYIN4_PORT         (PortB)
#define BSP_KEYIN4_PIN          (Pin05)

#define BSP_KEYIN5_PORT         (PortB)
#define BSP_KEYIN5_PIN          (Pin04)

#define BSP_PRINTF_DEVICE       (M4_USART1)
#define BSP_PRINTF_BAUDRATE     (115200)

#define BSP_PRINTF_PORT         (PortA)
#define BSP_PRINTF_PIN          (Pin12)
#define BSP_PRINTF_PORT_FUNC    (Func_Usart1_Tx)

#define TX_RX_SELECT            PORT_GetBit(PortB, Pin10)
#define TX_SELECT               GPIO_PIN_SET
#define RX_SELECT               GPIO_PIN_RESET

#define ADC1_SA_NORMAL_CHANNEL      (ADC1_CH0 | ADC1_CH10)
#define ADC1_SA_AVG_CHANNEL         (ADC1_CH12 | ADC1_CH13)
#define ADC1_SA_CHANNEL             (ADC1_SA_NORMAL_CHANNEL | ADC1_SA_AVG_CHANNEL)
#define ADC1_SA_CHANNEL_COUNT       (4u)
#define ADC1_AVG_CHANNEL            (ADC1_SA_AVG_CHANNEL)
#define ADC1_CHANNEL                (ADC1_SA_CHANNEL)
#define ADC1_SA_CHANNEL_SAMPLE_TIME { 0x30,     0x80,      0x50,      0x60 }
#define ADC1_SCAN_MODE              (AdcMode_SAOnce)
#define ADC_VREF                    (3.343f)//(3.288f)
#define ADC1_ACCURACY               (1ul << (12u))
#define TIMEOUT_VAL                 (10u)
//#define RX_TIMEOUT_VAL              (4000u)

void BSP_CLK_Init(void);
void BSP_KEY_Init(void);
void BSP_A15B03_Init(void);
void Spi_Config(void);
void BSP_LED_Init(void);
void RF_IRQ_Init(void);
void BSP_GPIO_Init(void);
void BSP_LED_Toggle(void);
void BAT_LED_ON(void);
void UsartRxErrProcess(void);
void Uart_Config(void);
void AdcConfig(void);
void BSP_PRINTF_PortInit(void);
void key_set_sf_routine(void);
void key_set_bw_routine(void);
void key_set_pl_routine(void);
void key_set_mode_routine(void);
void key_set_cr_routine(void);
void key_set_power_routine(void);
void key_clear_cnt(void);
void key_set_start_tx_routine(void);
void key_set_fq_routine(void);
void dis_init(void);
void key_scan(void);
void key_event_process(void);
void Timer4_Init(void);
void timer6_open_ms(uint32_t ms);
void Timer6_Cfg(void);
void Timera_Init(void);
void timera_open_ms(uint32_t ms);
void CAD_PA09_Init(void);
void Cad_Irq_Callback(void);
void rf_tx_done_routine(void);
void rf_rx_done_routine(void);
#endif /* BSP_EV_HC32F460_LQFP100_V2 */

#ifdef __cplusplus
}
#endif

#endif /* __EV_HC32F460_LQFP100_V2_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
