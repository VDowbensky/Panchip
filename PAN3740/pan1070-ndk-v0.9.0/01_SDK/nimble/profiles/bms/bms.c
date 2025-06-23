/*
 * bms.c (Bond Management Service)
 */

#include "bms.h"
#include "pan_ble.h"
#include "pan_sys_log.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
#define BMS_CP_DATA_LEN		100//511

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
uint16_t bms_ctrl_point_handle;
uint16_t bms_feature_handle;

typedef struct{
	uint32_t 			feature;
	bms_bond_cbs_t  	bond_funcs;
}__attribute__((packed)) bms_t;

bms_t bms_cb;
uint8_t bms_cp_buf[1 + BMS_CP_DATA_LEN] = {0};

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static int ble_svc_bms_access(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);
int ble_svc_bms_cp_handler(uint16_t conn_handle, struct os_mbuf *om);

static const struct ble_gatt_svc_def  ble_svc_bms_def[] =
{
	{
		/* Service: Bond Management Service */
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID16_DECLARE(BLE_SVC_BMS_UUID16),
		.characteristics = (struct ble_gatt_chr_def[])
		{
			{
				/* Characteristic: Bond Management Control Point */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_BMS_CHR_UUID16_BM_CP),
				.access_cb = ble_svc_bms_access,
				.val_handle = &bms_ctrl_point_handle,
				.flags = BLE_GATT_CHR_F_WRITE,
			},
			{
				/* Characteristic: Bond Management Feature */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_BMS_CHR_UUID16_BM_FEATURE),
				.access_cb = ble_svc_bms_access,
				.val_handle = &bms_feature_handle,
				.flags = BLE_GATT_CHR_F_READ, //|BLE_GATT_CHR_F_INDICATE
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

static int ble_svc_bms_access(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint16_t uuid = 0;

	switch(ctxt->op)
	{
	case BLE_GATT_ACCESS_OP_READ_CHR:
	{
		uuid = ble_uuid_u16(ctxt->chr->uuid);
		if(uuid == BLE_SVC_BMS_CHR_UUID16_BM_FEATURE) {
			os_mbuf_append(ctxt->om, (uint8_t*)&bms_cb.feature, 3);
			return 0;
		}
		break;
	}
	case BLE_GATT_ACCESS_OP_WRITE_CHR:
	{
		uuid = ble_uuid_u16(ctxt->chr->uuid);
		if(uuid == BLE_SVC_BMS_CHR_UUID16_BM_CP){
			return ble_svc_bms_cp_handler(conn_handle, ctxt->om);
		}
		break;
	}
	default:
		break;
	}

	return BLE_ATT_ERR_UNLIKELY;
}

int ble_svc_bms_cp_handler(uint16_t conn_handle, struct os_mbuf *om)
{
    uint16_t len = OS_MBUF_PKTLEN(om);
	if(len > sizeof(bms_cp_buf)){
		return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
	}

	int rc = os_mbuf_copydata(om, 0, len, bms_cp_buf);
	if(rc){
		return BLE_ATT_ERR_UNLIKELY;
	}

	uint8_t *pdata = bms_cp_buf;
	uint8_t opcode = pdata[0];
	pdata++;
	len -= 1;

	switch(opcode)
	{
	case BMS_OPCODE_DEL_BOND_REQ_DEVICE_LE_ONLY:
		if(bms_cb.bond_funcs.delete_req_device){
			rc = bms_cb.bond_funcs.delete_req_device(conn_handle);
			if(rc == 0){
				return 0;
			}
		}
		SYS_WRN("BMS_OPCODE_DEL_BOND_REQ_DEVICE_LE_ONLY \r\n");
		return BLE_ATT_ERR_OP_FAILED;

	case BMS_OPCODE_DEL_ALL_BONDS_ON_SERVER_LE:
		if(bms_cb.bond_funcs.delete_all){
			rc = bms_cb.bond_funcs.delete_all(conn_handle);
			if(rc == 0){
				return 0;
			}
		}
		SYS_WRN("BMS_OPCODE_DEL_ALL_BONDS_ON_SERVER_LE \r\n");
		return BLE_ATT_ERR_OP_FAILED;

	case BMS_OPCODE_DEL_ALL_BUT_ACTIVE_BOND_LE:
		if(bms_cb.bond_funcs.delete_all_except_req){
			rc = bms_cb.bond_funcs.delete_all_except_req(conn_handle);
			if(rc == 0){
				return 0;
			}
		}
		SYS_WRN("BMS_OPCODE_DEL_ALL_BUT_ACTIVE_BOND_LE \r\n");
		return BLE_ATT_ERR_OP_FAILED;

	default:
		return BLE_ATT_ERR_OPCODE_NOT_SUP;
	}
}

void ble_svc_bms_register_bond_handler(bms_bond_cbs_t *bond_func)
{
	bms_cb.bond_funcs = *bond_func;
}

void ble_svc_bms_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(ble_svc_bms_def);
	SYS_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_bms_def);
	SYS_ASSERT(rc == 0);
}


void bms_profile_init(bms_init_t *bms_init)
{
	/* set support featrue */
	//bms_cb.feature = BMS_FEAT_DEL_REQUESTING_DEVICE_LE|BMS_FEAT_DEL_ALL_BONDS_LE;
	bms_cb.feature    = bms_init->featrue;
	bms_cb.bond_funcs = bms_init->bms_bond_cb;
}


#if 0
int ble_svc_bms_del_bond_req_device(uint8_t *p, uint32_t len)
{
	ble_addr_t peer_id_addr;

	peer_id_addr.type = p[0];
	memcpy(peer_id_addr.val, p+1, 6);

	ble_store_util_delete_peer(&peer_id_addr);

	return 0;
}

int ble_svc_bms_del_all_bonds_on_server(uint8_t *p, uint32_t len)
{
	ble_store_clear();
	return 0;
}
#endif
