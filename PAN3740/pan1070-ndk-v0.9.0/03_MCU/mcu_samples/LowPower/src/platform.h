/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      platform.h
 * @brief     Header file of PAN10xx SoC platform init flow
*******************************************************************************/
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>

enum {
    SOC_RST_REASON_CHIP_RESET               = 0,
    SOC_RST_REASON_PIN_RESET                = 1,
    SOC_RST_REASON_WDT_RESET                = 2,
    SOC_RST_REASON_LVR_RESET                = 3,
    SOC_RST_REASON_BOD_RESET                = 4,
    SOC_RST_REASON_SYS_RESET                = 5,
    SOC_RST_REASON_POR_RESET                = 6,
    SOC_RST_REASON_STBM0_EXTIO_WAKEUP       = 10,    /* P00/P01/P02 */
    SOC_RST_REASON_STBM1_SLPTMR0_WAKEUP     = 11,
    SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP     = 12,
    SOC_RST_REASON_STBM1_SLPTMR2_WAKEUP     = 13,
    SOC_RST_REASON_STBM1_GPIO_WAKEUP        = 14,    /* All GPIOs */
    SOC_RST_REASON_UNKNOWN_RESET            = 255
};

extern void SOC_DisableLogUartRxPath(void);
extern void SOC_ReenableLogUartRxPath(void);

extern uint8_t SOC_ResetReasonGet(void);
extern void SOC_PlatformInit(void);

#endif  //__PLATFORM_H__
