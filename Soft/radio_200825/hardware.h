/*******************************************************************************
 * @note Copyright (C) 2011-2022, Shanghai Panchip Microelectronics Co., Ltd. 
 * @SPDX-License-Identifier: Apache-2.0
 * @history - V0.1, 2022-09-28
*******************************************************************************/
#ifndef __HARDWARE_H_
#define __HARDWARE_H_

#include "rtc.h"
#include "lpm.h"
#include "gpio.h"
#include "flash.h"
#include "fifo.h"
#include "uart.h"
#include "userdef.h"

#define  USB_DONGLE
//#define BLACK_NODE
//#define BLUE_NODE

/* DREAMLNK SETA/SETB/AUX Pin */
#define GPIO_PIN_IRQ GpioPin2
#define GPIO_PORT_IRQ GpioPortA

#define GPIO_PIN_KEY GpioPin1
#define GPIO_PORT_KEY GpioPortB

#define GPIO_PIN_LED GpioPin0
#define GPIO_PORT_LED GpioPortB

#define DBG_P1_PORT GpioPortA
#define DBG_P1_PIN GpioPin11

#define DBG_P2_PORT GpioPortA
#define DBG_P2_PIN GpioPin12

#define GPIO_PIN_CAD GpioPin15
#define GPIO_PORT_CAD GpioPortA

#define GPIO_AUX_LEVEL_GET() LEVEL_INACTIVE // Gpio_ReadOutputIO(GPIO_PORT_AUX, GPIO_PIN_AUX)

#define GPIO_AUX_LEVEL_SET_H 0 // Gpio_WriteOutputIO(GPIO_PORT_AUX, GPIO_PIN_AUX, 1)
#define GPIO_AUX_LEVEL_SET_L 0 // Gpio_WriteOutputIO(GPIO_PORT_AUX, GPIO_PIN_AUX, 0)

#define GPIO_CAD_LEVEL_GET() Gpio_GetInputIO(GPIO_PORT_CAD, GPIO_PIN_CAD)
#define GPIO_IRQ_LEVEL_GET() Gpio_GetInputIO(GPIO_PORT_IRQ, GPIO_PIN_IRQ)

#ifdef USB_DONGLE
    #define GPIO_PIN_SETA GpioPin3
    #define GPIO_PORT_SETA GpioPortB
    #define GPIO_PIN_SETB GpioPin4
    #define GPIO_PORT_SETB GpioPortB
    #define GPIO_PIN_AUX GpioPin6
    #define GPIO_PORT_AUX GpioPortB
    #define GPIO_SETA_LEVEL_GET() LEVEL_INACTIVE
    #define GPIO_SETB_LEVEL_GET() LEVEL_ACTIVE
    #define EN_DBG_PIN 1
#else
    #define GPIO_PIN_SETA GpioPin3
    #define GPIO_PORT_SETA GpioPortB
    #define GPIO_PIN_SETB GpioPin4
    #define GPIO_PORT_SETB GpioPortB
    #define GPIO_PIN_AUX GpioPin5
    #define GPIO_PORT_AUX GpioPortB
    #define GPIO_SETA_LEVEL_GET() Gpio_GetInputIO(GPIO_PORT_SETA, GPIO_PIN_SETA)
    #define GPIO_SETB_LEVEL_GET() Gpio_GetInputIO(GPIO_PORT_SETB, GPIO_PIN_SETB)
    #define EN_DBG_PIN 0
#endif

#define LEVEL_INACTIVE          0
#define LEVEL_ACTIVE            1

#define DBG_P1_SET() \
    if (EN_DBG_PIN)  \
    Gpio_WriteOutputIO(DBG_P1_PORT, DBG_P1_PIN, TRUE)

#define DBG_P1_RESET() \
    if (EN_DBG_PIN)    \
    Gpio_WriteOutputIO(DBG_P1_PORT, DBG_P1_PIN, FALSE)

#define DBG_P1_FLIP() \
    if (EN_DBG_PIN)   \
    Gpio_WriteOutputIO(DBG_P1_PORT, DBG_P1_PIN, Gpio_ReadOutputIO(DBG_P1_PORT, DBG_P1_PIN) ? 0 : 1)

#define DBG_P2_SET() \
    if (EN_DBG_PIN)  \
    Gpio_WriteOutputIO(DBG_P1_PORT, DBG_P1_PIN, TRUE)

#define DBG_P2_RESET() \
    if (EN_DBG_PIN)    \
    Gpio_WriteOutputIO(DBG_P1_PORT, DBG_P1_PIN, FALSE)

#define DBG_P2_FLIP() \
    if (EN_DBG_PIN)   \
    Gpio_WriteOutputIO(DBG_P1_PORT, DBG_P1_PIN, Gpio_ReadOutputIO(DBG_P1_PORT, DBG_P1_PIN) ? 0 : 1)

#define AUX_MODE_EMPTY 0
#define AUX_MODE_FULL 1

typedef enum
{
    AUX_INIT,
    AUX_FIFO,
    AUX_JOIN,
    AUX_DATA,
    AUX_SLEEP,
    AUX_ERROR,
}aux_type_t;

typedef struct
{
    bool init;    //aux level for init state
    bool fifo;    //aux level for uart fifo level
    bool join;    //aux level for join state
    bool data;    //aux level for data state
    bool sleep;   //aux level for sleep state
    bool error;   //aux level for error indicate
}aux_level_t;

extern Fifo_t uart_rx_fifo;
extern Fifo_t uart_tx_fifo;
extern bool uart_rx_busy_flag;
extern int aux_mode;
extern volatile aux_level_t aux_level;

uint8_t hw_pll_init(void);
bool aux_level_set(aux_type_t type, bool level);
void hw_gpio_cfg(void);
void hw_rtc_cfg(int on, uint8_t second);
void hw_spi_cfg(void);
void hw_spi_gpio_init(void);
void hw_uart_cfg(void);
void hw_uart_gpio_init(void);
int uart_fifo_read(u8 *buf, int bufLen);
void mcu_enter_lp(void);
void mcu_exit_lp(void);
void hw_timer3_cfg(void);
void hw_timer1_restart(void);
void hw_timer0_cfg(void);
void uart_rx_fifo_flush(void);
void hw_rng_cfg(void);
void app_uart0_dma_cfg(void);
int hw_uart0_dma_send(u8 *buf, int len);
void hw_timer1_cfg(void);
void spi_cs_low(void);
void spi_cs_high(void);
uint8_t spi_writereadbyte(uint8_t tx_data);
#ifdef USB_DONGLE
void hw_timer2_cfg(void);
uint32_t Micros_timer2(void);
void Hal_Pwm_Init(void);
#endif
#endif

