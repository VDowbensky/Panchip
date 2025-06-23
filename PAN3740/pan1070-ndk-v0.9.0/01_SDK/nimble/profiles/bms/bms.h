/*
 * bms.h (Bond Management Service)
 */
#ifndef BMS_H
#define BMS_H

#include <stdint.h>

/**@brief BMS UUID */
#define BLE_SVC_BMS_UUID16						0x181E
#define BLE_SVC_BMS_CHR_UUID16_BM_CP        	0x2AA4
#define BLE_SVC_BMS_CHR_UUID16_BM_FEATURE		0x2AA5


/**@defgroup BMS opcode
 * @{ */
#define BMS_OPCODE_DEL_BOND_REQ_DEVICE_BR_LE			   0x01  //!< Delete bond of requesting device (BR/EDR and LE)
#define BMS_OPCODE_DEL_BOND_REQ_DEVICE_BR_ONLY			   0x02  //!< Delete bond of requesting device (only BR/EDR)
#define BMS_OPCODE_DEL_BOND_REQ_DEVICE_LE_ONLY		 	   0x03  //!< Delete bond of requesting device (only LE)
#define BMS_OPCODE_DEL_ALL_BONDS_ON_SERVER_BR_LE		   0x04  //!< Delete all bonds on server
#define BMS_OPCODE_DEL_ALL_BONDS_ON_SERVER_BR			   0x05  //!< Delete all bonds on server (only BR/EDR)
#define BMS_OPCODE_DEL_ALL_BONDS_ON_SERVER_LE		       0x06  //!< Delete all bonds on server (only LE)
#define BMS_OPCODE_DEL_ALL_BUT_ACTIVE_BOND_BR_LE      	   0x07  //!< Delete all bond devices except the currently active ones.
#define BMS_OPCODE_DEL_ALL_BUT_ACTIVE_BOND_BR      		   0x08  //!< Delete all bond devices except the currently active ones(only BR/EDR).
#define BMS_OPCODE_DEL_ALL_BUT_ACTIVE_BOND_LE      		   0x09  //!< Delete all bond devices except the currently active ones(only LE).
/** @} */


/** @defgroup  BMS feature bits
 * @{ */
#define BMS_FEAT_DEL_REQUESTING_DEVICE_BR_LE                      (1u << 0)  //!< Delete bond of the requesting device (BR/EDR and LE).
#define BMS_FEAT_DEL_REQUESTING_DEVICE_BR_LE_AUTH_CODE            (1u << 1)  //!< Delete bond of the requesting device (BR/EDR and LE) with an authorization code.
#define BMS_FEAT_DEL_REQUESTING_DEVICE_BR                         (1u << 2)  //!< Delete bond of the requesting device (BR/EDR transport only).
#define BMS_FEAT_DEL_REQUESTING_DEVICE_BR_AUTH_CODE               (1u << 3)  //!< Delete bond of the requesting device (BR/EDR transport only) with an authorization code.
#define BMS_FEAT_DEL_REQUESTING_DEVICE_LE                         (1u << 4)  //!< Delete bond of the requesting device (LE transport only).
#define BMS_FEAT_DEL_REQUESTING_DEVICE_LE_AUTH_CODE               (1u << 5)  //!< Delete bond of the requesting device (LE transport only) with an authorization code.
#define BMS_FEAT_DEL_ALL_BONDS_BR_LE                              (1u << 6)  //!< Delete all bonds on the device (BR/EDR and LE).
#define BMS_FEAT_DEL_ALL_BONDS_BR_LE_AUTH_CODE                    (1u << 7)  //!< Delete all bonds on the device (BR/EDR and LE) with an authorization code.
#define BMS_FEAT_DEL_ALL_BONDS_BR                                 (1u << 8)  //!< Delete all bonds on the device (BR/EDR transport only).
#define BMS_FEAT_DEL_ALL_BONDS_BR_AUTH_CODE                       (1u << 9)  //!< Delete all bonds on the device (BR/EDR transport only) with an authorization code.
#define BMS_FEAT_DEL_ALL_BONDS_LE                                 (1u << 10) //!< Delete all bonds on the device (LE transport only).
#define BMS_FEAT_DEL_ALL_BONDS_LE_AUTH_CODE                       (1u << 11) //!< Delete all bonds on the device (LE transport only) with an authorization code.
#define BMS_FEAT_DEL_ALL_EXCEPT_REQUESTING_DEVICE_BR_LE           (1u << 12) //!< Delete all bonds on the device except for the bond of the requesting device (BR/EDR and LE).
#define BMS_FEAT_DEL_ALL_EXCEPT_REQUESTING_DEVICE_BR_LE_AUTH_CODE (1u << 13) //!< Delete all bonds on the device except for the bond of the requesting device (BR/EDR and LE) with an authorization code.
#define BMS_FEAT_DEL_ALL_EXCEPT_REQUESTING_DEVICE_BR              (1u << 14) //!< Delete all bonds on the device except for the bond of the requesting device (BR/EDR transport only).
#define BMS_FEAT_DEL_ALL_EXCEPT_REQUESTING_DEVICE_BR_AUTH_CODE    (1u << 15) //!< Delete all bonds on the device except for the bond of the requesting device (BR/EDR transport only) with an authorization code.
#define BMS_FEAT_DEL_ALL_EXCEPT_REQUESTING_DEVICE_LE              (1u << 16) //!< Delete all bonds on the device except for the bond of the requesting device (LE transport only).
#define BMS_FEAT_DEL_ALL_EXCEPT_REQUESTING_DEVICE_LE_AUTH_CODE    (1u << 17) //!< Delete all bonds on the device except for the bond of the requesting device (LE transport only) with an authorization code.
/** @} */


/**@brief ATT Error Code */
#define BLE_ATT_ERR_OPCODE_NOT_SUP		0x80
#define BLE_ATT_ERR_OP_FAILED			0x81

typedef int (*bms_bond_handler_t)(uint16_t conn_handle);

typedef struct{
	bms_bond_handler_t	delete_req_device;
	bms_bond_handler_t  delete_all;
	bms_bond_handler_t  delete_all_except_req;
}bms_bond_cbs_t;

typedef struct{
	uint32_t 			featrue;
	bms_bond_cbs_t      bms_bond_cb;
}bms_init_t;

void ble_svc_bms_init(void);

void bms_profile_init(bms_init_t *bms);

#endif



