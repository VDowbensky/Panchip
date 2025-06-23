/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
  * $Date: 16/02/ 10:37 $
 * @brief    Transmit and receive data from PC terminal through RS232 interface.
 *
 * @note
 * Copyright (C) 2016 Shanghai Panchip Microelectronics Co., Ltd.   All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "fmc_common.h"

extern const bool PanFlashEnhanceEnable;
extern const uint32_t PanFlashLineMode;

void Clock_Init(void)
{
    // Enable Fsyn LDO
    ANA->LP_FSYN_LDO |= 0X1;
    // Enable XTH Clock
    CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
    CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);
    // Enable DPLL 48MHz refer to XTH
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M);
    // Switch system clock to DPLL
    CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

    // Enable clock of hw modules
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_All,ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_All,ENABLE);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_All,ENABLE);
}

static void Sys_Init(void)
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

void platform_init()
{
    Clock_Init();
    Sys_Init();
    Peri_Init();

    pan10xx_hw_calib_init();
}

void FMC_PrintSampleInfo()
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
    SYS_TEST("|                       PAN10xx FMC Sample Code                       | \n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to start specific testcase:                            | \n");
    SYS_TEST("|                                                                     | \n");
    SYS_TEST("|    Input '0'    Testcase 0: Basic Read/Write/Erase Test.            | \n");
    SYS_TEST("|    Input '1'    Testcase 1: Small Data Read/Write Test.             | \n");
    SYS_TEST("|    Input '2'    Testcase 2: Hardware CRC Check Test.                | \n");
    SYS_TEST("|    Input '3'    Testcase 3: Flash Info Area Read Test.              | \n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
}

int main(void)
{
    char input;

    platform_init();

    SYS_TEST("\nCPU @ %dHz, Flash Code Area Size = %d KB\n", SystemCoreClock, FMC_GetFlashCodeAreaSize(FLCTL) / 1024);
    FMC_PrintSampleInfo();
    UART_ResetRxFifo(UART0);
    input = getchar();
    while (input) {
        SYS_TEST("\n");
        switch (input) {
            case '0':FMC_TestFunctionEnter(0);break;
            case '1':FMC_TestFunctionEnter(1);break;
            case '2':FMC_TestFunctionEnter(2);break;
            case '3':FMC_TestFunctionEnter(3);break;
            default:break;
        }
        FMC_PrintSampleInfo();
        input = getchar();
    }
}
