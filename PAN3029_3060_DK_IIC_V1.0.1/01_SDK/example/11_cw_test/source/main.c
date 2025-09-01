/**
 *************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060发送连续波信号的功能。
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例通过以下步骤实现发送连续波信号的功能：
 *          1. 初始化系统时钟、定时器、GPIO、UART、SPI等外设。
 *          2. 初始化RF模块并设置发射功率和频率。
 *          3. 开始发送连续波信号，持续5000ms。
 *          4. 停止发送连续波信号。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *************************************************************************
 */

#include "bsp.h"
#include "pan_rf.h"

int32_t main(void)
{
    int ret;

    BSP_ClockInit();        /* 初始化系统时钟 */
    BSP_SystickInit(1000u); /* 初始化SysTick定时器，1ms */
    BSP_TimerInit();        /* 初始化定时器，1us */
    BSP_GpioInit();         /* 初始化GPIO */
    BSP_UartInit();         /* 初始化UART */
    BSP_RFBusInit();        /* 初始化SPI/I2C */

    ret = RF_Init();        /* PAN3029/3060初始化 */
    if (ret != RF_OK)
    {
        printf("RF init fail\r\n");
        while(1);
    }
    
    RF_SetFreq(490000000); /* 设置频率为490MHz */
    RF_SetTxPower(22);     /* 设置发射功率为最大值22档 */

    printf("RF continuous wave test start.\r\n");
    RF_StartTxContinuousWave(); /* 开始发送连续波 */
    RF_DelayMs(5000);           /* 延时5000ms */
    RF_StopTxContinuousWave();  /* 停止发送连续波 */
    printf("RF continuous wave test stop.\r\n");
    while(1);
}
