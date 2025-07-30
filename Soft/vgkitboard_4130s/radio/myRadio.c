#include "board.h"
#include "myRadio.h"
#include "myRadio_gpio.h"

/**-------------------------radio include----------------------------------**/
#include "radio.h"
#include "pan3029.h"
/**-------------------------radio include end----------------------------------**/

static int8_t rfTxPower;
static uint32_t rfFrequence;
static uint32_t rfBaudrate;
static uint8_t rfSyncword;
static uint8_t rf_sf;
static uint8_t rf_bw;
static uint8_t rf_cr;
static rfRxCallBack rxCb;
static uint8_t rfRxBuffer[255];
static uint32_t rf_handle;
static uint8_t rf_workProcess = RF_PRC_IDLE;
static uint8_t chipType;
/**-------------------------radio params----------------------------------**/

typedef struct 
{
    int8_t power;
    uint8_t regValue;
}rfPowerReg_ts;


rfPowerReg_ts rfPowerRegTab[RF_TX_PWR_MAX_COUNT] = 
{
    {
        .power = -26,
        .regValue = 1,
    },
    {
        .power = -17,
        .regValue = 2,
    },
    {
        .power = -5,
        .regValue = 3,
    },
    {
        .power = -3,
        .regValue = 4,
    },
    {
        .power = -2,
        .regValue = 5,
    },
    {
        .power = -1,
        .regValue = 6,
    },
    {
        .power = 2,
        .regValue = 7,
    },
    {
        .power = 4,
        .regValue = 8,
    },
    {
        .power = 5,
        .regValue = 9,
    },
    {
        .power = 7,
        .regValue = 10,
    },
    {
        .power = 8,
        .regValue = 11,
    },
    {
        .power = 9,
        .regValue = 12,
    },
    {
        .power = 10,
        .regValue = 13,
    },
    {
        .power = 11,
        .regValue = 14,
    },
    {
        .power = 12,
        .regValue = 15,
    },
    {
        .power = 13,
        .regValue = 16,
    },
    {
        .power = 14,
        .regValue = 17,
    },
    {
        .power = 15,
        .regValue = 18,
    },
    {
        .power = 16,
        .regValue = 19,
    },
    {
        .power = 17,
        .regValue = 20,
    },
    {
        .power = 18,
        .regValue = 21,
    },
    {
        .power = 19,
        .regValue = 22,
    },
    {
        .power = 20,
        .regValue = 23,
    }
};
const loraBaudrateFrame_ts loraBaudrateFrame[MAX_RF_BAUDRATE_COUNT] = 
{
    {//244.14bps,SF=12,BW=62.5kHz(6),CR=2
    .SpreadingFactor = 12,        
    .SignalBw = 6,               
    .ErrorCoding = 2,            
    },
    {//627.79 bps,SF=9,BW=62.5kHz(6),CR=3
    .SpreadingFactor = 9,        
    .SignalBw = 6,               
    .ErrorCoding = 4,            
    },
    {//1,255.58 bps,SF=9,BW=125kHz(7),CR=3
    .SpreadingFactor = 9, 
    .SignalBw = 7, 
    .ErrorCoding = 3,            
    },
    {//2,511.16 bps,SF=8,BW=125kHz(7),CR=2
    .SpreadingFactor = 8,        
    .SignalBw = 7,               
    .ErrorCoding = 2,            
    },
    {//5022.32bps,SF=8,BW=250kHz(8),CR=2
    .SpreadingFactor = 8,        
    .SignalBw = 8,               
    .ErrorCoding = 2,            
    },
    {//12500bps,SF=8,BW=500kHz(9),CR=1
    .SpreadingFactor = 8,                                                                                
    .SignalBw = 9,               
    .ErrorCoding = 1,            
    },
    {//20400bps,SF=8,BW=500kHz(9),CR=1
    .SpreadingFactor = 7,        
    .SignalBw = 9,               
    .ErrorCoding = 1,            
    },
    {//62500bps,SF=5,BW=500kHz(9),CR=1
    .SpreadingFactor = 7,        
    .SignalBw = 9,               
    .ErrorCoding = 1,            
    },
};
bool rf_ifq;

uint8_t regulatorMode = DCDC_OFF;
extern struct RxDoneMsg RxDoneParams;
uint8_t getRfPowerTabIndex(int8_t power);
/**-------------------------radio params end----------------------------------**/
void myRadio_delay(uint32_t time_ms)
{
    delay1ms(time_ms);
}
/**
 * @brief IO口中断回调
 *      IO口产生中断后会执行该函数
 *      用于接收射频工作的中断响应
 * 
 * @param index 
 */
void myRadio_gpioCallback(uint8_t index)
{
    rf_ifq = true;
}
/**
 * @brief 射频初始化
 * 
 * @param agr0 
 * @param agr1_ptr 无线工作状态响应回调
 *          产生回调给外部使用，@rfRxCallBack
 */
