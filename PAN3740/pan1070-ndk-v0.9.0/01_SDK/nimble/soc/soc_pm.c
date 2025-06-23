/**
 *******************************************************************************
 * @file     soc_pm.c
 * @create   2024-12-11
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "soc_api.h"

#if CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
#include "FreeRTOS.h"
#include "task.h"
/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_INT_CTRL_REG                 ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVSET_BIT                ( 1UL << 28UL )
#define portNVIC_PEND_SYSTICK_SET_BIT         ( 1UL << 26UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT       ( 1UL << 25UL )
#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI                  ( portMIN_INTERRUPT_PRIORITY << 24UL )
#define configSYSTICK_CLOCK_HZ             ( configCPU_CLOCK_HZ )
#define portNVIC_SYSTICK_CLK_BIT_CONFIG    ( portNVIC_SYSTICK_CLK_BIT )

#define portSY_FULL_READ_WRITE    ( 15 )

extern uint32_t ulTimerCountsForOneTick;
extern void UpdateTickAndSch(void);
extern const uint32_t PanFlashLineMode;
extern const bool PanFlashEnhanceEnable;
#endif // CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET

#define FLASH_RDP_WT_CNT        0x400 // Flash RDP wait cycle count

#if CONFIG_PM

static uint32_t stbm1_gpio_int_flag = 0;

uint32_t soc_stbm1_gpio_wakeup_src_get(void)
{
    return stbm1_gpio_int_flag;
}

CONFIG_RAM_CODE void LP_IRQHandler()
{
    /*
	 * Clear DeepSleep int flag (write 1 to clear) in this register, but still
	 * retain all other ctrl/status flags.
	 */
	ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_DP_FLAG_Msk)
		& ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
		| ANAC_INT_STANDBY_M1_FLAG_Msk | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);

	/* Re-disable LP IRQ after use */
	NVIC_DisableIRQ(LP_IRQn);
}

void deepsleep_init(void)
{
    uint32_t RamPowerCtrl = 0x1F;   // Enable power of all sram in Deepsleep mode

    // Enable SLPTMR interrupt and wakeup
    ANA->LP_INT_CTRL |= ANAC_INT_SLEEP_TMR_WK_EN_Msk;

#if CONFIG_DEEPSLEEP_MODE_2
    // Configure power of deepsleep to mode 2 (Only LPLDOH in use)
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ANA->LP_LP_LDO_3V &= ~ANAC_LPLDO_L_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_LDO_ISOLATE_EN_Msk;
#else
    // Configure power of deepsleep to mode 1 (Both LPLDOH & LPLDOL in use)
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_LDO_POWER_CTL_Msk | ANAC_FL_LDO_ISOLATE_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_LDOL_POWER_CTL_Msk;
#endif /* CONFIG_DEEPSLEEP_MODE_2 */

    // Enable LPDOH mode 2 to save power
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;

#if CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Keep flash power in Deepsleep mode
#if CONFIG_FLASH_LDO_EN
    ANA->LP_FL_CTRL_3V |= ANAC_FL_FLASH_LP_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_BP_EN_Msk;
#else
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_LP_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_FL_FLASH_BP_EN_Msk;
#endif /* CONFIG_FLASH_LDO_EN */
    // Configure rdp wait cnt for auto dp use
    FMC_SetRdpWaitCount(FLCTL, FLASH_RDP_WT_CNT);
#else
    // Power down flash in Deepsleep mode
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_LP_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_BP_EN_Msk;
#endif /* CONFIG_KEEP_FLASH_POWER_IN_LP_MODE */

    // Configure power of SRAM
    ANA->LP_FL_CTRL_3V = ((RamPowerCtrl & 0x1f) << 24u) | (ANA->LP_FL_CTRL_3V & 0xe0FFFFFF);

    // Configure LP delay
    ANA->LP_DLY_CTRL_3V &= ~0x3ff;
    ANA->LP_DLY_CTRL_3V |= LP_DLY_TICK;

    // Configure LP irq
    NVIC_SetPriority(LP_IRQn, 3);       // Lowest prio

#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
    system_watch_dog_init();
#endif
}

void soc_pm_init(void)
{
    // Init deepsleep flow
    deepsleep_init();

    // Clear phy rxlna icore register (R0062) to avoid power leakage in low power mode
    PHY_SingleRegWrite(0x62, 0x00);
//    SYS_PRINT("phy R0062 reg val after clear: 0x%02x\n", PHY_SingleRegRead(0x62));

    /* Store gpio int flags if any in case of waking up from standby mode 1 by gpio interrupt */
    *((uint8_t *)&stbm1_gpio_int_flag + 0) = P0->INTSRC;
    *((uint8_t *)&stbm1_gpio_int_flag + 1) = P1->INTSRC;
    *((uint8_t *)&stbm1_gpio_int_flag + 2) = P2->INTSRC;
    *((uint8_t *)&stbm1_gpio_int_flag + 3) = P3->INTSRC;
}

