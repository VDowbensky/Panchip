/**
 *******************************************************************************
 * @file     app_track.c
 * @create   2024-12-31
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app_track.h"
#include "PanSeries.h"

#if CONFIG_IO_TIMING_TRACK

static void misc_io_timing_track_init(void)
{
    // Init DeepSleep and Sleep state pin to 1
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_DEEPSLEEP_MODE, 1);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_SLEEP_MODE, 1);
    // Init misc IRQs pin to 0
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_LL_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_BLE_EVNT_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_OS_TICK_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_SLPTMR_IRQ, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_PIN_HARDFAULT_IRQ, 0);
    // Init IO Timing Track Channels for User App Use
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN0, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN1, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN2, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN3, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN4, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN5, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN6, 0);
    PAN_IO_TIMING_TRACK_INIT_PIN(CONFIG_TRACK_USER_APP_CHN7, 0);
}

void track_pin_init(void)
{
    // Enable GPIO clock for confirmation (Default enabled already for PAN10xx SoC)
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);

    // Track HAL IRQs
    HAL_IoTimingTrackInit();

    // Track SoC platform misc IRQs and states
    misc_io_timing_track_init();

    // Track BLE controller internal events when needed
#if (BLE_EN && CONFIG_BT_CTLR_LINK_LAYER_DEBUG)
    DbgPin_t debug_io;
    memset(&debug_io, 0, sizeof(debug_io)); //!!!must

    /** 
    * debug pin initilization. 
    * !!!Note: All debug pins must work in gpio mode. 
    *          By default, most GPIOs work in gpio mode.
    *          If a GPIO does not work in gpio mode, you need to configure it to work in gpio mode.
    */
    GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT); P15 = 0;
    GPIO_SetMode(P1, BIT3, GPIO_MODE_OUTPUT); P13 = 0;
    GPIO_SetMode(P1, BIT4, GPIO_MODE_OUTPUT); P14 = 0;
    GPIO_SetMode(P1, BIT2, GPIO_MODE_OUTPUT); P12 = 0;
    GPIO_SetMode(P0, BIT5, GPIO_MODE_OUTPUT); P05 = 0;
    GPIO_SetMode(P0, BIT3, GPIO_MODE_OUTPUT); P03 = 0;
    GPIO_SetMode(P2, BIT2, GPIO_MODE_OUTPUT); P22 = 0;
    GPIO_SetMode(P2, BIT3, GPIO_MODE_OUTPUT); P23 = 0;

    debug_io.dbg_chn0_pin = (uint32_t)&P15; //scan
    debug_io.dbg_chn1_pin = (uint32_t)&P13; //adv
    debug_io.dbg_chn2_pin = (uint32_t)&P14; //ll timer irq
    debug_io.dbg_chn3_pin = (uint32_t)&P12; //ll done irq
    debug_io.dbg_chn4_pin = (uint32_t)&P05; //connection0
    debug_io.dbg_chn5_pin = (uint32_t)&P03; 
    debug_io.dbg_chn6_pin = (uint32_t)&P22; 
    debug_io.dbg_chn7_pin = (uint32_t)&P23; 

    /* Register debug pin to track system. */
    TrackTimingInitEx(&debug_io);
#endif // BLE_EN && CONFIG_BT_CTLR_LINK_LAYER_DEBUG
}

#endif // CONFIG_IO_TIMING_TRACK