void myRadio_init(int agr0, void *agr1_ptr)
{
    myRadio_gpio_init(myRadio_gpioCallback);
/**-------------------------radio init----------------------------------**/
    uint32_t ret = 0;
#ifdef SPI_SOFT_3LINE
    PAN3029_write_reg(REG_SYS_CTL, 0x03);
    PAN3029_write_reg(0x1A, 0x83);
#endif

	ret = rf_init();
	if(ret != OK)
	{
		// printf("  RF Init Fail");
		while(1);
	}

	rf_set_default_para();

    rf_set_dcdc_mode(regulatorMode);
/**-------------------------radio init end----------------------------------**/
    myRadio_delay(10);
    RF_EXT_PA_TO_IDLE();
    if ((rfRxCallBack )agr1_ptr)
    {
        rxCb = (rfRxCallBack )agr1_ptr;
    }
    rf_handle = 0xe5;
}
void RadioSetregulatorMode(uint8_t mode)
{
    regulatorMode = mode;
}
/**
 * @brief 射频底层执行程序
 *      要放在主循环中执行
 * 
 */
void myRadio_process(void)
{
    rfRxPacket_ts rfRxPacket;
    if (rf_handle == 0)
    {
        return;
    }
    if (rf_ifq == false)
    {
        return;
    }
    rf_ifq = false;
    if (!((rf_workProcess == RF_PRC_TX) || (rf_workProcess == RF_PRC_RX)))
    {
        return;
    }

    uint8_t plhd_len;
    uint8_t irq = rf_get_irq();

    if(irq & REG_IRQ_RX_PLHD_DONE)
    {
        plhd_len = rf_get_plhd_len();
        rf_set_recv_flag(RADIO_FLAG_PLHDRXDONE);
        RxDoneParams.PlhdSize = rf_plhd_receive(RxDoneParams.PlhdPayload,plhd_len);
        //PAN3029_rst();//stop it

    }
    if(irq & REG_IRQ_RX_DONE)
    {
        RxDoneParams.Snr = rf_get_snr();
        RxDoneParams.Rssi = rf_get_rssi();
        rf_set_recv_flag(RADIO_FLAG_RXDONE);
        RxDoneParams.Size = rf_receive(RxDoneParams.Payload);
        rf_set_recv_flag(RADIO_FLAG_IDLE); 
        if (rxCb)
        {
            rfRxPacket.rssi = RxDoneParams.Rssi;
            rfRxPacket.len = RxDoneParams.Size;
            memcpy(rfRxPacket.payload, RxDoneParams.Payload, RxDoneParams.Size);
            rxCb(RX_STA_SECCESS, rfRxPacket);
        }
        
    }
    if(irq & REG_IRQ_CRC_ERR)
    {
        rf_set_recv_flag(RADIO_FLAG_RXERR);
        rf_clr_irq();

    }
    if(irq & REG_IRQ_RX_TIMEOUT)
    {
        rf_set_refresh();
        rf_set_recv_flag(RADIO_FLAG_RXTIMEOUT);
        rf_clr_irq();
        rf_set_recv_flag(RADIO_FLAG_IDLE); 
        RF_EXT_PA_TO_IDLE();
        if (rxCb)
        {
            rxCb(RX_STA_TIMEOUT, rfRxPacket);
        }
    }
    if(irq & REG_IRQ_TX_DONE)
    {
        rf_set_transmit_flag(RADIO_FLAG_TXDONE);
        rf_clr_irq();

        RF_EXT_PA_TO_IDLE();
        if (rxCb)
        {
            rxCb(TX_STA_SECCESS, rfRxPacket);
        }
    }
}
/**
 * @brief 退出射频进入休眠
 * 
 */
void myRadio_abort(void)
{
    if (rf_handle == 0)
    {
        return;
    }
    RF_EXT_PA_TO_IDLE();
    // rf_deepsleep();
    rf_sleep();
}
/**
 * @brief 获取射频工作中心频率
 * 
 * @return uint32_t 
 */
uint32_t myRadio_getFrequency(void)
{
    if (rf_handle == 0)
    {
        return 0;
    }
    return rfFrequence;
}
/**
 * @brief 设置射频工作中心频率
 * 
 * @param freq 
 *      具体频点，单位：Hz
 */
void myRadio_setFrequency(uint32_t freq)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfFrequence = freq;
    rf_set_para(RF_PARA_TYPE_FREQ, rfFrequence);
}
/**
 * @brief 获取发射功率
 * 
 * @return int8_t 
 */
int8_t myRadio_getTxPower(void)
{
    if (rf_handle == 0)
    {
        return 0;
    }
    return rfTxPower;
}
/**
 * @brief 设置发射功率
 * 
 * @param power 
 *          单位：dbm
 */
