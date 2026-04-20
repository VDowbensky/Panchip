#include "xcmd_confg.h"
#include "xcmd_default_cmds.h"
#include "xcmd.h"
#include <stdlib.h>
#include "userlog.h"
#include "chirp_networks.h"
#include "radio.h"
#include "LED_light.h"

static int cmd_clear(int argc, char* argv[])
{
    xcmd_print("\033c");
    return 0;
}

static int cmd_help(int argc, char* argv[])
{
    xcmd_t *p = xcmd_cmdlist_get();
    while(p)
    {
        xcmd_print("%-20s %s\r\n",p->name, p->help);
        p = p->next;
    }
    return 0;
}

static int cmd_keys(int argc, char* argv[])
{
    xcmd_key_t *p = xcmd_keylist_get();
    while(p)
    {
        xcmd_print("0x%08X\t", p->key);
        xcmd_print("%s\r\n",p->help);
        p = p->next;
    }
    return 0;
}

static int cmd_set_cr(int argc, char* argv[])
{
    if(argc < 2) {
        BASLOG(LOG_LEVEL_ERROR, "param too few\n");
        return -1;
    }

    uint8_t cr = atoi(argv[1]);

    rf_set_para(RF_PARA_TYPE_CR, cr);
    BASLOG(LOG_LEVEL_DEBUG, "cr %d set success\n", cr);
    return 0;
}

static int cmd_set_bw(int argc, char* argv[])
{
    if(argc < 2) {
        BASLOG(LOG_LEVEL_ERROR, "param too few\n");
        return -1;
    }

    uint8_t bw = atoi(argv[1]);

    rf_set_para(RF_PARA_TYPE_BW, bw);
    BASLOG(LOG_LEVEL_DEBUG, "bw %d set success\n", bw);
    return 0;
}

static int cmd_set_sf(int argc, char* argv[])
{
    if(argc < 2) {
        BASLOG(LOG_LEVEL_ERROR, "param too few\n");
        return -1;
    }

    uint8_t sf = atoi(argv[1]);

    rf_set_para(RF_PARA_TYPE_SF, sf);
    BASLOG(LOG_LEVEL_DEBUG, "sf %d set success\n", sf);
    return 0;
}

static int cmd_set_ch(int argc, char* argv[])
{
    if(argc < 2) {
        BASLOG(LOG_LEVEL_ERROR, "param too few\n");
        return -1;
    }

    uint32_t ch = atoi(argv[1]);

    rf_set_para(RF_PARA_TYPE_FREQ, ch);
    BASLOG(LOG_LEVEL_DEBUG, "ch %d set success\n", ch);
    return 0;
}

static int cmd_send(int argc, char* argv[])
{
    uint8_t data[20];
    int len = 0, mode = 0;
    uint32_t tx_time;

    if(argc < 5) {
        BASLOG(LOG_LEVEL_ERROR, "param too few\n");
        return -1;
    }

    for(int i = 1; i < argc; i+=2) {
        if(strcmp(argv[i], "-l") == 0) {
            len = atoi(argv[i+1]);
            if(len < 1 || len > 20) {
                BASLOG(LOG_LEVEL_ERROR, "len is invalid\n");
                return -1;
            }
        }
        else if(strcmp(argv[i], "-m") == 0) {
            if(strcmp(argv[i+1], "g") == 0) {
                mode = 0;
            }
            else if(strcmp(argv[i+1], "n") == 0) {
                mode = 1;
            }
            else {
                BASLOG(LOG_LEVEL_ERROR, "-m paramter invalid\n");
                return -1;
            }
        }
        else {
            BASLOG(LOG_LEVEL_ERROR, "paramter invalid\n");
            return -1;
        }
    }

    for(int i = 0; i < len; i++) {
        data[i] = i;
    }
    pan_err_t ret;
    BASLOG(LOG_LEVEL_DEBUG, "start send data\n");
    if(mode) {
        ret = chirp_send(data, len);
        if(ret != PAN_OK) {
            BASLOG(LOG_LEVEL_ERROR, "send data fail [%d]\n", ret);
            return -1;
        }
        BASLOG(LOG_LEVEL_DEBUG, "data %d is sending\n", len);
    }
    else {
        if(rf_single_tx_data(data, len, &tx_time) == FAIL) {
            BASLOG(LOG_LEVEL_ERROR, "send data fail\n");
            return -1;
        }
        while(rf_get_transmit_flag() != RADIO_FLAG_TXDONE);
        rf_set_transmit_flag(RADIO_FLAG_IDLE);
        BASLOG(LOG_LEVEL_DEBUG, "send data end\n");
    }

    return 0;
}
	extern struct RxDoneMsg RxDoneParams;
   	struct RxDoneMsg rf_get_rx_msg(void);
static int cmd_recv(int argc, char* argv[])
{
    rf_set_cad();
    rf_enter_continous_rx();

    BASLOG(LOG_LEVEL_DEBUG, "recv data ......\n");

    while(1) {
        if(rf_get_recv_flag() == RADIO_FLAG_RXDONE) {
            rf_set_recv_flag(RADIO_FLAG_IDLE);
            struct RxDoneMsg rx_msg = rf_get_rx_msg();
            printf("recv: ");
            for(int i = 0; i < rx_msg.Size; i++)
   									{
                printf("%x ", rx_msg.Payload[i]);
            }
            printf("\n");
            break;
        }
        else if(rf_get_recv_flag() == RADIO_FLAG_RXERR || rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) {
            BASLOG(LOG_LEVEL_ERROR, "recv fail\n");
            break;
        }
    }
    BASLOG(LOG_LEVEL_DEBUG, "RX end\n");
    return 0;
}

static int cmd_rgb(int argc, char *argv[])
{
    uint8_t r,g,b;

    if(argc < 7) {
        BASLOG(LOG_LEVEL_ERROR, "param too few\n");
        return -1;
    }

    for(int i = 1; i < argc; i+=2) {
        if(strcmp(argv[i], "-r") == 0) {
            r = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-g") == 0) {
            g = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-b") == 0) {
            b = atoi(argv[i+1]);
        }
        else {
            BASLOG(LOG_LEVEL_ERROR, "param is invlid\n");
            return -1;
        }
    }

    BASLOG(LOG_LEVEL_DEBUG, "r: %d, g: %d, b: %d\n", r, g, b);
    return 0;
}

static int cmd_rgb_on(int argc, char *argv[])
{
    led_on();
    return 0;
}

static int cmd_rgb_off(int argc, char *argv[])
{
    led_off();
    return 0;
}

static xcmd_t cmds[] =
{
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"keys", cmd_keys, "show keys", NULL},
    {"cr", cmd_set_cr, "set cr: cr [val]", NULL},
    {"bw", cmd_set_bw, "set bw: bw [val]", NULL},
    {"sf", cmd_set_sf, "set sf: sf [val]", NULL},
    {"ch", cmd_set_ch, "set ch: ch [val]", NULL},
    {"send", cmd_send, "send data,: send [-m] [g/n] [-l] [len:range 1-20]", NULL},
    {"recv", cmd_recv, "recv data", NULL},
    {"led_on", cmd_rgb_on, "led open"},
    {"led_off", cmd_rgb_off, "led close"},
    {"led", cmd_rgb, "led -r [r] -g [g] -b [b]"}
};

void default_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
