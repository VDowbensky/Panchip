#include "main.h"
#include "ReadKey.h"
#include "key.h"
#include "ReadKey.h"
#include "crc8.h"
#include "led.h"
#include "eventUnit.h"
#include "myADC.h"
#include "myInputCapture.h"
#include "myLcd.h"
#include "myDisplayUnit.h"
#include "myFlashData.h"
#include "myTim.h"
#include "myUart.h"
#include "myUart3.h"
#include "myRadio.h"

#define SOFT_VERSION 0x13
#define SET_RF_FREQ_HZ(base, ch,step) base+ch*step*10*1000
//---------------key
KeyParamExt_ts *getKeyReturn;
key_value_te keyPressValue;


static uint16_t present_adcValue;
static uartPacket_ts uart3Packet;
static uartPacket_ts uartPackage_Rx;

static bool startToCountingRx = false;
static float present_moduleCurrendValue;
static float validPackageCount = 0;
static uint32_t rfContinuousFreq = 1;
static float rfRxTestRate = 1;

static uint8_t packetTxMode;
static uint8_t packetRxMode;

static rfRxPacket_ts rfRecvPacket;
static rfTxPacket_ts rfTxPacket;
static uint32_t rfTxCount = 0;
static uint32_t rfRxCount = 0;
static uint32_t rfTxAndGetAckTime_ms = 0;
static uint32_t rfTxAndGetAckTimeSet_ms = 1000;
static uint32_t rfTxReTmCount = 0;
static bool rfTxGetAckStatus = false;
static uint8_t rfCtrlMode;
const uint32_t rfBaseFreqList[DVTP_MAX_COUNT] = 
{
    /*"1"*/410000000,
    /*"2"*/490000000,
    /*"3"*/850000000,
    /*"4"*/910000000,

};
const uint32_t rfBaudrateList[MAX_RF_BAUDRATE_COUNT] = 
{
    244, 627, 1220, 2441, 5022, 12500, 20400, 62500
};
const int8_t rfTxPowerList[RF_TX_PWR_MAX_COUNT] = 
{
    -26,-17,-5,-3,-2,-1,2,4,5,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
};
static char deviceNameList[DVTP_MAX_COUNT][20] = 
{
    /*"1"*/"VG4130S433N0S1",
    /*"2"*/"VG4130S490N0S1",
    /*"3"*/"VG4130S868N0S1",
    /*"4"*/"VG4130S915N0S1",
};
static char rf_cr_name[][20] = 
{
    /*"0"*/"4/5",
    /*"1"*/"4/6",
    /*"2"*/"4/7",
    /*"3"*/"4/8",
};
static char rf_bw_name[][20] = 
{
    /*"0"*/"62.5k",
    /*"1"*/"125k",
    /*"2"*/"250k",
    /*"3"*/"500k",
};
static char rf_rgm_name[][20] = 
{
    /*"0"*/"LDO",
    /*"1"*/"DCDC",
};
userParams_ts deviceInforDef = 
{
    .projectModel = "VG4130",
    .deviceId = 1,
    .rfChannel = 30,
    .channelStep = 100,
    .txPower = RF_TX_PWR_MAX_COUNT - 1,
    .rfBaudrate = RF_BAUDRATE_1220,
    .chipType = DVTP_VG4130S433N0S1,
    .packetLen = 12,
    .sendPacketCounts = 100,
    .rf_sf = 9,
    .rf_bw = 7,
    .rf_cr = 3,
    .rf_regulatorMode = 0x01,
};
userParams_ts deviceInfor;
#define EVENT_TIME_CYCLE_10ms         0
#define EVENT_TIME_CYCLE_500ms      1
#define EVENT_UART3_RECV     2
#define EVENT_UART_RECV         3
// #define EVENT_TEST_RX_TIMEOUT       4
#define EVENT_RF_CONTINUOUS_TX       5
#define EVENT_RF_CONTINUOUS_RX       6
#define EVENT_RF_IDLE       7
#define EVENT_RF_CONTINUOUS_TX_MD       8
#define EVENT_TIMEOUT_TO_SAVE_PARAMS       9
#define EVENT_RF_GET_RX_PACKET       10
#define EVENT_RF_PACKET_TX       11
#define EVENT_RF_PACKET_RX       12
#define EVENT_TIMEOUT_CHECK_RF_PACKET       13
#define EVENT_RF_RX_ERROR       14
static uint16_t eventReturn;

