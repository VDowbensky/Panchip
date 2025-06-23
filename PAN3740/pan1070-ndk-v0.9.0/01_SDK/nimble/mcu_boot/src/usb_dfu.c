#include "soc_api.h"
#include "app_log.h"
#include "usb_dfu.h"
#include "img_hdr.h"

#include "musbfsfc.h"
#include "descript.h"
#include "endpoint.h"
#include "endpoint0.h"
#include "PanSeries.h"
#include "usb_panchip.h"

volatile uint8_t ready_to_jump_app = 0;
static uint8_t crc_cmp_status = 0;

#define DFU_LOG_ENABLE          0
#define DFU_TEST                0
#if DFU_LOG_ENABLE
#define DFU_LOG                 APP_LOG
#else
#define DFU_LOG(...)
#endif
#define USB_VENDOR_CONNECT                                      0x55
#define USB_VENDOR_DFU_GET_VERSION                              0x00
#define USB_VENDOR_DFU_CHECK_VERSION                            0x01
#define USB_VENDOR_DFU_START                                    0x02
#define USB_VENDOR_DFU_FINISH                                   0x03
#define USB_VENDOR_DFU_END                                      0x04
#define USB_VENDOR_DFU_RESET                                    0x05
#define USB_VENDOR_DFU_FORCE_UPGRADE                            0x06
#define USB_VENDOR_DFU_TRANSFER                                 0x10
#define USB_VENDOR_DFU_EP1_CONTROL                              0x20
#define USB_VENDOR_BLE_TEST                                     0x30
#define USB_VENDOR_PRF_TEST                                     0x31
#define USB_GET_OWN_ADDR                                        0x35
#define USB_GET_PAIR_ADDR                                       0x36
#define USB_EMI_CHECK_VERSION                                   0x37
#define USB_VENTDOR_TEST                                        0x40

struct usb_vendor_rx_data_t {
	uint8_t usb_report_id;
	uint8_t usb_cmd;
	uint8_t usb_rx_data[64];
	uint8_t usb_rx_len;
};

struct usb_vendor_tx_data_t {
	uint8_t usb_report_id;
	uint8_t usb_cmd;
	uint8_t usb_tx_data[64];
	uint8_t usb_tx_len;
};

struct usb_vendor_dfu_t {
	bool connect_status;
	uint16_t data_cache_size;
	uint8_t dfu_data_cache[320];
	uint8_t image_area;
	uint32_t image_start_addr;
	struct image_version check_image_version;
	uint32_t image_size;
	int32_t image_crc32;
	uint8_t status;
	uint16_t page_num;
	uint16_t image_last_page;
	uint16_t image_last_page_size;
	uint32_t dfu_load_addr;
	struct image_version controller_version;
	struct image_version app_version;
};
struct usb_vendor_dfu_t usb_vendor_dfu;

#if DFU_LOG_ENABLE
static void dfu_print_data(uint8_t const *data, uint32_t len)
{
#if DFU_LOG_PRINT_DATA
	uint32_t i = 0;

	if (len == 0) {
		return;
	}

	for (; i < len; i++) {
		DFU_LOG("0x%02X ", data[i]);
	}
	DFU_LOG("\n");
#endif
}
#endif // DFU_LOG_ENABLE

#define CRC32_INIT      0xffffffff
#define CRC32_XOROT     0xffffffff
static const uint32_t CRC32_Table[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

uint32_t CRC_CalcCrc32(uint8_t *pStart, uint32_t uSize)
{
	uint32_t uCRCValue;
	uint8_t *pData;

	/* init the start value */
	uCRCValue = CRC32_INIT;
	pData = pStart;

	/* Flush I-cache before reading data from flash using C pointer */
	extern void FMC_FlushCache(void);
	FMC_FlushCache();

	/* calculate CRC */
	while (uSize--) {
		uCRCValue = CRC32_Table[(uCRCValue ^ *pData++) & 0xFF] ^ (uCRCValue >> 8);
	}
	/* XOR the output value */
	return uCRCValue ^ CRC32_XOROT;
}

static void usb_vendor_ep2_in(uint8_t len, uint8_t *data)
{
	uint8_t send_data[64];

	memset(send_data, 0xff, sizeof(send_data));
	memcpy(send_data, data, len);
	WRITE_REG(USB->INDEX, 2);
	USB_Write((uint32_t)2, 64, (void *)send_data);
	WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);
}

