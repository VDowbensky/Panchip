/**
 *******************************************************************************
 * @file     app_timer.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app_timer.h"
#include "PanSeries.h"
#include "utils/fifo.h"
#include "utils/critical.h"
#include "sort_queue.h"
#include "soc_api.h"

#if defined(IP_107x) || defined(IP_101x)
static app_timer_t *cur_active_timer = NULL;
static bool global_active = false;

/**
 * @brief Timer requests types.
 */
typedef enum{
    TIMER_REQ_START,
    TIMER_REQ_STOP,
    TIMER_REQ_STOP_ALL
}timer_req_type_t;

/**
 * @brief Operation request structure.
 */
typedef struct{
    app_timer_t         *p_timer; /**< Timer instance. */
    timer_req_type_t 	 type;    /**< Request type. */
}timer_req_t;

FIFO_DEFINE(timer_req_fifo, MEM_ALIGNED4(sizeof(timer_req_t)), APP_TIMER_REQ_FIFO_ITEM_NUM);

bool sort_list_compare_func(sort_queue_item_t *item0, sort_queue_item_t *item1);
SORT_QUEUE_INIT(app_timer_sort_queue, sort_list_compare_func);


static bool time_little(uint32_t t1, uint32_t t2)
{
	return (uint32_t)(t1 - t2) > BIT(30) ? true:false;
}

bool sort_list_compare_func(sort_queue_item_t *item0, sort_queue_item_t *item1)
{
	app_timer_t *ptimer0 = (app_timer_t *)item0;
	app_timer_t *ptimer1 = (app_timer_t *)item1;

	if(time_little(ptimer0->endVal, ptimer1->endVal)){
		return true;
	}
	return false;
}

uint32_t sort_list_cnt(void)
{
	return sort_queue_cnt(&app_timer_sort_queue);
}

void sort_list_add(app_timer_t *ptimer)
{
	sort_queue_add(&app_timer_sort_queue, (sort_queue_item_t *)ptimer);
}

app_timer_t *sort_list_pop(void)
{
	return (app_timer_t *)sort_queue_pop(&app_timer_sort_queue);
}

app_timer_t *sort_list_peek(void)
{
	return (app_timer_t *)sort_queue_peek(&app_timer_sort_queue);
}

void sort_list_remove(app_timer_t *ptimer)
{
	sort_queue_remove(&app_timer_sort_queue, (sort_queue_item_t *)ptimer);
}

void sort_list_remove_all(void)
{
	app_timer_t *p_cur_timer = NULL;
	do {
		p_cur_timer = sort_list_pop();
		if(p_cur_timer){
			p_cur_timer->endVal = APP_TIMER_IDLE_VAL;
		}
	}while(p_cur_timer);
}

static void timer_req_trigger(void)
{
	NVIC_SetPendingIRQ(SLPTMR_IRQn);
}

static int timer_req_send(app_timer_t *ptimer, timer_req_type_t type)
{
	fifo_t *pfifo = &timer_req_fifo;
	if(FIFO_IsFull(pfifo)){
		return APP_TIMER_NO_MEM;
	}

	timer_req_t *ptimer_req = (timer_req_t *)FIFO_GetWriteBuf(pfifo);

	ptimer_req->type = type;
	ptimer_req->p_timer = ptimer;

	FIFO_MoveToNextWriteBuf(pfifo); 

	timer_req_trigger();

	return APP_TIMER_OK;
}

static void timer_req_handler(void)
{
	fifo_t *pfifo = &timer_req_fifo;

	while(pfifo->w != pfifo->r)
	{
		timer_req_t *ptimer_req = (timer_req_t *)FIFO_GetReadBuf(pfifo);
		app_timer_t *ptimer = ptimer_req->p_timer;

		switch(ptimer_req->type)
		{
		case TIMER_REQ_START:
			if(!APP_TIMER_IS_IDLE(ptimer)){
				sort_list_add(ptimer);
			}
			break;

		case TIMER_REQ_STOP:
			if(ptimer == cur_active_timer){
				cur_active_timer = NULL;
			}
			else{
				sort_list_remove(ptimer);
			}
			break;

		case TIMER_REQ_STOP_ALL:
			sort_list_remove_all();
			global_active = true;
			break;

		default:
			break;
		}

		FIFO_MoveToNextReadBuf(pfifo); // must
	}
}

uint32_t app_timer_get_cnt_tick(void)
{
#if 1
	soc_busy_wait(35);//must wait 1 32k cycle when read 32k cnt
	return ANA->LP_SLPTMR;
#else
	return soc_lptmr_cycle_get(); //!!!Note: BLE MAC clock must be enabled.
#endif
}

void app_timer_set_cmp(uint32_t tick)
{
	ANA->LP_SPACING_TIME1 = tick;
}

static void app_timer_expired(app_timer_t *ptimer)
{
	if(global_active == false)
		return;

	uint32_t cpu_state = 0;

	if(ptimer)
	{
		ENTER_CRITICAL(cpu_state);
		if(ptimer->reloadPeriod == 0){
			ptimer->endVal = APP_TIMER_IDLE_VAL;
		}
		EXIT_CRITICAL(cpu_state);

		if(ptimer->handler){
			ptimer->handler(ptimer->ctxt);
		}

		ENTER_CRITICAL(cpu_state);
		/* check active flag as it may have been stopped in the user handler */
		if(ptimer->reloadPeriod && !APP_TIMER_IS_IDLE(ptimer)){
			ptimer->endVal += ptimer->reloadPeriod;

			//push timer to queue
			sort_list_add(ptimer);
		}
		EXIT_CRITICAL(cpu_state);
	}
}

