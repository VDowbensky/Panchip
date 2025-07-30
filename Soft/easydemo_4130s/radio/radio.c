/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.c
 * @brief
 *
 * @history - V0.7, 2024-3
*******************************************************************************/
#include "pan3029_port.h"
#include "pan3029.h"

extern bool pan3029_irq_trigged_flag;
/*
 * flag that indicate if a new packet is received.
*/
static int packet_received = RADIO_FLAG_IDLE;

/*
 * flag that indicate if transmision is finished.
*/
static int packet_transmit = RADIO_FLAG_IDLE;

struct RxDoneMsg RxDoneParams;

uint32_t cad_tx_timeout_flag = MAC_EVT_TX_CAD_NONE;
/**
 * @brief get receive flag
 * @param[in] <none>
 * @return receive state
 */
uint32_t rf_get_recv_flag(void)
{
    return packet_received;
}

/**
 * @brief set receive flag
 * @param[in] <status> receive flag state to set
 * @return none
 */
void rf_set_recv_flag(int status)
{
    packet_received = status;
}

/**
 * @brief get transmit flag
 * @param[in] <none>
 * @return reansmit state
 */
uint32_t rf_get_transmit_flag(void)
{
    return packet_transmit;
}

/**
 * @brief set transmit flag
 * @param[in] <status> transmit flag state to set
 * @return none
 */
void rf_set_transmit_flag(int status)
{
    packet_transmit = status;
}

/**
 * @brief do basic configuration to initialize
 * @param[in] <none>
 * @return result
 */
uint32_t rf_init(void)
{
    if(PAN3029_deepsleep_wakeup() != OK)
    {
        return FAIL;
    }

    if(PAN3029_ft_calibr() != OK)
    {
        return FAIL;
    }

    if(PAN3029_init() != OK)
    {
        return FAIL;
    }

    if(rf_set_agc(AGC_ON) != OK)
    {
        return FAIL;
    }

    rf_port.antenna_init();

    return OK;
}

/**
 * @brief change PAN3029 mode from deep sleep to wakeup(STB3)
 * @param[in] <none>
 * @return result
 */
uint32_t rf_deepsleep_wakeup(void)
{
    if(PAN3029_deepsleep_wakeup() != OK)
    {
        return FAIL;
    }

    if(PAN3029_init() != OK)
    {
        return FAIL;
    }

    if(rf_set_agc(AGC_ON) != OK)
    {
        return FAIL;
    }

    rf_port.antenna_init();

    return OK;
}

/**
 * @brief change PAN3029 mode from sleep to wakeup(STB3)
 * @param[in] <none>
 * @return result
 */
uint32_t rf_sleep_wakeup(void)
{
    if(PAN3029_sleep_wakeup() != OK)
    {
        return FAIL;
    }
    rf_port.antenna_init();
    return OK;
}

/**
 * @brief change PAN3029 mode from standby3(STB3) to deep sleep, PAN3029 should set DCDC_OFF before enter deepsleep
 * @param[in] <none>
 * @return result
 */
uint32_t rf_deepsleep(void)
{
    rf_port.antenna_close();
    return PAN3029_deepsleep();
}

/**
 * @brief change PAN3029 mode from standby3(STB3) to deep sleep, PAN3029 should set DCDC_OFF before enter sleep
 * @param[in] <none>
 * @return result
 */
uint32_t rf_sleep(void)
{
    rf_port.antenna_close();
    return PAN3029_sleep();
}

/**
 * @brief calculate tx time
 * @param[in] <size> tx len
 * @return tx time(us)
 */
uint32_t rf_get_tx_time(uint8_t size)
{
    return PAN3029_calculate_tx_time(size);
}

/**
 * @brief set rf mode
 * @param[in] <mode>
 *			  PAN3029_MODE_DEEP_SLEEP / PAN3029_MODE_SLEEP
 *            PAN3029_MODE_STB1 / PAN3029_MODE_STB2
 *            PAN3029_MODE_STB3 / PAN3029_MODE_TX / PAN3029_MODE_RX
 * @return result
 */
