#include "FreeRTOS.h"
#include "task.h"
#include "soc_api.h"

#if BLE_EN
#include "pan_ble.h"
#endif

#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
extern void WDT_Stop(void);
extern void WDT_Start(void);
void system_watch_dog_init(void)
{
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_WDT | CLK_APB1Periph_MILI_CLK, ENABLE);
	// Select Clock Source
    CLK_SetWdtClkSrc(CLK_APB1_WDTSEL_MILLI_PULSE);

    // Enable WDT, disable Reset Mode, disable Wakeup Signal
    WDT_Open(WDT_TIMEOUT_2POW16, WDT_RESET_DELAY_2CLK, TRUE, FALSE);
	WDT_Start();
}
#endif /* CONFIG_SYSTEM_WATCH_DOG_ENABLE */

#if (CONFIG_OS_EN && configUSE_TICKLESS_IDLE)
extern void vPortSysTickRestart(uint32_t tick);

/*************** Do not modify these definition ***************/

#define SETUP_TIME              14
#ifdef IP_107x
#define WAKEUP_CNT              (2 + LP_DLY_TICK + 7)
#elif defined(IP_101x)
#define WAKEUP_CNT              (2 + LP_DLY_TICK + 7 + 4)
#endif
#define REG_CNT                 30
#define MINIEST_SLP_CNT         25
#define LP_TIMER_FOREVER        0xFFFFFFFF

extern uint32_t ulTimerCountsForOneTick;
extern void UpdateTickAndSch(void);
extern const uint32_t PanFlashLineMode;
extern const bool PanFlashEnhanceEnable;

volatile static bool ble_more_closer = false;

/*************************************************************/

void sleep_timer0_handler(void)
{
    /* Overrides the default slptmr 0 handler defined in soc_pm.c and just do nothing here */
}

CONFIG_RAM_CODE void deepsleep_pre_process( uint32_t xLP_SleepTime )
{
#if 1//CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Enable flash auto dp
    FLCTL->X_FL_DP_CTL |= 1u;
#endif
    /* Set timeout of slptmr0 (for os tick use) */
    ANA->LP_SPACING_TIME0 = (xLP_SleepTime - WAKEUP_CNT);

    /* Insure we are going to enter hw-deep-sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk) | (LP_MODE_SEL_DEEPSLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

    /* Do AON register (3v) sync manually */
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;
    while(ANA->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk));
}

CONFIG_RAM_CODE void deepsleep_post_process( uint32_t xExpectedIdleTime )
{
    /* Stop slptmr0 after waking up from deepsleep mode */
    ANA->LP_SPACING_TIME0 = 0;

    /* Clear CPU core deepsleep ctrl flag */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    /* Reset SoC lp mode to sleep mode (1.2v area, do not need 3v sync) */
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk) | (LP_MODE_SEL_SLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

#if 1//CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Re-disable flash auto dp
    FLCTL->X_FL_DP_CTL &= ~1u;
#endif

#if 0
	/* Wait for 3v sync ready in case that cpu may be waked up too quick  */
	ANA->LP_REG_SYNC |= (ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk);
	while (ANA->LP_REG_SYNC & ANAC_LP_REG_SYNC_3V_TRG_Msk) {}
#endif

#if CONFIG_LATENCY_ENHANCE
    pan_ll_pm_post_handler();
#endif
}

#if BLE_EN
CONFIG_RAM_CODE uint32_t pan_deep_sleep_flow(TickType_t xExpectedIdleTime)
{
    uint32_t ll_remain_sleep_cyc = pan_get_ll_idle_time();
    
    uint32_t os_decided_sleep_cyc;
    
    if(xExpectedIdleTime == portMAX_DELAY)
    {    
        os_decided_sleep_cyc = LP_TIMER_FOREVER;
    } else {
        os_decided_sleep_cyc = (uint32_t)xExpectedIdleTime;
    }

	/* Check if the active 32K is used to LL timer */
	if (ANA->ACT_32K_CTRL & ANAC_ACT_32K_SEL_Msk) {
		return 0;
	}
    
	/*
	 * Update final sleep time to meet the need of BLE Controller if it needs
	 * an earlier wakeup than other threads.
	 */
	if (ll_remain_sleep_cyc < os_decided_sleep_cyc) {
        ble_more_closer = true;
		if ((ll_remain_sleep_cyc > (SETUP_TIME  + WAKEUP_CNT + REG_CNT)) && (ll_remain_sleep_cyc < 400000)) {
			if (rf_check_sleep_state()) {
				return ll_remain_sleep_cyc;
			} else {
				/* Fall back to Sleep Flow */
				return 0;
			}
		} else {
			/* Fall back to Sleep Flow */
			return 0;
		}
	} else {
        ble_more_closer = false;
		if (rf_check_sleep_state()) {
			if (os_decided_sleep_cyc > (MINIEST_SLP_CNT)) {
				return os_decided_sleep_cyc;
			} else {
				/* Fall back to Sleep Flow */
				return 0;
			}
		} else {
			/* Fall back to Sleep Flow */
			return 0;
		}
	}
}
#else /*deep sleep check without ble event*/
CONFIG_RAM_CODE uint32_t pan_deep_sleep_flow(TickType_t xExpectedIdleTime)
{    
    uint32_t os_decided_sleep_cyc;
    
    if(xExpectedIdleTime == portMAX_DELAY)
    {    
        os_decided_sleep_cyc = LP_TIMER_FOREVER;
    } else {
        os_decided_sleep_cyc = (uint32_t)xExpectedIdleTime;
    }

    /* Check if the active 32K is used to LL timer */
    if (ANA->ACT_32K_CTRL & ANAC_ACT_32K_SEL_Msk) {
        return 0;
    }
    
    if (os_decided_sleep_cyc > (MINIEST_SLP_CNT)) {
        return os_decided_sleep_cyc;
    } else {
        /* Fall back to Sleep Flow */
        return 0;
    }
}
#endif

