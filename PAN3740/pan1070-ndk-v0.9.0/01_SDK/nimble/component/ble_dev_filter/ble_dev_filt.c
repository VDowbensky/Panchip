/**
 *******************************************************************************
 * @file     ble_dev_filt.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "ble_dev_filt.h"

/*******************************************************************************
 * Macro Define
 ******************************************************************************/


/*******************************************************************************
 * Variable Define
 ******************************************************************************/
typedef union{
	struct{
		uint16_t deviceName   : 1;
		uint16_t rssi         : 1;
		uint16_t addr         : 1;
		uint16_t addrType     : 1;
		uint16_t uuid16       : 1;
		uint16_t uuid128      : 1;
		uint16_t svrData      : 1;
		uint16_t manuData     : 1;
	};
	uint16_t val;
}filter_set_t;

typedef struct{
	uint8_t   val[6];
	uint8_t   matchLen;
}dev_addr_t;

typedef struct{
	filter_set_t  filterSet;
	uint8_t       mode;

	char         *name;
	uint8_t       nameLen;

	int8_t        rssi;

	dev_addr_t    addr[BLE_DEV_FILT_MAX_ADDR_NUM];
	uint8_t       addrCnt;
	uint8_t       addrType;

	//UUID
	uuid16_t      uuid16;
	uuid128_t     uuid128;

	// service data
	uint8_t      *serviceData;
	uint8_t       serviceDataOffset;
	uint8_t       serviceDataLen;

	//manufacturer data
	uint8_t      *manuData;
	uint8_t       manuDataOffset;
	uint8_t       manuDataLen;
}device_filter_t;

device_filter_t deviceFltr;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
extern void *ble_dev_filt_alloc(uint32_t size);
extern void ble_dev_filt_free(void *p);

/*******************************************************************************
 * Function Define
 ******************************************************************************/
void ble_dev_filt_reset(void)
{
	// Release memory
	ble_dev_filt_free(deviceFltr.name);
	deviceFltr.name = NULL;

	ble_dev_filt_free(deviceFltr.serviceData);
	deviceFltr.serviceData = NULL;

	ble_dev_filt_free(deviceFltr.manuData);
	deviceFltr.manuData = NULL;

	//clear 0
	deviceFltr.mode 			= BLE_DEV_FILT_MODE_ALL;
	deviceFltr.rssi 			= 0;
	deviceFltr.addrCnt 			= 0;
	deviceFltr.addrType 		= 0;
	deviceFltr.nameLen 			= 0;
	deviceFltr.serviceDataOffset= 0;
	deviceFltr.serviceDataLen 	= 0;
	deviceFltr.manuDataOffset 	= 0;
	deviceFltr.manuDataLen 		= 0;

	memset(&deviceFltr.addr, 0, sizeof(deviceFltr.addr));

	deviceFltr.filterSet.val = 0;
}

void ble_dev_filt_init(void)
{
	ble_dev_filt_reset();
}

void ble_dev_filt_set_match_mode(uint8_t mode)
{
	deviceFltr.mode = mode;
}

