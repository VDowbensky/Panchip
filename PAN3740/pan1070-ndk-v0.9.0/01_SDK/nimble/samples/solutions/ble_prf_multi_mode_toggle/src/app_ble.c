/**
 *******************************************************************************
 * @file     app_ble.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

#include <string.h>
#include <stdio.h>

#include "PANSeries.h"
#include "pan_ble.h"

#include "app_log.h"
#include "app.h"
#include "key.h"
#include "comm_prf.h"
#include "info.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/**@brief CONN Param Update. */
#define APP_CONN_UPD_EN                  0

#define PRF_TRX_TEST					 0	

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
/**@brief Connection handle */
static uint16_t conn_handle = 0xFFFF;

/**@brief BLE device name. */
static const char *device_name = "ble_hr";

/**@brief Heart-rate data send timer. */
static struct ble_npl_callout  hr_tx_timer;

/**@brief Variable to simulate heart rate */
static uint8_t heartrate = 90;

static bool notify_state = 0;

static volatile uint8_t ant_end_flag = 0;

TimerHandle_t conn_upd_timer;

#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif

int prf_id;

volatile bool connect_stat = false;


/* Variable Declaration */
extern uint16_t hrs_hrm_handle;

extern void RF_RefreshPhySeqRAM(uint8_t isBleMode);

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
 // BLE API
static int app_ble_gap_event(struct ble_gap_event *event, void *arg);
static void app_ble_advertise_start(void);

// hr data tx API
static void app_ble_hr_tx_timer_start(void);
static void app_ble_hr_tx_timer_stop(void);
static void app_ble_hr_tx_timer_cb(struct ble_npl_event *ev);

// connection parameter update API
void app_ble_conn_upd_timer_start(void);

