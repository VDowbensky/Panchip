/************************************************************************
 * @file     pan_sys_log.h
 * @version  V1.00
 * $Revision: 0 $
 * $Date: 2023/11/08 $
 * @brief    Header file for the Panchip Series system logging functionality.
 *
 * @note     This library provides functions and definitions for system
 *           logging and debugging features specific to the Panchip Series
 *           microcontroller family.
 * Copyright (C) 2023-2025 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __PAN_SYS_LOG_H__
#define __PAN_SYS_LOG_H__

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "printk.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************** Default SYS Log Configurations (Can be overrode by application) *********************/
/* Enable Log output or not */
#ifndef PAN_LOG_ENABLE
#define PAN_LOG_ENABLE                  1
#endif

/* Enable SYS Log output or not */
#ifndef PAN_SYS_LOG_ENABLE
#define PAN_SYS_LOG_ENABLE              1
#endif

/* SYS Log Level Select */
#ifndef PAN_SYS_LOG_LVL
#define PAN_SYS_LOG_LVL                 PAN_SYS_LOG_LVL_DBG
#endif

/* Print Log Level String or not */
#ifndef PAN_SYS_LOG_PRINT_LEVEL_STRING
#define PAN_SYS_LOG_PRINT_LEVEL_STRING              1
#endif

/* Print Log Verbose String or not */
#ifndef PAN_SYS_LOG_PRINT_VERBOSE_STRING
#define PAN_SYS_LOG_PRINT_VERBOSE_STRING            0
#endif

/*
 * Configure SYS Log Backend
 * 0: Use default c-lib printf() implementation
 * 1: Use panchip printk() implementation, which can significantly reduce the cost of stack and code
 */
#ifndef PAN_SYS_LOG_PRINT_MODE
#define PAN_SYS_LOG_PRINT_MODE          0
#endif

/* Enable SYS Assert Macro or not */
#ifndef PAN_SYS_ASSERT_EN
#define PAN_SYS_ASSERT_EN               1
#endif

#if PAN_LOG_ENABLE
// Enable Simple Print Macro Functions defined in simple_print.h
#define CONFIG_SIMPLE_PRINT_ENABLE      1
#else
// Disable Simple Print Macro Functions defined in simple_print.h
#define CONFIG_SIMPLE_PRINT_ENABLE      0
#endif // PAN_LOG_ENABLE
#include "simple_print.h"

/******************************************* SYS Log Internals ********************************************/
/* SYS Log Level Defines */
#define PAN_SYS_LOG_LVL_NONE            0
#define PAN_SYS_LOG_LVL_ERR             1
#define PAN_SYS_LOG_LVL_WRN             2
#define PAN_SYS_LOG_LVL_INF             3
#define PAN_SYS_LOG_LVL_DBG             4

#define PAN_SYS_LOG_LVL_STR(level)             \
    (PAN_SYS_LOG_LVL_DBG    == level ? "[DBG] " :   \
    (PAN_SYS_LOG_LVL_INF    == level ? "[INF] " :   \
    (PAN_SYS_LOG_LVL_WRN    == level ? "[WRN] " :   \
    (PAN_SYS_LOG_LVL_ERR    == level ? "[ERR] " :   \
     "[UNKNOWN] "))))

#define __SYS_LOG_LEVEL_STRING_PRINT(level)                 \
        if (PAN_SYS_LOG_PRINT_LEVEL_STRING) {                   \
            __SYS_PRINT_IMPL_FUNC(PAN_SYS_LOG_LVL_STR(level));  \
        }

#define __SYS_LOG_VERBOSE_STRING_PRINT()                                        \
        if (PAN_SYS_LOG_PRINT_VERBOSE_STRING) {                                         \
            __SYS_PRINT_IMPL_FUNC("<%s(): %s: %d> ", __FUNCTION__, __FILE__, __LINE__); \
        }

#if PAN_LOG_ENABLE
/* SYS Log Backend */
#if PAN_SYS_LOG_PRINT_MODE == 1
// Declares in printk.h
extern void printk(const char *fmt, ...);
// SYS Print family API implementations
#define __SYS_PRINT_IMPL_FUNC(...)    printk(__VA_ARGS__)
#elif PAN_SYS_LOG_PRINT_MODE == 0
#define __SYS_PRINT_IMPL_FUNC(...)    printf(__VA_ARGS__)
#endif // PAN_SYS_LOG_PRINT_MODE
#else
#define __SYS_PRINT_IMPL_FUNC(...)
#endif // PAN_LOG_ENABLE

#if PAN_LOG_ENABLE && PAN_SYS_LOG_ENABLE
#define __SYS_LOG(level, ...)                       \
    do {                                                \
        if (PAN_SYS_LOG_LVL >= level) {                 \
            __SYS_LOG_LEVEL_STRING_PRINT(level);        \
            __SYS_LOG_VERBOSE_STRING_PRINT();           \
            __SYS_PRINT_IMPL_FUNC(__VA_ARGS__);         \
        }                                               \
    } while (0)
#else
#define __SYS_LOG(flags, ...)
#endif // PAN_LOG_ENABLE && PAN_SYS_LOG_ENABLE


/************************************* SYS Log Mechanism Implementations **********************************/
//
// SYS Log APIs with level, take effect when PAN_LOG_ENABLE==1 && PAN_SYS_LOG_ENABLE==1
//

#define SYS_DBG(fmt, ...)   __SYS_LOG(PAN_SYS_LOG_LVL_DBG, fmt, ##__VA_ARGS__)
#define SYS_INF(fmt, ...)   __SYS_LOG(PAN_SYS_LOG_LVL_INF, fmt, ##__VA_ARGS__)
#define SYS_WRN(fmt, ...)   __SYS_LOG(PAN_SYS_LOG_LVL_WRN, fmt, ##__VA_ARGS__)
#define SYS_ERR(fmt, ...)   __SYS_LOG(PAN_SYS_LOG_LVL_ERR, fmt, ##__VA_ARGS__)


/******************************************** SYS Basic APIs **********************************************/
/* Basic SYS Print APIs, take effect when PAN_LOG_ENABLE==1 */
#define SYS_TEST(...)       __SYS_PRINT_IMPL_FUNC(__VA_ARGS__)
#define SYS_PRINT(...)      __SYS_PRINT_IMPL_FUNC(__VA_ARGS__)

/* Basic SYS Abort API, can be used anywhere, but the log print only takes effect when PAN_LOG_ENABLE==1 */
#define SYS_ABORT()                                                         \
    do {                                                                        \
        SYS_PRINT("!!!!!!!!!!!!!!!!!! System Aborted !!!!!!!!!!!!!!!!!\n");     \
        do {} while (1);                                                        \
    } while (0)


/*********************************** SYS Assert Mechanism Implementation **********************************/
//
// SYS Assert API, takes effect when PAN_ASSERT_ENABLE==1 && PAN_SYS_ASSERT_EN==1, and meanwhile the log
// print only takes effect when PAN_LOG_ENABLE==1
//
#if PAN_ASSERT_ENABLE && PAN_SYS_ASSERT_EN
#define SYS_ASSERT(expr)    ((expr) ? (void)0u : SYS_AssertError((uint8_t*)__FILE__, __LINE__, (uint8_t*)PAN_STRINGIFY(expr)))
extern void SYS_AssertError(uint8_t* file, uint32_t line, uint8_t* expr);
#else
#define SYS_ASSERT(expr)    ((void)(expr))  // To suppress compiler warnings
#endif


#ifdef __cplusplus
}
#endif

#endif /* __PAN_SYS_LOG_H__ */
