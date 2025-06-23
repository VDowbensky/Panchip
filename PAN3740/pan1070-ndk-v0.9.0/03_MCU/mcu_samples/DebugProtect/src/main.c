/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date:    19/11/12 16:42 $
 * @brief
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"

uint32_t verify_debug_key1, verify_debug_key2, verify_debug_key3, verify_debug_key4;
uint8_t dbg_key_in_efuse[8];

void Sys_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

#ifdef IP_101x
    /* Init I/O Multi-function  */
    SYS_SET_MFP(P1, 1, UART0_TX);
    SYS_SET_MFP(P1, 2, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT2);
#else
    /* Init I/O Multi-function  */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
#endif

    /* Relock protected registers */
    SYS_LockReg();
}

void Peri_Init()
{
    UART_InitTypeDef Init_Struct;

    Init_Struct.UART_BaudRate = 921600;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;

    /* Init UART0 */
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
}

void Clock_Init(void)
{
    ANA->LP_FSYN_LDO |= 0X1;
    //MCU
    CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
    CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M);
    CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

    //AHB/APB Enable
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_All,ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_All,ENABLE);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_All,ENABLE);
}

void DbgProtPrintSampleInfo(void)
{
    SYS_TEST("\n");
    SYS_TEST("+--------------------------------------------------------------------+ \n");
    SYS_TEST("|               PAN107X Debug Protect Sample Code.                   | \n");
    SYS_TEST("+--------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to start specific testcase:                           | \n");
    SYS_TEST("|                                                                    | \n");
    SYS_TEST("|    Input '0'    Testcase 0: Generate Debug Key.                    | \n");
    SYS_TEST("|    Input '1'    Testcase 1: Write Generated Debug Key to eFuse.    | \n");
    SYS_TEST("|    Input '2'    Testcase 2: Apply debug key to enable SWD Debug.   | \n");
    SYS_TEST("|    Input '3'    Testcase 3: Clear debug key to disable SWD Debug.  | \n");
    SYS_TEST("+--------------------------------------------------------------------+ \n");
}

void GenerateDebugKey(void)
{
#if 0
    srand(300);
    verify_debug_key1 = (uint32_t)rand();
    srand(21);
    verify_debug_key2 = (uint32_t)rand();
    srand(4952);
    verify_debug_key3 = (uint32_t)rand();
    srand(569);
    verify_debug_key4 = (uint32_t)rand();
#else
    verify_debug_key1 = 0x0a320bfa;
    verify_debug_key2 = 0x32a24f95;
    verify_debug_key3 = 0x29fe9e58;
    verify_debug_key4 = 0x18e2403f;
#endif
    dbg_key_in_efuse[0] = (uint8_t)(verify_debug_key1 & 0x000000FFu);
    dbg_key_in_efuse[1] = (uint8_t)((verify_debug_key1 & 0xFF000000u) >> 24);
    dbg_key_in_efuse[2] = (uint8_t)(~((verify_debug_key2 & 0x0000FF00u) >> 8));
    dbg_key_in_efuse[3] = (uint8_t)((verify_debug_key1 & 0xFF000000u) >> 24);
    dbg_key_in_efuse[4] = (uint8_t)(verify_debug_key3 & 0x000000FFu);
    dbg_key_in_efuse[5] = (uint8_t)((verify_debug_key3 & 0x0000FF00u) >> 8);
    dbg_key_in_efuse[6] = (uint8_t)(~((verify_debug_key3 & 0x00FF0000u) >> 16));
    dbg_key_in_efuse[7] = (uint8_t)((verify_debug_key4 & 0x00FF0000u) >> 16);

    SYS_TEST("Debug Key in Registers: 0x%08x 0x%08x 0x%08x 0x%08x\n", verify_debug_key1, verify_debug_key2, verify_debug_key3, verify_debug_key4);
    SYS_TEST("Debug Key in eFuse: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
        dbg_key_in_efuse[0], dbg_key_in_efuse[1], dbg_key_in_efuse[2], dbg_key_in_efuse[3], dbg_key_in_efuse[4], dbg_key_in_efuse[5], dbg_key_in_efuse[6], dbg_key_in_efuse[7]);
}