void dealKeyPressProccess(void)
{
    if (getKeyReturn->haveKey == false)
    {
        return;
    }
    getKeyReturn->haveKey = false;

    switch (getKeyReturn->value)
    {
    case LEFT_KEY:
    {
        if(getLongKeySt() == true)
        {
            clearLongKey(); 

            EnableReleaseKey();
            myDisplay_enter(ENTER_LAST_PAGE);
        }
        else
        {
            if (getReleaseKeySt())
            {

            }
            else
            {
                EnableLongKey(5);   
            }
        }
    }
        break;
    case RIGHT_KEY:
    {
        if(getLongKeySt() == true)
        {
            clearLongKey(); 

            EnableReleaseKey();
        }
        else
        {
            if (getReleaseKeySt())
            {

            }
            else
            {
                EnableLongKey(5);   
            }
        }
    }
        break;
    case TOP_KEY:
    {
        if(getCyclicKeySt() == true)
        {
            EnableCyclicKey(30); 
            myDisplay_change(1);
        }
        else
        {
            if (getReleaseKeySt())
            {
                beep_shortBeep();
                myDisplay_change(1);
            }
            else
            {
                EnableReleaseKey();
                EnableCyclicKey(300);   
            }
        }
    }
        break;
    case BOTTOM_KEY:
    {
        if(getCyclicKeySt() == true)
        {
            EnableCyclicKey(30); 
            myDisplay_change(0);
        }
        else
        {
            if (getReleaseKeySt())
            {
                beep_shortBeep();
                myDisplay_change(0);
            }
            else
            {
                EnableReleaseKey();
                EnableCyclicKey(300);   
            }
        }
    }
        break;
    case OK_KEY:
    {
        if(getLongKeySt() == true)
        {
            clearLongKey(); 

            EnableReleaseKey();
            beep_shortBeep();
            myDisplay_enter(ENTER_NEXT_PAGE);
        }
        else
        {
            if (getReleaseKeySt())
            {

            }
            else
            {
                EnableLongKey(5);   
            }
        }
    }
        break;
    default:
        break;
    }
}

/**
 * 
 * 串口回调函数，当串口有硬件超时时会调用该函数
*/
static void rcc_init(void)
{
    //---------普通IO口时钟使能
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE ); 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE ); 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE ); 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD, ENABLE ); 

    //----------SPI1时钟使能
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE );  
    //----------复用功能时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    #if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
    /* ADCCLK = PCLK2/2 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);
    #else
    /* ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
    #endif
}
/**
 * 
 * 串口回调函数，当串口有硬件超时时会调用该函数
*/
void UART1_CALLBACK(uint8_t *buf, uint16_t len)
{
    if(uartPackage_Rx.isValid == 0)
    {
        memcpy(uartPackage_Rx.packet, buf, len);
        uartPackage_Rx.len = len;
        uartPackage_Rx.isValid = 1;
        event_post(EVENT_UART_RECV);
    }
}
void UART3_CALLBACK(uint8_t *buf, uint16_t len)
{
    if(uart3Packet.isValid == 0)
    {
        memcpy(uart3Packet.packet, buf, len);
        uart3Packet.len = len;
        uart3Packet.isValid = true;
        event_post(EVENT_UART3_RECV);
    }
}
/**
 * 
 * 定时器中断回调，当产生定时器中断会调用该函数
*/
void TIM3_CALLBACK(void)
{
    static uint8_t timeCnt_1ms = 0;

    beep_onDriver();
    if( ++timeCnt_1ms == 5)
    {
        timeCnt_1ms = 0;
        rfTxAndGetAckTime_ms ++;
        eventDriver();
    }
}

