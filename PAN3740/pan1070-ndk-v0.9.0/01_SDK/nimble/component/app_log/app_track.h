/**
 *******************************************************************************
 * @file     app_track.h
 * @create   2024-12-31    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
 
#ifndef APP_TRACK_H
#define APP_TRACK_H
 
#include <stdint.h>
#include "sdk_config.h"
#include "pan_hal.h"
#if (BLE_EN && CONFIG_BT_CTLR_LINK_LAYER_DEBUG)
#include "utils/track.h"
#endif

/******************************************************************************
 * App Track APIs
 ******************************************************************************/

#define APP_TRACK_CHN0_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN0, 1);

#define APP_TRACK_CHN0_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN0, 0);

#define APP_TRACK_CHN0_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN0);

#define APP_TRACK_CHN1_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN1, 1);

#define APP_TRACK_CHN1_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN1, 0);

#define APP_TRACK_CHN1_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN1);

#define APP_TRACK_CHN2_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN2, 1);

#define APP_TRACK_CHN2_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN2, 0);

#define APP_TRACK_CHN2_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN2);

#define APP_TRACK_CHN3_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN3, 1);

#define APP_TRACK_CHN3_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN3, 0);

#define APP_TRACK_CHN3_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN3);

#define APP_TRACK_CHN4_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN4, 1);

#define APP_TRACK_CHN4_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN4, 0);

#define APP_TRACK_CHN4_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN4);

#define APP_TRACK_CHN5_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN5, 1);

#define APP_TRACK_CHN5_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN5, 0);

#define APP_TRACK_CHN5_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN5);

#define APP_TRACK_CHN6_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN6, 1);

#define APP_TRACK_CHN6_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN6, 0);

#define APP_TRACK_CHN6_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN6);

#define APP_TRACK_CHN7_HIGH         \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN7, 1);

#define APP_TRACK_CHN7_LOW          \
        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_USER_APP_CHN7, 0);

#define APP_TRACK_CHN7_TOGGLE       \
        PAN_IO_TIMING_TRACK_TOGGLE(CONFIG_TRACK_USER_APP_CHN7);

/******************************************************************************
 * Internal Functions
 ******************************************************************************/
void track_pin_init(void);


#endif /* End of APP_TRACK_H */
