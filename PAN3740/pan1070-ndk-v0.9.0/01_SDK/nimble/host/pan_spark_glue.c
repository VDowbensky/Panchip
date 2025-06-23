/**
 *******************************************************************************
 * @file     nimble_glue_spark.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "pan_spark_glue.h"
#include "FreeRTOS.h"
#include "task.h"

#include "nimble_syscfg.h"
#include "nimble/hci_common.h"
#include "pan_ble.h"

#include "comm_prf.h"

#include "ble_config.h"

#include "utility.h"

#if CONFIG_LOW_SPEED_CLOCK_SRC == 0 || CONFIG_LOW_SPEED_CLOCK_SRC == 1
#include "soc_api.h"
#endif

#if CONFIG_OS_EN == 0
#error "RTOS should be enabled in sdk_config.h..."
#endif

#if BLE_EN == 0
#error "BLE should be enabled in sdk_config.h..."
#endif

/**@brief enable ll thread.
  *       !!!Note: user need to increase OS Heap size.!!!
  */
#define BLE_LL_THREAD_EN            		0
#define BLE_LL_THREAD_STACK_SIZE    		256 /* unit: in word */
#define BLE_LL_TRHEAD_PRIO          		4

/**
 * @brief BLE host thread parameter config.
 */
#define BLE_HOST_THREAD_STACK_SIZE  		CONFIG_BLE_HOST_THREAD_STACK_SIZE
#define BLE_HOST_THREAD_PRIO        		CONFIG_BLE_HOST_THREAD_PRIO

/**
 * @brief BLE LL IRQ Priority Config.
 */
#ifndef CONFIG_BLE_LL_IRQ_PRIO
#define CONFIG_BLE_LL_IRQ_PRIO            	0
#endif

/**
 * @brief BLE Event Handler IRQ Priority Config.
 */
#ifndef CONFIG_BLE_EVT_HANDLER_IRQ_PRIO
#define CONFIG_BLE_EVT_HANDLER_IRQ_PRIO   	1
#endif


#if (configUSE_TICKLESS_IDLE == 1)
extern void UpdateTickAndSch(void);
#endif

static TaskHandle_t host_task_h;
static uint8_t ll_enable = false;

#if BLE_LL_THREAD_EN
SemaphoreHandle_t   ll_semphr = NULL;
#endif


#define HCI_BUF_SIZE_ALIGN4(payload)    MEM_ALIGNED4(4 + payload)

#ifdef IP_107x
#define PAN_BLE_CTLR_BUFFER_SIZE    MEM_ALIGNED4(2148 +                                                                          \
												(HCI_BUF_SIZE_ALIGN4(MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE) << 3) +                  \
												(HCI_BUF_SIZE_ALIGN4(65) << 3) +                                                 \
												(500+256) * (CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL) +   \
												80 * CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM +                                     \
												36 * (2 + CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL)        \
												)


#elif defined(IP_101x)
#define PAN_BLE_CTLR_BUFFER_SIZE    MEM_ALIGNED4(1024 +                                                                           \
												(HCI_BUF_SIZE_ALIGN4(251)*1) +                                                    \
												(HCI_BUF_SIZE_ALIGN4(65) *1) +                                                    \
												(500 + 256) * (CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL) +  \
												80 * CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM +                                      \
												36 * (2 + CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL)         \
												)
#endif

static uint32_t mem_buffer[PAN_BLE_CTLR_BUFFER_SIZE / 4];
static uint32_t mem_pos = 0;

static void *mem_alloc(uint32_t size)
{
	void *mem_ret = NULL;
	char *p_mem = (char *)mem_buffer;

	if (PAN_BLE_CTLR_BUFFER_SIZE - mem_pos >= size) {
		mem_ret = &p_mem[mem_pos];
		mem_pos += size;
	} else {
		SYS_ERR("BLE Heap buffer allocated Failed, need:%d B(%d)\n",size,PAN_BLE_CTLR_BUFFER_SIZE);
	}

	SYS_INF("BLE Heap size:%d B, remain:%d B\n", PAN_BLE_CTLR_BUFFER_SIZE, PAN_BLE_CTLR_BUFFER_SIZE - mem_pos);

	return mem_ret;
}