static void app_timer_update(void)
{
	app_timer_t *ptimer = NULL;
	bool reconfig = false;
	while(1)
	{
		ptimer = sort_list_peek();
		if(ptimer == NULL){
			break;
		}

		if(APP_TIMER_IS_IDLE(ptimer)){
			sort_list_pop();
			continue;
		}
		else if(cur_active_timer == NULL){
			reconfig = true;
		}
		else if(time_little(ptimer->endVal, cur_active_timer->endVal)){
			if(!APP_TIMER_IS_IDLE(cur_active_timer)){
				sort_list_add(cur_active_timer);
			}
			reconfig = true;
		}

		if(!reconfig){
			break;
		}

		ptimer = sort_list_pop();
		if((uint32_t)((app_timer_get_cnt_tick()+APP_TIMER_MIN_TIMEOUT_TICKS) - ptimer->endVal) > BIT(30)){
			cur_active_timer = ptimer;
			app_timer_set_cmp((uint32_t)(ptimer->endVal - app_timer_get_cnt_tick()));
			break; //select ok
		}
		else{
			//timer expired
			app_timer_expired(ptimer);
			cur_active_timer = NULL;

			// select next timer object
		}
	}
}

void sleep_timer1_handler(void)
{
	if(cur_active_timer){
		app_timer_expired(cur_active_timer);
		cur_active_timer = NULL;
        
        if(sort_list_cnt() == 0){
			app_timer_set_cmp(0); 
		}
	}
}

void sleep_timer_post_irq_handler(void)
{
	timer_req_handler();
	app_timer_update();
}

void sleep_timer_it_config(uint8_t en)
{
	#define LP_INT_REG_WR_BIT_MASK  (BIT(0) | BIT(16) | BIT(20))
	
	uint32_t state = ANA->LP_INT_CTRL;

	if(en){
		ANA->LP_INT_CTRL = (state & LP_INT_REG_WR_BIT_MASK) | ANAC_INT_SLEEP_TMR_INT_EN_Msk;
	}
	else{
		ANA->LP_INT_CTRL = (state & LP_INT_REG_WR_BIT_MASK) & (~ANAC_INT_SLEEP_TMR_INT_EN_Msk);
	}
}

int app_timer_init(void)
{
    static uint8_t inited = false;
    if(inited){
        return APP_TIMER_OK;
    }
    inited = true;
    
	sleep_timer_it_config(ENABLE);
	NVIC_ClearPendingIRQ(SLPTMR_IRQn);
	NVIC_EnableIRQ(SLPTMR_IRQn);

	global_active = true;
	
	return APP_TIMER_OK;
}

int app_timer_create(app_timer_t *ptimer, app_timer_mode_t mode, app_timer_timeout_handler_t timeout_handler)
{
	ptimer->endVal  = APP_TIMER_IDLE_VAL;
	ptimer->handler = timeout_handler;
	ptimer->reloadPeriod = (mode == APP_TIMER_MODE_REPEATED ? 1:0);
	
	return APP_TIMER_OK;
}

int app_timer_start(app_timer_t *ptimer, uint32_t timeout_ticks, void *ctxt)
{
	uint32_t cpu_state = 0;

	if(APP_TIMER_IS_IDLE(ptimer))
	{
		ENTER_CRITICAL(cpu_state);
		ptimer->endVal = app_timer_get_cnt_tick() + timeout_ticks;
		EXIT_CRITICAL(cpu_state);

		ptimer->ctxt = ctxt;

		if(ptimer->reloadPeriod){
			ptimer->reloadPeriod = timeout_ticks;
		}

		return timer_req_send(ptimer, TIMER_REQ_START);
	}
	return APP_TIMER_OK;
}

int app_timer_stop(app_timer_t *ptimer)
{
	if(APP_TIMER_IS_IDLE(ptimer)){
		return APP_TIMER_OK;
	}

	uint32_t cpu_state = 0;
	ENTER_CRITICAL(cpu_state);
	ptimer->endVal = APP_TIMER_IDLE_VAL;
	EXIT_CRITICAL(cpu_state);

	return timer_req_send(ptimer, TIMER_REQ_STOP);
}

int app_timer_stop_all(void)
{
	global_active = false;
	return timer_req_send(NULL, TIMER_REQ_STOP_ALL);
}




#if APP_TIMER_TEST_EN
#include "pan_ble.h"
app_timer_t app_timer1;
app_timer_t app_timer2;


void app_timer1_cb(void *ctxt)
{
	static uint8_t flag = 1;
	
	DBG_CHN6_TOGGLE

	if(flag){
		app_timer_stop(&app_timer2);
		flag = 0;
	}
	else{
		flag = 1;
		app_timer_start(&app_timer2, (100*32), NULL);
	}
}

void app_timer2_cb(void *ctxt)
{
	DBG_CHN5_TOGGLE
}

void app_timer_test(void)
{
	app_timer_init();

	app_timer_create(&app_timer1, APP_TIMER_MODE_REPEATED, app_timer1_cb);
	app_timer_create(&app_timer2, APP_TIMER_MODE_REPEATED, app_timer2_cb);

	app_timer_start(&app_timer1, (500*32), NULL);
	app_timer_start(&app_timer2, (100*32), NULL);
}
#endif

#endif
