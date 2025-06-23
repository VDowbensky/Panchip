/*
 * Copyright (c) 2020-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef FLASH_MANAGER__H_
#define FLASH_MANAGER__H_

#include "PanSeries.h"

#if CONFIG_APP_USE_IMAGE_HEADER
#include "img_hdr.h"
typedef struct img_hdr image_header_t;
#define APP_IMG_HEADER_SIZE         sizeof(image_header_t)
#else
#define APP_IMG_HEADER_SIZE         0
#endif

#define FLASH_AREA_BACK_UP_START    CONFIG_FLASH_PARTITION_APP_BACKUP_ADDR
#define FLASH_AREA_IMAGE_START      CONFIG_FLASH_PARTITION_APP_ADDR
#define FLASH_IMAGE_MAX_SIZE        CONFIG_FLASH_PARTITION_APP_SIZE

void fm_status_refresh(void);
void fm_write_flash(unsigned int addr, unsigned char *buf, unsigned int len);
int fm_read_flash(unsigned int addr, unsigned char *buf, unsigned int len);

bool fm_image_magic_check(uint32_t im_addr);
bool fm_image_completed_check(uint32_t im_addr);
int fm_image_make_invalid(uint32_t im_addr);
void fm_image_move(uint32_t src_addr, uint32_t dst_addr);

#endif // FLASH_MANAGER__H_