int ble_dev_filt_set_device_name(char *name, uint32_t len)
{
	if(name == NULL || len == 0){
		return BDF_INVALID_PARAM;
	}

	if(deviceFltr.name){
		ble_dev_filt_free(deviceFltr.name);
		deviceFltr.name = NULL;
	}

	deviceFltr.name = ble_dev_filt_alloc(len + 1);
	if(deviceFltr.name == NULL){
		return BDF_ALLOC_FAILED;
	}

	memcpy(deviceFltr.name, name, len);
	deviceFltr.nameLen = len;
	deviceFltr.filterSet.deviceName = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_device_name(void)
{
	if(deviceFltr.name){
		ble_dev_filt_free(deviceFltr.name);
	}
	deviceFltr.name = NULL;
	deviceFltr.nameLen = 0;
	
	deviceFltr.filterSet.deviceName = 0;
	
	return BDF_OK;
}

int ble_dev_filt_set_rssi(int8_t rssi)
{
	deviceFltr.rssi = rssi;
	deviceFltr.filterSet.rssi = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_rssi(void)
{
	deviceFltr.rssi = -127;
	deviceFltr.filterSet.rssi = 0;
	
	return BDF_OK;
}

int ble_dev_filt_set_addr_type(uint8_t addrType)
{
	deviceFltr.addrType = addrType;
	deviceFltr.filterSet.addrType = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_addr_type(void)
{
	deviceFltr.addrType = 0;
	deviceFltr.filterSet.addrType = 0;

	return BDF_OK;
}

int ble_dev_filt_add_addr(uint8_t addr[6], uint8_t matchLen)
{
	if(addr == NULL){
		return BDF_INVALID_PARAM;
	}

	if(deviceFltr.addrCnt >= BLE_DEV_FILT_MAX_ADDR_NUM){
		return BDF_RES_NO_ENOUGH;
	}

	dev_addr_t *pAddr = &deviceFltr.addr[deviceFltr.addrCnt++];

	memcpy(pAddr->val, addr, 6);
	pAddr->matchLen = matchLen;
	deviceFltr.filterSet.addr = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_addr(uint8_t addr[6])
{
	device_filter_t *pDevFltr = &deviceFltr;

	for(uint32_t i=0; i<pDevFltr->addrCnt; i++)
	{
		if(memcmp(addr, pDevFltr->addr[i].val, 6) == 0)
		{
			for(uint32_t k=i; k<pDevFltr->addrCnt-1; k++){
				pDevFltr->addr[k] = pDevFltr->addr[k+1];
			}

			if(pDevFltr->addrCnt){
				pDevFltr->addrCnt--;
			}
		}
	}
	
	if(pDevFltr->addrCnt == 0){
		deviceFltr.filterSet.addr = 0;
	}
	return BDF_OK;
}

int ble_dev_filt_remove_all_addr(void)
{
	deviceFltr.addrCnt = 0;
	deviceFltr.filterSet.addr = 0;
	return BDF_OK;
}

int ble_dev_filt_set_uuid16(uint16_t uuid16)
{
	deviceFltr.uuid16.val = uuid16;
	deviceFltr.filterSet.uuid16 = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_uuid16(void)
{
	deviceFltr.filterSet.uuid16 = 0;
	return BDF_OK;
}

int ble_dev_filt_set_uuid128(uint8_t uuid[16])
{
	memcpy(deviceFltr.uuid128.val, uuid, 16);
	deviceFltr.filterSet.uuid128 = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_uuid128(void)
{
	memset(deviceFltr.uuid128.val, 0, 16);
	deviceFltr.filterSet.uuid128 = 0;
	
	return BDF_OK;
}

int ble_dev_filt_set_manu_data(uint8_t *pdata, uint32_t len)
{
	if(pdata == NULL || len == 0){
		return BDF_INVALID_PARAM;
	}

	if(deviceFltr.manuData){
		ble_dev_filt_free(deviceFltr.manuData);
		deviceFltr.manuData = NULL;
	}

	uint8_t *p = ble_dev_filt_alloc(len);
	if(p == NULL){
		return BDF_ALLOC_FAILED;
	}

	deviceFltr.manuData = p;
	deviceFltr.manuDataLen = len;
	memcpy(p, pdata, len);
	deviceFltr.filterSet.manuData = 1;

	return BDF_OK;
}

int ble_dev_filt_remove_manu_data(void)
{
	if(deviceFltr.manuData){
		ble_dev_filt_free(deviceFltr.manuData);
	}
	deviceFltr.manuData = NULL;
	
	deviceFltr.manuDataLen = 0;
	deviceFltr.filterSet.manuData = 0;
	
	return BDF_OK;
}

bool ble_dev_filt_is_match_internal(uint8_t addr[6], uint8_t addrType, int8_t rssi, const uint8_t *pAdvData, uint32_t len)
{
	filter_set_t fltrSet;
	fltrSet.val = 0;

	device_filter_t *pDevFltr = &deviceFltr;

	if(pDevFltr->filterSet.val == 0){
		return false;
	}

	/* Address Match */
	if(pDevFltr->filterSet.addr)
	{
		for(uint32_t i=0; i<pDevFltr->addrCnt; i++)
		{
			dev_addr_t *pAddr = &pDevFltr->addr[i];
			if(memcmp(pAddr->val, addr, pAddr->matchLen) == 0){
				fltrSet.addr = 1;
				break;
			}
		}
	}

	/* Address Type Match */
	if(pDevFltr->filterSet.addrType)
	{
		if(addrType == pDevFltr->addrType){
			fltrSet.addrType = 1;
		}
	}

	/* RSSI Match */
	if(pDevFltr->filterSet.rssi)
	{
		if(rssi >= pDevFltr->rssi){
			fltrSet.rssi = 1;
		}
	}

	/* Adv Data Match */
	if(pAdvData  && len)
	{
		struct ble_hs_adv_fields adv_fields = {0};

		int rc = ble_hs_adv_parse_fields(&adv_fields, pAdvData, len);
		if(rc == 0)
		{
			//name
			if(pDevFltr->filterSet.deviceName && adv_fields.name_len){
				if(adv_fields.name_len == pDevFltr->nameLen &&
				  !memcmp(pDevFltr->name, adv_fields.name, adv_fields.name_len)){
					fltrSet.deviceName = 1;
				}
			}

			//uuid16
			if(pDevFltr->filterSet.uuid16 && adv_fields.num_uuids16)
			{
				for(int i=0; i<adv_fields.num_uuids16; i++){
					if(pDevFltr->uuid16.val == adv_fields.uuids16[i].value){
						fltrSet.uuid16 = 1;
						break;
					}
				}
			}

			//uuid128
			if(pDevFltr->filterSet.uuid128 && adv_fields.num_uuids128)
			{
				for(int i=0; i<adv_fields.num_uuids128; i++){
					if(!memcmp(pDevFltr->uuid128.val, adv_fields.uuids128[i].value, 16)){
						fltrSet.uuid128 = 1;
						break;
					}
				}
			}

			//Manufacturer data
			if(pDevFltr->filterSet.manuData && adv_fields.mfg_data_len){
				if(!memcmp(pDevFltr->manuData, adv_fields.mfg_data, adv_fields.mfg_data_len)){
					fltrSet.manuData = 1;
				}
			}
		}
	}

	if(pDevFltr->mode == BLE_DEV_FILT_MODE_ALL && (fltrSet.val & pDevFltr->filterSet.val) == pDevFltr->filterSet.val){
		return true;
	}
	if(pDevFltr->mode == BLE_DEV_FILT_MODE_SINGLE && (fltrSet.val & pDevFltr->filterSet.val)){
		return true;
	}
	return false;
}


bool ble_dev_filt_is_match(const struct ble_gap_disc_desc *desc)
{
	return ble_dev_filt_is_match_internal((uint8_t*)desc->addr.val, desc->addr.type, desc->rssi, desc->data, desc->length_data);
}
