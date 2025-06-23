/**
 *******************************************************************************
 * @file     nimble_glue_spark.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef _NIMBLE_GLUE_H_
#define _NIMBLE_GLUE_H_

#include "PanSeries.h"
#include "stack/ble_types.h"
#include "pan_ble_stack.h"

/* Here we borrow NVIC ADC IRQ as BLE event for LL interrupt offloading */
#define BLE_EVENT_PROC_IRQ    ADC_IRQHandler
#define BLE_EVENT_PROC_IRQn   ADC_IRQn

/**@brief ble stack init complete call-back. */
typedef void ble_stack_enabled_cb_t(void);

/**
 * @brief : initialize ble stack
 * @param : ble_stack_enabled_cb     ble stack enabled call-back
 * @return: none.
 */
void pan_ble_stack_init(ble_stack_enabled_cb_t *ble_stack_enabled_cb);

/**
 * @brief : create ble stack thread
 *           !!!Note user need to set MYNEWT_VAL_BLE_HOST_THREAD_STACK_SIZE & MYNEWT_VAL_BLE_HOST_THREAD_PRIORITY
 * @param : none.
 * @return: none.
 */
void pan_ble_thread_create(void);

/**
 * @brief: ble stack execute entry. 
 *         !!!Note: user need place this API to a thread.
 * @param : none.
 * @return: none.
 */
void pan_ble_stack_start(void);


/* Misc API */
extern uint8_t rf_check_sleep_state(void);



/* !!! Only Internal use API. */
extern uint32_t pan_ble_hci_acl_nimble_handle(uint8_t *p_data, uint16_t data_len);
typedef int (*host_copydata_t)(void *from, void *dst, uint16_t len);
extern void pan_ll_register_hostcopy_cb(host_copydata_t func);
int host_copydata(void *from, void *dst, uint16_t len);
void ble_hci_evt_ll_to_host_cbk(uint8_t *p_evt, uint16_t evt_len);
void ble_hci_acl_ll_to_host_cbk(uint8_t *p_acl, uint16_t acl_len);

#endif  /* _NIMBLE_GLUE_H_ */