void myRadio_setTxPower(int8_t power)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfTxPower = power;
    rf_set_para(RF_PARA_TYPE_TXPOWER, rfPowerRegTab[getRfPowerTabIndex(rfTxPower)].regValue);
}
/**
 * 获取射频波特率
 * @param : br->
*/
uint32_t myRadio_getBaudrate(void)
{
    if (rf_handle == 0)
    {
        return 0;
    }
    return rfBaudrate;
}
/**
 * 设置射频波特率
 * @param : br->
*/
void myRadio_setBaudrate(uint32_t br)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfBaudrate = br;
    rf_set_para(RF_PARA_TYPE_CR, loraBaudrateFrame[br].ErrorCoding);
    rf_set_para(RF_PARA_TYPE_BW, loraBaudrateFrame[br].SignalBw);
    rf_set_para(RF_PARA_TYPE_SF, loraBaudrateFrame[br].SpreadingFactor);
    rf_set_ldr(LDR_OFF);
}
void myRadio_setSyncWord(uint8_t syncword)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfSyncword = syncword;
    rf_set_syncword(syncword);
}
void myRadio_setRfParams(uint8_t sf, uint8_t bw, uint8_t cr)
{
    if (rf_handle == 0)
    {
        return;
    }
    rf_sf = sf;
    rf_bw = bw;
    rf_cr = cr;
    rf_set_para(RF_PARA_TYPE_CR, cr);
    rf_set_para(RF_PARA_TYPE_BW, bw);
    rf_set_para(RF_PARA_TYPE_SF, sf);
    rf_set_ldr(LDR_OFF);
}
/**
 * @brief 设置模组型号
 * 
 * @param type 
 */
void myRadio_setChipType(uint8_t type)
{
    chipType = type;
}
/**
 * @brief 获取模组型号
 * 
 * @return uint8_t 
 */
uint8_t myRadio_getChipType(void)
{
    return chipType;
}
int16_t myRadio_getRssi(void)
{
    return (int16_t)PAN3029_get_rssi();
}
/**
 * @brief 无线发送数据包
 * 
 * @param packet 
 */
void myRadio_transmit(rfTxPacket_ts *packet)
{
    if (rf_handle == 0)
    {
        return;
    }
    RF_EXT_PA_TO_TX();
    uint32_t getTxtime;
    if (rf_get_mode() == PAN3029_MODE_DEEP_SLEEP)
    {
        rf_deepsleep_wakeup();
        myRadio_setFrequency(rfFrequence);
        myRadio_setTxPower(rfTxPower);
        myRadio_setRfParams(rf_sf, rf_bw, rf_cr);
        myRadio_delay(10);
    }
    if (rf_get_mode() == PAN3029_MODE_SLEEP)
    {
        rf_sleep_wakeup();
        myRadio_delay(10);
    }
    
    if(rf_single_tx_data(packet->payload, packet->len, &packet->absTime) != OK)	
    {
    }
    else
    {
    }
    rf_workProcess = RF_PRC_TX;
}
/**
 * @brief 进入无线接收
 * 
 */
void myRadio_receiver(void)
{ 
    if (rf_handle == 0)
    {
        return;
    }
    RF_EXT_PA_TO_RX();
    if (rf_get_mode() == PAN3029_MODE_DEEP_SLEEP)
    {
        rf_deepsleep_wakeup();
        myRadio_setFrequency(rfFrequence);
        myRadio_setTxPower(rfTxPower);
        myRadio_setRfParams(rf_sf, rf_bw, rf_cr);
        myRadio_delay(10);
    }
    if (rf_get_mode() == PAN3029_MODE_SLEEP)
    {
        rf_sleep_wakeup();
        myRadio_delay(10);
    }
    rf_enter_continous_rx();
    rf_workProcess = RF_PRC_RX;
}
void myRadio_setCtrl(controlMode_te mode, uint32_t value)
{
    if (rf_handle == 0)
    {
        return;
    }
    rf_workProcess = RF_PRC_TEST_TX;
    myRadio_init(0, 0);
    myRadio_setSyncWord(0x45);
    switch (mode)
    {
    case RADIO_EXT_CONTROL_TX_UNMODULATED:
    {
        rf_workProcess = RF_PRC_TEST_TX;
        RF_EXT_PA_TO_TX();
        PAN3029_set_carrier_wave_on();
        myRadio_setTxPower(rfTxPower);
        PAN3029_set_carrier_wave_freq(rfFrequence);
    }
        break;
    case RADIO_EXT_CONTROL_TX_MODULATED:
    {
        RF_EXT_PA_TO_TX();
        PAN3029_set_carrier_wave_on();
        myRadio_setTxPower(rfTxPower);
        PAN3029_set_carrier_wave_freq(rfFrequence);
        rf_workProcess = RF_PRC_TEST_TX;
    }
        break;
    case RADIO_EXT_CONTROL_RX_SENSITIVITY:
    {
        RF_EXT_PA_TO_RX();
        myRadio_receiver();
        rf_workProcess = RF_PRC_RX;
    }
        break;
    
    default:
        break;
    }
}

/**-------------------------radio funtion end----------------------------------**/
uint8_t getRfPowerTabIndex(int8_t power)
{
    for (int i = 0; i < sizeof(rfPowerRegTab)/2; i++)
    {
        if (rfPowerRegTab[i].power >= power)
        {
            return i;
        }
    }
    return sizeof(rfPowerRegTab)/2 - 1;
}