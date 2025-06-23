/**
 *******************************************************************************
 * @file     hid.h
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef HID_H
#define HID_H

#include <stdint.h>

#define BT_UUID_HIDS                0x1812

/** @def BT_UUID_HIDS_INFO_VAL
 *  @brief HID Information Characteristic UUID value
 */
#define BT_UUID_HIDS_INFO           0x2a4a

/** @def BT_UUID_HIDS_REPORT_MAP_VAL
 *  @brief HID Report Map Characteristic UUID value
 */
#define BT_UUID_HIDS_REPORT_MAP     0x2a4b

/** @def BT_UUID_HIDS_CTRL_POINT_VAL
 *  @brief HID Control Point Characteristic UUID value
 */
#define BT_UUID_HIDS_CTRL_POINT     0x2a4c

/** @def BT_UUID_HIDS_REPORT_VAL
 *  @brief HID Report Characteristic UUID value
 */
#define BT_UUID_HIDS_REPORT         0x2a4d

#define BT_UUID_HIDS_REPORT_REF     0x2908

/** @def BT_UUID_HIDS_PROTOCOL_MODE_VAL
 *  @brief HID Protocol Mode Characteristic UUID value
 */
#define BT_UUID_HIDS_PROTOCOL_MODE  0x2a4e

/** @def BT_UUID_RSC_MEASUREMENT_VAL
 *  @brief RSC Measurement Characteristic UUID value
 */
#define BT_UUID_RSC_MEASUREMENT     0x2a53

extern uint16_t hid_input_handle;
extern uint16_t hid_consumer_input_handle;
extern uint16_t hid_mouse_input_handle;

int ble_svc_hid_init(void);

int ble_svc_hid_send_report(uint16_t conn_handle, uint8_t *pdata, uint32_t len);
int ble_svc_hid_send_customer_report(uint16_t conn_handle, uint8_t *pdata, uint32_t len);

#endif