uint32_t rf_set_mode(uint8_t mode)
{
    return PAN3029_set_mode(mode);
}

/**
 * @brief get rf mode
 * @param[in] <none>
 * @return mode
 *		   PAN3029_MODE_DEEP_SLEEP / PAN3029_MODE_SLEEP
 *         PAN3029_MODE_STB1 / PAN3029_MODE_STB2
 *         PAN3029_MODE_STB3 / PAN3029_MODE_TX / PAN3029_MODE_RX
 */
uint8_t rf_get_mode(void)
{
    return PAN3029_get_mode();
}

/**
 * @brief set rf Tx mode
 * @param[in] <mode>
 *			  PAN3029_TX_SINGLE/PAN3029_TX_CONTINOUS
 * @return result
 */
uint32_t rf_set_tx_mode(uint8_t mode)
{
    return PAN3029_set_tx_mode(mode);
}

/**
 * @brief set rf Rx mode
 * @param[in] <mode>
 *			  PAN3029_RX_SINGLE/PAN3029_RX_SINGLE_TIMEOUT/PAN3029_RX_CONTINOUS
 * @return result
 */
uint32_t rf_set_rx_mode(uint8_t mode)
{
    return PAN3029_set_rx_mode(mode);
}

/**
 * @brief set timeout for Rx. It is useful in PAN3029_RX_SINGLE_TIMEOUT mode
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
uint32_t rf_set_rx_single_timeout(uint32_t timeout)
{
    return PAN3029_set_timeout(timeout);
}

/**
 * @brief get snr value
 * @param[in] <none>
 * @return snr
 */
float rf_get_snr(void)
{
    return PAN3029_get_snr();
}

/**
 * @brief get rssi value
 * @param[in] <none>
 * @return rssi
 */
int8_t rf_get_rssi(void)
{
    return PAN3029_get_rssi();
}

/**
 * @brief current channel energy detection
 * @param[in] <none>
 * @return rssi
 */
int8_t rf_get_channel_rssi(void)
{
    return PAN3029_get_channel_rssi();
}
/**
 * @brief get irq
 * @param[in] <none>
 * @return irq
 */
uint8_t rf_get_irq(void)
{
    return PAN3029_get_irq();
}

/**
 * @brief clr irq
 * @param[in] <none>
 * @return result
 */
uint8_t rf_clr_irq(void)
{
    return PAN3029_clr_irq();
}

/**
 * @brief set refresh
 * @param[in] <none>
 * @return result
 */
uint32_t rf_set_refresh(void)
{
    return PAN3029_refresh();
}

/**
 * @brief set preamble
 * @param[in] <reg> preamble
 * @return result
 */
uint32_t rf_set_preamble(uint16_t pream)
{
    return PAN3029_set_preamble(pream);
}

/**
 * @brief get preamble
 * @param[in] <none>
 * @return preamble
 */
uint32_t rf_get_preamble(void)
{
    return PAN3029_get_preamble();
}

/**
 * @brief CAD function enable
 * @param[in] <threshold>
			  CAD_DETECT_THRESHOLD_0A / CAD_DETECT_THRESHOLD_10 / CAD_DETECT_THRESHOLD_15 / CAD_DETECT_THRESHOLD_20
			<chirps>
			  CAD_DETECT_NUMBER_1 / CAD_DETECT_NUMBER_2 / CAD_DETECT_NUMBER_3 /
 * @return  result
 */
uint32_t rf_set_cad(uint8_t threshold, uint8_t chirps)
{
    return PAN3029_cad_en(threshold, chirps);
}

/**
 * @brief CAD function disable
 * @param[in] <none>
 * @return  result
 */
uint32_t rf_set_cad_off(void)
{
    return PAN3029_cad_off();
}

/**
 * @brief set rf syncword
 * @param[in] <sync> syncword
 * @return result
 */
