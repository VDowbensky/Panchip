/**
 *******************************************************************************
 * @file     main.c
 * @create   2024-12-10
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app.h"

/**
 *******************************************************************************
 * @brief This is main OS Task entry if OS enabled (CONFIG_OS_EN = 1)
 *        This is main Loop entry if OS disabled (CONFIG_OS_EN = 0)
 *******************************************************************************
 */
int main(void)
{
    /* Application initialization */
    app_setup();

    /* Application main infinite loop */
    app_main_loop();

    return 0;
}
