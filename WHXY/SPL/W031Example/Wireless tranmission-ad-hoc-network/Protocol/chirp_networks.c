/********************************************************************************************
 * @file networking.c
 * @brief
********************************21-12-13**************************************************************/
#include <stdlib.h>
#include <string.h>
#include "chirp_networks.h"
#include "pan3028.h"
#include "radio.h"
#include "userlog.h"
#include "time.h"
#define rf_switch_rx()     rf_set_cad(); \
                           rf_enter_continous_rx()

typedef struct {
    transmission_state_t trans_state;
    chirp_send_callback tx_cb;
    chirp_recv_callback rx_cb;
    delayms delayMs;
    uint16_t check_rand_val[3];
    uint8_t rIndex;
} chirp_net_t;
extern uint16_t valueAdc;
unsigned int random(unsigned int xx)
{
  unsigned int value,ii;

  for(ii=0;ii<xx;ii++)
  {
			value = valueAdc +100+rand()%5 ;        //伪随机数 
  }
  return value;
}

static chirp_net_t chirp_obj;
static uint8_t chirp_send_flag = 0;
static uint8_t *Send_Data = NULL;
static uint8_t Send_Lens = 0;

void chirp_set_send_flag(uint8_t flag, uint8_t *data, uint8_t len)
{
    chirp_send_flag = flag;
    Send_Data = data;
    Send_Lens = len;
}

uint8_t chirp_get_send_flag()
{
    return chirp_send_flag;
}

static uint32_t wait_time = 0;
pan_err_t chirp_get_send_result()
{
    pan_err_t ret;
    
    chirp_obj.delayMs(1);
    if(RADIO_FLAG_TXDONE != rf_get_transmit_flag()) {
        if(++wait_time >= SEND_TIMEOUT_MAX) {
            ret = TRANS_SEND_TIMEOUT;
            goto END;
        }
        return TRANS_SEND_WAIT;
    }
    else{
        rf_set_transmit_flag(RADIO_FLAG_IDLE);
        ret = PAN_OK;
    }

    END:
    wait_time = 0;
    chirp_obj.trans_state = TRANSMISSION_STATE_NONE;
    rf_switch_rx();
    return ret;
}

static void save_rand(uint16_t rand)
{
    if(chirp_obj.rIndex >= 3) {
        chirp_obj.rIndex = 0;
    }
    chirp_obj.check_rand_val[chirp_obj.rIndex] = rand;
    chirp_obj.rIndex++;
}

pan_err_t chirp_send(uint8_t *data, uint8_t len)
{
    if(chirp_obj.trans_state != TRANSMISSION_STATE_NONE) {
        BASLOG(LOG_LEVEL_WARNING, "rf is busy, Try again later\n");
        return TRANS_RF_BUSY;
    }

    if(data == NULL || len == 0 || len > 250) {
        BASLOG(LOG_LEVEL_TEMP, "data is Null or len is invalid\n");
        chirp_set_send_flag(0, NULL, 0);
        return NET_SEND_DATA_CHECK_ERR;
    }

    // 检测当前信道是否有chirp信号
    uint32_t chirp_iot = getOnepreamble(PAN_CFG_RF_BW, PAN_CFG_RF_SF);
    uint8_t cad_cnt = 0;
    for(uint8_t i = 0; i < PAN_CFG_NET_CONFLICT_TIMEOUT; i++){
        chirp_obj.delayMs(chirp_iot);
        if(CHECK_CAD()){
            cad_cnt++;
        }
        else{
            break;
        }
    }
    if(cad_cnt == 3){
        BASLOG(LOG_LEVEL_WARNING, "channel is busy, Try again later\n");
        return TRANS_RF_BUSY;
    }

    uint32_t tx_time = 0;
    uint8_t *buff = (uint8_t *)malloc(len + 5);
    if(buff == NULL) {
        BASLOG(LOG_LEVEL_TEMP, "malloc buff fail\n");
        chirp_set_send_flag(0, NULL, 0);
        return TRANS_SEND_FAIL;
    }
    net_package_t *pack = (net_package_t *)buff;

    pack->sync = SYNC_WORD;
    pack->ttl = DEFAULT_TTL_VALUE;
    pack->rand = NET_RANDOM();
    pack->len = len;
    memcpy(buff + 5, data, len);

    chirp_obj.trans_state = TRANSMISSION_STATE_SEND;
    BASLOG(LOG_LEVEL_DEBUG, "send start\n");
    if(rf_single_tx_data(buff, len + 5, &tx_time) == FAIL) {
        BASLOG(LOG_LEVEL_TEMP, "send data fail\n");
        chirp_obj.trans_state = TRANSMISSION_STATE_NONE;
        rf_switch_rx();
        free(buff);
        chirp_set_send_flag(0, NULL, 0);
        return TRANS_SEND_FAIL;
    }
    BASLOG(LOG_LEVEL_DEBUG, "send end\n");
    save_rand(pack->rand);
    free(buff);
    chirp_set_send_flag(0, NULL, 0);
    return PAN_OK;
}