/**@brief BLE Configuration */
pan_ble_cfg ble_cfg = {
	.sleep_clock_source   = CONFIG_LOW_SPEED_CLOCK_SRC,
	.sleep_clock_accuracy = CONFIG_BT_CTLR_SCA,
	.max_num_of_states    = 0,                                 /* unused param */
	.tx_power             = CONFIG_BT_CTLR_TX_POWER_DFT,

	.pf_mem_init          = mem_alloc,
	.link_layer_debug     = ((CONFIG_IO_TIMING_TRACK && CONFIG_BT_CTLR_LINK_LAYER_DEBUG) ? true:false),
	.agc_cfg_mode         = 0,

	/* BLE Controller Configuration Parameter. */
   #if CONFIG_PM
	.pmEnable    = true,
   #endif

	.mstMargin   = CONFIG_BLE_CONTROLLER_MASTER_LINK_MARGIN,
	.wlNum       = CONFIG_BLE_CONTROLLER_WIHTELIST_NUM,
	.rlNum       = CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM,

	.maxMst      = CONFIG_BT_MAX_NUM_OF_CENTRAL,
	.maxSlv      = CONFIG_BT_MAX_NUM_OF_PERIPHERAL,

	.maxAclLen   = MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE,
	.numRxBufs   = CONFIG_BLE_CONTROLLER_RF_RX_BUF_NUM,
	.numTxBufs   = CONFIG_BLE_CONTROLLER_RF_TX_BUF_NUM,
	.numMoreData = CONFIG_BLE_CONTROLLER_MORE_DATA_NUM,
	.llEncTime   = CONFIG_BLE_CONTROLLER_LL_ENC_TIME,
};

#if BLE_LL_THREAD_EN
void pan_ble_ll_thread_entry(void *p)
{
	while(1)
	{
		xSemaphoreTake(ll_semphr, pdMS_TO_TICKS(0xFFFFFFFF));

		P22 = 1;
		pan_ble_handle();
		P22=0;
	}
}

void ble_ll_thread_create(void)
{
	ll_semphr = xSemaphoreCreateBinary();//xSemaphoreCreateCounting(10, 1);
	if(ll_semphr == NULL){
		SYS_ERR("create ll semphr failed\n");
	}

	xTaskCreate(pan_ble_ll_thread_entry, "ll", BLE_LL_THREAD_STACK_SIZE, NULL, BLE_LL_TRHEAD_PRIO, NULL);
}
#endif

CONFIG_RAM_CODE int ll_semphr_cback(void)
{
#if BLE_LL_THREAD_EN
	if(ll_semphr)
	{
		if(IN_ISR()){
			BaseType_t pxHigherPriorityTaskWoken = 0;
			xSemaphoreGiveFromISR(ll_semphr, &pxHigherPriorityTaskWoken);
		}
		else{
			xSemaphoreGive(ll_semphr);
		}
	}
#else
	NVIC_EnableIRQ(BLE_EVENT_PROC_IRQn);
	NVIC_SetPendingIRQ(BLE_EVENT_PROC_IRQn);
#endif
	return 0;
}


void ll_init(void)
{
	if(ll_enable){
		return;
	}

	SYS_INF("Spark Controller Init Start\n");

	/* register 32k clock measure function to controller. (RCL/XTL)*/
#if CONFIG_LOW_SPEED_CLOCK_SRC == 0 || CONFIG_LOW_SPEED_CLOCK_SRC == 1
	pan_misc_register_32k_track_cb(clktrim_32k_clk_measure_start, clktrim_32k_clk_measure_value_get);
#endif

	pan_ll_register_hostcopy_cb(host_copydata);
	pan_ll_register_semphr_cback(ll_semphr_cback);
	pan_ble_hci_init(ble_hci_evt_ll_to_host_cbk, ble_hci_acl_ll_to_host_cbk);
	pan_ble_init(&ble_cfg);

	ll_enable = true;

	SYS_INF("Spark Controller Init OK\n");

}