static void DumpEfuseKeysAndSecureCtrlFlag(void)
{
    // Unlock Protected Registers
    SYS_UnlockReg();
    // Init eFuse module
    EFUSE_Init(EFUSE);
    // Elevate privilege
    EFUSE->EF_FLASH_PERMISSION |= EFUSE_FLASH_PERMISSION_CTRL_Msk;

    SYS_TEST("Debug Key in eFuse:\n");
    SYS_TEST("\tAddr:Data\tAddr:Data\tAddr:Data\tAddr:Data\n\t");
    for (size_t address = 0x10; address < 0x18; address++)
    {
        // Read eFuse
        uint8_t data = EFUSE_ReadByte(EFUSE, address);

        SYS_TEST("0x%02x:0x%02x", address, data);

        // Check if previous Read Operation succeeded or not
        if (EFUSE_GetErrorStatus(EFUSE) == EFUSE_STATUS_FAIL)
        {
            EFUSE_ClrErrorStatus(EFUSE);
            SYS_TEST("(N/A)");
        }
        else
        {
            SYS_TEST("(OK)");
        }
        if ((address + 1) % 4 == 0)
            SYS_TEST("\n\t");
        else
            SYS_TEST("\t");
    }
    SYS_TEST("\nSecure Enable Control Flag in eFuse:\n\t0x1B:0x%02x\n", EFUSE_ReadByte(EFUSE, 0x1B));

    // Reduce privilege
    EFUSE->EF_FLASH_PERMISSION &= ~EFUSE_FLASH_PERMISSION_CTRL_Msk;
    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);
    // Relock Protected Registers
    SYS_UnlockReg();
}

void WriteDebugKeyToEfuse(void)
{
    uint32_t secure_enable_addr = 0x1B;
    uint8_t secure_enable_data = 0x04;  // Set BIT2

    // Unlock Protected Registers
    SYS_UnlockReg();
    // Init eFuse module
    EFUSE_Init(EFUSE);
    // Elevate privilege
    EFUSE->EF_FLASH_PERMISSION |= EFUSE_FLASH_PERMISSION_CTRL_Msk;

    // Write eFuse Debug Key
    if (!EFUSE_Write(EFUSE, 0x10, dbg_key_in_efuse, 8))
    {
        SYS_TEST("eFuse Write Debug Key Error!\n");
    }

    // Write eFuse Debug Protect Enable Bit
    if (!EFUSE_Write(EFUSE, secure_enable_addr, &secure_enable_data, 1))
    {
        SYS_TEST("eFuse Write Debug Protect Enable Bit Error!\n");
    }

    // Reduce privilege
    EFUSE->EF_FLASH_PERMISSION &= ~EFUSE_FLASH_PERMISSION_CTRL_Msk;
    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);
    // Relock Protected Registers
    SYS_UnlockReg();

    // Dump eFuse Debug Key and Secure Enable Control Flags
    SYS_TEST("eFuse content after write before reset:\n");
    DumpEfuseKeysAndSecureCtrlFlag();

    // Do chip reset to apply the new configuration in eFuse
    SYS_TEST("\nTrigger reset to apply the new configuration in eFuse..\n");
    SYS_delay_10nop(0x1000);    // Delay to wait uart print done.
    CLK_ResetChip();
    while(1);
}

void ApplyDebugKeyToReg(void)
{
    // Write Debug Key to Register
    EFUSE->EF_VERIFY_DEBUG1 = verify_debug_key1;
    EFUSE->EF_VERIFY_DEBUG2 = verify_debug_key2;
    EFUSE->EF_VERIFY_DEBUG3 = verify_debug_key3;
    EFUSE->EF_VERIFY_DEBUG4 = verify_debug_key4;

    SYS_TEST("Debug Key Applied to Enable SWD Debug: 0x%08x 0x%08x 0x%08x 0x%08x\n",
        EFUSE->EF_VERIFY_DEBUG1, EFUSE->EF_VERIFY_DEBUG2, EFUSE->EF_VERIFY_DEBUG3, EFUSE->EF_VERIFY_DEBUG4);
}