void uiEnterCallback(int pageId, int cursorCount, int status, int value)
{
    switch (pageId - 1)
    {
    case UI_PAGE_ID_ITEM_MODE:
    {

    }break;
    case UI_PAGE_ID_RF_CONTINUOUS:
    {
        switch (cursorCount)
        {
        case CNT_ITEM_INDEX_TX:
        {
            // event_post(status ? EVENT_RF_CONTINUOUS_TX : EVENT_RF_CONTINUOUS_RX);
            event_post(EVENT_RF_CONTINUOUS_TX);
            rfCtrlMode = status ? UI_PAGE_ID_RF_CONTINUOUS : 0;
        }
            break;
        case CNT_ITEM_INDEX_RX:
        {
            event_post(status ? EVENT_RF_CONTINUOUS_RX : EVENT_RF_IDLE);
        }
            break;
        case CNT_ITEM_INDEX_TX_MD:
        {
            event_post(status ? EVENT_RF_CONTINUOUS_TX_MD : EVENT_RF_IDLE);
        }
            break;
        default:
            break;
        }
    }
        break;
    case UI_PAGE_ID_TX_PACKET:
    {
        switch (cursorCount)
        {
        case 0://signle pcaket tx
        case 1:// pcaket tx and wait ack
        case 2://packet tx unitl max count
        {
            setEvent(status ? EVENT_RF_PACKET_TX : EVENT_RF_IDLE, 
                    status ? true : false, 
                    status ? 500 : 0);
            rfCtrlMode = status ? UI_PAGE_ID_TX_PACKET : 0;
            rfTxCount = 0;
            rfRxCount = 0;
            packetTxMode = cursorCount;
        }
            break;
        default:
            break;
        }
    }
        break;
    case UI_PAGE_ID_RX_PACKET:
    {
        switch (cursorCount)
        {
        case 0:
        case 1:
        {
            event_post(status ? EVENT_RF_PACKET_RX : EVENT_RF_IDLE);
            rfCtrlMode = status ? UI_PAGE_ID_RX_PACKET : 0;
            rfTxCount = 0;
            rfRxCount = 0;
            packetRxMode = cursorCount;
        }
            break;
        default:
            break;
        }
    }
        break;
    case UI_PAGE_ID_SETTING:
    {
        switch (cursorCount)
        {
        case SET_ITEM_INDEX_TYPE://chipType
        {
            deviceInfor.chipType = value;
            myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_TXPOWER, 
                                            deviceInfor.txPower, 
                                            RF_TX_PWR_N_26, RF_TX_PWR_MAX_COUNT - 1, 1);
            myRadio_setTxPower(rfTxPowerList[deviceInfor.txPower]);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_TXPOWER, "%ddBm", rfTxPowerList[deviceInfor.txPower]);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_TYPE, deviceNameList[deviceInfor.chipType]);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_FREQ, "%03u->%6.2fMHz", deviceInfor.rfChannel, (float)(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep)) / 1000000.0);
            myRadio_setFrequency(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_FREQ://Freq
        {
            deviceInfor.rfChannel = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_FREQ, "%03u->%6.2fMHz", deviceInfor.rfChannel, (float)(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep)) / 1000000.0);
            myRadio_setFrequency(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_STEP://channelStep
        {
            deviceInfor.channelStep = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_STEP, "%uHz", deviceInfor.channelStep*10*1000);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_FREQ, "%03u->%6.2fMHz", deviceInfor.rfChannel, (float)(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep)) / 1000000.0);
            myRadio_setFrequency(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_TXPOWER://TxPower
        {
            deviceInfor.txPower = value;

            myRadio_setTxPower(rfTxPowerList[deviceInfor.txPower]);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_TXPOWER, "%ddBm", rfTxPowerList[deviceInfor.txPower]);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
        break;
        case SET_ITEM_INDEX_RFBAUDRATE://RFBAUDRATE
        {
            deviceInfor.rfBaudrate = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_RFBAUDRATE, "%ubps", rfBaudrateList[deviceInfor.rfBaudrate]);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
            deviceInfor.rf_sf = loraBaudrateFrame[deviceInfor.rfBaudrate].SpreadingFactor;
            deviceInfor.rf_bw = loraBaudrateFrame[deviceInfor.rfBaudrate].SignalBw;
            deviceInfor.rf_cr = loraBaudrateFrame[deviceInfor.rfBaudrate].ErrorCoding;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_SF, "%u", deviceInfor.rf_sf);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_BW, "%u->%s", deviceInfor.rf_bw, rf_bw_name[deviceInfor.rf_bw - 6]);
            myDisplay_setting_loadParams(SET_ITEM_INDEX_CR, "%u->%s", deviceInfor.rf_cr, rf_cr_name[deviceInfor.rf_cr - 1]);
        }
            break;
        case SET_ITEM_INDEX_PACKET_LEN://
        {
            deviceInfor.packetLen = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_PACKET_LEN, "%u Byte", deviceInfor.packetLen);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_SF://
        {
            deviceInfor.rf_sf = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_SF, "%u", deviceInfor.rf_sf);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_BW://
        {
            deviceInfor.rf_bw = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_BW, "%u->%s", deviceInfor.rf_bw, rf_bw_name[deviceInfor.rf_bw - 6]);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_CR://
        {
            deviceInfor.rf_cr = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_CR, "%u->%s", deviceInfor.rf_cr, rf_cr_name[deviceInfor.rf_cr - 1]);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        case SET_ITEM_INDEX_RGM://
        {
            deviceInfor.rf_regulatorMode = value;
            myDisplay_setting_loadParams(SET_ITEM_INDEX_RGM, "%u->%s", deviceInfor.rf_regulatorMode, rf_rgm_name[deviceInfor.rf_regulatorMode]);
            setEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS, false, 200);
        }
            break;
        default:
            break;
        }
    }
        break;

    default:
        break;
    }
}
void rfRx_callback(uint8_t status, rfRxPacket_ts packet)
{
    
    
    switch (status)
    {
        case RX_STA_SECCESS:
        {
            rfRecvPacket = packet;
            myRadio_receiver();
            event_post(EVENT_RF_GET_RX_PACKET);
        }
        break;
        case RX_STA_TIMEOUT:
        {
            event_post(EVENT_RF_RX_ERROR);
        }
        break;
        case RX_STA_PAYLOAD_ERROR:
        {
            event_post(EVENT_RF_RX_ERROR);
        }
        break;
        case TX_STA_SECCESS:
        {
            LED1_ON_ONE();
            if (rfCtrlMode == UI_PAGE_ID_TX_PACKET)
            {
                switch (packetTxMode)
                {
                case 0://signle pcaket tx
                {
                    setEvent( EVENT_RF_PACKET_TX, false, 50);
                }break;
                case 1:// pcaket tx and wait ack
                {
                    setEvent( EVENT_RF_PACKET_TX, false, (uint32_t)((float)(rfTxAndGetAckTime_ms>100?rfTxAndGetAckTime_ms:100) * 1.5));
                    myRadio_receiver();
                }break;
                case 2://packet tx unitl max count
                {
                    if (rfTxCount < deviceInfor.sendPacketCounts)
                    {
                        setEvent( EVENT_RF_PACKET_TX, false, 50);
                    }
                    else
                    {
                        event_clear(EVENT_RF_PACKET_TX);
                    }
                    
                }
                    break;
                default:
                    break;
                }
                myDisplay_ui_rf_tx_packet_consumeTime(rfTxAndGetAckTime_ms);
            }
            else
            {
                myRadio_receiver();
            }
        }
        break;
        default:
            break;
    }
}
int main(void)
{
    userParams_ts userParamsTemp;
#ifdef BOOTLOADER_APP
	SCB->VTOR = FLASH_BASE | 0x000C800;
#endif
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    rcc_init();
        
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag , 开启swd
    //读取本地保存数据
    myFlash_read((uint8_t*)&deviceInfor, sizeof(userParams_ts));
    if (crc8_gernCheckT((unsigned char*)&deviceInfor, 
                        sizeof(userParams_ts) - 1, 
                        deviceInfor.checkSum) == 0)
    {
        deviceInfor = deviceInforDef;
    }
    if (memcmp(deviceInfor.projectModel, deviceInforDef.projectModel, strlen(deviceInforDef.projectModel)) != 0)
    {
        deviceInfor = deviceInforDef;
    }
    //初始化按键
    key_init();

    //初始化LED灯
    LED_Init();

    //初始化串口
    // myUart3_init(115200, UART3_CALLBACK);    //用于透传模块测试，需要时再打开
    myUart1_init(115200, UART1_CALLBACK);

    //初始化定时器
    myTim1_init(200, TIM3_CALLBACK);

    //初始化模拟转换ADC，用于无线模块驱动电流检测
    myADC_init();

    //蜂鸣器初始化
    beep_init();
    beep_setFreq(deviceInfor.beepNumb);

    //初始化射频
    myRadio_setChipType(deviceInfor.chipType);
    RadioSetregulatorMode(deviceInfor.rf_regulatorMode);
    myRadio_init(0, rfRx_callback);
    myRadio_setFrequency(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
    myRadio_setTxPower(rfTxPowerList[deviceInfor.txPower]);
    myRadio_setRfParams(deviceInfor.rf_sf, deviceInfor.rf_bw, deviceInfor.rf_cr);
    myRadio_setSyncWord(0x45);
    myRadio_receiver();
    // LCD显示屏初始化界面显示
    myDisplay_init(uiEnterCallback);
    myDisplay_ui_firstUi_setDeviceName(deviceNameList[deviceInfor.chipType]);
    myDisplay_ui_firstUi_setFreq(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
    myDisplay_ui_firstUi_setRfPower(rfTxPowerList[deviceInfor.txPower]);
    myDisplay_ui_firstUi_setRfparams(deviceInfor.rf_sf, rf_bw_name[deviceInfor.rf_bw - 6], deviceInfor.rf_cr);
    myDisplay_ui_firstUi_setRfRgm(rf_rgm_name[deviceInfor.rf_regulatorMode]);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_TYPE, deviceInfor.chipType, 0, DVTP_MAX_COUNT - 1, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_FREQ, deviceInfor.rfChannel, 0, 255, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_STEP, deviceInfor.channelStep, 25, 200, 1);

    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_TXPOWER, 
                                    deviceInfor.txPower, 
                                    RF_TX_PWR_N_26, RF_TX_PWR_MAX_COUNT - 1, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_RFBAUDRATE, deviceInfor.rfBaudrate, RF_BAUDRATE_RESV1, MAX_RF_BAUDRATE_COUNT - 1, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_PACKET_LEN, deviceInfor.packetLen, 1, 255, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_SF, deviceInfor.rf_sf, 5, 12, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_BW, deviceInfor.rf_bw, 6, 9, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_CR, deviceInfor.rf_cr, 1, 4, 1);
    myDisplay_setSettingParamsProfile(SET_ITEM_INDEX_RGM, deviceInfor.rf_regulatorMode, 0, 1, 1);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_FREQ, "%03u->%6.2fMHz", deviceInfor.rfChannel, (float)(SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep)) / 1000000.0);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_STEP, "%uHz", deviceInfor.channelStep*10*1000);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_TYPE, deviceNameList[deviceInfor.chipType]);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_RFBAUDRATE, "%ubps", rfBaudrateList[deviceInfor.rfBaudrate]);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_TXPOWER, "%ddBm", rfTxPowerList[deviceInfor.txPower]);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_SF, "%u", deviceInfor.rf_sf);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_BW, "%u->%s", deviceInfor.rf_bw, rf_bw_name[deviceInfor.rf_bw - 6]);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_CR, "%u->%s", deviceInfor.rf_cr, rf_cr_name[deviceInfor.rf_cr - 1]);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_RGM, "%u->%s", deviceInfor.rf_regulatorMode, rf_rgm_name[deviceInfor.rf_regulatorMode]);
    myDisplay_setting_loadParams(SET_ITEM_INDEX_PACKET_LEN, "%u Byte", deviceInfor.packetLen);
    myDisplay_ui_deviceInfor_setVer(SOFT_VERSION);
    myDisplay_ui_deviceInfor_setModule("VGKitBoard_4130S");
    //上电长想一声
    beep_longBeep();

    setEvent(EVENT_TIME_CYCLE_10ms, true, 10);
    setEvent(EVENT_TIME_CYCLE_500ms, true, 500);
    while(1)
    {   
        eventReturn = event_pend();

        if (getEvent(EVENT_TIME_CYCLE_10ms))
        {
            getKeyReturn = KeyValueChange(keyPressValue);
            dealKeyPressProccess();
        }
        if (getEvent(EVENT_TIME_CYCLE_500ms))
        {
            present_moduleCurrendValue = myADC_getVoltageValue()/50/0.5*1000;
            myDisplay_ui_rf_continuos_txCurrent(present_moduleCurrendValue);
            myDisplay_ui_rf_rxPacket_rxCurrent(present_moduleCurrendValue);
            myDisplay_ui_rf_continuos_rxRssi(myRadio_getRssi());
            uiTimerFlash_callBack();
        }
        if (getEvent(EVENT_RF_CONTINUOUS_RX))
        {
            myRadio_setCtrl(RADIO_EXT_CONTROL_RX_SENSITIVITY, 
                        SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
            startToCountingRx = true;
            validPackageCount = 0;
        }
        if (getEvent(EVENT_RF_CONTINUOUS_TX))
        {
            myRadio_setCtrl(RADIO_EXT_CONTROL_TX_UNMODULATED, 
                        SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
        }
        if (getEvent(EVENT_RF_CONTINUOUS_TX_MD))
        {
            myRadio_setCtrl(RADIO_EXT_CONTROL_TX_MODULATED, 
                        SET_RF_FREQ_HZ(rfBaseFreqList[deviceInfor.chipType], deviceInfor.rfChannel, deviceInfor.channelStep));
        }
        if (getEvent(EVENT_RF_PACKET_TX))
        {
            rfTxCount ++;
            event_clear(EVENT_RF_PACKET_TX);
            memset(rfTxPacket.payload, 0, sizeof(rfTxPacket.payload));
            sprintf(rfTxPacket.payload, "%05u-hello...", rfTxCount);
            rfTxPacket.len = strlen(rfTxPacket.payload);
            memset(&rfTxPacket.payload[rfTxPacket.len + 1], 0x77, sizeof(rfTxPacket.payload) - rfTxPacket.len - 1);
            rfTxPacket.len = deviceInfor.packetLen;
            myDisplay_ui_rf_tx_packet_buffer(rfTxPacket.payload);
            myDisplay_ui_rf_tx_packet_counts(deviceInfor.packetLen);
            rfTxAndGetAckTime_ms = 0;
            myRadio_transmit(&rfTxPacket);
            event_clear(EVENT_TIMEOUT_CHECK_RF_PACKET);
            printf("rfTxPacket.absTime = %u\n", rfTxPacket.absTime);
            if (rfCtrlMode == UI_PAGE_ID_TX_PACKET)
            {  
                switch (packetTxMode)
                {
                case 0://signle pcaket tx
                case 2://packet tx unitl max count
                {
                    setEvent(EVENT_RF_PACKET_TX, false, (rfTxPacket.absTime + 1000));
                }break;
                case 1:// pcaket tx and wait ack
                {
                        setEvent(EVENT_RF_PACKET_TX, false, (rfTxPacket.absTime + 1000));
                    if (rfTxCount > deviceInfor.sendPacketCounts)
                    {
                        myDisplay_ui_rf_tx_packet_rate((float)rfRxCount/rfTxCount * 100);
                        rfTxCount = 0;
                        rfRxCount = 0;
                    }
                    else
                    {
                    }
                    
                }break;
                default:
                    break;
                }
            }
        }
        if (getEvent(EVENT_RF_PACKET_RX))
        {
            myRadio_receiver();
            validPackageCount  = 0;
        }
        if (getEvent(EVENT_RF_GET_RX_PACKET))
        {
            rfRxCount ++;
            myDisplay_ui_rf_continuos_rxLen(0, validPackageCount);
            if (rfCtrlMode == UI_PAGE_ID_TX_PACKET)
            {   
                switch (packetTxMode)
                {
                case 0://signle pcaket tx
                {
                }break;
                case 1:// pcaket tx and wait ack
                {
                    event_post(EVENT_RF_PACKET_TX);
                    myDisplay_ui_rf_tx_packet_ackRssi(rfRecvPacket.rssi);
                }break;
                case 2://packet tx unitl max count
                {
                }
                    break;
                default:
                    break;
                }
            }else if (rfCtrlMode == UI_PAGE_ID_RX_PACKET)
            {   
                switch (packetRxMode)
                {
                case 0:
                {
                }break;
                case 1:
                {
                    rfTxCount ++;
                    memset(rfTxPacket.payload, 0, sizeof(rfTxPacket.payload));
                    sprintf(rfTxPacket.payload, "%05u-hello...", rfTxCount);
                    rfTxPacket.len = strlen(rfTxPacket.payload);
                    memset(&rfTxPacket.payload[rfTxPacket.len + 1], 0x77, sizeof(rfTxPacket.payload) - rfTxPacket.len - 1);
                    rfTxPacket.len = deviceInfor.packetLen;
                    myRadio_transmit(&rfTxPacket);
                }break;
                default:
                    break;
                }
                myDisplay_ui_rf_rxPacket_rssi(rfRecvPacket.rssi);
                myDisplay_ui_rf_rxPacket_count(rfRxCount);
                if (atoi(rfRecvPacket.payload) == 1)
                {
                    rfRxCount = 1;
                }
                
                float rxRate = (float)rfRxCount / atoi(rfRecvPacket.payload) * 100;
                myDisplay_ui_rf_rxPacket_rate(rxRate);
                myDisplay_ui_rf_rxPacket_scroll_buffer(rfRecvPacket.payload, 0);
                myDisplay_ui_rf_rxContinue_scroll_buffer(rfRecvPacket.payload, 0);
            }
            else if (rfCtrlMode == UI_PAGE_ID_FIRST_UI)
            {   
                setEvent( EVENT_RF_PACKET_TX, false, 0);
            }
            myUart1_sendArray(rfRecvPacket.payload, rfRecvPacket.len);
            memset(rfRecvPacket.payload, 0, sizeof(rfRecvPacket.payload));
            LED2_ON_ONE();
        }
        if (getEvent(EVENT_RF_RX_ERROR))
        {
            myRadio_receiver();
        }
        if (getEvent(EVENT_RF_IDLE))
        {
            startToCountingRx = false;
            myRadio_abort();
            event_clear(EVENT_RF_PACKET_TX);
        }
        if (getEvent(EVENT_UART_RECV))
        {
            if (uartPackage_Rx.isValid)
            {
                uartPackage_Rx.isValid = false;
                myDisplay_ui_rf_rxPacket_buffer(uartPackage_Rx.packet, 10);
                rfTxPacket.len = uartPackage_Rx.len;
                memcpy(rfTxPacket.payload, uartPackage_Rx.packet, rfTxPacket.len);
                myRadio_transmit(&rfTxPacket);
                uartPackage_Rx.len = 0;
            }
        }
        if (getEvent(EVENT_UART3_RECV))
        {
            if (uart3Packet.isValid)
            {
                uart3Packet.isValid = false;
                myDisplay_ui_rf_rxPacket_buffer(uart3Packet.packet, 10);
                rfTxPacket.len = uart3Packet.len;
                memcpy(rfTxPacket.payload, uart3Packet.packet, rfTxPacket.len);
                myRadio_transmit(&rfTxPacket);
                uart3Packet.len = 0;
            }
        }
        if (getEvent(EVENT_TIMEOUT_TO_SAVE_PARAMS))
        {
            deviceInfor.checkSum = crc8_ger((unsigned char*)&deviceInfor, sizeof(userParams_ts) - 1);
            myFlash_write((uint8_t*)&deviceInfor, sizeof(userParams_ts));
        }
        
        keyPressValue = keyScan();
        myRadio_process();
        
    }     
} 

