/*******************************************************************************
 *  @file:  pro_config.c
 *  @brief: Global configuration file
 *******************************************************************************/
#ifndef PRO_CONFIG_H
#define PRO_CONFIG_H

#include "pan3028.h"
#include "userlog.h"

/*RF config*/
#define PAN_CFG_RF_BW                        BW_250K
#define PAN_CFG_RF_SF                        SF_7
#define PAN_CFG_RF_FREQ                      470000000
#define PAN_CFG_RF_CR                        CODE_RATE_45

/*network config*/
#define PAN_CFG_NET_SEND_TIMEOUT             1000                //发送超时时间
#define PAN_CFG_NET_CONFLICT_TIMEOUT         3                   //发送前检测信号冲突的次数
#define PAN_CFG_NET_TTL_MAX                  7                   //数据包存活节点数

/*memery config*/
#define PAN_CFG_MEM_UART_CACHE               256                 //Debug串口接收缓存区大小

/*log config*/
#define PAN_CFG_LOG_LEVEL                    LOG_LEVEL_DEBUG     //日志打印级别

#endif //PRO_CONFIG_H
