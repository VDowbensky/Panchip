/**
 *******************************************************************************
 * @file     pan_flash.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "pan_flash.h"
#include "utility.h"

struct flash_area g_code_area = {
    .fa_device_id = 0,
    .fa_id 	      = FLASH_AREA_IMAGE_0,
    .fa_off       = CONFIG_FLASH_PARTITION_APP_ADDR,
    .fa_size      = CONFIG_FLASH_PARTITION_APP_SIZE,
};

struct flash_area g_swap_area = {
    .fa_device_id = 1,
    .fa_id 	      = FLASH_AREA_IMAGE_1,
    .fa_off       = CONFIG_FLASH_PARTITION_APP_BACKUP_ADDR,
    .fa_size      = CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE,
};

#if FLASH_TIMING_OPTIMIZE_EN
uint8_t flash_page[256] = {0};
uint32_t flash_page_data_len = 0;
uint32_t flash_offset = 0;
struct flash_area *flash_area = NULL;

void img_mgmt_impl_reset_flash_param(void)
{
	flash_area = NULL;
	flash_offset = 0;

	memset(flash_page, 0xFF, 256);
	flash_page_data_len = 0;
}

void img_mgmt_impl_post_handler(void)
{
	if(flash_area && flash_page_data_len)
	{
		taskENTER_CRITICAL();
	    FMC_WriteStream(FLCTL, flash_area->fa_off + flash_offset, flash_page, flash_page_data_len);
		taskEXIT_CRITICAL();

		//SYS_PRINT("[W] write flash data end - addr:0x%08X, offset:0x%04X len = %d\n", flash_area->fa_off , flash_offset, flash_page_data_len);
	}

	flash_page_data_len = 0;
	flash_area = NULL;

	flash_offset = 0;
	memset(flash_page, 0xFF, 256);
}
#endif

void mcu_reboot(void)
{
	SYS_PRINT("[W] Reset MCU\r\n");

    /* reset chip */
	CLK_ResetChip();
}

int flash_area_open(uint8_t id, const struct flash_area **area)
{
    if (id == FLASH_AREA_IMAGE_0){
       *area = &g_code_area;
        return 0;
    }
    else if(id == FLASH_AREA_IMAGE_1){
       *area = &g_swap_area;
        return 0;
    }
    return -1;
}

int flash_area_id_from_image_slot(int slot)
{
    SYS_ASSERT(slot == FLASH_AREA_IMAGE_0 || slot == FLASH_AREA_IMAGE_1);
    return slot;
}

void flash_area_close(const struct flash_area *fa)
{
    /* nothing to do for now */
}

static inline bool is_in_flash_area_bounds(const struct flash_area *fa, uint32_t off, uint32_t len)
{
	return ((off + len) <= fa->fa_size);
}

int flash_area_read(const struct flash_area *fa, uint32_t off, void *dst, uint32_t len)
{
    int ret;

    if (!is_in_flash_area_bounds(fa, off, len)) {
        ret = -1;
        return ret;
    }

    taskENTER_CRITICAL();
    ret = FMC_ReadStream(FLCTL, fa->fa_off + off, CMD_DREAD, dst, len);
    taskEXIT_CRITICAL();
    return ret;
}

int flash_area_write(const struct flash_area *fa, uint32_t off, const void *src, uint32_t len)
{
    int ret;

    if (!is_in_flash_area_bounds(fa, off, len)) {
        ret = -1;
        return ret;
    }

    //SYS_PRINT("[E] flash_area_write - addr:0x%08X, offset:0x%04X, len = %d\n", fa->fa_off, off, len);

#if FLASH_TIMING_OPTIMIZE_EN
    uint8_t *p = (uint8_t *)src;

	if(flash_area == NULL){
		flash_offset = off;
		flash_area   = (struct flash_area *)fa;

		//SYS_PRINT("[E] write flash data - addr:0x%08X, offset:0x%04X len = %d\n", fa->fa_off , flash_offset, len);
	}

	#define min(x, y)   (x) <(y) ? (x) : (y);

	while(len > 0)
	{
		uint32_t fragLen = min(len, 256 - flash_page_data_len);

		memcpy(flash_page + flash_page_data_len, p, fragLen);
		flash_page_data_len += fragLen;

		if(flash_page_data_len == 256)
		{
			taskENTER_CRITICAL();
			ret = FMC_WriteStream(FLCTL, fa->fa_off + flash_offset, flash_page, 256);
			taskEXIT_CRITICAL();

			//SYS_PRINT("[W] write flash data - addr:0x%08X, offset:0x%04X len = %d\n", fa->fa_off , flash_offset, 256);

			flash_offset += 256;
			flash_page_data_len = 0;
		}

		p += fragLen;
		if(len > fragLen){
			len -= fragLen;
		}else{
			len = 0;
		}
	}

	return 0;
#else
    taskENTER_CRITICAL();
    ret = FMC_WriteStream(FLCTL, fa->fa_off + off, (void *)src, len);
    taskEXIT_CRITICAL();
	return ret;
#endif
}

int flash_area_erase(const struct flash_area *fa, uint32_t off, uint32_t len)
{
    int ret;

    if (!is_in_flash_area_bounds(fa, off, len)) {
        ret = -1;
        return ret;
    }

	//SYS_PRINT("[W] Erase Flash - start addr: 0x%08X, len: 0x%04x\r\n", fa->fa_off + off, len);

    taskENTER_CRITICAL();
    ret = FMC_EraseCodeArea(FLCTL, fa->fa_off + off, len);
    taskEXIT_CRITICAL();

    return ret;
}

int flash_area_arase_image_swap(void)
{
	const struct flash_area *fa = NULL;

	flash_area_open(flash_area_id_from_image_slot(1), &fa) ;

	flash_area_erase(fa, 0, fa->fa_size);

	return 0;
}

uint8_t flash_area_erased_val(const struct flash_area *fa)
{
    return 0xFF;
}

int flash_area_is_empty(const struct flash_area *fa, bool *empty)
{
    uint32_t t1 = 0;
    uint32_t t2 = 0;
    uint32_t t3 = 0;

    flash_area_read(fa, 0, &t1, 4);
    flash_area_read(fa, fa->fa_size / 2, &t2, 4);
    flash_area_read(fa, fa->fa_size - 4, &t3, 4);

    if(t1 != 0xFFFFFFFF || t2 != 0xFFFFFFFF || t3 != 0xFFFFFFFF){
        *empty = false;
    }
    else{
        *empty = true;
    }

    return 0;
}

uint16_t flash_area_align(const struct flash_area *fa)
{
    return 1;
}
