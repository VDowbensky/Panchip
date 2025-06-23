/**
 *******************************************************************************
 * @file     sort_queue.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

#ifndef SORT_QUEUE_H_
#define SORT_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include "os/queue.h"

struct sort_queue_item_s{
	SLIST_ENTRY(sort_queue_item_s) next;
};
typedef struct sort_queue_item_s sort_queue_item_t;

typedef bool (*sort_compare_func_t)(sort_queue_item_t *item0, sort_queue_item_t *item1);

typedef struct{
	SLIST_HEAD(, sort_queue_item_s) list;
	sort_compare_func_t  			compare_func;
}sort_queue_t;

#define SORT_QUEUE_INIT(_name, _compare_func) \
	static sort_queue_t _name = {			  \
		.list.slh_first = NULL,				  \
		.compare_func = _compare_func,		  \
	}

void sort_queue_add(sort_queue_t *sq, sort_queue_item_t *pitem);
void *sort_queue_pop(sort_queue_t *sq);
void *sort_queue_peek(sort_queue_t *sq);
void sort_queue_remove(sort_queue_t *sq, sort_queue_item_t *pitem);

uint32_t sort_queue_cnt(sort_queue_t *sq);

#define SORT_QUEUE_TEST_EN  0
#if SORT_QUEUE_TEST_EN
void sort_queue_test(void);
#endif

#endif /* SORT_QUEUE_H_ */
