/**
 *******************************************************************************
 * @file     key_led.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app_ui.h"
#include "utility.h"
#include "PANSeries.h"
#include "pan_ble.h"
#include "app_log.h"

/******************************************************************************* 
 * Macro
 ******************************************************************************/
#define LED_R  P24
#define LED_G  P23
#define LED_B  P22


/******************************************************************************* 
 * Variable Define & Declaration 
 ******************************************************************************/
volatile uint8_t phy_mode = 0;  /* 0 ==> 1M, 1 ==> 2M, 2 ==>Coded S2,  3 ==>Coded S8 */
volatile bool f_key_1 = false;
volatile bool f_key_2 = false;

SemaphoreHandle_t gpio_sem = NULL;

extern uint16_t conn_handle;


/******************************************************************************* 
 * Function Declaration
 ******************************************************************************/
 
 
/******************************************************************************* 
 * Function Define
 ******************************************************************************/
void GPIO0_IRQHandlerOverlay(void)
{
    if (GPIO_GetIntFlag(P0, BIT6)) {
        GPIO_ClrIntFlag(P0, BIT6);
		
        f_key_1 = true;
        APP_LOG_INFO("KEY1(P06) pressed\n");
		
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}

void GPIO1_IRQHandlerOverlay(void)
{
    if (GPIO_GetIntFlag(P1, BIT2)) {
        GPIO_ClrIntFlag(P1, BIT2);
		
        f_key_2 = true;
        APP_LOG_INFO("KEY2(P12) pressed\n");
		
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}

char *phy_name[] = {
	"1M PHY",
	"2M PHY",
	"Coded S2",
	"Coded S8",
};

void app_ui_led_process(void)
{
	if(conn_handle == 0xFFFF){
		return;
	}
	
	APP_LOG_INFO("update phy to :%s\n", phy_name[phy_mode]);
				
    switch(phy_mode)
    {
	case 0:
		LED_R = 0;
		LED_G = 0;
		LED_B = 0;
		ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_1M_MASK, BLE_GAP_LE_PHY_1M_MASK, BLE_GAP_LE_PHY_CODED_ANY);
		break;
	case 1:
		LED_R = 1;
		LED_G = 0;
		LED_B = 0;
		ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_CODED_ANY);
		break;
	case 2:
		LED_R = 0;
		LED_G = 1;
		LED_B = 0;
		ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_S2);
		break;
	case 3:
		LED_R = 0;
		LED_G = 0;
		LED_B = 1;
		ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_S8);
		break;
	default:
		break;
    }
}

void key_1_pro(void)
{
    phy_mode ++;
    phy_mode %= 4;
	
    app_ui_led_process();
}

void app_ui_key_process(void)
{
    if(f_key_1)
    {
        f_key_1 = false;
        key_1_pro();
    }
    
    if(f_key_2)
    {
        f_key_2 = false;
		APP_LOG_INFO("Current PHY :%s\n", phy_name[phy_mode]);
    }
}

void app_ui_thread_entry(void *parameter)
{
    while(1) {
        xSemaphoreTake(gpio_sem, portMAX_DELAY);
        app_ui_key_process();
    }
}

void app_ui_key_init(void)
{
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_128);
    
    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);

    GPIO_EnableDebounce(P0, BIT6);
    GPIO_EnableDebounce(P1, BIT2);

    GPIO_EnableDigitalPath(P0, BIT6);
    GPIO_EnableDigitalPath(P1, BIT2);

    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT6);
    
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P1, BIT2);
    
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
	
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

void app_ui_led_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
	
    SYS_SET_MFP(P2, 2, GPIO);
    GPIO_SetMode(P2, BIT2, GPIO_MODE_OUTPUT);
	
    SYS_SET_MFP(P2, 3, GPIO);
    GPIO_SetMode(P2, BIT3, GPIO_MODE_OUTPUT);
	
    SYS_SET_MFP(P2, 4, GPIO);
    GPIO_SetMode(P2, BIT4, GPIO_MODE_OUTPUT);
	
	LED_R = 0;
    LED_G = 0;
    LED_B = 0;
}

void app_ui_init(void)
{
	app_ui_key_init();
	
    app_ui_led_init();
	
    gpio_sem = xSemaphoreCreateBinary();
		
    xTaskCreate(app_ui_thread_entry, "usr_thread", 200, NULL, 2, NULL);
}