#if defined(BLE_HCI_PROJ_EN) && BLE_HCI_PROJ_EN
void pan_ble_controller_init(void)
{
	SYS_INF("ble stack init start...\n");

	/* Spack controller initialization. */
	ll_init();

#ifdef IP_101x
	extern void *panchip_mem_init(void);
	if (panchip_mem_init() == NULL) {
		SYS_ERR("%s %d pan101 allocate memory failed from ll data ram\r\n", __FILE__, __LINE__);
	}
#endif

    /* nimble initialization. */
	nimble_port_init();

	/* LL IRQ initialization. */
	NVIC_EnableIRQ(LL_IRQn);
    NVIC_SetPriority(LL_IRQn, CONFIG_BLE_LL_IRQ_PRIO); //defult: highest

    /* Create LL Thread */
#if BLE_LL_THREAD_EN
	ble_ll_thread_create();
#else
	NVIC_EnableIRQ(BLE_EVENT_PROC_IRQn);
    NVIC_SetPriority(BLE_EVENT_PROC_IRQn, CONFIG_BLE_EVT_HANDLER_IRQ_PRIO);//defult: high
#endif

	SYS_INF("ble controller init ok\n");
}

#else
void on_reset(int reason)
{
	SYS_ERR("%s %d: ble stack error occurs\n", __FILE__, __LINE__);
}

void pan_ble_stack_init(ble_stack_pre_init_cb_t ble_stack_pre_init_cb, ble_stack_enabled_cb_t *ble_stack_enabled_cb)
{
	SYS_INF("ble stack init start..\n");

	/* Spack controller initialization. */
	ll_init();

#ifdef IP_101x
	extern void *panchip_mem_init(void);
	if (panchip_mem_init() == NULL) {
		SYS_ERR("%s %d pan101 allocate memory failed from ll data ram\r\n", __FILE__, __LINE__);
	}
#endif

	/* nimble initialization. */
	nimble_port_init();

	/* BLE Pre init */
	if(ble_stack_pre_init_cb){
		ble_stack_pre_init_cb();
	}

	/* Register BLE stack enabled call-back. */
	ble_hs_cfg.reset_cb = on_reset;
	ble_hs_cfg.sync_cb = ble_stack_enabled_cb;

	/* SMP initialization. */
#if MYNEWT_VAL_BLE_SM_SC || MYNEWT_VAL_BLE_SM_LEGACY
	ble_hs_cfg.store_status_cb   = ble_store_util_status_rr; // use for SMP

    /* pair info store initialization. */
	extern void ble_store_config_init(void);
    ble_store_config_init();
#endif

	/* LL IRQ initialization. */
	NVIC_EnableIRQ(LL_IRQn);
    NVIC_SetPriority(LL_IRQn, CONFIG_BLE_LL_IRQ_PRIO); //defult: highest

	/* Create LL Thread */
#if BLE_LL_THREAD_EN
	ble_ll_thread_create();
#else
	NVIC_EnableIRQ(BLE_EVENT_PROC_IRQn);
    NVIC_SetPriority(BLE_EVENT_PROC_IRQn, CONFIG_BLE_EVT_HANDLER_IRQ_PRIO);//defult: high
#endif

	SYS_INF("ble host init ok -> ble thread start..\n");

	/* BLE Host Thread Create. */
	pan_ble_thread_create();
}

static void pan_ble_thread_entry(void *parameter)
{
	nimble_port_run();
}

/**
 * @brief : create ble stack thread
 * @param : none.
 * @return: none.
 */
void pan_ble_thread_create(void)
{
	xTaskCreate(pan_ble_thread_entry, "host", BLE_HOST_THREAD_STACK_SIZE,
			    NULL, BLE_HOST_THREAD_PRIO, &host_task_h);
}

/**
 * @brief: ble stack execute entry.
 *         !!!Note: user need place this API to a thread.
 * @param : none.
 * @return: none.
 */
void pan_ble_stack_start(void)
{
	struct ble_npl_event *ev;

    //while (1) {
        ev = ble_npl_eventq_get(nimble_port_get_dflt_eventq(), BLE_NPL_TIME_FOREVER);
        ble_npl_event_run(ev);
    //}
}
#endif

void vApplicationUserHook(void)
{
	if(ll_enable){
		pan_update_stimer();
	}
}