static bool report_id_check(uint8_t data)
{
	if (data == 0x0a) {
		DFU_LOG("true report id 0x%02x\n", data);
		return true;
	} else {
		DFU_LOG("false report id 0x%02x\n", data);
		return false;
	}
}

static void dfu_get_version_area(uint8_t image_area, uint8_t *p_reply_data)
{

//	if (image_area == 0x00) {
//		DFU_LOG("controller image\n");
//		memcpy(p_reply_data, &usb_vendor_dfu.controller_version, 8);
//	} else {
//		DFU_LOG("app image\n");
//		memcpy(p_reply_data, &usb_vendor_dfu.app_version, 8);
//	}
	struct img_hdr *image_header_p = (void *)CONFIG_FLASH_PARTITION_APP_ADDR;

	if (image_header_p->ih_magic == IMAGE_MAGIC) {
		DFU_LOG("APP image header check passed, image version: %d.%d.%d.%d\n",
			image_header_p->ih_ver.iv_major,
			image_header_p->ih_ver.iv_minor,
			image_header_p->ih_ver.iv_revision,
			image_header_p->ih_ver.iv_build_num
			);
		memcpy(p_reply_data, &(image_header_p->ih_ver), 8);
	}
}

static bool check_image_size(void)
{
#if DFU_TEST
	return true;
#else
	if (usb_vendor_dfu.image_size > 0x28000) {
		return false;
	} else {
		return true;
	}
#endif

}

static void cal_last_data(void)
{
	if (usb_vendor_dfu.image_size % 256) {
		usb_vendor_dfu.image_last_page = usb_vendor_dfu.image_size / 256;
		usb_vendor_dfu.image_last_page_size = usb_vendor_dfu.image_size % 256;
	} else {
		usb_vendor_dfu.image_last_page = usb_vendor_dfu.image_size / 256 - 1;
		usb_vendor_dfu.image_last_page_size = 256;
	}
	DFU_LOG("image_last_page %d, image_last_page_size %d\n",
		usb_vendor_dfu.image_last_page, usb_vendor_dfu.image_last_page_size);
}

static void dfu_check_version_area(uint8_t *check_version_infor, uint8_t *p_reply_data)
{
	/* uint32_t image_exsit_version; */
	uint8_t check_status = 0x00;

	memcpy(&usb_vendor_dfu.image_start_addr, check_version_infor, 4);
	memcpy(&usb_vendor_dfu.check_image_version, check_version_infor + 4, 8);
	memcpy(&usb_vendor_dfu.image_size, check_version_infor + 12, 4);

	memcpy(&usb_vendor_dfu.image_crc32, check_version_infor + 16, 4);

	DFU_LOG("image_start_addr 0x%08x\n", usb_vendor_dfu.image_start_addr);
	DFU_LOG("iv_major  0x%02x\n", usb_vendor_dfu.check_image_version.iv_major);
	DFU_LOG("iv_minor 0x%02x\n", usb_vendor_dfu.check_image_version.iv_minor);
	DFU_LOG("iv_revision 0x%04x\n", usb_vendor_dfu.check_image_version.iv_revision);
	DFU_LOG("iv_build_num 0x%08x\n", usb_vendor_dfu.check_image_version.iv_build_num);
	DFU_LOG("image_size 0x%08x\n", usb_vendor_dfu.image_size);
	DFU_LOG("image_crc32 0x%08x\n ", usb_vendor_dfu.image_crc32);

	/* check image_area */
	memcpy(&usb_vendor_dfu.image_area, check_version_infor, 4);
	if (usb_vendor_dfu.image_start_addr == CONTROLLER_HEADER_ADDR) {
		usb_vendor_dfu.image_area = 0x00;
	} else if (usb_vendor_dfu.image_start_addr == APP_HEADER_ADDR) {
		usb_vendor_dfu.image_area = 0x01;
	}
	/* check image_version */

	/* check image_size */
	if (check_image_size()) {
		DFU_LOG("size fit %d\n", usb_vendor_dfu.image_size);
	} else {
		DFU_LOG("image over size %d max 160k", usb_vendor_dfu.image_size);
	}
	/* check image_crc32 */

	memcpy(p_reply_data, &check_status, 1);
	memcpy(p_reply_data + 1, &usb_vendor_dfu.image_area, 1);

}

