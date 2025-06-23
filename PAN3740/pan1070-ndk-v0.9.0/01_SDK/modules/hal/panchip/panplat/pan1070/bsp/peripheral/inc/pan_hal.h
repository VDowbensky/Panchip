#ifndef __PAN_HAL_H__
#define __PAN_HAL_H__

#include <stdint.h>
//#include "sdk_config.h"

typedef enum {
    HAL_SUCCESS,
    HAL_INVALID_PARAMS,
    HAL_IO_ERROR,
    HAL_DEVICE_BUSY,
    HAL_NO_MEM,
    HAL_NO_HARDWARE_SOURCE,
    HAL_UNKNOWN,
} HAL_Error;

typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_Status;

#define HAL_TIME_FOREVER        0xFFFFFFFFU
#define HAL_ASSERT              SYS_ASSERT

extern uint32_t HAL_TimeConvMsToTick(uint32_t ms);
extern uint32_t HAL_TimeGetCurrTick(void);
extern void HAL_DelayMs(uint32_t ms);
extern void HAL_DriverInit(void);
extern void HAL_IoTimingTrackInit(void);

#include "pan_hal_gpio.h"
#include "pan_hal_uart.h"
#include "pan_hal_dmac.h"
#include "pan_hal_spi.h"
#include "pan_hal_i2c.h"

#include "pan_hal_wdt.h"
#include "pan_hal_wwdt.h"
#include "pan_hal_pwm.h"
#include "pan_hal_timer.h"
#include "pan_hal_adc.h"

/* IO Timing Track for IRQs of HAL Driver */
#if CONFIG_IO_TIMING_TRACK

// Define an alternative name of PDIOs to support keil configuration wizard of sdk config file
#define P0x00               P00
#define P0x01               P01
#define P0x02               P02
#define P0x03               P03
#define P0x04               P04
#define P0x05               P05
#define P0x06               P06
#define P0x07               P07
#define P0x10               P10
#define P0x11               P11
#define P0x12               P12
#define P0x13               P13
#define P0x14               P14
#define P0x15               P15
#define P0x16               P16
#define P0x17               P17
#define P0x20               P20
#define P0x21               P21
#define P0x22               P22
#define P0x23               P23
#define P0x24               P24
#define P0x25               P25
#define P0x26               P26
#define P0x27               P27
#define P0x30               P30
#define P0x31               P31

__STATIC_FORCEINLINE void HAL_SetMfpGpio(uint8_t port, uint8_t bit)
{
    (&SYS->P0_MFP)[port] = (&SYS->P0_MFP)[port] & ~(SYS_MFP_P00_Msk << bit);
}

__STATIC_FORCEINLINE void HAL_GpioSetModeOutput(uint8_t port, uint8_t bit)
{
    GPIO_SetMode((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit, GPIO_MODE_OUTPUT);
}

__STATIC_FORCEINLINE void HAL_GpioToggle(uint8_t port, uint8_t bit)
{
    GPIO_Toggle((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

#define __GPIO_OUTPUT_INIT_PIN(pinId, initLevel)   \
        do {                                            \
            uint32_t port = pinId >> 4;                 \
            uint32_t bit = pinId & 0x0F;                \
            HAL_SetMfpGpio(port, bit);                  \
            HAL_GpioSetModeOutput(port, bit);           \
            GPIO_PIN_ADDR(port, bit) = initLevel;       \
        } while (0)

#define __PAN_COND_CODE_0x99(_flag, _if_0x99_code, _else_code)      \
        __PAN_COND_CODE(_NNNNN##_flag, _if_0x99_code, _else_code)

#define _NNNNN0x99      _ZZZZZ,

/**
 * @brief Like PAN_COND_CODE_1() except tests if _flag is 0x99.
 *
 * This is like PAN_COND_CODE_1(), except that it tests whether _flag
 * expands to the integer literal 0x99. It expands to _if_0x99_code if
 * so, and _else_code otherwise; both of these must be enclosed in
 * parentheses.
 *
 * @param _flag evaluated flag
 * @param _if_0x99_code result if _flag expands to 0x99; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 * @see PAN_COND_CODE_1()
 */
#define PAN_COND_CODE_0x99(_flag, _if_0x99_code, _else_code)    \
        __PAN_COND_CODE_0x99(_flag, _if_0x99_code, _else_code)

/**
 * @brief Init specified GPIO pin for IO Timing Track
 */
#define PAN_IO_TIMING_TRACK_INIT_PIN(_track_pin, _init_level)   \
        PAN_COND_CODE_0x99(_track_pin, (), (__GPIO_OUTPUT_INIT_PIN(_track_pin, _init_level)))

/**
 * @brief Set level for specified IO timing track pin
 */
#define PAN_IO_TIMING_TRACK_LEVEL(_track_pin, _level)           \
        PAN_COND_CODE_0x99(_track_pin, (), (PAN_CONCAT(P, _track_pin) = _level))

/**
 * @brief Toggle specified IO timing track pin
 */
#define PAN_IO_TIMING_TRACK_TOGGLE(_track_pin)                  \
        PAN_COND_CODE_0x99(_track_pin, (), (do{uint32_t port = _track_pin >> 4; uint32_t bit = _track_pin & 0x0F; HAL_GpioToggle(port, bit);} while(0)))

#else

#define PAN_IO_TIMING_TRACK_INIT_PIN(...)
#define PAN_IO_TIMING_TRACK_LEVEL(...)
#define PAN_IO_TIMING_TRACK_TOGGLE(...)

#endif // CONFIG_IO_TIMING_TRACK

#endif // __PAN_HAL_H__