/**
 * @brief RF IRQ entry
 */
CONFIG_RAM_CODE void LL_IRQHandler(void)
{
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_LL_IRQ, 1);

#ifdef PRF_BLE_DUAL_MODE
	if (panchip_prf_ble_handler()) {
		return;
	}
#endif
	pan_ble_irq();

    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_LL_IRQ, 0);
}

#if !BLE_LL_THREAD_EN
/**
 * @brief BLE controller event handler
 */
CONFIG_RAM_CODE void BLE_EVENT_PROC_IRQ(void)
{
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_BLE_EVNT_IRQ, 1);

	pan_ble_handle();

#if (configUSE_TICKLESS_IDLE == 1)
	UpdateTickAndSch();
#endif

    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_BLE_EVNT_IRQ, 0);
}
#endif


int host_copydata(void *from, void *dst, uint16_t len)
{
	struct os_mbuf *om = from;

	return os_mbuf_copydata(om, 0, OS_MBUF_PKTLEN(om), dst);
}

void ble_hci_evt_ll_to_host_cbk(uint8_t *p_evt, uint16_t evt_len)
{
	void *buf;

	if (p_evt[2] == BLE_HCI_LE_SUBEV_ADV_RPT) {
		buf = ble_transport_alloc_evt(1);
	} else {
		buf = ble_transport_alloc_evt(0);
        if (buf == NULL) {
            SYS_ERR("%s %d: Host transport alloc HCI Evt buffer failed\n", __FILE__, __LINE__);
        }
	}

	if (buf) {
		memcpy(buf, p_evt, evt_len);
		ble_transport_to_hs_evt(buf);
	}
}

void ble_hci_acl_ll_to_host_cbk(uint8_t *p_acl, uint16_t acl_len)
{
	int ret;
	struct os_mbuf *om;

	om = ble_transport_alloc_acl_from_ll();

	if (om) {
		ret = os_mbuf_append(om, p_acl, acl_len);
		if (ret) {
			os_mbuf_free_chain(om);
		} else {
			ble_transport_to_hs_acl(om);
		}
	}
	else{
		SYS_ERR("%s %d: Host transport alloc HCI ACL buffer failed\n", __FILE__, __LINE__);
	}
}

int ble_transport_to_ll_acl_impl(struct os_mbuf *om)
{
	int rc = 0;

	rc = pan_ble_hci_acl_nimble_handle((void *)om, OS_MBUF_PKTLEN(om));

    if(rc == 0){
        os_mbuf_free_chain(om);
    }

	return (rc != 0) ? BLE_ERR_MEM_CAPACITY : 0;
}

int ble_transport_to_ll_cmd_impl(void *buf)
{
	int rc;
	struct ble_hci_cmd *cmd = buf;

	rc = pan_ble_hci_cmd_handle(buf, sizeof(struct ble_hci_cmd) + cmd->length, 0, 0);

    if(rc == 0){
        ble_transport_free(cmd);
    }

	return (rc != 0) ? BLE_ERR_MEM_CAPACITY : 0;
}

void assert_failed(uint8_t exp, uint32_t errCode, char *fileName, int line)
{
	if (exp == 0) {
		SYS_PRINT("[ASSERT] File: %s Line: %d Error:0x%08x\r\n", fileName, line, errCode);
		while(1) {}
	}
}

void pan_misc_set_ll_rx_time_enable(uint8_t en)
{
	pan_svc_interface(SVC_LL_ENABLE_RX_TIME, &en);
}


__weak void pan_misc_init_pa_pin(void)
{
    BB_SetDebugChn(8, 0x14); // TX
    BB_SetDebugChn(9, 0x15); // RX
    
    SYS_SET_MFP(P0, 3, LL_DBG_8); //TX
    SYS_SET_MFP(P0, 4, LL_DBG_9); //RX
}

void pan_misc_set_pa_enable(uint8_t en)
{
    if(en){
        pan_misc_init_pa_pin();
        pan_misc_register_rf_debug_signal(pan_misc_init_pa_pin);
    }
    else{
        pan_misc_register_rf_debug_signal(NULL);
    }
}
