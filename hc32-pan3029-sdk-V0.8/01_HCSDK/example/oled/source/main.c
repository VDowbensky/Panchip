/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file main.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "hc32_ddl.h"
#include "pan_rf.h"
#include "oled.h"
#include "display.h"

extern struct RxDoneMsg RxDoneParams;
extern uint8_t tx_carrywave_flag;
uint32_t app_tx_mode = 0;
static uint16_t m_au16Adc1Value[ADC1_CH_COUNT];
void Timer0A_CallBack(void);

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

uint8_t vbat_cnt = 0;
uint8_t vbat_low_flag = 0;
uint8_t vbat_charging_flag = 0;
double bat_value = 0;
void Timer0A_CallBack(void)
{
    vbat_cnt++;//500ms
    if(vbat_cnt > 4)
    {
        vbat_cnt = 0;
        ADC_PollingSa(M4_ADC1, m_au16Adc1Value, ADC1_CH_COUNT, TIMEOUT_VAL);
        bat_value = ((float)m_au16Adc1Value[0u] * ADC_VREF * (1.500f)) / (float)ADC1_ACCURACY;

        if(bat_value < 3.7)
        {
            vbat_low_flag = 1;
            if(!vbat_charging_flag)
            {
                BAT_LED_Toggle();
            }
        } else {
            vbat_low_flag = 0;
        }
    }
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int32_t main(void)
{
    uint32_t ret = 0;


    BSP_CLK_Init();
    Timer4_Init(); //delay

    SysTick_Init(1000u);
    SysTick_IncTick();

    BSP_GPIO_Init();
    OLED_Init();
    RF_IRQ_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    Spi_Config();
    Uart_Config();
    AdcConfig();
    Timer0_Init();

    if(TX_RX_SELECT == TX_SELECT)
    {
        app_tx_mode = 1;
    }

    ret = rf_init();
    set_test_mode1_reg();
    if(ret == OK)
    {
        rf_set_default_para();
        show_menu();
        dis_init();
        OLED_Refresh_Gram();
        if(app_tx_mode)
        {
            DDL_Printf("tx mode\r\n");
        }
        else
        {
            DDL_Printf("rx mode\r\n");
            rf_enter_continous_rx();
        }
    } else
    {
        dis_err(" RF Init Fail");
        OLED_Refresh_Gram();
    }
	
    while(1) {
        vbat_scan();
        if(!tx_carrywave_flag)
        {
          rf_irq_process();
        }
        key_scan(); 
        key_event_process();
        process_rf_events();
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