#if CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
__ASM void wfi_with_core_regs_backup_and_resume(void)
{
    push {r0-r7}    /* Backup r0 ~ r7 */
    mov r0, r8
    mov r1, r9
    mov r2, r10
    mov r3, r11
    mov r4, r12
    mov r5, lr
    push {r0-r5}    /* Backup r8 ~ r12 and lr */
    wfi             /* Trigger hw to enter low power mode */
    pop {r0-r5}     /* Restore r8 ~ r12 and lr */
    mov r8, r0
    mov r9, r1
    mov r10, r2
    mov r11, r3
    mov r12, r4
    mov lr, r5
    pop {r0-r7}     /* Restore r0 ~ r7 */
    bx lr           /* Function return */
}
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */

/*
 * Several hw modules can be selectable to retain or lose power in this mode
 */
void soc_enter_standby_mode_1(uint32_t wakeup_src, uint32_t retention_sram)
{
    /* Mask all IRQs when we are on the way to enter standby mode */
    __disable_irq();

    /* Disable Systick clock */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    /* Clear pending flag of systick (PENDSTCLR) if any */
    SCB->ICSR = (SCB->ICSR & (~BIT26)) | BIT25;

    /* StandbyM1 Power Mode 1, use both LPLDOH and LPLDOL */
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_FL_LDO_ISOLATE_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~(ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk);

    /* LPDOH switch to mode 2 for better power consumption performance */
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;

    /* Power down Flash in lp mode discard the dedicated Flash LDO enabled or not */
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~(0x3 << 12u)) | (0x0 << 12u);

    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }

    /*
     * Configure retention SRAM modules in low power mode:
     * - (BIT2) The 256B Decrypt SRAM should be enabled in standby mode 1 in case Firmware Encryption enabled
     * - (BIT3) The 256 B PHY SEQ RAM + PHY Registers should be always enabled in standby mode 1 to avoid
     *          possible power leakage of RF phy rxlna icore registers
     * - (BIT0/BIT1/BIT4) The other SRAMs (32KB SRAM0 / 16KB SRAM1 / 8KB LLSRAM) can be configured by user to
     *                    enable or disable the power supply in standby mode 1
     */
#if CONFIG_FIRMWARE_ENCRYPTION
    retention_sram |= BIT2; // Decrypt SRAM
#endif
    retention_sram |= BIT3; // PHY SEQ RAM + PHY Registers
    ANA->LP_FL_CTRL_3V = ((retention_sram & 0x1f) << 24u) | (ANA->LP_FL_CTRL_3V & 0xe0FFFFFF);

    /* Set digital delay with 32k tick unit */
    ANA->LP_DLY_CTRL_3V &= ~0x3ff;
    ANA->LP_DLY_CTRL_3V |= LP_DLY_TICK;

    /* Insure we are going to enter hw standby mode 1 */
    LP_SetSleepMode(ANA, LP_MODE_SEL_STANDBY_M1_MODE);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Trigger 3v sync */
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;

//    if (wakeup_src & STBM1_WAKEUP_SRC_GPIO) {
//        /* Do nothing here */
//    } else {
//        /* Reset GPIO module to default state */
//        CLK->IPRST1 = CLK_IPRST1_GPIORST_Msk;
//        CLK->IPRST1 = 0x0;
//    }

    /* Set specific slptmr timeout cnt if needed */
    if (wakeup_src & STBM1_WAKEUP_SRC_SLPTMR) {
        // Clear configured OS wakeup timer (sleep timer 0) to avoid unexpected wakeup
        ANA->LP_SPACING_TIME0 = 0;
    } else {
        /* Disable SLPTMR interrupt and wakeup in lp mode (Only enable LP interrupt) */
        ANA->LP_INT_CTRL = ANAC_INT_LP_INT_EN_Msk;
        // Clear configured time of sleep timers
        ANA->LP_SPACING_TIME0 = 0;
        ANA->LP_SPACING_TIME1 = 0;
        ANA->LP_SPACING_TIME2 = 0;
    }

    /* Clear all lowpower related int flags if any */
    ANA->LP_INT_CTRL = ANA->LP_INT_CTRL;
#if 0
    /* Reset all hw peripheral modules except eFuse and GPIO */
    CLK->IPRST0 = 0x1CC;
    CLK->IPRST0 = 0x0;
    CLK->IPRST1 = 0x17FFF;
    CLK->IPRST1 = 0x0;
#endif
    /* Disable all IRQs and clear all pending IRQs on NVIC */
    NVIC->ICER[0U] = 0xFFFFFFFF;
    NVIC->ICPR[0U] = 0xFFFFFFFF;

    /* Wait until 3v sync done */
    while (ANA->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk)) {
        __NOP();
    }

