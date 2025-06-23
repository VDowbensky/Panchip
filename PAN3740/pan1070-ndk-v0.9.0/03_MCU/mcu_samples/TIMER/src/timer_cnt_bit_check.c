/**************************************************************************//**
 * @file     timer_counting_modes_test.c
 * @version  V1.0
 * $Date:    19/10/30 18:00 $
 * @brief    Timer test case 1, test Timer Counting Modes, including One-Shot
 *           Mode, Periodic Mode, Toggle-Output Mode, Continuous-Counting Mode.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "timer_common.h"


static void TIMER_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to choose timer:                                   | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Timer0 selected.                                | \n");
    SYS_TEST("|    Input 'B'    Timer1 selected.                                | \n");
    SYS_TEST("|    Input 'C'    Timer2 selected.                                | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

T_TIMER_TEST_RESULT TIMER_BitCheckTestCase5(void)
{
		TIMER_T *test_timer = TIMER0;
		IRQn_Type test_irq = TMR0_IRQn;
		uint8_t timer_id = 0;
	
		TIMER0->CTL |= (0X7 << 8);
		TIMER1->CTL |= (0X7 << 8);
		TIMER2->CTL |= (0X7 << 8);
	
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency
    uint32_t tmrCmpValue = 0;       //Comparison Value

		TIMER_PrintInfoCase1();

		char c = getchar();
		switch (c) {
			case 'a': 
			case 'A': 
				test_timer = TIMER0;test_irq = TMR0_IRQn; break;
			case 'b': 
			case 'B':
				test_timer = TIMER1;test_irq = TMR1_IRQn; break;
			case 'c': 
			case 'C': 
				test_timer = TIMER2;test_irq = TMR2_IRQn; break;
			default: break;
		}
    // Select Timer clock source
		if (test_timer == TIMER0){
			CLK_SetTmrClkSrc(test_timer, CLK_APB1_TMR0SEL_APB1CLK);
			timer_id = 0;
		} else if (test_timer == TIMER1) {
			CLK_SetTmrClkSrc(test_timer, CLK_APB2_TMR1SEL_APB2CLK);
			timer_id = 1;
		}else if (test_timer == TIMER2) {
			CLK_SetTmrClkSrc(test_timer, CLK_APB2_TMR2SEL_APB2CLK);
			timer_id = 2;
		}

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = FREQ_48MHZ;
    tmrRealCntFreq = TIMER_Open(test_timer, TIMER_ONESHOT_MODE, tmrExpCntFreq);

    // Set compare value (to 16M)
    // expect_timeout = tmrCmpValue / tmrExpCntFreq = 16M / 16MHz = 1s
    // real_timeout = tmrCmpValue / tmrRealCntFreq = 16M / tmrRealCntFreq
    // tmrCmpValue = 0x1007D05; //bigger than 24bit      //0xF42400; //16000000
    tmrCmpValue = 0x1000000+0xF0000; //bigger than 24bit      //0xF42400; //16000000

    TIMER_SetCmpValue(test_timer, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Enable interrupt
    TIMER_EnableInt(test_timer);
    NVIC_EnableIRQ(test_irq);

		// if ((test_timer == TIMER1) || (test_timer == TIMER2)) {
		// 	tmrCmpValue = (tmrCmpValue >= 0x1000000) ? (tmrCmpValue - 0x1000000):(tmrCmpValue);
		// }
		
    // Start Timer counting
    SYS_TEST("\nTimer%d, Compare Value:%d\n",timer_id, tmrCmpValue);
    SYS_TEST("Expected Cnt Freq:%d, Real Cnt Freq:%d\n", tmrExpCntFreq, tmrRealCntFreq);
    if ((test_timer == TIMER1) || (test_timer == TIMER2))
    {
      SYS_TEST("Expected Timeout:%llums, Real Timeout:%llums\n",
               1000ull * (tmrCmpValue-0x1000000) / tmrExpCntFreq, 1000ull * (tmrCmpValue-0x1000000) / tmrRealCntFreq);
              // 1000ull * (0xF0000) / tmrExpCntFreq, 1000ull * (0xF0000) / tmrRealCntFreq);

    }
    else if(test_timer == TIMER0)
    {
      SYS_TEST("Expected Timeout:%llums, Real Timeout:%llums\n",
               1000ull * (tmrCmpValue) / tmrExpCntFreq, 1000ull * tmrCmpValue / tmrRealCntFreq);
    }

    SYS_TEST("Start Timer...\n");
		P23 = 0;
    TIMER_Start(test_timer);

    while(!tmr_int_trigger_cnt);
		P23 = 1;
    tmr_int_trigger_cnt = 0;

    // Disable interrupt
    TIMER_DisableInt(test_timer);
    NVIC_DisableIRQ(test_irq);
/*
    CNT (Count Value) is automatically cleared by hardware in One-Shot Mode.
    CNTEN (Count Enable) is automatically reset to 0 by hardware in One-Shot Mode.
*/
OUT:
    return TIMER_TST_OK;
}



