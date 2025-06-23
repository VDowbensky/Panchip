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


ADC_HandleTypeDef adc_battery = {
    .id = ADC_CH_BATTERY,
};

ADC_HandleTypeDef adc_temperature = {
    .id = ADC_CH_TEMP,
};

ADC_HandleTypeDef adc_ch1 = {
    .id = ADC_CH1,
};

TimerHandle_t xtimers[2];

void TimerCallbacks(TimerHandle_t xTimer)
{
    float value;
    if (xTimer == xtimers[0]) {
        HAL_ADC_GetValue(&adc_ch1, &value);
        APP_LOG_INFO("->> ADC Channel (CH1): %.2f mV\n\n", value);
    } else {
        HAL_ADC_GetValue(&adc_temperature, &value);
        APP_LOG_INFO("->> SoC Temperature: %.2f C\n\n", value);
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

    HAL_ADC_Init(&adc_battery);
    HAL_ADC_Init(&adc_temperature);
    HAL_ADC_Init(&adc_ch1);

    /* Create software timer */
    for (uint8_t x = 0; x < 2; x++) {
        xtimers[x] = xTimerCreate("AppTimer",               // Just a text name, not used by the kernel.
                                   (1000 * (x + 1)),        // The timer period in ticks.
                                    pdTRUE,                 // The timers will auto-reload themselves when they expire.
                                    NULL,                   // Assign each timer a unique id equal to its array index.
                                    TimerCallbacks          // Each timer calls the same callback when it expires.
                                   );
    }

    xTimerStart(xtimers[0], 0);
    xTimerStart(xtimers[1], 2000);
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    uint16_t v;

    while (1) {
        vTaskDelay(2000);
        HAL_ADC_GetValue(&adc_battery, &v);
        APP_LOG_INFO("->> Baterry Voltage (VBAT): %d mV\n\n", v);
    }
}
