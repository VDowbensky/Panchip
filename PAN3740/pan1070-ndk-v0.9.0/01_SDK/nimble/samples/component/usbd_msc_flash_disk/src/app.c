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
#include "ff.h"

FATFS fs;                       /* FatFs File System Object */
FIL fil;                        /* File Object */
FRESULT res;                    /* File Operation Result */
UINT fnum;                      /* Number of bytes successfully read/write */
BYTE read_buffer[128] = {0};
BYTE text_buffer[] = "Hello Panchip World!\r\n";

static void fatfs_flash_disk_init(void)
{
    // Mount the file system and check if it exists
    APP_LOG_INFO("Try to mount file system..\n");
    res = f_mount(&fs, "flash:/", 1);  // Mount to logical drive "flash:/"
    if (res == FR_NO_FILESYSTEM) {
        APP_LOG_WRN("File system does not exist, formatting..\n");
        // Format the disk
        static const MKFS_PARM opt = {FM_FAT, 0, 0, 0, 0};
        res = f_mkfs("flash:/", &opt, NULL, FF_MAX_SS);
        if (res != FR_OK) {
            APP_LOG_ERR("File system formatting failed! Error code: %d\n", res);
            return;
        }
        APP_LOG_INFO("File system formatting completed, remounting file system..\n");
        res = f_mount(&fs, "flash:/", 1);  // Remount
    }
    if (res != FR_OK) {
        APP_LOG_ERR("Failed to mount file system! Error code: %d\n", res);
        return;
    }

    // Open and check if file exists, create and write data to it if not
    APP_LOG_INFO("Try to open file..\n");
    res = f_open(&fil, "flash:/file.txt", FA_READ);
    if (res == FR_NO_FILE) {
        APP_LOG_WRN("File does not exist, try to create one..\n");
        res = f_open(&fil, "flash:/file.txt", FA_WRITE | FA_CREATE_ALWAYS);
        if (res != FR_OK) {
            APP_LOG_ERR("Failed to create file! Error code: %d\n", res);
            f_unmount("flash:/");
            return;
        }
        APP_LOG_INFO("Try to write data to the created file..\n");
        res = f_write(&fil, text_buffer, strlen((const char *)text_buffer), &fnum);
        if (res != FR_OK || fnum != strlen((const char *)text_buffer)) {
            APP_LOG_ERR("Failed to write to file! Error code: %d\n", res);
            f_close(&fil);
            f_unmount("flash:/");
            return;
        }
        APP_LOG_INFO("Close the opened file..\n");
        res = f_close(&fil); // This moves data from ram buffer to the physical storage
        if (res != FR_OK) {
            APP_LOG_ERR("Failed to close file! Error code: %d\n", res);
            f_unmount("flash:/");
            return;
        }
        APP_LOG_INFO("Try to open the created file again..\n");
        res = f_open(&fil, "flash:/file.txt", FA_READ);
        if (res != FR_OK) {
            APP_LOG_ERR("Failed to open file! Error code: %d\n", res);
            f_unmount("flash:/");
            return;
        }
    }
    if (res != FR_OK) {
        APP_LOG_ERR("Failed to open file! Error code: %d\n", res);
        f_unmount("flash:/");
        return;
    }

    APP_LOG_INFO("Try to read data from the opened file..\n");
    res = f_read(&fil, read_buffer, sizeof(read_buffer) - 1, &fnum); // Reserve one byte for string terminator
    if (res != FR_OK) {
        APP_LOG_ERR("Failed to read file! Error code: %d\n", res);
        f_close(&fil);
        f_unmount("flash:/");
        return;
    }
    read_buffer[fnum] = '\0';  // Add string terminator

    APP_LOG_INFO("File reading successfully, content: %s\n", read_buffer);

    APP_LOG_INFO("Close the opened file..\n");
    res = f_close(&fil);
    if (res != FR_OK) {
        APP_LOG_ERR("Failed to close file! Error code: %d\n", res);
        f_unmount("flash:/");
        return;
    }

    // Unmount the file system after use
    APP_LOG_INFO("Try to unmount file system..\n");
    res = f_unmount("flash:/");
    if (res != FR_OK) {
        APP_LOG_ERR("Failed to unmount file system! Error code: %d\n", res);
        return;
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

    /* Init FatFs */
    fatfs_flash_disk_init();

    /* Init USBD MSC (Mass Storage Class) */
    usbd_msc_init();
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