static void flase_erase_for_dfu(uint32_t blob_size)
{
	DFU_LOG("blob_size = 0x%x\n", blob_size);
	if (blob_size % 4096) {
		blob_size = blob_size - blob_size % 4096 + 4096;
	} else {
		/* do nothing */
	}

	DFU_LOG("fit blob_size = 0x%x\n", blob_size);

	usb_vendor_dfu.dfu_load_addr = APP_HEADER_ADDR;

	DFU_LOG("usb_vendor_dfu.dfu_load_addr = 0x%x\n", usb_vendor_dfu.dfu_load_addr);

	// flash erase

	FMC_EraseCodeArea(FLCTL, APP_HEADER_ADDR, blob_size);


}

static void dfu_erase_image(void)
{
	flase_erase_for_dfu(usb_vendor_dfu.image_size);
}

static int32_t crc_cal_flash(uint32_t start_addr, uint32_t size)
{
    uint32_t chksum;

#if 1
    FMC_GetCrc32Sum(FLCTL, start_addr, size, &chksum);  // HW CRC32, 30ms+
#else
    chksum = CRC_CalcCrc32((uint8_t*)start_addr, size); // SW CRC32, 80ms+
#endif

    DFU_LOG("Calculated CRC32: 0x%08X\n", chksum);

    return chksum;
}

static void dfu_finish_check_crc(uint8_t *p_reply_data)
{
	int32_t crc_cal32;

	crc_cal32 = crc_cal_flash(usb_vendor_dfu.dfu_load_addr, usb_vendor_dfu.image_size);

	DFU_LOG("usb_vendor_dfu.image_crc32: 0x%08X\n", usb_vendor_dfu.image_crc32);

	if (crc_cal32 == usb_vendor_dfu.image_crc32) {
		/* success */
		DFU_LOG("crc pass\n");
		crc_cmp_status = 0;
	} else {
		/* fail */
		DFU_LOG("crc fail\n");
		crc_cmp_status = 1;
	}

	memcpy(p_reply_data, &crc_cmp_status, 1);
	memcpy(p_reply_data + 1, &crc_cal32, 4);

}

static void block_write_flash(uint32_t write_size)
{
	uint32_t addr;

	// flash write
	addr = usb_vendor_dfu.dfu_load_addr + usb_vendor_dfu.page_num * 256;

	if (FMC_WriteStream(FLCTL, addr, usb_vendor_dfu.dfu_data_cache, write_size)) {
		DFU_LOG("ERROR: Write Flash Fail\n");
	}

}

static void dfu_data_transfer(uint8_t *transfer_data, uint8_t *block_num, uint8_t data_len)
{
	uint8_t usb_reply_data[4] = { 0x0a, 0x10 };

	DFU_LOG("usb_vendor_dfu.data_cache_size %d\n", usb_vendor_dfu.data_cache_size);
	memcpy(usb_vendor_dfu.dfu_data_cache + usb_vendor_dfu.data_cache_size, transfer_data, data_len);

	usb_vendor_dfu.data_cache_size = usb_vendor_dfu.data_cache_size + data_len;
	DFU_LOG("usb_vendor_dfu.data_cache_size %d\n", usb_vendor_dfu.data_cache_size);
#if DFU_LOG_ENABLE
	dfu_print_data(usb_vendor_dfu.dfu_data_cache, usb_vendor_dfu.data_cache_size);
#endif
	if ((usb_vendor_dfu.image_last_page == usb_vendor_dfu.page_num)
	    && (usb_vendor_dfu.data_cache_size >= usb_vendor_dfu.image_last_page_size)) {
		DFU_LOG("last data\n");
		block_write_flash(usb_vendor_dfu.image_last_page_size);
		DFU_LOG("page_num %d\n", usb_vendor_dfu.page_num);
		usb_vendor_dfu.data_cache_size = usb_vendor_dfu.image_last_page_size;
		DFU_LOG("data_cache_size %d\n", usb_vendor_dfu.data_cache_size);
#if DFU_LOG_ENABLE
		dfu_print_data(usb_vendor_dfu.dfu_data_cache, usb_vendor_dfu.data_cache_size);
#endif
		memcpy(usb_reply_data + 2, &(usb_vendor_dfu.page_num), 2);
		usb_vendor_ep2_in(4, usb_reply_data);
		return;
	}

	if (usb_vendor_dfu.data_cache_size >= 256) {
		DFU_LOG("%d ", usb_vendor_dfu.page_num);
		if ((usb_vendor_dfu.image_last_page == usb_vendor_dfu.page_num + 1)
		    && ((usb_vendor_dfu.data_cache_size - 256) >= usb_vendor_dfu.image_last_page_size)) {
			/* usb_vendor_dfu.data_cache_size = usb_vendor_dfu.image_last_page_size; */
			DFU_LOG("last data write %d %d\n",
				usb_vendor_dfu.data_cache_size, usb_vendor_dfu.page_num);
			block_write_flash(usb_vendor_dfu.data_cache_size);
			memcpy(usb_reply_data + 2, &(usb_vendor_dfu.page_num), 2);
			usb_vendor_ep2_in(4, usb_reply_data);
			return;
		}
		/* flash write 256 */
		DFU_LOG("flash write 256\n");
		block_write_flash(256);
		usb_vendor_dfu.data_cache_size = usb_vendor_dfu.data_cache_size - 256;
		memcpy(usb_vendor_dfu.dfu_data_cache,
		       usb_vendor_dfu.dfu_data_cache + 256, usb_vendor_dfu.data_cache_size);
		DFU_LOG("page_num %d\n", usb_vendor_dfu.page_num);
		memcpy(usb_reply_data + 2, &(usb_vendor_dfu.page_num), 2);
		usb_vendor_ep2_in(4, usb_reply_data);
		usb_vendor_dfu.page_num++;


	} else {
		DFU_LOG("do nothing\n");
	}
#if DFU_LOG_ENABLE
	dfu_print_data(usb_vendor_dfu.dfu_data_cache, usb_vendor_dfu.data_cache_size);
#endif
	DFU_LOG("usb_vendor_dfu.data_cache_size %d\n", usb_vendor_dfu.data_cache_size);
}
bool dfu_on_flag;
static void dfu_ep1_control(uint8_t ep1_onoff_flag)
{
	if (ep1_onoff_flag == 0x00) {
		DFU_LOG("ep1 off\n");
		dfu_on_flag = true;
	} else {
		DFU_LOG("ep1 on\n");
	}
}

