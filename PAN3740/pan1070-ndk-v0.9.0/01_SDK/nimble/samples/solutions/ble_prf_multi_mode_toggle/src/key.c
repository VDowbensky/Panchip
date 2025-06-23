#include "key.h"
#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "PANSeries.h"
#include "host/ble_gatt.h"
#include "nimble/nimble_npl.h"
#include "app_log.h"

#define task_delay_ms(PAR_MS) vTaskDelay(pdMS_TO_TICKS(PAR_MS))

static TaskHandle_t task_key;
static SemaphoreHandle_t gpio_sem = NULL;

extern uint16_t conn_handle;

__IO bool f_key_up = false;
__IO bool f_key_down = false;


void GPIO0_IRQHandlerOverlay(void)
{
    if (GPIO_GetIntFlag(P0, BIT6)) {
        GPIO_ClrIntFlag(P0, BIT6);
        f_key_up = true;
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}

void GPIO1_IRQHandlerOverlay(void)
{
    if (GPIO_GetIntFlag(P1, BIT2)) {
        GPIO_ClrIntFlag(P1, BIT2);
        f_key_down = true;
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}


void key_vol_up(void)
{
    APP_LOG_INFO("ble mode\n");
	panchip_multi_ble_mode();
}

void key_vol_down(void)
{
    APP_LOG_INFO("prf mode\n");
	panchip_multi_prf_mode();
}

void key_process(void)
{
    if(f_key_up)
    {
        f_key_up = false;
        key_vol_up();
    }
    
    if(f_key_down)
    {
        f_key_down = false;
        key_vol_down();
    }
}


void key_thread_entry(void *parameter)
{
    while(1) {
        xSemaphoreTake(gpio_sem, portMAX_DELAY);
        key_process();
    }
}


void key_init(void)
{
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);
    GPIO_EnableDigitalPath(P0, BIT6);
    GPIO_EnableDigitalPath(P1, BIT2);
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT6);
    GPIO_EnablePullupPath(P1, BIT2);
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

void key_thread_init(void)
{
    gpio_sem = xSemaphoreCreateBinary();
    key_init();
    xTaskCreate(key_thread_entry, "key", 200, NULL, 2, &task_key);
}


