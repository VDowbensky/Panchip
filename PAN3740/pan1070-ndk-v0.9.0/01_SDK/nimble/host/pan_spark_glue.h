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
#include "pan_svc_call.h"

/* Here we borrow NVIC ADC IRQ as BLE event for LL interrupt offloading */
#define BLE_EVENT_PROC_IRQ    ADC_IRQHandler
#define BLE_EVENT_PROC_IRQn   ADC_IRQn


/** Panchip add Host function. */
#define PAN_BLE_HS_TX_CMPL_EVT_EN           1
#define PAN_BLE_HS_FIXED_SM_SEC_REQ_BUG_EN  1
#define PAN_BLE_HS_SM_API_EN                1

/**@brief ble stack pre init call-back. */
typedef void ble_stack_pre_init_cb_t(void);

/**@brief ble stack init complete call-back. */
typedef void ble_stack_enabled_cb_t(void);

/**
 * @brief : initialize ble stack
 * @brief : ble_stack_pre_init_cb_t   ble stack prev init call-back
 * @param : ble_stack_enabled_cb      ble stack enabled call-back
 * @return: none.
 */
void pan_ble_stack_init(ble_stack_pre_init_cb_t ble_stack_pre_init_cb, ble_stack_enabled_cb_t *ble_stack_enabled_cb);

/**
 * @brief : ble controller initialization.(only use for ble_hci)
 * @param : none.
 * @return: none.
 */
void pan_ble_controller_init(void);

/**
 * @brief : create ble stack thread
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

void pan_misc_set_pa_enable(uint8_t en);


/* !!! Only Internal use API. */
extern uint32_t pan_ble_hci_acl_nimble_handle(uint8_t *p_data, uint16_t data_len);
typedef int (*host_copydata_t)(void *from, void *dst, uint16_t len);
extern void pan_ll_register_hostcopy_cb(host_copydata_t func);
int host_copydata(void *from, void *dst, uint16_t len);
void ble_hci_evt_ll_to_host_cbk(uint8_t *p_evt, uint16_t evt_len);
void ble_hci_acl_ll_to_host_cbk(uint8_t *p_acl, uint16_t acl_len);

#endif  /* _NIMBLE_GLUE_H_ */