void dfu_flag_set(void)
{
	const uint32_t flash_sector_start_addr = CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR + CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE - 0x1000; // last user custom sector

	FMC_EraseCodeArea(FLCTL, flash_sector_start_addr, 4096);
	FMC_WriteByte(FLCTL, flash_sector_start_addr, USB_DFU_ENTER_FLAG);
}

void dfu_flag_check_clear(void)
{
	const uint32_t flash_sector_start_addr = CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR + CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE - 0x1000; // last user custom sector
	uint8_t dfu_flag;

	dfu_flag = FMC_ReadByte(FLCTL, flash_sector_start_addr, CMD_DREAD);
	if (dfu_flag == USB_DFU_ENTER_FLAG) {
		FMC_WriteByte(FLCTL, flash_sector_start_addr, 0x00); // Clear DFU flag to 0x00
	}
}

bool is_dfu_flag_valid(void)
{
	const uint32_t flash_sector_start_addr = CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR + CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE - 0x1000; // last user custom sector
	uint8_t dfu_flag;

	dfu_flag = FMC_ReadByte(FLCTL, flash_sector_start_addr, CMD_DREAD);

	if (dfu_flag == USB_DFU_ENTER_FLAG) {
		return true;
	} else {
		return false;
	}
}

void usb_vendor_ep_out(uint8_t ep_index)
{
	uint8_t usb_data[64];
	uint8_t usb_data_len;
	uint8_t usb_reply_data[10] = { 0x0 };

	WRITE_REG(USB->INDEX, ep_index);
	usb_data_len = READ_REG(USB->OUT_COUNT1);
	DFU_LOG("usb_data_len %d\n", usb_data_len);
	USB_Read(ep_index, usb_data_len, usb_data);
#if DFU_LOG_ENABLE
	dfu_print_data(usb_data, usb_data_len);
#endif
	WRITE_REG(USB->INDEX, ep_index);

	WRITE_REG(USB->OUT_CSR1, 0);

	report_id_check(usb_data[0]);

	DFU_LOG("\ncmd: (0x%02X) ", usb_data[1]);

	switch (usb_data[1]) {
	case USB_VENDOR_CONNECT:
		DFU_LOG("USB_VENDOR_CONNECT\n");
		usb_vendor_dfu.connect_status = true;
		memcpy(usb_reply_data, usb_data, 2);
		usb_vendor_ep2_in(2, usb_reply_data);
		break;

	case USB_VENDOR_DFU_GET_VERSION:
		DFU_LOG("USB_VENDOR_DFU_GET_VERSION\n");
		memcpy(usb_reply_data, usb_data, 3);
		dfu_get_version_area(usb_data[2], &usb_reply_data[3]);
		usb_vendor_ep2_in(11, usb_reply_data);
		break;

	case USB_VENDOR_DFU_CHECK_VERSION:
		DFU_LOG("USB_VENDOR_DFU_CHECK_VERSION\n");
		memcpy(usb_reply_data, usb_data, 3);
		dfu_check_version_area(&usb_data[2], &usb_reply_data[2]);
		usb_vendor_ep2_in(4, usb_reply_data);
		break;

	case USB_VENDOR_DFU_START:
		DFU_LOG("USB_VENDOR_DFU_START\n");
		memcpy(usb_reply_data, usb_data, 2);
		usb_vendor_ep2_in(2, usb_reply_data);
		cal_last_data();
		dfu_erase_image();
		usb_vendor_dfu.page_num = 0;
		usb_vendor_dfu.data_cache_size = 0;
		usb_reply_data[2] = 0x00;
		usb_vendor_ep2_in(3, usb_reply_data);
		break;

	case USB_VENDOR_DFU_FINISH:
		DFU_LOG("USB_VENDOR_DFU_FINISH\n");
		memcpy(usb_reply_data, usb_data, 2);
		usb_vendor_ep2_in(2, usb_reply_data);
		dfu_finish_check_crc(&usb_reply_data[2]);
		usb_vendor_ep2_in(7, usb_reply_data);
		break;

	case USB_VENDOR_DFU_END:
		DFU_LOG("USB_VENDOR_DFU_END\n");
		if (usb_data[2] == 0x00) {
			DFU_LOG("reboot after 30ms\n");
			memcpy(usb_reply_data, usb_data, 2);
			// delay and junmp to app
			if (crc_cmp_status) {
				DFU_LOG("crc32 check fail, do not jump to App!!!!!!!!\n");
			} else {
				usb_vendor_ep2_in(2, usb_reply_data);
				ready_to_jump_app = 1;
			}
		} else {
			memcpy(usb_reply_data, usb_data, 2);
			usb_vendor_ep2_in(2, usb_reply_data);
			DFU_LOG("not reboot\n");
		}
		break;

	case USB_VENDOR_DFU_RESET:
		DFU_LOG("USB_VENDOR_DFU_RESET\n");
		usb_vendor_dfu.data_cache_size = 0;
		memcpy(usb_reply_data, usb_data, 2);
		usb_vendor_ep2_in(2, usb_reply_data);
		break;

	// Response of this command would be different with same flow in App
	case USB_VENDOR_DFU_FORCE_UPGRADE:
		DFU_LOG("USB_VENDOR_DFU_FORCE_UPGRADE\n");
		if (usb_data[2] == 0x00) {
			DFU_LOG("Notify DFU Mode to USB host: Mode%d\n", BOOT_USB_DFU_MODE);
			memcpy(usb_reply_data, usb_data, 2);
			// Assemble reply packet
			usb_reply_data[2] = 0x55;
			usb_reply_data[3] = 0xAA;
			usb_reply_data[4] = BOOT_USB_DFU_MODE;
			usb_reply_data[5] = (usb_reply_data[2] + usb_reply_data[3] + usb_reply_data[4]) & 0xFF;
			usb_vendor_ep2_in(6, usb_reply_data);
#if 0
			HAL_DelayMs(30);
			dfu_flag_set();
			/* reset */
			__disable_irq();
			SYS_UnlockReg();
			CLK_ResetChip();
#endif
		} else {
			DFU_LOG("Try to clear DFU flag on flash\n");
			memcpy(usb_reply_data, usb_data, 2);
			usb_vendor_ep2_in(2, usb_reply_data);
			dfu_flag_check_clear();
		}
		break;

	case USB_VENDOR_DFU_TRANSFER:
		DFU_LOG("USB_VENDOR_DFU_TRANSFER\n");
		dfu_data_transfer(&usb_data[2], &usb_reply_data[2], usb_data_len - 2);
		break;

	case USB_VENDOR_DFU_EP1_CONTROL:
		DFU_LOG("USB_VENDOR_DFU_EP1_CONTROL\n");
		dfu_ep1_control(usb_data[2]);
		memcpy(usb_reply_data, usb_data, 3);
		usb_vendor_ep2_in(3, usb_reply_data);
		break;

	case USB_VENTDOR_TEST:
		DFU_LOG("test 18B ep2 in\n");
		uint8_t test_data[18];
		usb_vendor_ep2_in(18, test_data);
		break;

	default:
		DFU_LOG("Unrecognized opcode!\n");
		break;
	}
}

