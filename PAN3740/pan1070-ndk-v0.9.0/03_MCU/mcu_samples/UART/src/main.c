/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Date: 19/09/25 13:47 $
 * @brief    UART Sample Code. Use UART with/without FIFO, polling and interrupt
 *           mode, auto flow control, communicate with DMA, 9-bit mode, etc.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"


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
    //MCU
   CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M);
	 CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

    //APB Enable
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_All,ENABLE);

    CLK_APB1PeriphClockCmd(CLK_APB1Periph_All,ENABLE);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_All,ENABLE);
}

void UART_PrintSampleInfo()
{
    SYS_TEST("\n");
    SYS_TEST("+--------------------------------------------------------------------------------------+ \n");
    SYS_TEST("|                      PAN107X UART Sample Code.                                       | \n");
    SYS_TEST("+--------------------------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to start specific testcase:                                             | \n");
    SYS_TEST("|                                                                                      | \n");
    SYS_TEST("|    Input '0'    Testcase 0: Register Default Value Check.                            | \n");
    SYS_TEST("|    Input '1'    Testcase 1: Data Length Select and Stop Bit Test.                    | \n");
    SYS_TEST("|    Input '2'    Testcase 2: Parity Check Test.                                       | \n");
    SYS_TEST("|    Input '3'    Testcase 3: Baudrate Test.                                           | \n");
    SYS_TEST("|    Input '4'    Testcase 4: FIFO and Threshold Test.                                 | \n");
    SYS_TEST("|    Input '5'    Testcase 5: FIFO and Line Status Interrupt Test.                     | \n");
    SYS_TEST("|    Input '6'    Testcase 6: Auto Flow Control Test.                                  | \n");
    SYS_TEST("|    Input '7'    Testcase 7: Transfer With DMA Test.                                  | \n");
    SYS_TEST("|    Input '8'    Testcase 8: 9-bit Mode Test.                                         | \n");
    SYS_TEST("+--------------------------------------------------------------------------------------+ \n");
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

    UART_TestModuleInit();
    UART_PrintSampleInfo();
    UART_ResetRxFifo(UART0);
    c = getchar();
    while( c )
    {
        SYS_TEST("\n");
        switch(c)
        {
            /* 0. Register Default Value Check */
            case '0':
                UART_TestFunctionEnter(0);
                break;
            /* 1. Data Length Select and Stop Bit Test */
            case '1':
                UART_TestFunctionEnter(1);
                break;
            /* 2. Parity Check Test */
            case '2':
                UART_TestFunctionEnter(2);
                break;
            /* 3. Baudrate Test */
            case '3':
                UART_TestFunctionEnter(3);
                break;
            /* 4. FIFO and Threshold Test */
            case '4':
                UART_TestFunctionEnter(4);
                break;
            /* 5. FIFO and Line Status Interrupt Test */
            case '5':
                UART_TestFunctionEnter(5);
                break;
            /* 6. Auto Flow Control Test */
            case '6':
                UART_TestFunctionEnter(6);
                break;
            /* 7. Transfer With DMA Test */
            case '7':
                UART_TestFunctionEnter(7);
                break;
            /* 8. 9-bit Mode Test */
            case '8':
                UART_TestFunctionEnter(8);
                break;
            default:
                SYS_WRN("Cannot find testcase %c!\n", c);
                break;
        }
        UART_PrintSampleInfo();
        c = getchar();
    }
}