uint32_t rf_set_syncword(uint8_t sync)
{
    return PAN3029_set_syncword(sync);
}

/**
 * @brief read rf syncword
 * @param[in] <none>
 * @return syncword
 */
uint8_t rf_get_syncword(void)
{
    return PAN3029_get_syncword();
}

/**
 * @brief RF IRQ server routine, it should be call at ISR of IRQ pin
 * @param[in] <none>
 * @return result
 */
void rf_irq_handler(void)
{
    PAN3029_irq_handler();
}

/**
 * @brief set rf plhd mode on , rf will use early interruption
 * @param[in] <addr> PLHD start addr,Range:0..7f
		      <len> PLHD len
			  PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
void rf_set_plhd_rx_on(uint8_t addr,uint8_t len)
{
    PAN3029_set_early_irq(PLHD_IRQ_ON);
    PAN3029_set_plhd(addr, len);
    PAN3029_set_plhd_mask(PLHD_ON);
}

/**
 * @brief set rf plhd mode off
 * @param[in] <none>
 * @return result
 */
void rf_set_plhd_rx_off(void)
{
    PAN3029_set_early_irq(PLHD_IRQ_OFF);
    PAN3029_set_plhd_mask(PLHD_OFF);
}

/**
 * @brief get plhd len reg value
 * @param[in] <none>
 * @return <len> PLHD_LEN8 / PLHD_LEN16
 */