/* USB DFU END */
static bool plug_in_status = false;

void USB_IRQHandler(void)
{
	volatile uint8_t IntrUSB;
	volatile int16_t IntrIn;
	volatile int16_t IntrOut;

	IntrUSB = READ_REG(USB->INT_USB);
	IntrIn = (uint16_t)READ_REG(USB->INT_IN2);
	IntrIn <<= 8;
	IntrIn |= (uint16_t)READ_REG(USB->INT_IN1);
	IntrOut = (uint16_t)READ_REG(USB->INT_OUT2);
	IntrOut <<= 8;
	IntrOut |= (uint16_t)READ_REG(USB->INT_OUT1);


	if (IntrOut & (1 << 2)) {
		usb_vendor_ep_out(2);
	}

	/* Check for system interrupts */
	if (IntrUSB & M_INTR_PLUG) {
		if ((IntrUSB & M_INTR_PLUG_OUT) == M_INTR_PLUG_OUT) {
			plug_in_status = false;
			APP_LOG_INFO("---USB plug out---\n");
		} else if (IntrUSB & M_INTR_PLUG_OUT) {
			plug_in_status = true;
			APP_LOG_INFO("---USB plug in---\n");
		}
	}

	if (IntrUSB & M_INTR_SUSPEND) {
		APP_LOG_INFO("USB isr in: Suspend evt\n");
	}

	if (IntrUSB & M_INTR_RESUME) {
		APP_LOG_INFO("USB isr in: Resume evt\n");
	}

	if (IntrUSB & M_INTR_RESET) {
		APP_LOG_INFO("USB isr in: Reset evt\n");
		/* Panchip USB Reset Proc */
		USB_Reset();
	}

	/* Check for endpoint 0 interrupt */
	if (IntrIn & M_INTR_EP0) {
		uint8_t index_before;

		index_before = USB->INDEX;
		WRITE_REG(USB->INDEX, 0);
		/* Panchip USB EP0 Proc */
		Endpoint0(M_EP_NORMAL);
		WRITE_REG(USB->INDEX, index_before);
	}
}

