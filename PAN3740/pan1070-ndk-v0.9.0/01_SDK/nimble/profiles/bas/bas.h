
#ifndef H_BLE_SVC_BAS_
#define H_BLE_SVC_BAS_

#include <stdint.h>

/* 16 Bit Battery Service UUID */
#define BLE_SVC_BAS_UUID16                                   0x180F

/* 16 Bit Battery Service Characteristic UUIDs */
#define BLE_SVC_BAS_CHR_UUID16_BATTERY_LEVEL                 0x2A19

void ble_svc_bas_init(void);

int ble_svc_bas_battery_level_set(uint8_t level);

#endif
