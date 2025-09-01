/**
 *********************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060进行自适应SF发送功能
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 计算给定的SF集合的各个SF对应的TxPreamLen。
 *          5. 进入主循环，根据当前SF索引号设置RF模块的SF和TxPreamLen参数。
 *          6. 将g_TxBuf缓冲区中的数据写入到RF模块中，并开始发送数据包。
 *          7. 在主循环中检测RF中断信号，并处理以下中断事件：
 *             7.1 发送完成中断：关闭内部PA，清除发送完成中断标志位，并打印发送次数。
 *             7.2 处理其他未处理的中断标志。
 *          8. 延时1秒后回到第5步，索引号加1，继续发送下一个数据包。
 * @note    该示例需要配合01_SDK/example/06_auto_sf/rx示例一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为接收端，使用01_SDK/example/06_auto_sf/rx示例代码进行自动识别SF接收数据包。
 *          2. 另一块开发板作为发送端，使用本示例代码进行发送数据。
 * @note    注意：
 *          1. 自适识别SF功能不能与CAD功能同时使用！
 *          2. 需要自适应的SF集合越多，发送端的发送前导码长度越长，发送时间越长。
 *          3. 接收端需要自适应的SF越多，搜索时间越长，搜索时间与SF数量成正比。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *********************************************************************************************
 */

#include "bsp.h"
#include "pan_rf.h"

typedef struct
{
    RfSpreadFactor_t Sf;       //!< Spreading factor
    uint16_t TxPreamLen;       //!< Tx preamble length according to SF
}AutoSfPream_t;

#define AUTO_SF_NUM  5

/**
 * @brief 需要自适应的SF集合
 * @note 该集合包含5个SF值，分别为RF_SF5、RF_SF6、RF_SF7、RF_SF8和RF_SF9
 * @note 该集合用于计算每个SF对应的TxPreamLen
 */
AutoSfPream_t g_AutoSfPream[AUTO_SF_NUM] = 
{
    {RF_SF5, 8},
    {RF_SF6, 8},
    {RF_SF7, 8},
    {RF_SF8, 8},
    {RF_SF9, 8},
};

/* g_TxCount为PAN3029/3060 发送计数变量 */
unsigned int g_TxCount = 0;

/* g_TxBuf为PAN3029/3060 发送数据缓冲区，大小16字节，内容为0~15 */
unsigned char g_TxBuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

unsigned char SF_Index = 0; /* SF索引号 */

/**
 * @brief 根据给定的SF值计算FFT点数
 * @param sf 传入的SF值
 * @return 返回单个chirp符号需要采样的FFT点数
 * @note 该函数根据SF值计算FFT点数，SF值范围为5到12
 */
uint16_t GetFFTPointsBySF(RfSpreadFactor_t sf)
{
    switch (sf)
    {
        case RF_SF5: return 32;    /* 2^5 = 32个采样点数 */
        case RF_SF6: return 64;    /* 2^6 = 64个采样点数 */
        case RF_SF7: return 128;   /* 2^7 = 128个采样点数 */
        case RF_SF8: return 256;   /* 2^8 = 256个采样点数 */
        case RF_SF9: return 512;   /* 2^9 = 512个采样点数 */
        case RF_SF10: return 1024; /* 2^10 = 1024个采样点数 */
        case RF_SF11: return 2048; /* 2^11 = 2048个采样点数 */
        case RF_SF12: return 4096; /* 2^12 = 4096个采样点数 */
        default: return 0;         /* Invalid SF */
    }
}

/**
 * @brief 计算给定SF集合的各个SF对应的TxPreamLen
 * @param pSfPream 指向AutoSfPream_t结构体数组的指针
 * @param SF_Num 集合中SF的数量
 * @note 该函数根据给定的SF集合计算每个SF对应的TxPreamLen
 */
void CalculateTxPreamLenBySF(AutoSfPream_t *pSfPream, int SF_Num)
{
    int i;
    uint16_t TotalPoint = 0; /* 搜索一遍给定的SF总共需要采样的FFT点数 */

    /* 计算接收端搜索一遍给定的SF集合总共所需要采样的FFT点数 */
    for (i = 0; i < SF_Num; i++)
    {
        TotalPoint += GetFFTPointsBySF(pSfPream[i].Sf) * 2; /* 每个SF需要采样两个chirp符号 */
    }

    /* 计算每个SF需要发送的前导码长度 */
    for (i = 0; i < SF_Num; i++)
    {
        uint16_t FFTPointPerChirp = GetFFTPointsBySF(pSfPream[i].Sf); /* 获取当前SF对应的FFT点数 */
        uint16_t quotient = TotalPoint / FFTPointPerChirp;            /* 用总点数除以每个chirp符号的点数，得到当前SF需要发送前导码的个数 */

        pSfPream[i].TxPreamLen = quotient + 2; /* 额外增加2个chirp符号更有利于接收机的捕获 */

        if (pSfPream[i].TxPreamLen < 8)
        {
            pSfPream[i].TxPreamLen = 8; /* 如果TxPreamLen小于8，则设置为8 */
        }
    }
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int32_t main(void)
{
    int ret = 0;

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
    
    printf("Auto sf tx test.\r\n");
    
    RF_ConfigUserParams();                               /* 配置 frequency、SF、BW、Preamble、CRC等参数  */
    CalculateTxPreamLenBySF(g_AutoSfPream, AUTO_SF_NUM); /* 计算SF集合中每个SF对应的TxPreamLen  */

    while(1)
    {
        RF_SetSF(g_AutoSfPream[SF_Index].Sf);               /* 设置SF值  */
        RF_SetPreamLen(g_AutoSfPream[SF_Index].TxPreamLen); /* 设置SF对应的前导码长度  */

        printf("Tx SF:%d, PreamLen:%d\r\n", g_AutoSfPream[SF_Index].Sf, g_AutoSfPream[SF_Index].TxPreamLen);

        if (++SF_Index >= AUTO_SF_NUM)
        {
            SF_Index = 0; /* 如果SF索引超过集合大小，则重置为0 */
        }

        memset(g_TxBuf, g_TxCount, sizeof(g_TxBuf));
        RF_TxSinglePkt(g_TxBuf, sizeof(g_TxBuf)); /* 发送数据包 */
        
        while (1)
        {
            if (CHECK_RF_IRQ()) /* 检测到RF中断，高电平表示有中断 */
            {
                uint8_t IRQFlag;
        
                IRQFlag = RF_GetIRQFlag();    /* 获取中断标志位 */
                if (IRQFlag & RF_IRQ_TX_DONE) /* 发送完成中断 */
                {
                    RF_TurnoffPA();                /* 发送完成后须关闭PA */
                    RF_ClrIRQFlag(RF_IRQ_TX_DONE); /* 清除发送完成中断标志位 */
                    IRQFlag &= ~RF_IRQ_TX_DONE;
                    RF_EnterStandbyState();        /* 发送完成后须设置为RF_STATE_STB3状态 */
                    printf("Tx Count:%d\r\n", ++g_TxCount); /* 打印发送次数 */
                    break;
                }
        
                if (IRQFlag)
                {
                    RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                }
            }
        }

        SysTick_Delay(1000); /* 延时1秒 */
    }
}
