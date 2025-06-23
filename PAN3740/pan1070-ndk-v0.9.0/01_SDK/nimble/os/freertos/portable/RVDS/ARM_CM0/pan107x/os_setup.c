#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "ble_config.h"
#include "pan_clktrim.h"
#include "pan_power.h"
#if BLE_EN
#include "pan_ble.h"
#endif
#if CONFIG_RTT_LOG_ENABLE
#include "SEGGER_RTT.h"
#endif
#include "app_log.h"
#include "soc_api.h"
#include "utility.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/


/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/


/*******************************************************************************
 * Function Declaration
 ******************************************************************************/


/*******************************************************************************
 * Function Define
 ******************************************************************************/
#if CONFIG_OS_EN

#if CONFIG_AUTO_OPTIMIZE_POWER_PARAM
#define TEMP_CHECK_INTERVAL_MS      (CONFIG_TEMP_SAMPLE_INTERVAL_S*1000)

static TimerHandle_t temp_check_timer;
static ADC_HandleTypeDef adc_temperature = {
    .id = ADC_CH_TEMP,
};

void temp_check_cb(TimerHandle_t xtimer)
{
    float temp;

    //SYS_INF("temp_check_cb\r\n");

    if (HAL_ADC_GetValue(&adc_temperature, &temp) == HAL_SUCCESS) {
        PW_AutoOptimizeParams((int)temp);

    #if BLE_EN
        /* Update SCA */
        static uint8_t config = 0xFF;
        uint16_t SCA = CONFIG_BT_CTLR_SCA;
        if(temp > 15 && temp < 30){
            if(config == 0)
                return;
            config = 0;
            SCA = CONFIG_BT_CTLR_SCA;
        }
        else{
            if(config == 1)
                return;
            config = 1;
            SCA = (CONFIG_LOW_SPEED_CLOCK_SRC==0 ? 1000:500);
        }
        pan_misc_upd_local_sca(SCA);

        //SYS_INF("update SCA:%d\n", SCA);
    #endif
    }
}

void temp_check_start(uint32_t ms)
{
    (void)ms;

    if(temp_check_timer){
        xTimerStart(temp_check_timer, 0);
    }
}

void temp_check_stop(void)
{
    if(temp_check_timer){
        xTimerStop(temp_check_timer, 0);
    }
}

void temp_check_init(void)
{
    temp_check_timer = xTimerCreate("pwr_param_upd_timer", pdMS_TO_TICKS(TEMP_CHECK_INTERVAL_MS), pdTRUE, NULL, temp_check_cb);
    if(temp_check_timer == NULL){
        SYS_ERR("%s %d temp check timer create failed\n", __FILE__, __LINE__);
    }
    temp_check_start(0);
}
#endif /* CONFIG_AUTO_OPTIMIZE_POWER_PARAM */


TaskHandle_t main_task_h;

void main_task(void *p)
{
#if CONFIG_AUTO_OPTIMIZE_POWER_PARAM
    if(!PW_ParamExists()) {
        SYS_ERR("Optimized power configuration is not found!\n");
        while(1);
    }
    HAL_ADC_Init(&adc_temperature);
    temp_check_cb(NULL);
    temp_check_init();
#endif /* CONFIG_AUTO_OPTIMIZE_POWER_PARAM */

    extern int $Super$$main(void);
    $Super$$main();

    vTaskDelete(NULL);
}

__NO_RETURN void os_schedule_main(void)
{
    xTaskCreate(main_task, "main_task", CONFIG_MAIN_TASK_STACK_SIZE, NULL, CONFIG_MAIN_TASK_PRIO, &main_task_h);

    /* Start OS */
    vTaskStartScheduler();

    SYS_ERR("This is not where the program should go\r\n");
    SYS_ABORT();
}

#endif /* CONFIG_OS_EN */
