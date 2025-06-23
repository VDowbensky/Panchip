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

#include "app.h"
#include "app_log.h"

static void DumpEfuseKeysAndSecureCtrlFlag(void)
{
    // Unlock Protected Registers
    SYS_UnlockReg();
    // Init eFuse module
    EFUSE_Init(EFUSE);
    // Elevate privilege
    EFUSE->EF_FLASH_PERMISSION |= EFUSE_FLASH_PERMISSION_CTRL_Msk;

    APP_LOG("Debug Protect Key in eFuse:\n");
    APP_LOG("\tAddr:Data\tAddr:Data\tAddr:Data\tAddr:Data\n\t");
    for (size_t address = 0x10; address < 0x18; address++)
    {
        // Read eFuse
        uint8_t data = EFUSE_ReadByte(EFUSE, address);

        APP_LOG("0x%02x:0x%02x", address, data);

        // Check if previous Read Operation succeeded or not
        if (EFUSE_GetErrorStatus(EFUSE) == EFUSE_STATUS_FAIL)
        {
            EFUSE_ClrErrorStatus(EFUSE);
            APP_LOG("(N/A)");
        }
        else
        {
            APP_LOG("(OK)");
        }
        if ((address + 1) % 4 == 0)
            APP_LOG("\n\t");
        else
            APP_LOG("\t");
    }
    APP_LOG("\nSecure Enable Control Flag in eFuse (Address 0x1B):\n\t0x%02x\n", EFUSE_ReadByte(EFUSE, 0x1B));
    APP_LOG("Note:\n");
    APP_LOG("\t- BIT0: FW Encryption Enable Ctrl\n");
    APP_LOG("\t- BIT1: Anti-injection Enable Ctrl\n");
    APP_LOG("\t- BIT2: Debug Protection Enable Ctrl\n");

    // Reduce privilege
    EFUSE->EF_FLASH_PERMISSION &= ~EFUSE_FLASH_PERMISSION_CTRL_Msk;
    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);
    // Relock Protected Registers
    SYS_UnlockReg();
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    APP_LOG("\nDump eFuse content after reset:\n");
    DumpEfuseKeysAndSecureCtrlFlag();
    APP_LOG("Done.\n");
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