#if (PRF_TRX_TEST==1)
//prf config
pan_prf_config_t tx_config = {
	.work_mode			= PRF_MODE_NORMAL,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= PRF_CHIP_MODE_SEL_XN297,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= PRF_TX_MODE,
	.phy				= PRF_PHY_1M,			//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= PRF_CRC_SEL_CRC16,
	.src				= PRF_SRC_SEL_NOSRC,
	.mode_conf			= PRF_BLE_CONF,
	.rx_timeout			= 50000,		//us
	.rf_channel			= 2410,
	.tx_no_ack			= DISABLE,
	.trf_type			= PRF_TRF_NORMAL,
	.rx_length			= 0,
	.sync_length		= 4,
	.crc_include_sync	= DISABLE,
	.src_include_sync	= DISABLE,
	.sync				= { 0x91, 0xd3, 0x91, 0xd3 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};
#else

//prf config
pan_prf_config_t rx_config = {
	.work_mode			= PRF_MODE_NORMAL,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= PRF_CHIP_MODE_SEL_XN297,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= PRF_RX_MODE,
	.phy				= PRF_PHY_1M,			//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= PRF_CRC_SEL_CRC16,
	.src				= PRF_SRC_SEL_NOSRC,
	.mode_conf			= PRF_BLE_CONF,
	.rx_timeout			= 7500,		//us
	.rf_channel			= 2410,
	.tx_no_ack			= DISABLE,
	.trf_type			= PRF_TRF_NRF52,
	.rx_length			= 5,
	.sync_length		= 4,
	.crc_include_sync	= DISABLE,
	.src_include_sync	= DISABLE,
	.sync				= { 0x91, 0xd3, 0x91, 0xd3 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};
#endif

panchip_prf_payload_t tx_payload = {
	.data_length		= 5,
	.data				= { 0x01, 0x02, 0x03, 0x04, 0x05 },
};

typedef enum PANCHIP_WORK_MODE{
	prf_work_mode = 0,
	ble_work_mode,
}panchip_work_mode_t;

ab_event_node_t prf_trx;
volatile panchip_work_mode_t work_mode = ble_work_mode;

/*******************************************************************************
 * Function Define
 ******************************************************************************/
static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;

    switch (event->type)
	{
    case BLE_GAP_EVENT_CONNECT:
		APP_LOG_INFO("connection %s; status=%d\n", event->connect.status == 0 ? "established" : "failed",
					event->connect.status);

		if (event->connect.status != 0) {
			app_ble_advertise_start();
			conn_handle = 0xFFFF;
			break;
		}

		ble_gap_conn_find(event->conn_update.conn_handle, &out_desc);
		APP_LOG_INFO("\t-conn_intvl:%d us\r\n\t-latency:%d\r\n\t-to:%d ms\r\n",
			               out_desc.conn_itvl*1250, out_desc.conn_latency, out_desc.supervision_timeout*10);

		conn_handle = event->connect.conn_handle;
		
		connect_stat = true;

	#if APP_CONN_UPD_EN
		app_ble_conn_upd_timer_start();
    #endif
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n", (uint8_t)event->disconnect.reason & 0xff);
        conn_handle = 0xFFFF;
		connect_stat = false;

        app_ble_advertise_start();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv duration expired - restart adv\n");
        app_ble_advertise_start();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        APP_LOG_INFO("subscribe event; cur_notify=%d, val_handle=%d\n",
                    event->subscribe.cur_notify, hrs_hrm_handle);

        if (event->subscribe.attr_handle == hrs_hrm_handle) {
            notify_state = event->subscribe.cur_notify;
            app_ble_hr_tx_timer_start();
        }
		else if (event->subscribe.attr_handle != hrs_hrm_handle) {
            notify_state = event->subscribe.cur_notify;
            app_ble_hr_tx_timer_stop();
        }
        break;

	case BLE_GAP_EVENT_NOTIFY_TX:
		APP_LOG_INFO("notify ok\n");
		break;

    case BLE_GAP_EVENT_MTU:
        APP_LOG_INFO("mtu update event; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
		break;

	case BLE_GAP_EVENT_CONN_UPDATE:
	{
		ble_gap_conn_find(event->conn_update.conn_handle, &out_desc);
		APP_LOG_INFO("conn upd cmpl: conn_handle:%d, itvl:%d us, latency:%d, to:%d ms\n",
					out_desc.conn_handle,
					out_desc.conn_itvl * 1250,
					out_desc.conn_latency,
					out_desc.supervision_timeout*10);
        break;
	}
	default:
		break;
    }

    return 0;
}

/*******************************************************************************
 * Connection parameter update
 ******************************************************************************/
void app_ble_conn_upd_timer_cb(TimerHandle_t xTimer)
{
	struct ble_gap_upd_params upd_params = {
		.itvl_min = BLE_GAP_CONN_ITVL_MS(CONFIG_CONN_INTRVL_MS),
		.itvl_max = BLE_GAP_CONN_ITVL_MS(CONFIG_CONN_INTRVL_MS),
		.latency  = 0,
		.supervision_timeout = 500,
	};

    ble_gap_update_params(conn_handle, &upd_params);
}


void app_ble_conn_upd_timer_start(void)
{
	xTimerStart(conn_upd_timer, 0);
}

void app_ble_conn_upd_timer_init(void)
{
	conn_upd_timer = xTimerCreate("conn_upd_timer", pdMS_TO_TICKS(2000), pdFALSE, 0, app_ble_conn_upd_timer_cb);
}


/*******************************************************************************
 * HeartRate Notify
 ******************************************************************************/
static void app_ble_hr_tx_timer_stop(void)
{
    ble_npl_callout_stop(&hr_tx_timer);
}

static void app_ble_hr_tx_timer_start(void)
{
    int rc;
	(void)rc;

    rc = ble_npl_callout_reset(&hr_tx_timer, configTICK_RATE_HZ);
    APP_ASSERT(rc == 0);
}

static void app_ble_hr_tx_timer_cb(struct ble_npl_event *ev)
{
    static uint8_t hrm[2];
    int rc;
	(void)rc;

    if (!notify_state) {
        app_ble_hr_tx_timer_stop();
        heartrate = 90;
        return;
    }

    hrm[0] = 0x06;      /* contact of a sensor */
    hrm[1] = heartrate; /* storing dummy data */

    /* Simulation of heart beats */
    heartrate++;
    if (heartrate == 160) {
        heartrate = 90;
    }

	rc = ble_svc_hrs_notify(conn_handle, hrm, sizeof(hrm));
	if(rc){
		APP_LOG_WRN("hrs send notify failed - rc:%d\n", rc);
	}

	/* restart timer */
    app_ble_hr_tx_timer_start();
}

static void app_ble_hr_tx_timer_init(void)
{
	ble_npl_callout_init(&hr_tx_timer, nimble_port_get_dflt_eventq(),
                          app_ble_hr_tx_timer_cb, NULL);
}


static void app_ble_advertise_start(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;
	(void)rc; //remove compiler warning

	/* set advertising data. */
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_ERR("setting advertisement data failed; rc=%d\n", rc);
        APP_ASSERT(rc == 0);
    }

    /* set advertising parameter. */
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

	adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(CONFIG_ADV_INTRVL_MS);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(CONFIG_ADV_INTRVL_MS + 100);

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
	                       app_ble_gap_event, NULL);
    if (rc != 0) {
        APP_LOG_ERR("enabling advertisement failed; rc=%d\n", rc);
        return;
    }
	APP_LOG_INFO("BLE adv start...\n");
}

