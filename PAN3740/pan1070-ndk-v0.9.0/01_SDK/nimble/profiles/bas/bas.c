#include <string.h>
#include "bas.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "pan_sys_log.h"

static uint16_t ble_svc_bas_battery_handle;

/* Battery level */
uint8_t ble_svc_bas_battery_level = 100;

static int ble_svc_bas_access(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_bas_defs[] =
{
    {
        /*** Battery Service. */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_BAS_UUID16),
        .characteristics = (struct ble_gatt_chr_def[])
		{
        	{
        		/*** Battery level characteristic */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_BAS_CHR_UUID16_BATTERY_LEVEL),
				.access_cb = ble_svc_bas_access,
				.val_handle = &ble_svc_bas_battery_handle,
				.flags = BLE_GATT_CHR_F_READ |BLE_GATT_CHR_F_NOTIFY ,
        	},
        	{
        		0, /* No more characteristics in this service. */
        	}
        },
    },
    {
        0, /* No more services. */
    },
};


static int ble_svc_bas_access(uint16_t conn_handle, uint16_t attr_handle,
							  struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);
    int rc;

    switch (uuid16)
    {
    case BLE_SVC_BAS_CHR_UUID16_BATTERY_LEVEL:
        SYS_ASSERT(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);
        rc = os_mbuf_append(ctxt->om, &ble_svc_bas_battery_level, sizeof ble_svc_bas_battery_level);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    default:
        SYS_ERR("Unrecognized BLE BAS Service CHR UUID16: 0x%04x\n", uuid16);
        return BLE_ATT_ERR_UNLIKELY;
    }
}

int ble_svc_bas_battery_level_set(uint8_t level)
{
    if(level > 100){
    	level = 100;
    }

    if(ble_svc_bas_battery_level != level)
    {
    	ble_svc_bas_battery_level = level;
        ble_gatts_chr_updated(ble_svc_bas_battery_handle);
    }
    return 0;
}

void ble_svc_bas_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(ble_svc_bas_defs);
    SYS_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_bas_defs);
    SYS_ASSERT(rc == 0);
}
