
#ifndef H_BLE_SVC_GAP_
#define H_BLE_SVC_GAP_

#include <stdint.h>

/*! GAP Service UUID */
#define BLE_SVC_GAP_UUID16                                  0x1800
#define BLE_SVC_GAP_CHR_UUID16_DEVICE_NAME                  0x2a00
#define BLE_SVC_GAP_CHR_UUID16_APPEARANCE                   0x2a01
#define BLE_SVC_GAP_CHR_UUID16_PERIPH_PREF_CONN_PARAMS      0x2a04
#define BLE_SVC_GAP_CHR_UUID16_CENTRAL_ADDRESS_RESOLUTION   0x2aa6

/*! GAP Appearance */
#define BLE_SVC_GAP_APPEARANCE_GEN_UNKNOWN                   0
#define BLE_SVC_GAP_APPEARANCE_GEN_COMPUTER                  128
#define BLE_SVC_GAP_APPEARANCE_GEN_HID                       960
#define BLE_SVC_GAP_APPEARANCE_CYC_SPEED_AND_CADENCE_SENSOR  1157

/*! GAP MAX Name Length */
#define BLE_SVC_GAP_NAME_MAX_LEN                             20

/*! GAP Enable Central Address Resolution  */
#define BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION   			 0

/*! GAP Enable PPCP */
#define BLE_SVC_GAP_PERIPH_PREF_CONN_PARAM_EN   			 0

/*! GAP prefrence connection parameter. */
#define BLE_SVC_GAP_PPCP_MIN_CONN_INTERVAL      			 8
#define BLE_SVC_GAP_PPCP_MAX_CONN_INTERVAL      			 8
#define BLE_SVC_GAP_PPCP_SLAVE_LATENCY          			 99
#define BLE_SVC_GAP_PPCP_SUPERVISION_TMO        			 400


typedef void (ble_svc_gap_chr_changed_fn) (uint16_t uuid);

void ble_svc_gap_init(void);

void ble_svc_gap_set_chr_changed_cb(ble_svc_gap_chr_changed_fn *cb);

// GAP name API
const char *ble_svc_gap_device_name(void);
int ble_svc_gap_device_name_set(const char *name);

// GAP appearance API
uint16_t ble_svc_gap_device_appearance(void);
int ble_svc_gap_device_appearance_set(uint16_t appearance);

#endif
