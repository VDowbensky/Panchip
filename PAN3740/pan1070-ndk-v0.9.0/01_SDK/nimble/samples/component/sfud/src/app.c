/**
 *******************************************************************************
 * @file     app.c
 * @create   2024-12-10
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "soc_api.h"

#include "app.h"
#include "app_log.h"

#include "sfud.h"

#define SFUD_TEST_FLASH_ADDRESS        0x0
#define SFUD_TEST_BUFFER_SIZE          0x400
static uint8_t sfud_test_buf[SFUD_TEST_BUFFER_SIZE];

static void sfud_test(uint32_t addr, size_t size, uint8_t *data)
{
    sfud_err result = SFUD_SUCCESS;
    const sfud_flash *flash = sfud_get_device(0);
    size_t i;
    /* prepare write data */
    for (i = 0; i < size; i++) {
        data[i] = i;
    }
    /* erase test */
    result = sfud_erase(flash, addr, size);
    if (result == SFUD_SUCCESS) {
        APP_LOG_INFO("Erase the %s flash data finish. Start from 0x%08X, size is %d.\n", flash->name, addr, size);
    } else {
        APP_LOG_ERR("Erase the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* write test */
    result = sfud_write(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        APP_LOG_INFO("Write the %s flash data finish. Start from 0x%08X, size is %d.\n", flash->name, addr, size);
    } else {
        APP_LOG_ERR("Write the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* read test */
    result = sfud_read(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        APP_LOG_INFO("Read the %s flash data success. Start from 0x%08X, size is %d. The data is:\n", flash->name, addr, size);
        APP_LOG("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
        for (i = 0; i < size; i++) {
            if (i % 16 == 0) {
                APP_LOG("[%08X] ", addr + i);
            }
            APP_LOG("%02X ", data[i]);
            if (((i + 1) % 16 == 0) || i == size - 1) {
                APP_LOG("\n");
            }
        }
        APP_LOG("\n");
    } else {
        APP_LOG_ERR("Read the %s flash data failed.\n", flash->name);
    }
    /* data check */
    for (i = 0; i < size; i++) {
        if (data[i] != i % 256) {
            APP_LOG_ERR("Read and check write data has an error. Write the %s flash data failed.\n", flash->name);
            break;
        }
    }
    if (i == size) {
        APP_LOG_INFO("The %s flash test is success.\n", flash->name);
    }
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init SFUD (Serial Flash Universal Driver) */
    if (sfud_init() != SFUD_SUCCESS) {
        APP_LOG_ERR("SFUD init fail!\n");
        return;
    }

    /* Run SFUD test code */
    sfud_test(SFUD_TEST_FLASH_ADDRESS, sizeof(sfud_test_buf), sfud_test_buf);
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    // Do nothing here.
    // We just omit the main task loop and let main() function return
    // back to the main_task() in os_setup.c and destroy itself then.
}