uint8_t rf_get_plhd_len(void)
{
    return PAN3029_get_plhd_len();
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
			   <len> PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint32_t rf_plhd_receive(uint8_t *buf,uint8_t len)
{
    return PAN3029_plhd_receive(buf, len);
}

/**
 * @brief set rf mapm mode on , rf will use mapm interruption
 * @param[in] <none>
 * @return result
 */
void rf_set_mapm_on(void)
{
    PAN3029_mapm_en();
    PAN3029_set_mapm_mask(MAPM_ON);
}

/**
 * @brief set mapm mode off
 * @param[in] <none>
 * @return result
 */
void rf_set_mapm_off(void)
{
    PAN3029_mapm_dis();
    PAN3029_set_mapm_mask(MAPM_OFF);
}

/**
 * @brief configure relevant parameters used in mapm mode
 * @param[in] <p_mapm_cfg>
              <fn>set the number of fields(range in 0x01~0xe0)
              <field_num_mux> The unit code word of the Field counter represents several Fields
              <group_fun_sel> The last group in the Field, its ADDR position function selection
              0:ordinary address      1:Field counter
              <gn> register for configuring the number of groups in a Field
              0 1group\1 2group\2 3group\3 4group
              <pgl>set the number of Preambles in first groups>
              <pgn> the number of Preambles in other groups
              <pn> the number of chirps before syncword after all fields have been sent
 * @return result
 */
void rf_set_mapm_para(stc_mapm_cfg_t *p_mapm_cfg)
{
    PAN3029_set_mapm_field_num(p_mapm_cfg->fn);
    PAN3029_set_mapm_field_num_mux(p_mapm_cfg->fnm);
    PAN3029_set_mapm_group_fun_sel(p_mapm_cfg->gfs);
    PAN3029_set_mapm_group_num(p_mapm_cfg->gn);
    PAN3029_set_mapm_firgroup_preamble_num(p_mapm_cfg->pg1);
    PAN3029_set_mapm_group_preamble_num(p_mapm_cfg->pgn);
    PAN3029_set_mapm_neces_preamble_num(p_mapm_cfg->pn);
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
 * @return length, it will return 0 when no data got
 */
uint32_t rf_receive(uint8_t *buf)
{
    return PAN3029_recv_packet(buf);
}

/**
 * @brief rf enter rx continous mode to receive packet
 * @param[in] <none>
 * @return result
 */
uint32_t rf_enter_continous_rx(void)
{
    if(PAN3029_set_mode(PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    rf_port.set_rx();

    if(PAN3029_set_rx_mode(PAN3029_RX_CONTINOUS) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_mode(PAN3029_MODE_RX) != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief rf enter rx single timeout mode to receive packet
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
uint32_t rf_enter_single_timeout_rx(uint32_t timeout)
{
    if(PAN3029_set_mode(PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    rf_port.set_rx();

    if(PAN3029_set_rx_mode(PAN3029_RX_SINGLE_TIMEOUT) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_timeout(timeout) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_mode(PAN3029_MODE_RX) != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief rf enter rx single mode to receive packet
 * @param[in] <none>
 * @return result
 */
uint32_t rf_enter_single_rx(void)
{
    if(PAN3029_set_mode(PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    rf_port.set_rx();

    if(PAN3029_set_rx_mode(PAN3029_RX_SINGLE) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_mode(PAN3029_MODE_RX) != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief rf enter single tx mode and send packet
 * @param[in] <buf> buffer contain data to send
 * @param[in] <size> the length of data to send
 * @param[in] <tx_time> the packet tx time(us)
 * @return result
 */
uint32_t rf_single_tx_data(uint8_t *buf, uint8_t size, uint32_t *tx_time)
{
    if(PAN3029_set_mode(PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_ldo_pa_on() != OK)
    {
        return FAIL;
    }

    rf_port.set_tx();

    if(PAN3029_set_tx_mode(PAN3029_TX_SINGLE) != OK)
    {
        return FAIL;
    }
    *tx_time = rf_get_tx_time(size);

    if(PAN3029_send_packet(buf, size) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief rf enter continous tx mode to ready send packet
 * @param[in] <none>
 * @return result
 */
uint32_t rf_enter_continous_tx(void)
{
    if(PAN3029_set_mode(PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_tx_mode(PAN3029_TX_CONTINOUS) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief rf continous mode send packet
 * @param[in] <buf> buffer contain data to send
 * @param[in] <size> the length of data to send
 * @return result
 */
uint32_t rf_continous_tx_send_data(uint8_t *buf, uint8_t size)
{
    if(PAN3029_set_ldo_pa_on() != OK)
    {
        return FAIL;
    }

    rf_port.set_tx();

    if(PAN3029_send_packet(buf, size) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief enable AGC function
 * @param[in] <state>
 *			  AGC_OFF/AGC_ON
 * @return result
 */
uint32_t rf_set_agc(uint32_t state)
{
    if(PAN3029_agc_enable(state) != OK)
    {
        return FAIL;
    }
    if(PAN3029_agc_config() != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set rf para
 * @param[in] <para_type> set type, rf_para_type_t para_type
 * @param[in] <para_val> set value
 * @return result
 */
uint32_t rf_set_para(rf_para_type_t para_type, uint32_t para_val)
{
    PAN3029_set_mode(PAN3029_MODE_STB3);
    switch(para_type)
    {
    case RF_PARA_TYPE_FREQ:
        PAN3029_set_freq(para_val);
        rf_set_refresh();
        break;
    case RF_PARA_TYPE_CR:
        PAN3029_set_code_rate(para_val);
        rf_set_refresh();
        break;
    case RF_PARA_TYPE_BW:
        PAN3029_set_bw(para_val);
        rf_set_refresh();
        break;
    case RF_PARA_TYPE_SF:
        PAN3029_set_sf(para_val);
        rf_set_refresh();
        break;
    case RF_PARA_TYPE_TXPOWER:
        PAN3029_set_tx_power(para_val);
        rf_set_refresh();
        break;
    case RF_PARA_TYPE_CRC:
        PAN3029_set_crc(para_val);
        rf_set_refresh();
        break;
    default:
        break;
    }
    return OK;
}

/**
 * @brief get rf para
 * @param[in] <para_type> get typ, rf_para_type_t para_type
 * @param[in] <para_val> get value
 * @return result
 */
uint32_t rf_get_para(rf_para_type_t para_type, uint32_t *para_val)
{
    PAN3029_set_mode(PAN3029_MODE_STB3);
    switch(para_type)
    {
    case RF_PARA_TYPE_FREQ:
        *para_val = PAN3029_read_freq();
        return OK;
    case RF_PARA_TYPE_CR:
        *para_val = PAN3029_get_code_rate();
        return OK;
    case RF_PARA_TYPE_BW:
        *para_val = PAN3029_get_bw();
        return OK;
    case RF_PARA_TYPE_SF:
        *para_val = PAN3029_get_sf();
        return OK;
    case RF_PARA_TYPE_TXPOWER:
        *para_val = PAN3029_get_tx_power();
        return OK;
    case RF_PARA_TYPE_CRC:
        *para_val = PAN3029_get_crc();
        return OK;
    default:
        return FAIL;
    }
}

/**
 * @brief set rf default para
 * @param[in] <none>
 * @return result
 */
void rf_set_default_para(void)
{
    PAN3029_set_mode(PAN3029_MODE_STB3);
    rf_set_para(RF_PARA_TYPE_FREQ, DEFAULT_FREQ);
    rf_set_para(RF_PARA_TYPE_CR, DEFAULT_CR);
    rf_set_para(RF_PARA_TYPE_BW, DEFAULT_BW);
    rf_set_para(RF_PARA_TYPE_SF, DEFAULT_SF);
    rf_set_para(RF_PARA_TYPE_CRC, CRC_ON);
    rf_set_para(RF_PARA_TYPE_TXPOWER, DEFAULT_PWR);
}

/**
 * @brief set dcdc mode, The default configuration is DCDC_OFF, PAN3029 should set DCDC_OFF before enter sleep/deepsleep
 * @param[in] <dcdc_val> dcdc switch
 *		      DCDC_ON / DCDC_OFF
 * @return result
 */
uint32_t rf_set_dcdc_mode(uint32_t dcdc_val)
{
    return PAN3029_set_dcdc_mode(dcdc_val);
}

/**
 * @brief set LDR mode
 * @param[in] <mode> LDR switch
 *		      LDR_ON / LDR_OFF
 * @return result
 */
uint32_t rf_set_ldr(uint32_t mode)
{
    return PAN3029_set_ldr(mode);
}

/**
 * @brief get LDR mode
 * @param[in] <none>
 * @return result
 */
uint32_t rf_get_ldr(void)
{
    return PAN3029_get_ldr();
}

/**
 * @brief set preamble by Spreading Factor,It is useful in all_sf_search mode
 * @param[in] <sf> Spreading Factor
 * @return result
 */
uint32_t rf_set_all_sf_preamble(uint32_t sf)
{
    return PAN3029_set_all_sf_preamble(sf);
}

/**
 * @brief open all sf auto-search mode
 * @param[in] <none>
 * @return result
 */
uint32_t rf_set_all_sf_search(void)
{
    return PAN3029_set_all_sf_search();
}

/**
 * @brief close all sf auto-search mode
 * @param[in] <none>
 * @return result
 */
uint32_t rf_set_all_sf_search_off(void)
{
    return PAN3029_set_all_sf_search_off();
}

/**
 * @brief set mode
 * @param[in] <mode> MODEM_MODE_NORMAL/MODEM_MODE_MULTI_SECTOR
 * @return result
 */
uint32_t rf_set_modem_mode(uint8_t mode)
{
    return PAN3029_set_modem_mode(mode);
}

/**
 * @brief RF IRQ server routine, it should be call at ISR of IRQ pin
 * @param[in] <none>
 * @return result
 */
void rf_irq_process(void)
{
    if(pan3029_irq_trigged_flag == true)
    {
        pan3029_irq_trigged_flag = false;

        uint8_t irq = rf_get_irq();
        if(irq & REG_IRQ_RX_PLHD_DONE)
        {
            RxDoneParams.PlhdSize = rf_get_plhd_len();
            rf_set_recv_flag(RADIO_FLAG_PLHDRXDONE);
            RxDoneParams.PlhdSize = rf_plhd_receive(RxDoneParams.PlhdPayload, RxDoneParams.PlhdSize);
        }
        if(irq & REG_IRQ_MAPM_DONE)
        {
            uint8_t addr_val = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x6e);
            RxDoneParams.mpam_recv_buf[RxDoneParams.mpam_recv_index++] = addr_val;
            rf_set_recv_flag(RADIO_FLAG_MAPM);
        }
        if(irq & REG_IRQ_RX_DONE)
        {
            RxDoneParams.Snr = rf_get_snr();
            RxDoneParams.Rssi = rf_get_rssi();
            rf_set_recv_flag(RADIO_FLAG_RXDONE);
            RxDoneParams.Size = rf_receive(RxDoneParams.Payload);
        }
        if(irq & REG_IRQ_CRC_ERR)
        {
            PAN3029_read_fifo(REG_FIFO_ACC_ADDR, RxDoneParams.TestModePayload, 10);
            rf_set_recv_flag(RADIO_FLAG_RXERR);
        }
        if(irq & REG_IRQ_RX_TIMEOUT)
        {
            rf_set_refresh();
            rf_set_recv_flag(RADIO_FLAG_RXTIMEOUT);
        }
        if(irq & REG_IRQ_TX_DONE)
        {
            PAN3029_set_ldo_pa_off();
            rf_set_transmit_flag(RADIO_FLAG_TXDONE);
        }
        rf_clr_irq();
    }
}

/**
 * @brief get one chirp time
 * @param[in] <bw>,<sf>
 * @return <time> us
 */
uint32_t get_chirp_time(uint32_t bw,uint32_t sf)
{
    switch(bw)
    {
    case 6:
        bw = 62500;
        break;
    case 7:
        bw = 125000;
        break;
    case 8:
        bw = 250000;
        break;
    case 9:
        bw = 500000;
        break;
    default:
        return FAIL;
    }
    return (1000000/bw)*(1<<sf);
}

/**
 * @brief check cad rx inactive
 * @param[in] <one_chirp_time>
 * @return <result> LEVEL_ACTIVE/LEVEL_INACTIVE
 */
uint32_t check_cad_rx_inactive(uint32_t one_chirp_time)
{
    rf_delay_us(one_chirp_time*7);
    rf_delay_us(360);

    if(CHECK_CAD() != 1)
    {
        rf_set_mode(PAN3029_MODE_STB3);
        return LEVEL_INACTIVE;
    }

    return LEVEL_ACTIVE;
}

/**
 * @brief check cad tx inactive
 * @param[in] <none>
 * @return <result> OK/FAIL
 */
uint32_t check_cad_tx_inactive(void)
{
    uint32_t bw,sf;

    rf_get_para(RF_PARA_TYPE_BW, &bw);
    rf_get_para(RF_PARA_TYPE_SF, &sf);
    uint32_t one_chirp_time = get_chirp_time(bw,sf);//us

    if(rf_set_cad(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3) != OK)
    {
        return FAIL;
    }

    cad_tx_timeout_flag = MAC_EVT_TX_CAD_NONE;

    if(rf_enter_continous_rx() != OK)
    {
        return FAIL;
    }

    SET_TIMER_MS(one_chirp_time*7/1000 + 1);

    return OK;
}

/**
 * @brief dcdc calibration
 * @param[in] <none>
 * @return <result> OK/FAIL
 */
uint32_t rf_set_dcdc_calibr_on(void)
{
    if(PAN3029_set_dcdc_mode(DCDC_OFF) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_dcdc_calibr_on(CALIBR_REF_CMP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(100);

    if(PAN3029_set_dcdc_calibr_on(CALIBR_ZERO_CMP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(100);

    if(PAN3029_set_dcdc_calibr_on(CALIBR_IMAX_CMP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(100);

    if(PAN3029_set_dcdc_calibr_off() != OK)
    {
        return FAIL;
    }

    return OK;
}
