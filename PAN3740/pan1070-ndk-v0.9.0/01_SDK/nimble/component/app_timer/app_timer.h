/**
 *******************************************************************************
 * @file     app_timer.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef APP_TIMER_H_
#define APP_TIMER_H_

#include <stdint.h>
#include <stdbool.h>
#include "os/queue.h"
#include "soc_api.h"

/*! ms to ticks */
#define APP_TIMER_MS_TO_TICKS(ms)       ((uint32_t)((ms)*(uint64_t)soc_32k_clock_freq_get() / 1000))

/*! Invalid value used to indicate that timer is idle. */
#define APP_TIMER_IDLE_VAL 				0xFFFFFFFFFFFFFFFFULL

/*! Check if timer is idle */
#define APP_TIMER_IS_IDLE(timer) 		(timer->endVal == APP_TIMER_IDLE_VAL)

/*! Minimum value of the timeout_ticks parameter of app_timer_start(). */
#define APP_TIMER_MIN_TIMEOUT_TICKS     10

/*! Capacity of timer requests FIFO item number.  */
#define APP_TIMER_REQ_FIFO_ITEM_NUM 	10

enum{
	APP_TIMER_OK,
	APP_TIMER_NO_MEM,
};

/**@brief timer timeout call-back. */
typedef void (*app_timer_timeout_handler_t)(void *ctxt);

/**
 * @brief app_timer control block
 */
struct app_timer_s{
	SLIST_ENTRY(app_timer_s)    next;
	uint64_t					endVal;
	uint32_t    				reloadPeriod;
	app_timer_timeout_handler_t handler;
	void 						*ctxt;
};
typedef struct app_timer_s app_timer_t;

/**@brief Timer modes. */
typedef enum{
    APP_TIMER_MODE_SINGLE_SHOT,                 /**< The timer will expire only once. */
    APP_TIMER_MODE_REPEATED                     /**< The timer will restart each time it expires. */
}app_timer_mode_t;

int app_timer_init(void);

int app_timer_create(app_timer_t *ptimer, app_timer_mode_t mode, app_timer_timeout_handler_t timeout_handler);

int app_timer_start(app_timer_t *ptimer, uint32_t timeout_ticks, void *ctxt);

int app_timer_stop(app_timer_t *ptimer);

int app_timer_stop_all(void);


#define APP_TIMER_TEST_EN 0
#if APP_TIMER_TEST_EN
void app_timer_test(void);
#endif


#endif /* APP_TIMER_H_ */
