
#ifndef H_BLE_SVC_GATT_
#define H_BLE_SVC_GATT_

#include <inttypes.h>

struct ble_hs_cfg;

/* GATT UUID */
#define BLE_SVC_GATT_CHR_SERVICE_CHANGED_UUID16     0x2A05

void ble_svc_gatt_init(void);

void ble_svc_gatt_changed(uint16_t start_handle, uint16_t end_handle);


#endif
