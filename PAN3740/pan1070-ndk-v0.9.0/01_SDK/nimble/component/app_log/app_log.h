/**
 *******************************************************************************
 * @file     app_log.h
 * @create   2024-11-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef _APP_LOG_H_
#define _APP_LOG_H_

#include <stdint.h>
#include <stdio.h>
#include "sdk_config.h"

#include "app_log_uart.h"
#include "app_track.h"

/*!< app log level define. */
#define APP_LOG_LVL_NONE   0
#define APP_LOG_LVL_ERR    1
#define APP_LOG_LVL_WRN    2
#define APP_LOG_LVL_INFO   3
#define APP_LOG_LVL_DEBUG  4

#ifndef APP_LOG_LVL
#define APP_LOG_LVL        APP_LOG_LVL_DEBUG
#endif

void app_log_print_data(uint8_t *pdata, uint32_t len);

#if PAN_LOG_ENABLE
#if APP_LOG_PRINT_MODE == 1
    extern void printk(const char *fmt, ...);   // Declares in printk.h
    #define APP_LOG_PRINT(...)   printk(__VA_ARGS__)
#elif APP_LOG_PRINT_MODE == 0
    #define APP_LOG_PRINT(...)   printf(__VA_ARGS__)
#endif // APP_LOG_PRINT_MODE
#else
    #define APP_LOG_PRINT(...)
#endif // PAN_LOG_ENABLE

#if PAN_LOG_ENABLE && APP_LOG_EN && (APP_LOG_LVL > APP_LOG_LVL_NONE)

/**@brief Enable/Disable output log level info */
#if APP_LOG_LVL_OUTPUT_EN
#define APP_LOG_PRINT_LVL(level)  APP_LOG_PRINT("%s", level)
#else
#define APP_LOG_PRINT_LVL(level)
#endif

/**@brief Enable/Disable output trace info */
#if APP_LOG_TRACE_OUTPUT_EN
#define APP_LOG_PRINT_TRACE(...)  APP_LOG_PRINT(__VA_ARGS__)
#else
#define APP_LOG_PRINT_TRACE(...)
#endif

#define APP_LOG(...)    APP_LOG_PRINT(__VA_ARGS__)

#define APP_LOG_DEBUG(...)                                                   \
    do{                                                                      \
        if(APP_LOG_LVL >= APP_LOG_LVL_DEBUG){                                \
            APP_LOG_PRINT_LVL("[D] ");                                       \
            APP_LOG_PRINT_TRACE("<%s(): %s: %d> ", __FUNCTION__, __FILE__, __LINE__);  \
            APP_LOG_PRINT(__VA_ARGS__);                                      \
        }                                                                    \
    }while(0)

#define APP_LOG_INFO(...)                                                    \
    do{                                                                      \
        if(APP_LOG_LVL >= APP_LOG_LVL_INFO){                                 \
            APP_LOG_PRINT_LVL("[I] ");                                       \
            APP_LOG_PRINT_TRACE("<%s(): %s: %d> ", __FUNCTION__, __FILE__, __LINE__);  \
            APP_LOG_PRINT(__VA_ARGS__);                                      \
        }                                                                    \
    }while(0)

#define APP_LOG_WRN(...)                                                     \
    do{                                                                      \
        if(APP_LOG_LVL >= APP_LOG_LVL_WRN){                                  \
            APP_LOG_PRINT_LVL("[W] ");                                       \
            APP_LOG_PRINT_TRACE("<%s(): %s: %d> ", __FUNCTION__, __FILE__, __LINE__);  \
            APP_LOG_PRINT(__VA_ARGS__);                                      \
        }                                                                    \
    }while(0)

#define APP_LOG_ERR(...)                                                     \
    do{                                                                      \
        if(APP_LOG_LVL >= APP_LOG_LVL_ERR){                                  \
            APP_LOG_PRINT_LVL("[E] ");                                       \
            APP_LOG_PRINT_TRACE("<%s(): %s: %d> ", __FUNCTION__, __FILE__, __LINE__);  \
            APP_LOG_PRINT(__VA_ARGS__);                                      \
        }                                                                    \
    }while(0)

#define APP_LOG_DATA(pdata, len)   app_log_print_data(pdata, len)

#else

#define APP_LOG(...)
#define APP_LOG_DEBUG(...)
#define APP_LOG_INFO(...)
#define APP_LOG_WRN(...)
#define APP_LOG_ERR(...)
#define APP_LOG_DATA(pdata, len)

#endif /* End of PAN_LOG_ENABLE && APP_LOG_EN && (APP_LOG_LVL > APP_LOG_LVL_NONE) */

#include "app_assert.h"

#endif /* End of _APP_LOG_H_ */
