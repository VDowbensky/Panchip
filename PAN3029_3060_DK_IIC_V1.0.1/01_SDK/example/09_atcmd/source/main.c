/*******************************************************************************
 * @file main.c
 * @brief AT命令服务器示例
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note 本示例演示基于HC32平台实现PAN3029/3060的AT命令服务器。
 * 主要功能包括：
 *   - 初始化硬件外设（如串口、GPIO等）
 *   - 接收并解析PAN3029/3060相关的AT命令
 *   - 根据AT命令执行相应操作并返回结果
 *
 * 使用方法：
 *   1. 编译并下载程序到开发板
 *   2. 通过串口工具连接开发板
 *   3. 输入AT命令进行测试
 *
 * 注意事项：
 *   - 请根据实际硬件连接情况修改串口配置
 *   - 示例代码仅供参考，实际项目中请根据需求完善命令解析与处理逻辑
 ********************************************************************************
 */

#include "bsp.h"
#include "fifo.h"
#include "pan_rf.h"
#include <at.h>

static uint8_t UART_RxBuf[AT_SERVER_RECV_BUFF_LEN];                   /* UART接收FIFO的缓冲区 */
Fifo_t g_UartRxFifo = {0, 0, AT_SERVER_RECV_BUFF_LEN, 0, UART_RxBuf}; /* 初始化UART接收FIFO */

/**
 * @brief 读取UART接收FIFO中的数据
 * @param[out] Char 读取到的数据
 * @return 读取成功返回1，失败返回0
 */
int UART_ReadFifo(uint8_t *Char)
{
    if(!is_fifo_empty(&g_UartRxFifo))
    {
        *Char = fifo_pop(&g_UartRxFifo);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 清空UART接收FIFO
 */
void UART_FlushRxFifo(void)
{
    fifo_flush(&g_UartRxFifo);
}

int32_t main(void)
{
    BSP_ClockInit();        /* 初始化系统时钟 */
    BSP_SystickInit(1000u); /* 初始化SysTick定时器，1ms */
    BSP_GpioInit();         /* 初始化GPIO */
    BSP_UartInit();         /* 初始化UART */
    BSP_RFBusInit();        /* 初始化SPI/I2C */

    at_server_init();       /* 初始化AT命令服务器 */

    printf("RF atcmd server start.\r\n");

    while (1)
    {
        at_server_process(); /* 处理AT命令 */
    }
}
