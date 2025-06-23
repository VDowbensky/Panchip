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

#include "kv_store_api.h"

static uint8_t* kv_result_str(uint32_t result)
{
    uint8_t* str;

    switch (result) {
    case 0:
        str = (uint8_t*)"KVSTORE_OK";
        break;
    case MTB_KVSTORE_BAD_PARAM_ERROR:
        str = (uint8_t*)"KVSTORE_BAD_PARAM_ERROR";
        break;
    case MTB_KVSTORE_ALIGNMENT_ERROR:
        str = (uint8_t*)"KVSTORE_ALIGNMENT_ERROR";
        break;
    case MTB_KVSTORE_MEM_ALLOC_ERROR:
        str = (uint8_t*)"KVSTORE_MEM_ALLOC_ERROR";
        break;
    case MTB_KVSTORE_INVALID_DATA_ERROR:
        str = (uint8_t*)"KVSTORE_INVALID_DATA_ERROR";
        break;
    case MTB_KVSTORE_ERASED_DATA_ERROR:
        str = (uint8_t*)"KVSTORE_ERASED_DATA_ERROR";
        break;
    case MTB_KVSTORE_ITEM_NOT_FOUND_ERROR:
        str = (uint8_t*)"KVSTORE_ITEM_NOT_FOUND_ERROR";
        break;
    case MTB_KVSTORE_STORAGE_FULL_ERROR:
        str = (uint8_t*)"KVSTORE_STORAGE_FULL_ERROR";
        break;
    case MTB_KVSTORE_BUFFER_TOO_SMALL:
        str = (uint8_t*)"KVSTORE_BUFFER_TOO_SMALL";
        break;
    default:
        str = (uint8_t*)"Unrecognized KVStore Error";
        break;
    }

    return str;
}

static void flash_kvstore_test(void)
{
    cy_rslt_t r;
    uint8_t kv_write_data[4];
    uint8_t kv_read_data[4];
    uint32_t kv_read_data_len;

    kv_write_data[0] = 0x11;
    kv_write_data[1] = 0x22;
    kv_write_data[2] = 0x33;
    kv_write_data[3] = 0x44;
    kv_read_data_len = 4;

    APP_LOG_INFO("Try to write kv data, key=\"test_data\", len=%d\n", 4);
    for (int i = 0; i < 4; i++) {
        APP_LOG_INFO("kv_write_data[%d]: 0x%x\n", i, kv_write_data[i]);
    }

    // Write the whole 4-bytes data to the key "test_data"
    r = app_kv_write("test_data", (const uint8_t*)kv_write_data, 4);
    APP_LOG_INFO("KV write done, return code: 0x%x (%s)\n\n", r, kv_result_str(r));

    APP_LOG_INFO("Try to read kv data, key:\"test_data\", len:%d\n", kv_read_data_len);
    r = app_kv_read("test_data", (uint8_t*)kv_read_data, &kv_read_data_len);
    APP_LOG_INFO("KV read done, return code: 0x%x (%s)\n", r, kv_result_str(r));

    APP_LOG_INFO("Actual read length = %d\n", kv_read_data_len);
    for (int i = 0; i < kv_read_data_len; i++) {
        APP_LOG_INFO("kv_read_data[%d]: 0x%x\n", i, kv_read_data[i]);
    }
    APP_LOG("\n");

    kv_write_data[0] = 0x55;
    kv_write_data[1] = 0x66;
    kv_write_data[2] = 0x77;
    kv_read_data_len = 10;  // The actual data length is 3

    APP_LOG_INFO("Try to write kv data again, key:\"test_data\", len:%d\n", 3);
    for (int i = 0; i < 3; i++) {
        APP_LOG_INFO("kv_write_data[%d]: 0x%x\n", i, kv_write_data[i]);
    }

    // Write only the first 3-bytes data to the key "test_data" to replace the old 4-bytes data
    r = app_kv_write("test_data", (const uint8_t*)kv_write_data, 3);
    APP_LOG_INFO("KV write done, return code: 0x%x (%s)\n\n", r, kv_result_str(r));

    APP_LOG_INFO("Try to read kv data again, key:\"test_data\", len:%d\n", kv_read_data_len);
    r = app_kv_read("test_data", (uint8_t*)kv_read_data, &kv_read_data_len);
    APP_LOG_INFO("KV read done, return code: 0x%x (%s)\n", r, kv_result_str(r));

    APP_LOG_INFO("Actual read length = %d\n", kv_read_data_len);
    for (int i = 0; i < kv_read_data_len; i++) {
        APP_LOG_INFO("kv_read_data[%d]: 0x%x\n", i, kv_read_data[i]);
    }
    APP_LOG("\n");

    // Check if the key "test_data" exists
    APP_LOG_INFO("Try to check if the key \"test_data\" exists..\n");
    r = app_kv_key_exists("test_data");
    APP_LOG_INFO("KV key check done, return code: 0x%x (%s)\n\n", r, kv_result_str(r));

    // Try to delete the key "test_data"
    APP_LOG_INFO("Try to delte the key \"test_data\"..\n");
    r = app_kv_delete("test_data");
    APP_LOG_INFO("KV key delete done, return code: 0x%x (%s)\n\n", r, kv_result_str(r));

    // Check again if the key "test_data" exists
    APP_LOG_INFO("Try to check again if the key \"test_data\" exists..\n");
    r = app_kv_key_exists("test_data");
    APP_LOG_INFO("KV key check done, return code: 0x%x (%s)\n\n", r, kv_result_str(r));
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init Flash KVStore module */
    kv_init();

    /* Run Flash KV-Store test code */
    flash_kvstore_test();
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
