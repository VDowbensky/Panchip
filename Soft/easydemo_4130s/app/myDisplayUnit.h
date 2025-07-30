#ifndef __MY_DISPLAYUNIT_H
#define __MY_DISPLAYUNIT_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef void (*uiFuntion) (bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr);
typedef void (*enterCallback) (int agr0, int agr1, int agr2, int agr3, void *agr4_ptr);
typedef void (*pageChangeCallback) (int agr0, int agr1, int agr2, int agr3);
typedef enum
{
    UI_PAGE_ID_FIRST_UI,
    UI_PAGE_ID_ITEM_MODE,
    UI_PAGE_ID_TX_PACKET,
    UI_PAGE_ID_RX_PACKET,
    UI_PAGE_ID_RF_CONTINUOUS,
    UI_PAGE_ID_SETTING,
    UI_PAGE_ID_DEVICE_INFOR,
    UI_PAGE_ID_MAX,
}uiPageIndex_te;
typedef enum
{
    MD_ITEM_INDX_TX_PACKET,
    MD_ITEM_INDX_RX_PACKET,
    MD_ITEM_INDX_CONTINUOUS,
    MD_ITEM_INDX_SETTING,
    MD_ITEM_INDX_MAX,
}modeItemIndex_te;
typedef enum
{
    CNT_ITEM_INDEX_TX,
    CNT_ITEM_INDEX_RX,
    CNT_ITEM_INDEX_TX_MD,
    CNT_ITEM_INDEX_MAX_COUNT,
}continuesItemIndex_te;
typedef enum
{
    ENTER_NEXT_PAGE,
    ENTER_LAST_PAGE,
    ENTER_CURRENT_PAGE,
    ENTER_RELEASE_PAGE,
}enter_te;
typedef enum
{
    SET_ITEM_INDEX_TYPE,
    SET_ITEM_INDEX_FREQ,
    SET_ITEM_INDEX_STEP,
    SET_ITEM_INDEX_TXPOWER,
    SET_ITEM_INDEX_RFBAUDRATE,
    SET_ITEM_INDEX_SF,
    SET_ITEM_INDEX_BW,
    SET_ITEM_INDEX_CR,
    SET_ITEM_INDEX_RGM,
    SET_ITEM_INDEX_PACKET_LEN,
    SET_ITEM_INDEX_MAX_COUNT,
}setting_item_index_te;
typedef enum
{
    TYPE_GERNERL_TEXT,
    TYPE_WRITE_TEXT,
    TYPE_NEXT_LINK,
}itemType_te;
typedef struct 
{
    int cursorCount;            //本界面总共可移动光标总数
    int cursorCounting;         //本界面的光标移动计数
    uiFuntion uiDriver;         //本界面显示驱动
    int id;                     //本界面ID
    int lastPageIdTab[SET_ITEM_INDEX_MAX_COUNT];       //返回上一界面超链接，对应界面的ID
    int nextPageIdTab[SET_ITEM_INDEX_MAX_COUNT];       //下一界面超链接，对应界面的ID
    int itemValueTab[SET_ITEM_INDEX_MAX_COUNT];        //该区域写入值缓存
    char itemStringTab[SET_ITEM_INDEX_MAX_COUNT][30];        //该区域写入值缓存
    int itemStepValueTab[SET_ITEM_INDEX_MAX_COUNT];     //该区域加减步进值
    int itemMinValueTab[SET_ITEM_INDEX_MAX_COUNT];     //该区域可写入最小值
    int itemMaxValueTab[SET_ITEM_INDEX_MAX_COUNT];     //该区域可写入最大值
    itemType_te itemTypeTab[SET_ITEM_INDEX_MAX_COUNT]; //=0普通文本，=1可编辑文本，=2下一界面超链接
    uint8_t writeStaTab[SET_ITEM_INDEX_MAX_COUNT];     //可写入操作状态，=1可以通过上下按键加减，同时该区域背景闪烁
    uint8_t writeStatusTab[SET_ITEM_INDEX_MAX_COUNT];  //控制该区域背景颜色
}uiPageUnit_ts;

int8_t myDisplay_getPageId(void);
void myDisplay_enter(uint8_t direct);
void myDisplay_change(uint8_t direct);
void myDisplay_ui_rf_continuos_txCurrent(float Current);
void myDisplay_ui_rf_tx_packet_buffer(uint8_t *buffer);
void myDisplay_ui_rf_tx_packet_counts(uint32_t count);
void myDisplay_ui_rf_tx_packet_rate(float rate);
void myDisplay_ui_rf_tx_packet_consumeTime(uint32_t time);
void myDisplay_ui_rf_tx_packet_ackRssi(int16_t rssi);
void myDisplay_ui_rf_continuos_rxErrorRate(float rate);
void myDisplay_ui_rf_continuos_rxContinuousFreq(uint32_t freq);
void myDisplay_ui_rf_continuos_rxPacket(uint8_t *buf, uint16_t len);
void myDisplay_ui_rf_continuos_rxPacketCount(uint8_t status, uint16_t count);
void myDisplay_ui_rf_continuos_rxPacketGetCount(uint32_t count);
void myDisplay_ui_rf_continuos_rxRssi(int16_t rssi);
void myDisplay_ui_rf_continuos_rfFreq(void);
void myDisplay_ui_rf_continuos_rfBr(void);
void myDisplay_ui_rf_continuos_rfPwr(void);
void myDisplay_ui_rf_rxPacket_buffer(uint8_t *buf, uint16_t len);
void myDisplay_ui_rf_rxPacket_scroll_buffer(uint8_t *buf, uint16_t len);
void myDisplay_ui_rf_rxContinue_scroll_buffer(uint8_t *buf, uint16_t len);
void myDisplay_ui_rf_continuos_rxLen(float rate, uint16_t len);
void myDisplay_ui_rf_rxPacket_count(uint32_t count);
void myDisplay_ui_rf_rxPacket_rate(float rate);
void myDisplay_ui_rf_rxPacket_rssi(int16_t rssi);
void myDisplay_ui_rf_rxPacket_rxCurrent(float current);
void myDisplay_ui_firstUi_setDeviceName(char *name);
void myDisplay_ui_firstUi_setFreq(uint32_t freq);
void myDisplay_ui_firstUi_setRfBr(uint32_t br);
void myDisplay_ui_firstUi_setRfparams(uint8_t sf, char *bw, uint8_t cr);
void myDisplay_ui_firstUi_setRfRgm(char *mode);
void myDisplay_init(enterCallback cb);
void myDisplay_ui_firstUi_setRfPower(int power);
void myDisplay_setSettingParamsProfile(uint8_t index, int value, int min, int max, int step);
void myDisplay_ui_rf_setting_freq(uint32_t freq);
void myDisplay_ui_rf_setting_rfPower(int8_t rfPower);
void myDisplay_ui_rf_setting_type(char *type);
void myDisplay_ui_rf_setting_channelStep(uint32_t channelStep);
void myDisplay_ui_rf_setting_rfBr(uint32_t br);
void myDisplay_setting_loadParams(int index, const char *fmt, ...);
void myDisplay_ui_deviceInfor_setVer(uint8_t ver);
void myDisplay_ui_deviceInfor_setModule(char *moduleName);
void uiTimerFlash_callBack(void);
#endif