/*
 * This function can be override by APP to add customized procedure
 * BEFORE SoC enter DeepSleep State
 */
__WEAK void vSocDeepSleepEnterHook(void)
{
    /* Do nothing here */
}

/*
 * This function can be override by APP to add customized procedure
 * AFTER SoC exit DeepSleep State
 */
__WEAK void vSocDeepSleepExitHook(void)
{
    /* Do nothing here */
}

__WEAK uint8_t user_is_sleep_allow(void)
{
	return true;
}

CONFIG_RAM_CODE void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    uint32_t xLPSleepTime;
    eSleepModeStatus eSleepStatus;

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
     * method as that will mask interrupts that should exit sleep mode. */
    __set_PRIMASK(1);

    /* If a context switch is pending or a task is waiting for the scheduler
     * to be unsuspended then abandon the low power entry. */
    eSleepStatus = eTaskConfirmSleepModeStatus();
    
    if( eSleepStatus == eAbortSleep )
    {
        vTaskTickSet(lp_get_curr_tmr_cnt());
        vTaskStepTick(0);
        /* Re-enable interrupts - see comments above the __disable_irq()
         * call above. */
        __set_PRIMASK(0);
        return;
    }

     /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
     * set its parameter to 0 to indicate that its implementation contains
     * its own wait for interrupt or wait for event instruction, and so wfi
     * should not be executed again.  However, the original expected idle
     * time variable must remain unmodified, so a copy is taken. */        
    xLPSleepTime = pan_deep_sleep_flow(xExpectedIdleTime);
	
	if(!user_is_sleep_allow()){
		goto user_sleep_enter;
	}
	
    if(xLPSleepTime)
    {
        /* Stop the SysTick momentarily.  The time the SysTick is stopped for
         * is accounted for as best it can be, but using the tickless mode will
         * inevitably result in some tiny drift of the time maintained by the
         * kernel with respect to calendar time. */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        
        /*
         * This function can be override by APP to add customized procedure
         * BEFORE SoC enter DeepSleep State
         */
        vSocDeepSleepEnterHook();
        
        configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

	#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
        WDT_Stop();
	#endif

		/* Store and Clear NVIC Interrupt Enable Register */
		uint32_t nvicInt = NVIC->ISER[0U];
		NVIC->ICER[0U] = 0xFFFFFFFF;
        /* Enable LP IRQ to make sure related ISR would trigger after wake up */
		NVIC_EnableIRQ(LP_IRQn);

		/* Deepsleep Pre handler */
		deepsleep_pre_process( xLPSleepTime );

        /* Records the time the current timestamp is used to calculate the sleep
         * value after the system wakes up */
        vTaskTickSet(lp_get_curr_tmr_cnt());

        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_DEEPSLEEP_MODE, 0);

        __WFI();    // Try to enter DeepSleep mode

        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_DEEPSLEEP_MODE, 1);

		/* Restore NVIC Interrupt Enable Register */
		NVIC->ISER[0U] = nvicInt;

	#if ((!CONFIG_KEEP_FLASH_POWER_IN_LP_MODE) && (CONFIG_FLASH_LINE_MODE == FLASH_X4_MODE))
        /* Set en_burst_wrap in FMC */
        FLCTL->X_FL_X_MODE |= (0x1 << 16);
        /* Re-issue burst_wrap command to flash if is X4 mode */
        FLCTL->X_FL_CTL = (0 << 8) | (0x05 << 0);
        FLCTL->X_FL_WD[0] = CMD_BURST_READ;
        FLCTL->X_FL_WD[4] = BURST_READ_MODE_32 << 5;
        FLCTL->X_FL_TRIG = CMD_TRIG;
        while (FLCTL->X_FL_TRIG) {
            __NOP();
        }
	#endif

	#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
        WDT_Start();
	#endif
		/* Deepsleep Post handler */
        deepsleep_post_process( xExpectedIdleTime );

        configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

        UpdateTickAndSch();

        /* restart systick, use configTICK_ON_WAKING_RATE_HZ */
		vPortSysTickRestart(ulTimerCountsForOneTick);
		
        /*
         * This function can be override by APP to add customized procedure
         * AFTER SoC exit DeepSleep State
         */
        vSocDeepSleepExitHook();

        /* Exit with interrupts enabled. */
        __set_PRIMASK(0);
    }
    else /* enter mcu sleep, not deep sleep */
    {
user_sleep_enter:
    #if CONFIG_SYSTEM_WATCH_DOG_ENABLE
        WDT_Stop();
    #endif

        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_SLEEP_MODE, 0);

    #if CONFIG_HCLK_OPTIMIZE_EN
        uint32_t tmp = CLK->CLK_TOP_CTRL_3V;
        //CLK_HCLKConfig(15);  // 4MHz 
        CLK_HCLKConfig(7);     // 8MHz 
        //CLK_HCLKConfig(3);   // 16MHz
        __WFI();
        CLK->CLK_TOP_CTRL_3V = tmp;
        
        __set_PRIMASK(0);
    #else
        __WFI();
    #endif

        PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_SLEEP_MODE, 1);

    #if CONFIG_SYSTEM_WATCH_DOG_ENABLE
        WDT_Start();
    #endif

        __set_PRIMASK(0);
    }
}

#endif /* CONFIG_OS_EN && configUSE_TICKLESS_IDLE */
