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

/*
 * !!ATTENTION!!
 *  Do not try to call an out-of-encrypt-section function but pass a Literal DATA which
 *  stores in the encrypted section if the Anti-injection Feature is enabled.
 *  For example, please don't try to print log in encrypted functions like this:
 *      printf("Enter Encrypt Function\n");
 *  Use the following form instead:
 *      static const char EncFuncEnterLog[] = "Enter Encrypt Function\n";
 *      printf(EncFuncEnterLog);
 */
ENCRYPT_SECTION void encrypted_test_function(uint32_t *calculation)
{
    /*
     * Do some secret operations/calculations here, and pass the result out.
     */
    *calculation = *(uint32_t *)((uint32_t)(&encrypted_test_function) & 0xFFFFFFFE);

    /* Define a static const char array (string), and linker would put it into the
       global RO-DATA area which resides out of the flash encrypt section, so that
       the out-of-encrypt-section function 'printf' could access this string. */
    static const char EncFuncHelloLog[] = "[ENC] Hello from %s!\n\n";
    printf(EncFuncHelloLog, __func__);
}

static void DumpEfuseKeysAndSecureCtrlFlag(void)
{
    uint32_t encrypt_offset_in_efuse;

    // Unlock Protected Registers
    SYS_UnlockReg();
    // Init eFuse module
    EFUSE_Init(EFUSE);
    // Elevate privilege
    EFUSE->EF_FLASH_PERMISSION |= EFUSE_FLASH_PERMISSION_CTRL_Msk;

    APP_LOG("FW Encryption AES Key in eFuse:\n");
    APP_LOG("\tAddr:Data\tAddr:Data\tAddr:Data\tAddr:Data\n\t");
    for (size_t address = 0x00; address < 0x10; address++)
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

    encrypt_offset_in_efuse = EFUSE_ReadByte(EFUSE, 0x18);
    encrypt_offset_in_efuse |= EFUSE_ReadByte(EFUSE, 0x19) << 8;
    encrypt_offset_in_efuse |= EFUSE_ReadByte(EFUSE, 0x1A) << 16;
    APP_LOG("\nFW Encryption Flash Offset in eFuse (Address 0x18 ~ 0x1A):\n\t0x%06x\n", encrypt_offset_in_efuse);

    APP_LOG("\nSecure Enable Control Flag in eFuse (Address 0x1B):\n\t0x%02x\n", EFUSE_ReadByte(EFUSE, 0x1B));
    APP_LOG("Note:\n");
    APP_LOG("\t- BIT0: FW Encryption Enable Ctrl\n");
    APP_LOG("\t- BIT1: Anti-injection Enable Ctrl\n");
    APP_LOG("\t- BIT2: Debug Protection Enable Ctrl\n");

    APP_LOG("\n");
    if (encrypt_offset_in_efuse != CONFIG_ENCRYPT_FLASH_OFFSET) {
        APP_LOG_WRN("The CONFIG_ENCRYPT_FLASH_OFFSET value of current program is not corresponding with FW Encryption Flash Offset in eFuse!\n");
        APP_LOG_WRN("-> CONFIG_ENCRYPT_FLASH_OFFSET = 0x%06x\n", CONFIG_ENCRYPT_FLASH_OFFSET);
        APP_LOG_WRN("-> FW Encryption Flash Offset in eFuse = 0x%06x\n", encrypt_offset_in_efuse);
    }
    APP_LOG("\n");

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

    APP_LOG("Dump eFuse content after reset:\n\n");
    DumpEfuseKeysAndSecureCtrlFlag();
    APP_LOG("Done.\n\n");

    uint32_t secret_calc;

    encrypted_test_function(&secret_calc);
    APP_LOG_INFO("Secret calcucation result: 0x%x\n", secret_calc);
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
