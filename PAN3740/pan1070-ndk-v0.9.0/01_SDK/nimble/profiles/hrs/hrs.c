/**
 *******************************************************************************
 * @file     hrs.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#pragma diag_suppress 111

#include "hrs.h"
#include "host/ble_hs.h"
#include "pan_sys_log.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
uint16_t hrs_hrm_handle;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static int ble_svc_hrs_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);


static const struct ble_gatt_svc_def  ble_svc_hrs_def[] =
{
	{
        /* Service: Heart-rate */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_HRS_UUID16),
        .characteristics = (struct ble_gatt_chr_def[])
		{
			{
				/* Characteristic: Heart-rate measurement */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_HRS_CHR_UUID16_MEASUREMENT),
				.access_cb = ble_svc_hrs_access,
				.val_handle = &hrs_hrm_handle,
				.flags = BLE_GATT_CHR_F_NOTIFY,
			},
			{
				/* Characteristic: Body sensor location */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_HRS_CHR_UUID16_BODY_SENSOR_LOC),
				.access_cb = ble_svc_hrs_access,
				.flags = BLE_GATT_CHR_F_READ,
			},
			{
				0, /* No more characteristics in this service */
			},
		}
    },
	{
		0, /* No more services */
	},
};

/*******************************************************************************
 * Function Define
 ******************************************************************************/
static int ble_svc_hrs_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    /* Sensor location, set to "Chest" */
    static uint8_t body_sens_loc = 0x01;
    uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);
    int rc;

    switch(uuid16)
    {
    case BLE_SVC_HRS_CHR_UUID16_BODY_SENSOR_LOC:
    	SYS_ASSERT(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

		rc = os_mbuf_append(ctxt->om, &body_sens_loc, sizeof(body_sens_loc));
		return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    default:
		SYS_ERR("Unrecognized BLE HRS Service CHR UUID16: 0x%04x\n", uuid16);
		return BLE_ATT_ERR_UNLIKELY;
    }
}

int ble_svc_hrs_notify(uint16_t conn_handle, uint8_t *pdata, uint32_t len)
{
	int rc = 0;
	struct os_mbuf *om = ble_hs_mbuf_from_flat(pdata, len);
	if(om== NULL){
		return BLE_NPL_ENOMEM;
	}

	rc = ble_gatts_notify_custom(conn_handle, hrs_hrm_handle, om);

	return rc;
}


void ble_svc_hrs_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(ble_svc_hrs_def);
	SYS_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_hrs_def);
	SYS_ASSERT(rc == 0);
}