#if APP_USE_RANDOM_ADDR
static void app_ble_set_random_addr(void)
{
    int rc = 0;
    ble_addr_t addr;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    APP_ASSERT(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);
    APP_ASSERT(rc == 0);
}
#endif


static void app_ble_set_mac_addr(void)
{
    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};

#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
#endif

#if APP_USER_ROLL_MAC_ADDR
	extern uint8_t pan10x_roll_mac_addr_get(uint8_t *mac);
	pan10x_roll_mac_addr_get(pub_mac);
#endif

    pan_misc_set_bd_addr(pub_mac);
}

void event_tx_fun(void)
{
	APP_LOG_INFO("tx done\n");
}

void event_rx_fun(void)
{
	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
	APP_LOG_INFO("len:%d,rx data:",rx_payload.data_length);

	if (rf_config.work_mode == PRF_MODE_ENHANCE)
	{
		static panchip_prf_payload_t tx_payload =
		{
			.data_length = 10,
			.data = { 0x10, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa },
		};

		tx_payload.data[0]++;
		panchip_prf_set_ack_data(&tx_payload);
	}
	else
	{
		RF_RefreshPhySeqRAM(0);
		panchip_prf_set_phy(rf_config.phy);
		panchip_prf_dual_mode_start(&rf_config);
	}
}

void event_rx_timeout_fun(void)
{
	#if(PRF_TRX_TEST==1)
	APP_LOG_ERR("rx timeout\n");
	#endif
}

void event_crc_err_fun(void)
{
	APP_LOG_ERR("rx data crc err\n");
	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
	APP_LOG_ERR("len:%d,rx data:",rx_payload.data_length);
	RF_RefreshPhySeqRAM(0);
	panchip_prf_set_phy(rf_config.phy);
	panchip_prf_dual_mode_start(&rf_config);
}

void event_pid_err_fun(void)
{

}

void panchip_prf_isr_init(void)
{
	isr_cb.tx_cb = event_tx_fun;
	isr_cb.rx_cb = event_rx_fun;
	isr_cb.rx_timeout_cb = event_rx_timeout_fun;
	isr_cb.rx_crc_err_cb = event_crc_err_fun;
	isr_cb.rx_pid_err_cb = event_pid_err_fun;
}

void prf_start_event_handler(void)
{
	RF_RefreshPhySeqRAM(0);
	panchip_prf_set_phy(rf_config.phy);
	panchip_prf_dual_mode_start(&rf_config);

	ant_end_flag = 0;
}

void prf_end_event_handler(void)
{
	ant_end_flag = 1;
}