#if !CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
#if CONFIG_VECTOR_REMAP_TO_RAM
    /* Reset CPU Vector Remap register to avoid issue after waking up */
    ANA->CPU_ADDR_REMAP_CTRL = 0;
#endif

    /* Trigger hw to enter low power mode */
    __WFI();

    /*
     * ======== (Now SoC is expected in HW Standby Mode 1 and would never return back here) =========
     */
#else
    /*
     * Enable CPU core regs retention in standby mode 1
     * The CPU core registers PC, MSP, PSP and CONTROL would automatically
     * be saved and restored by hardware in SoC standby mode 1.
     */
    ANA->LP_FL_CTRL_3V |= ANAC_FL_CPU_RETENTION_EN_Msk;

    /* Backup the FMC remap register in case we configure it in bootloader */
    uint32_t fmc_remap_bkp = FLCTL->X_FL_REMAP_ADDR;

    /* Records the time the current timestamp is used to calculate the sleep
     * value after the system wakes up */
    vTaskTickSet(lp_get_curr_tmr_cnt());

    /*
     * 1. Backup CPU core registers which are not auto-saved by hardware
     * 2. Enter SoC Standby Mode 1 (WFI)
     * 3. Restore previous backup CPU core registers
     */
    wfi_with_core_regs_backup_and_resume();

    /* Restore FMC remap register after waking up */
    FLCTL->X_FL_REMAP_ADDR = fmc_remap_bkp;

    /* Reset SoC lp mode to sleep mode (1.2v area, do not need 3v sync) */
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk)
        | (LP_MODE_SEL_SLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

    /* Restore fmc and flash status */
    FMC_SetFlashMode(FLCTL, PanFlashEnhanceEnable, PanFlashEnhanceEnable);
    /* Reinit I-Cache */
    InitIcache(FLCTL, PanFlashEnhanceEnable);
    /*
     * Clear StandbyM1 int flag (write 1 to clear) in this register, but still
     * retain all other ctrl/status flags.
     */
    ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_STANDBY_M1_FLAG_Msk)
        & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
        | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);

    /* Restore 32K current counter register LPTMR_CURR_CNT_ENA_REG */
    (*(volatile uint32_t *)0x5002000C) |= BIT1;

    /* Update OS tick and scheduler */
    UpdateTickAndSch();

    /* Restart systick, use configTICK_ON_WAKING_RATE_HZ */
    portNVIC_SYSTICK_CTRL_REG = 0UL;
    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
    /* Configure SysTick to interrupt at the requested rate. */
    portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
    portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );

    /* Exit with interrupts enabled. */
    __enable_irq();
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */
}


/*
 * The most power saving mode in which can only be waked up by GPIO P02/P01/P00 pin
 */
void soc_enter_standby_mode_0(void)
{
    /* Mask all IRQs when we are on the way to enter standby mode */
    __disable_irq();

    /* Disable Systick clock */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    /* Clear pending flag of systick (PENDSTCLR) if any */
    SCB->ICSR = (SCB->ICSR & (~BIT26)) | BIT25;
#if 0
    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }
#else
    // Disable 32K Clock Source to save power
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
#endif
    /* Set digital delay with 32k tick unit */
    ANA->LP_DLY_CTRL_3V &= ~0x3ff;
    ANA->LP_DLY_CTRL_3V |= LP_DLY_TICK;

    /* Insure we are going to enter hw standby mode 1 */
    LP_SetSleepMode(ANA, LP_MODE_SEL_STANDBY_M0_MODE);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Trigger 3v sync */
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;

    /* Disable SLPTMR interrupt and wakeup in lp mode (Only enable LP interrupt) */
    ANA->LP_INT_CTRL = ANAC_INT_LP_INT_EN_Msk;
    // Clear configured time of sleep timers
    ANA->LP_SPACING_TIME0 = 0;
    ANA->LP_SPACING_TIME1 = 0;
    ANA->LP_SPACING_TIME2 = 0;

    /* Clear all lowpower related int flags if any */
    ANA->LP_INT_CTRL = ANA->LP_INT_CTRL;
#if 0
    /* Reset all hw peripheral modules except eFuse and GPIO */
    CLK->IPRST0 = 0x1CC;
    CLK->IPRST0 = 0x0;
    CLK->IPRST1 = 0x17FFF;
    CLK->IPRST1 = 0x0;
#endif
    /* Disable all IRQs and clear all pending IRQs on NVIC */
    NVIC->ICER[0U] = 0xFFFFFFFF;
    NVIC->ICPR[0U] = 0xFFFFFFFF;

    /* Wait until 3v sync done */
    while (ANA->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk)) {
        __NOP();
    }

    /* Trigger hw to enter low power mode */
    __WFI();

    /*
     * ======== (Now SoC is expected in HW Standby Mode 0 and would never return back here) =========
     */
}

#endif /* CONFIG_PM */