void ClearDebugKeyInReg(void)
{
    // Clear Debug Key in Register
    EFUSE->EF_VERIFY_DEBUG1 = 0;
    EFUSE->EF_VERIFY_DEBUG2 = 0;
    EFUSE->EF_VERIFY_DEBUG3 = 0;
    EFUSE->EF_VERIFY_DEBUG4 = 0;

    SYS_TEST("Debug Key Cleared to 0 to Disable SWD Debug\n");
}

static int pan10xx_hw_calib_init(void)
{
	OTP_STRUCT_T otp;

    SYS_TEST("Try to load HW calibration data..");
    if (!SystemHwParamLoader(&otp)) {
        SYS_TEST("\nWARNING: Cannot find valid calib data in current chip!\n");
    } else {
        SYS_TEST(" DONE.\n");
        SYS_TEST("- Chip Info         : 0x%x\n", otp.m.chip_info);
        SYS_TEST("- Chip CP Version   : %d\n", otp.m.cp_version);
        SYS_TEST("- Chip FT Version   : %d\n", otp.m.ft_version);
        if (otp.m.ft_version >= 2) {
			SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m_v2.mac_addr[0], otp.m_v2.mac_addr[1],
				otp.m_v2.mac_addr[2], otp.m_v2.mac_addr[3], otp.m_v2.mac_addr[4], otp.m_v2.mac_addr[5]);
		} else {
			SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m.mac_addr[0], otp.m.mac_addr[1],
				otp.m.mac_addr[2], otp.m.mac_addr[3], otp.m.mac_addr[4], otp.m.mac_addr[5]);
		}
        SYS_TEST("- Chip UID          : %02X%02X%02X%02X%02X%02X%02X%02X%02X\n", otp.m.uid[0], otp.m.uid[1],
            otp.m.uid[2], otp.m.uid[3], otp.m.uid[4], otp.m.uid[5], otp.m.uid[6], otp.m.uid[7], otp.m.uid[8]);

#if INIT_OPTIMIZE_ACTIVE_POWER
        uint32_t tmp;
        int32_t val;

        //buck out(DCDC): ft - 2
        val = (otp.m.buck_out_trim >> 1) - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_BUCK_3V;
        tmp &= ~(0xFu << 2);
        tmp |= (val << 2);
        ANA->LP_BUCK_3V = tmp;

        //HPLDO(DVDD): ft - 2
        val = otp.m.hp_ldo_trim - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFu << 3);
        tmp |= (val << 3);
        ANA->LP_HP_LDO = tmp;
#endif
    }
    SYS_TEST("- Chip Flash UID    : ");
    for (uint32_t i = 0; i < 16; i++) {
        SYS_TEST("%02X", flash_ids.uid[i]);
    }
    SYS_TEST("\n- Chip Flash Size   : %ld KB\n", BIT(flash_ids.memory_density_id) >> 10);

	return 0;
}

/*------------------
    Main function
 ------------------*/
int main(void)
{
    char c;

    Clock_Init();
    Sys_Init();
    Peri_Init();

    pan10xx_hw_calib_init();

    SYS_TEST("\nCPU @ %dHz\n", SystemCoreClock);

    if (CLK->RSTSTS & CLK_RSTSTS_CHIPRF_Msk)
    {
        CLK->RSTSTS = CLK->RSTSTS;  // Clear reset flags
    }

    SYS_TEST("\neFuse content after reset:\n");
    DumpEfuseKeysAndSecureCtrlFlag();

    DbgProtPrintSampleInfo();
    c = getchar();
    while( c )
    {
        SYS_TEST("\n");
        switch(c)
        {
            case '0':
                GenerateDebugKey();
                break;
            case '1':
                WriteDebugKeyToEfuse();
                break;
            case '2':
                ApplyDebugKeyToReg();
                break;
            case '3':
                ClearDebugKeyInReg();
                break;
            default:
                SYS_WRN("Cannot find testcase %c!\n", c);
                break;
        }
        DbgProtPrintSampleInfo();
        c = getchar();
    }
}
