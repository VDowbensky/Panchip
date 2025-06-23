
#ifndef H_BLE_SVC_DIS_
#define H_BLE_SVC_DIS_

/*!< DIS UUID */
#define BLE_SVC_DIS_UUID16					            0x180A
#define BLE_SVC_DIS_CHR_UUID16_SYSTEM_ID			    0x2A23
#define BLE_SVC_DIS_CHR_UUID16_MODEL_NUMBER			    0x2A24
#define BLE_SVC_DIS_CHR_UUID16_SERIAL_NUMBER			0x2A25
#define BLE_SVC_DIS_CHR_UUID16_FIRMWARE_REVISION 		0x2A26
#define BLE_SVC_DIS_CHR_UUID16_HARDWARE_REVISION 		0x2A27
#define BLE_SVC_DIS_CHR_UUID16_SOFTWARE_REVISION 		0x2A28
#define BLE_SVC_DIS_CHR_UUID16_MANUFACTURER_NAME		0x2A29


#define BLE_SVC_DIS_MODEL_NUMBER_DEFAULT        "PN"
#define BLE_SVC_DIS_SERIAL_NUMBER_DEFAULT       "202411141511"
#define BLE_SVC_DIS_FIRMWARE_REVISION_DEFAULT   "v0.0.0"
#define BLE_SVC_DIS_HARDWARE_REVISION_DEFAULT   "EVK-v1.5"
#define BLE_SVC_DIS_SOFTWARE_REVISION_DEFAULT   "NDK-v0.8.0"
#define BLE_SVC_DIS_MANUFACTURER_NAME_DEFAULT   "PanChip"
#define BLE_SVC_DIS_SYSTEM_ID_DEFAULT           "0001"

/**
 * Structure holding data for the main characteristics
 */
struct ble_svc_dis_data {
    /**
     * Model number.
     * Represent the model number that is assigned by the device vendor.
     */
    const char *model_number;
    /**
     * Serial number.
     * Represent the serial number for a particular instance of the device.
     */
    const char *serial_number;
    /**
     * Firmware revision.
     * Represent the firmware revision for the firmware within the device.
     */
    const char *firmware_revision;
    /**
     * Hardware revision.
     * Represent the hardware revision for the hardware within the device.
     */
    const char *hardware_revision;
    /**
     * Software revision.
     * Represent the software revision for the software within the device.
     */
    const char *software_revision;
    /**
     * Manufacturer name.
     * Represent the name of the manufacturer of the device.
     */
    const char *manufacturer_name;
    /**
     * System ID.
     * Represent the System Id of the device.
     */
    const char *system_id;
};

extern struct ble_svc_dis_data ble_svc_dis_data;

void ble_svc_dis_init(void);

const char *ble_svc_dis_model_number(void);
int ble_svc_dis_model_number_set(const char *value);

const char *ble_svc_dis_serial_number(void);
int ble_svc_dis_serial_number_set(const char *value);

const char *ble_svc_dis_firmware_revision(void);
int ble_svc_dis_firmware_revision_set(const char *value);

const char *ble_svc_dis_hardware_revision(void);
int ble_svc_dis_hardware_revision_set(const char *value);

const char *ble_svc_dis_software_revision(void);
int ble_svc_dis_software_revision_set(const char *value);

const char *ble_svc_dis_manufacturer_name(void);
int ble_svc_dis_manufacturer_name_set(const char *value);

const char *ble_svc_dis_system_id(void);
int ble_svc_dis_system_id_set(const char *value);

#endif
