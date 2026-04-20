/********************************************************************************************
 * @file networking.c
 * @brief
 *       +───────+───────+────────——————————+─────────────────————————──+
 *       | Byte  | NAME  | DESCRIBE         | NOTE                      |
 *       +───────+───────+──────——─————————─+──────────────────————————─+
 *       | 0     | SYNC  | Synchronous word | Preamble have a sync wrod |
 *       | 1     | TTL   | Time To Live     |                           |
 *       | 2-3   | RAND  | random           |                           |
 *       | 4     | LEN   | data length      |                           |
 *       | 5-    | DATA  | data             |                           |
 *       +───────+───────+──────——──————————+──────────────────——————─——+
 *********************************************************************************************/
#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdint.h>
#include "pan_err.h"

#include "pro_config.h"

#define  NET_RANDOM()  random(10)


#define SYNC_WORD              0xEC
#define DEFAULT_TTL_VALUE      PAN_CFG_NET_TTL_MAX

#define SEND_TIMEOUT_MAX       PAN_CFG_NET_SEND_TIMEOUT
#define CAD_TIMEOUT_MAX        PAN_CFG_NET_CONFLICT_TIMEOUT         //检测chirp信号超时时间，实际值为 （2 * CAD_TIMEOUT_MAX）ms

typedef enum{
    TRANSMISSION_STATE_RECV = 0x01,
    TRANSMISSION_STATE_SEND,
    TRANSMISSION_STATE_NONE
}transmission_state_t;

typedef struct{
    uint8_t sync;
    uint8_t ttl;
    uint16_t rand;
    uint8_t len;
}net_package_t;

typedef void (*chirp_send_callback)(void);
typedef void (*chirp_recv_callback)(uint8_t *data, uint8_t len, uint8_t ttl);
typedef void (*delayms)(uint32_t ms);

transmission_state_t chirp_get_transmission_state(void);
pan_err_t chirp_get_send_result(void);
pan_err_t chirp_send(uint8_t *data, uint8_t len);
pan_err_t chirp_recv(uint8_t *data, uint8_t *len, uint8_t *ttl);
pan_err_t chirp_init(chirp_send_callback tx_cb, chirp_recv_callback rx_cb, delayms delay);
pan_err_t chirp_task(void);
void chirp_set_send_flag(uint8_t flag, uint8_t *data, uint8_t len);
uint8_t chirp_get_send_flag(void);

#endif //NETWORKING_H
