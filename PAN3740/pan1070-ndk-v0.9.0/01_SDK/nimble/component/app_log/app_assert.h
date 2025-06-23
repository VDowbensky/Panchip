/**
 *******************************************************************************
 * @file     app_assert.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef APP_ASSERT_H_
#define APP_ASSERT_H_

#include "app_log.h"

#if PAN_ASSERT_ENABLE && APP_ASSERT_EN
#define APP_ASSERT(exp)             \
    do{                                 \
        if(!(exp)){                     \
            APP_LOG_PRINT("[A] ASSERTION FAIL! expr: \"%s\" (%s: %d)\n", PAN_STRINGIFY(exp), __FILE__, __LINE__);   \
            while(1);                   \
        }                               \
    }while(0)
#else
#define APP_ASSERT(exp)
#endif // PAN_ASSERT_ENABLE && APP_ASSERT_EN

#endif /* APP_ASSERT_H_ */
