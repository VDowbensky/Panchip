/**
 *******************************************************************************
 * @file     gw.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "gw.h"
#include "host/ble_hs.h"
#include "pan_sys_log.h"

uint16_t gwRxValHandle;
uint16_t gwTxValHandle;

static uint8_t gwRxVal[64];

extern void app_ble_rx(uint16_t connHandle, struct os_mbuf *om);

__WEAK void app_ble_rx(uint16_t connHandle, struct os_mbuf *om)
{
	//SYS_PRINT("Rx data 1(handle:%d):\r\n", *ctxt->chr->val_handle);
	//print_data(ctxt->om->om_data, ctxt->om->om_len);
	//memcpy(gwRxVal, ctxt->om->om_data, min(ctxt->om->om_len, sizeof(gwRxVal)));
}

int ble_svc_gw_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
int ble_svc_gw_dsc_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_gw_defs[] =
{
	/*** GW Service. */
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(BLE_SVC_GW_UUI128),
        .characteristics = (struct ble_gatt_chr_def[])
		{
        	/*** GW RX characteristic */
        	{
				.uuid = BLE_UUID128_DECLARE(BLE_SVC_GW_CHR_UUID128_RX),
				.access_cb = ble_svc_gw_access,
				.val_handle = (uint16_t *)&gwRxValHandle,
				.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
				.descriptors = (struct ble_gatt_dsc_def[])
				{
					{
						.uuid = BLE_UUID16_DECLARE(BLE_GATT_USR_DSC_CHR_UUID16),
						.att_flags = BLE_ATT_F_READ,
						.access_cb = ble_svc_gw_access,
					},
					{
						0, /* No more descriptors in this characteristic. */
					}
				}
			},

        	/*** GW TX characteristic */
        	{
				.uuid = BLE_UUID128_DECLARE(BLE_SVC_GW_CHR_UUID128_TX),
				.access_cb = ble_svc_gw_access,
				.val_handle = (uint16_t *)&gwTxValHandle,
				.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
				.descriptors = (struct ble_gatt_dsc_def[])
				{
					{
						.uuid = BLE_UUID16_DECLARE(BLE_GATT_USR_DSC_CHR_UUID16),
						.att_flags = BLE_ATT_F_READ,
						.access_cb = ble_svc_gw_access,
					},
					{
						0, /* No more descriptors in this characteristic. */
					}
				}
			},

			/* No more characteristics in this service. */
			{
				0,
			}
		},
    },

	/* No more services. */
    {
        0,
    },
};

int ble_svc_gw_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	int rc = 0;

	switch(ctxt->op)
	{
	case BLE_GATT_ACCESS_OP_READ_CHR:
		if(attr_handle == gwRxValHandle) {
			rc = os_mbuf_append(ctxt->om, &gwRxVal[0], sizeof gwRxVal);
		}
		return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

	case BLE_GATT_ACCESS_OP_WRITE_CHR:
		if(attr_handle == gwRxValHandle) {
			app_ble_rx(conn_handle, ctxt->om);
		}
		return 0;

	case BLE_GATT_ACCESS_OP_READ_DSC:
		if(attr_handle == gwRxValHandle+1)
		{
			char *str = "Rx Data";
			os_mbuf_append(ctxt->om, str, strlen(str));
		}
		else if(attr_handle == gwTxValHandle+2)
		{
			char *str = "Tx Data";
			os_mbuf_append(ctxt->om, str, strlen(str));
		}
		break;

	case BLE_GATT_ACCESS_OP_WRITE_DSC:
		return BLE_ATT_ERR_INVALID_PDU;

	default:
		return 0;
	}

	return 0;
}

int ble_svc_gw_notify(uint16_t conn_handle, uint8_t *pdata, uint32_t len)
{
	int rc = 0;

	struct os_mbuf *om = ble_hs_mbuf_from_flat(pdata, len);
	if(om)
	{
		rc = ble_gatts_notify_custom(conn_handle, gwTxValHandle, om);
	}
	else{
		rc = BLE_NPL_ENOMEM;
	}

	return rc;
}


int  ble_svc_gw_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(ble_svc_gw_defs);
    SYS_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_gw_defs);
    SYS_ASSERT(rc == 0);

	return rc;
}
