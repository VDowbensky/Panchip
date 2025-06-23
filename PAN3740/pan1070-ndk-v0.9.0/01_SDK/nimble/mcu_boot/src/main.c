/*
 * Copyright (c) 2020-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "soc_api.h"
#include "app_log.h"
#include "signal_slot_manager.h"
#include "uart_dfu.h"
#include "usb_dfu.h"
#include "flash_manager.h"
#include "prf_ota.h"

static void on_image_load_enter(void)
{
    APP_LOG_INFO("Try to move image from App Backup Partition to App Partition..\n");

    fm_image_move(FLASH_AREA_BACK_UP_START, FLASH_AREA_IMAGE_START);
}

static void boot_cleanup(void)
{
    /* Mask all IRQs */
    __disable_irq();

    /* Reset specific hardware modules used in signal.c */
    sig_hardware_recovery();

    /* Reset all hw peripheral modules except eFuse and GPIO */
    CLK->IPRST0 = 0x1CC;
    CLK->IPRST0 = 0x0;
    CLK->IPRST1 = 0x17FFF;
    CLK->IPRST1 = 0x0;

    /* Disable all IRQs and clear all pending IRQs on NVIC before running into app */
    NVIC->ICER[0U] = 0xFFFFFFFF;
    NVIC->ICPR[0U] = 0xFFFFFFFF;

    // Do a short flash read operation here to avoid risk of APP flash x4 mode
    // switch fail in specific scenario (Workaround #5663)
    FMC_ReadByte(FLCTL, 0x0, CMD_DREAD);

    // Disable I-cache here, and would enable again in APP SystemInit flow
    CR->X_CACHE_EN = 0x00;
}

static void jump_to_app(void)
{
    void (*app_init)();

    uint32_t msp = *(volatile uint32_t*)(FLASH_AREA_IMAGE_START + APP_IMG_HEADER_SIZE);
    uint32_t addr = *(uint32_t*)(FLASH_AREA_IMAGE_START + APP_IMG_HEADER_SIZE + 4);
    app_init = (void(*)())(uint32_t*)addr;

    FLCTL->X_FL_REMAP_ADDR = FLASH_AREA_IMAGE_START + APP_IMG_HEADER_SIZE; /* fmc remap */

    __set_MSP(msp);

    app_init();
}

int main(void)
{
    APP_LOG("Bootloader in..\n\n");

    /* If backup image is valid, the on_image_load_enter function will be handled */
    ss_connect(0, sig_back_up_is_completed_image, on_image_load_enter);

#if BOOT_ENABLE_UART_DFU
    /* when detecting key1 down, the on_uart_dfu_enter function will be handled */
    ss_connect(1, sig_key1_push_down, on_uart_dfu_enter);
#endif

#if BOOT_ENABLE_USB_DFU
#if BOOT_USB_DFU_MODE == 0x00 // Use a GPIO Key to trigger entering USB DFU Mode
    /* If key2 is pressed down, the on_usb_dfu_enter function will be handled */
    ss_connect(2, sig_key2_push_down, on_usb_dfu_enter);
#endif // BOOT_USB_DFU_MODE == 0x00
    /* If the DFU flag on flash is detected, the on_usb_dfu_enter function will be handled */
    ss_connect(3, sig_usb_dfu_enter_check, on_usb_dfu_enter);

#endif // BOOT_ENABLE_USB_DFU

#if BOOT_ENABLE_PRF_OTA
    /* If a 2.4G ota start packet is received, the on_prf_ota_enter function will be handled */
    ss_connect(4, sig_ota_start_received, on_prf_ota_enter);
#endif

    /* Start to handle all of events related signal functions */
    ss_events_handle();

    /* Check if there is a valid App Image */
    if (fm_image_magic_check(FLASH_AREA_IMAGE_START) != true) {
        APP_LOG("\nWARNING: Cannot find a valid APP image in App Flash Partition!\n");
        SYS_ABORT();
    }

#if APP_LOG_EN
    APP_LOG_INFO("Clean up and try to jump into app image..\n");
    APP_LOG_INFO("- app partition start addr: 0x%X\n", FLASH_AREA_IMAGE_START);
    APP_LOG_INFO("- app partition size      : %d KB\n", FLASH_IMAGE_MAX_SIZE >> 10);
    APP_LOG_INFO("- app image header size   : %d B\n\n", APP_IMG_HEADER_SIZE);
    soc_busy_wait(10000); // Wait for print done
#endif
    /*
     * Reset hardware modules possible used in bootloader, and clear other configurations
     * (such as IRQs) before running into app.
     */
    boot_cleanup();

    /* Now let's try to find app image and jump into the entry of it */
    jump_to_app();

    return 0;
}
