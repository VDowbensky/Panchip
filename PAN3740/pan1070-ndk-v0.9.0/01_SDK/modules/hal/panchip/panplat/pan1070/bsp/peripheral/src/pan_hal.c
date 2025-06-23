/**************************************************************************//**
* @file     pan_hal.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip HAL common source file.
* @note
* Copyright (C) 2023-2024 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "pan_hal.h"

void HAL_DelayMs(uint32_t ms)
{
    uint32_t timeout_tick = HAL_TimeConvMsToTick(ms);
    uint32_t uptime_ref = HAL_TimeGetCurrTick();

    while (HAL_TimeGetCurrTick() - uptime_ref < timeout_tick)
    {
        // Busy wait
    }
}

__WEAK uint32_t HAL_TimeConvMsToTick(uint32_t ms)
{
    uint64_t timeout_tick = (uint64_t)ms * (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v ? 32768 : 32000) / 1000ull;

    if (timeout_tick > UINT32_MAX)
    {
        timeout_tick = HAL_TIME_FOREVER;
    }

    return (uint32_t)timeout_tick;
}

__WEAK uint32_t HAL_TimeGetCurrTick(void)
{
#if 0
    return (*(volatile uint32_t *)(0x50020014)); // current 32K counter in llctlr
#else
    return LP_GetSlptmrCurrCount(ANA);
#endif
}

#if CONFIG_IO_TIMING_TRACK
void HAL_IoTimingTrackInit(void)
{
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_DMA_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_GPIO0_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_GPIO1_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_GPIO2_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_GPIO3_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_I2C_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_SPI0_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_SPI1_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_TMR0_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_TMR1_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_TMR2_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_UART0_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_UART1_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_WDT_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_WWDT_IRQ, 0);
}
#endif // CONFIG_IO_TIMING_TRACK

void HAL_DriverInit(void)
{
    // Init 32K timer counter
    ANA->LP_FL_CTRL_3V |= ANAC_FL_SLEEP_CNT_EN_Msk; // slptmr counter en
#if 0
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_BLE_32M | CLK_AHBPeriph_BLE_32K, ENABLE);
    (*(volatile uint32_t *)0x5002000C) |= BIT1; // current 32K counter en in llctlr
#endif
}