void panchip_prf_trx_trans_time(pan_prf_config_t *p_config)
{
	uint16_t trans_wait_time = 0;

	if( p_config->trx_mode == PRF_RX_MODE) {
		trans_wait_time = 350;
	} else if( p_config->trx_mode == PRF_TX_MODE) {
		trans_wait_time = 300;
	}

	PRI_RF_SetTrxTransWaitTime(PRI_RF, trans_wait_time);
}

__ramfunc void panchip_prf_ble_resume(void)
{
	RF_RefreshPhySeqRAM(1);
	
	PRI_RF_ChipModeSel(PRI_RF, PRF_CHIP_MODE_SEL_BLE);

	PHY_ResetChannel();
}

void panchip_ble_stop(void)
{
	if(conn_handle == 0) {
		ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
		while(connect_stat);
		vTaskDelay(50);
	}
	APP_LOG_INFO("adv stop\n");
	ble_gap_adv_stop();
}

void panchip_multi_prf_mode(void)
{
	if(work_mode == prf_work_mode) {
		return;
	}

	work_mode = prf_work_mode;
	panchip_ble_stop();

	#if(PRF_TRX_TEST==1)
	prf_id = pan_ant_create(&prf_trx);
	#else
	PRI_RF_SetRxWaitTime(PRI_RF, 0);
	#endif
	RF_RefreshPhySeqRAM(0);
	panchip_prf_set_phy(rf_config.phy);
	panchip_prf_set_trx_mode(rf_config.trx_mode);
	panchip_prf_dual_mode_start(&rf_config);
}

void panchip_multi_ble_mode(void)
{
	if(work_mode == ble_work_mode) {
		return;
	}

	work_mode = ble_work_mode;
	#if(PRF_TRX_TEST==1)
	pan_ant_delete(prf_id);
	#endif

	panchip_prf_ble_resume();
	
	app_ble_advertise_start();
}

/**
 * @brief: BLE Stack Enabled call-back.
 */
static void app_ble_enabled_cb(void)
{
    int rc;
	(void)rc;

#if APP_USE_RANDOM_ADDR
    /* Set random address. */
    app_ble_set_random_addr();
#endif

    /* print device id address. */
    uint8_t addr[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr, NULL);
	APP_ASSERT(rc == 0);
    APP_LOG_WRN("Device Id Address: ");
    APP_LOG_DATA(addr, 6);

	pan_ant_init();

	extern uint32_t BB_UsToTick(uint32_t us);

	uint32_t tick = (*(volatile uint32_t *)0x50020014);
	#if(PRF_TRX_TEST == 1)
	prf_trx.interval      = BB_UsToTick(1000);
	prf_trx.slot_duration = 1;
	#else
	prf_trx.interval      = BB_UsToTick(10000);
	prf_trx.slot_duration = 7;
	#endif
	prf_trx.before_point  = 0;
	prf_trx.anchor_point  = tick;
	prf_trx.AntStartCback = prf_start_event_handler;
	prf_trx.AntStopCback  = prf_end_event_handler;
	prf_trx.priority      = 0;
	
	if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
    } else {
		phy_value_init_from_code_prf();
	}

	#if(PRF_TRX_TEST == 1)
	panchip_prf_init(&tx_config);
	panchip_prf_set_chn(tx_config.rf_channel);
	#else
	panchip_prf_init(&rx_config);
	panchip_prf_set_chn(rx_config.rf_channel);
	#endif

	/*adr match bit */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);
	panchip_prf_set_data(&tx_payload);
	panchip_prf_ble_resume();

    /* start advertising */
    app_ble_advertise_start();
}

/**
 * @brief: ble stack pre init call-back
 *         !!!Note: 1. The Bluetooth MAC Address must be set in this callback.
 *                  2. The BLE Services must be registered in this callback.
 *                  3. the SDP initialzation must be executed in this callback.
 */
void app_ble_pre_init_cb(void)
{
	/* Set MAC address. */
    app_ble_set_mac_addr();

	/* BLE Service Register */
    app_ble_svc_init();

	/* BLE Application */
	app_ble_hr_tx_timer_init();
	
	key_thread_init();

#if APP_CONN_UPD_EN
	app_ble_conn_upd_timer_init();
#endif
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
