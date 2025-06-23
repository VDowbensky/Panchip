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

// In I2C Master Receive with DMA scenario, the I2C receive size should be
// equal or less than 256 due to HAL driver limitation.
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

uint8_t TestBuf[TEST_BYTE_LEN] = {0};

static void Test_Callback(I2C_Cb_Flag_Opt Flag, uint8_t *Buf, uint16_t Size)
{
    APP_LOG_INFO("master DMA callback\n");
    if (Flag == I2C_CB_FLAG_DMA) {
        APP_LOG_INFO("Flag is I2C_CB_FLAG_DMA\n");
    }
    for (uint32_t i = 0; i < Size; i++) {
        APP_LOG("%02x ", Buf[i]);
    }
    APP_LOG("\n");
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

    I2C_OBJ.I2C_InitObj.Role = I2C_ROLE_MASTER;
    I2C_OBJ.I2C_InitObj.DutyCycle = I2C_DUTYCYCLE_2;
    I2C_OBJ.I2C_InitObj.AddressMode = I2C_ADDR_7BIT;
    I2C_OBJ.I2C_InitObj.ClockSpeed = I2C_SPEED_100K;
    I2C_OBJ.I2C_InitObj.OwnAddress = I2C_MASTER_ADDRESS7;

    HAL_I2C_Init(&I2C_OBJ);

    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_DMAC ,ENABLE);
    I2C_OBJ.DMA_SRC = DMAC_Peripheral_I2C0_Rx;
    I2C_OBJ.DMA_DST = DMAC_Peripheral_I2C0_Tx;

    APP_LOG_INFO("master receive start\n");
    HAL_I2C_Master_ReceiveData_DMA(&I2C_OBJ, I2C_SLAVE_ADDRESS7, TestBuf, TEST_BYTE_LEN, Test_Callback);
    APP_LOG_INFO("master receive stop\n");
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
    while (1) {
        // Do nothing here
    }
}
