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
#include "soc_api.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "app.h"
#include "app_log.h"


#define I2C_SLAVE_ADDRESS7          (0x50)
#define I2C_MASTER_ADDRESS7         (0x51)

#define TEST_BYTE_LEN               (32)

I2C_HandleTypeDef I2C_OBJ = {
    .I2Cx = I2C0,

    .pTxBuffPtr = NULL, /*Used for TX*/
    .TxXferSize = 0,
    .TxXferCount = 0,

    .pRxBuffPtr = NULL, /*Used for RX*/
    .RxXferSize = 0,
    .RxXferCount = 0,

    .IRQn = I2C0_IRQn,
};

void TestFillBuffer(uint8_t *pBuffer, uint32_t size)
{
    uint32_t idx = 0;

    for(idx = 0; idx < size; idx++) {
        pBuffer[idx] = (uint8_t)(idx % 0x100);
    }
}

static void hal_bsp_init(void)
{
    /* Configure Pinmux for I2C0 SCL and SDA */
    SYS_SET_MFP(P1, 4, I2C0_SCL);
    GPIO_EnableDigitalPath(P1, BIT4);
    GPIO_EnablePullupPath(P1, BIT4);

    SYS_SET_MFP(P1, 3, I2C0_SDA);
    GPIO_EnableDigitalPath(P1, BIT3);
    GPIO_EnablePullupPath(P1, BIT3);

    I2C_OBJ.I2C_InitObj.Role = I2C_ROLE_SLAVE;
    I2C_OBJ.I2C_InitObj.DutyCycle = I2C_DUTYCYCLE_2;
    I2C_OBJ.I2C_InitObj.AddressMode = I2C_ADDR_7BIT;
    I2C_OBJ.I2C_InitObj.ClockSpeed = I2C_SPEED_100K;
    I2C_OBJ.I2C_InitObj.OwnAddress = I2C_SLAVE_ADDRESS7;

    HAL_I2C_Init(&I2C_OBJ);
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init I2C module */
    hal_bsp_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    uint8_t TestWriteBuf[TEST_BYTE_LEN] = {0};

    while(1) {
        APP_LOG_INFO("start i2c slave poll start receive\n");
        HAL_I2C_Slave_ReceiveData(&I2C_OBJ, TestWriteBuf, TEST_BYTE_LEN, 0);
        APP_LOG_INFO("start i2c slave poll received: ");

        for(uint8_t i=0; i<TEST_BYTE_LEN; i++){
            APP_LOG("%02X ",TestWriteBuf[i]);
        }
        APP_LOG("\n");
    }
}