bool get_plug_in_status(void)
{
	return plug_in_status;
}

void usb_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_USB_AHB | CLK_AHBPeriph_USB_48M, ENABLE);
	ANA->ANA_MISC_3V |= ((0x1 << 27) | (0x1 << 28));
	ANA->ANA_MISC_3V |= ((0X1 << 26));
	/*usb plug in debounce time*/
	SYS->CTRL1 = ((SYS->CTRL1 & ~0xffff) | 100);

	WRITE_REG(USB->INT_USBE, M_INTR_PLUG_ENABLE | M_INTR_SOF_ENABLE | M_INTR_RESET_ENABLE);

	APP_LOG_INFO("USB init done, INT_USBE: 0x%x\n", USB->INT_USBE);
	NVIC_SetPriority(USB_IRQn, 0);
	NVIC_EnableIRQ(USB_IRQn);
}

void on_usb_dfu_enter(void)
{
#if APP_LOG_EN
	APP_LOG_INFO("Entering USB DFU flow..\n");
	soc_busy_wait(10000); // Wait for print done
#endif

#if ROM_USB_DFU
	// Switch to ROM mode and setup the ROM USB DFU flow
    // The corresponding host PC tool is: 1) Panchip Toolbox 2) Pan10xxRomDfuConsoleTool
	__disable_irq();
	CLK_ResetSystemToRomMode();
#endif

	struct img_hdr *image_header_p = (void *)CONFIG_FLASH_PARTITION_APP_ADDR;
	if (image_header_p->ih_magic == IMAGE_MAGIC) {
		APP_LOG_INFO("Found an App Image, version: %d.%d.%d.%d\n",
			image_header_p->ih_ver.iv_major,
			image_header_p->ih_ver.iv_minor,
			image_header_p->ih_ver.iv_revision,
			image_header_p->ih_ver.iv_build_num);
	}

	usb_init();
	APP_LOG_INFO("Wait for DFU Comm..\n");
	while (1) {
		if (ready_to_jump_app) {
			APP_LOG_INFO("DFU done, exit USB DFU flow!\n\n");
			soc_busy_wait(10000);
			break;
		}
	}
	usb_self_plug_out();

	SYS_LockReg();
}
