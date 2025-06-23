/**
 *******************************************************************************
 * @file     pan_flash.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

#ifndef PAN_FLASH_H
#define PAN_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <Panseries.h>

#define FLASH_TIMING_OPTIMIZE_EN  1

/**@brief Flash Area Id */
#define FLASH_AREA_IMAGE_0  0
#define FLASH_AREA_IMAGE_1  1


struct flash_area {
    /** ID number */
    uint8_t     fa_id;
    /** Provided for compatibility with MCUboot */
    uint8_t     fa_device_id;
    uint16_t    pad16;
    /** Start offset from the beginning of the flash device */
    uint32_t    fa_off;
    /** Total size */
    uint32_t    fa_size; 
   
    /* Name of the flash device */
    const char *fa_dev_name;
};

void mcu_reboot(void);

int flash_area_open(uint8_t id, const struct flash_area **area);

int flash_area_id_from_image_slot(int slot);

int flash_area_read(const struct flash_area *fa, uint32_t off, void *dst, uint32_t len);

int flash_area_write(const struct flash_area *fa, uint32_t off, const void *src, uint32_t len);

int flash_area_erase(const struct flash_area *fa, uint32_t off, uint32_t len);
int flash_area_arase_image_swap(void);

void flash_area_close(const struct flash_area *fa);

uint8_t flash_area_erased_val(const struct flash_area *fa);

int flash_area_is_empty(const struct flash_area *fa, bool *empty);

uint16_t flash_area_align(const struct flash_area *fa);

int flash_area_id_from_image_slot(int slot);


#if FLASH_TIMING_OPTIMIZE_EN
void img_mgmt_impl_reset_flash_param(void);
void img_mgmt_impl_post_handler(void);
#endif

#endif
