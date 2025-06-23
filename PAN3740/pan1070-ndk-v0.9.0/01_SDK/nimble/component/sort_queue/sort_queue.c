/**
 *******************************************************************************
 * @file     sort_queue.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "sort_queue.h"
#include "utils/critical.h"
#include "pan_sys_log.h"

void sort_queue_add(sort_queue_t *sq, sort_queue_item_t *pitem)
{
	SYS_ASSERT(sq != NULL);
	SYS_ASSERT(pitem != NULL);
	SYS_ASSERT(sq->compare_func != NULL);

	sort_queue_item_t *p_pre_item = NULL;
	sort_queue_item_t *p_cur_item = NULL;

	uint32_t cpu_state = 0;
	ENTER_CRITICAL(cpu_state);

	if(SLIST_FIRST(&sq->list) == NULL){
		SLIST_INSERT_HEAD(&sq->list, pitem, next);

		EXIT_CRITICAL(cpu_state);
		return;
	}

	SLIST_FOREACH(p_cur_item, &sq->list, next)
	{
		if(sq->compare_func(pitem, p_cur_item)){
			if(p_pre_item == NULL){
				SLIST_INSERT_HEAD(&sq->list, pitem, next);
			}
			else{
				SLIST_INSERT_AFTER(p_pre_item, pitem, next);
			}

			EXIT_CRITICAL(cpu_state);
			return;
		}
		p_pre_item = p_cur_item;
	}

	if(p_pre_item){
		SLIST_INSERT_AFTER(p_pre_item, pitem, next);
	}
	else{
		SYS_ASSERT(0);
	}

	EXIT_CRITICAL(cpu_state);
}

void *sort_queue_pop(sort_queue_t *sq)
{
	SYS_ASSERT(sq != NULL);

	uint32_t cpu_state = 0;
	ENTER_CRITICAL(cpu_state);

	void *pitem = SLIST_FIRST(&sq->list);
	if(pitem != NULL){
		SLIST_REMOVE_HEAD(&sq->list, next);
	}

	EXIT_CRITICAL(cpu_state);

	return pitem;
}

void *sort_queue_peek(sort_queue_t *sq)
{
	SYS_ASSERT(sq != NULL);

	return (void *)SLIST_FIRST(&sq->list);
}

void sort_queue_remove(sort_queue_t *sq, sort_queue_item_t *pitem)
{
	SYS_ASSERT(sq != NULL);
	SYS_ASSERT(pitem != NULL);

	if(SLIST_FIRST(&sq->list) == NULL){
		return;
	}

	uint32_t cpu_state = 0;
	ENTER_CRITICAL(cpu_state);
	SLIST_REMOVE(&sq->list, pitem, sort_queue_item_s, next);
	EXIT_CRITICAL(cpu_state);
}

uint32_t sort_queue_cnt(sort_queue_t *sq)
{
	SYS_ASSERT(sq != NULL);

	int cnt = 0;

	sort_queue_item_t *item = NULL;
	SLIST_FOREACH(item, &sq->list, next){
		cnt++;
	}

	return cnt;
}


#if SORT_QUEUE_TEST_EN
struct test{
	SLIST_ENTRY(test) next;
	uint8_t data;
};

struct test item0 = {.data = 10};
struct test item1 = {.data = 1};
struct test item2 = {.data = 5};
struct test item3 = {.data = 0};

bool compare_func(sort_queue_item_t *item0, sort_queue_item_t *item1)
{
	struct test *p0 = (struct test*)item0;
	struct test *p1 = (struct test*)item1;

	if(p0->data < p1->data){
		return true;
	}
	return false;
}
SORT_QUEUE_INIT(test_queue, compare_func);

void sort_queue_test(void)
{
	sort_queue_add(&test_queue, (sort_queue_item_t *)&item0);
	sort_queue_add(&test_queue, (sort_queue_item_t *)&item1);
	sort_queue_add(&test_queue, (sort_queue_item_t *)&item2);
	sort_queue_add(&test_queue, (sort_queue_item_t *)&item3);

	SYS_PRINT("queue length:%d\r\n", sort_queue_cnt(&test_queue));
	while(1)
	{
		struct test *item = sort_queue_pop(&test_queue);
		if(item == NULL)
			break;

		SYS_PRINT("queue pop data: %d queue length:%d\n", item->data, sort_queue_cnt(&test_queue));
	}
	SYS_PRINT("\n");
}
#endif