static int check_rand(uint16_t rand)
{
    for(uint8_t i = 0; i < 3; i++) {
        if(chirp_obj.check_rand_val[i] == rand) {
            return -1;
        }
    }

    save_rand(rand);
    return 0;
}
extern struct RxDoneMsg RxDoneParams;
	struct RxDoneMsg rf_get_rx_msg()
{
    return RxDoneParams;
}
pan_err_t chirp_recv(uint8_t *data, uint8_t *len, uint8_t *ttl)
{
    net_package_t *pack = NULL;
    uint32_t tx_time = 0;
    uint8_t copylen;
    pan_err_t ret;

    //检查是否接收到数据
    if(rf_get_recv_flag() != RADIO_FLAG_RXDONE) {
        *len = 0;
        return TRANS_RECV_NODATA;
    }
    //获取数据信息
    chirp_obj.trans_state = TRANSMISSION_STATE_RECV;
    struct RxDoneMsg rx_msg = rf_get_rx_msg();     //????
    pack = (net_package_t*)rx_msg.Payload;
    
    rf_set_recv_flag(RADIO_FLAG_IDLE);
    //检查同步字
    if(pack->sync != SYNC_WORD) {
        BASLOG(LOG_LEVEL_ERROR, "data error\n");
        ret = TRANS_RECV_DATA_ERROR;
        goto END;
    }
    //检查包重复
    if(check_rand(pack->rand)) {
        BASLOG(LOG_LEVEL_INF, "data package repeat [%d]\n", pack->rand);
        *len = 0;
        ret = TRANS_RECV_NODATA;
        goto END;
    }
    //解包
    *ttl = pack->ttl;
    pack->ttl--;
    copylen = rx_msg.Size > (*len) ? (*len) : rx_msg.Size;
    if(pack->len + 5 > rx_msg.Size) {
        *len = copylen;
        memcpy(data, rx_msg.Payload + 5, copylen - 5);
        ret = TRANS_RECV_NO_COMPLETE;
        goto END;
    }
    *len = pack->len;
    memcpy(data, rx_msg.Payload + 5, pack->len);
    //转发
    if(pack->ttl > 0) {
        if(rf_single_tx_data(rx_msg.Payload, rx_msg.Size, &tx_time) == FAIL) {
            BASLOG(LOG_LEVEL_WARNING, "send data fail\n");
            rf_switch_rx();
            ret = TRANS_SEND_FAIL;
            goto END;
        }
        while(chirp_get_send_result() != PAN_OK);
        BASLOG(LOG_LEVEL_TEMP, "forwarding success\n");
				printf("forwarding success ttl = %d\r\n",pack->ttl);
        rf_switch_rx();
    }
    ret = PAN_OK;

    END:
    chirp_obj.trans_state = TRANSMISSION_STATE_NONE;
    return ret;
}

pan_err_t chirp_task()
{
    uint8_t data[255];
    uint8_t len;
    uint8_t ttl;
    pan_err_t ret = PAN_OK;

    //检查rf是否接收到数据
    if(chirp_obj.trans_state == TRANSMISSION_STATE_NONE) {
        ret = chirp_recv(data, &len, &ttl);
        if(ret == PAN_OK) {
            if(chirp_obj.rx_cb)
                chirp_obj.rx_cb(data, len, ttl);
            ret = TRANS_RECV_SUCCESS;
        }
        else if(ret == TRANS_RECV_NODATA){
            ret = PAN_OK;
        }
    }

    //检查是否是发送状态，是则检查发送结果
    if(chirp_obj.trans_state == TRANSMISSION_STATE_SEND) {
        ret = chirp_get_send_result();
        if(ret == PAN_OK) {
            if(chirp_obj.tx_cb)
                chirp_obj.tx_cb();
            ret = TRANS_SNED_SUCCESS;
        }
        else if(ret == TRANS_SEND_TIMEOUT) {
            BASLOG(LOG_LEVEL_DEBUG, "send timeout\n");
        }
        else if(ret == TRANS_SEND_WAIT){
            ret = PAN_OK;
        }
    }

    //检查是否要发送数据
    if(chirp_get_send_flag()) {
        ret = chirp_send(Send_Data, Send_Lens);
        if(ret != PAN_OK) {
            BASLOG(LOG_LEVEL_DEBUG, "send fail\n");
        }
        else {
            BASLOG(LOG_LEVEL_TEMP, "sending\n");
        }
    }

    return ret;
}

pan_err_t chirp_init(chirp_send_callback tx_cb, chirp_recv_callback rx_cb, delayms delay)
{
    uint32_t ret = rf_init();
    if(ret != OK)
    {
        BASLOG(LOG_LEVEL_ERROR, "RF Init Fail\n");
        return NET_RF_INIT_FAIL;
    }
    rf_set_default_para();

    chirp_obj.tx_cb = tx_cb;
    chirp_obj.rx_cb = rx_cb;
    chirp_obj.delayMs = delay;
    chirp_obj.trans_state = TRANSMISSION_STATE_NONE;
    chirp_obj.rIndex = 0;
    rf_set_cad();
    rf_enter_continous_rx();

    return PAN_OK;
}
