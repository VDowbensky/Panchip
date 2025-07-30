#include "myDisplayUnit.h"
#include "myLcd.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

int8_t uiPageIdAddress = ~(0);
int8_t uiPageCount = 1;
int8_t rx_tx_count;
int8_t rx_tp_count;
int8_t setting_count;
int8_t tx_tp_count;
uiPageUnit_ts uiPageParams[UI_PAGE_ID_MAX];
enterCallback enterCb;
uint32_t buffer_rfBr;
uint32_t rfBr;
uint32_t buffer_channelStep;
uint32_t buffer_freq;
int8_t buffer_rfPower;
char *buffer_type;

void myDisplay_ui_firstUi_setDeviceName(char *name)
{
    myLCD_str8x16(IM_NOMALE, 80, 1, name);
}
void myDisplay_ui_firstUi_setFreq(uint32_t freq)
{
    myLCD_str8x16(IM_NOMALE, 80, 2, "Freq.%6.2fMHz", (float)freq/1000000);
}
void myDisplay_ui_firstUi_setRfPower(int power)
{
    myLCD_str8x16(IM_NOMALE, 95, 3, "Pwr.%ddBm", power);
}
void myDisplay_ui_firstUi_setRfBr(uint32_t br)
{
    myLCD_str8x16(IM_NOMALE, 95, 4, "Br.%.3fkb", (float)br/1000);
}
void myDisplay_ui_firstUi_setRfparams(uint8_t sf, char *bw, uint8_t cr)
{
    myLCD_str8x16(IM_NOMALE, 95, 4, "s%u %s c%u", sf, bw, cr);
}
void myDisplay_ui_firstUi_setRfRgm(char *mode)
{
    myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 5, "%s", mode);
}
void myDisplay_ui_firstUi(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
    if (uiPageIdAddress != UI_PAGE_ID_FIRST_UI)
    {
        myLCD_16x16(IM_INVERSE, 10,  11, "深圳市沃进科技有限公司");
        myLCD_displayImage(IM_NOMALE, 1, 1, IMG_SRC_VOLLGO);
        myLCD_str8x16(IM_NOMALE, 80, 1, "VG------------");
        myLCD_str8x16(IM_NOMALE, 95, 2, "Freq.***");
        myLCD_str8x16(IM_NOMALE, 95, 3, "Pwr.**");
    }
    else
    {
        myLCD_str8x16(agr0 == 0&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                        HORIZONTAL_RIGHT, 5, "ENTER");
    }
  uiPageIdAddress = UI_PAGE_ID_FIRST_UI;
}
void myDisplay_ui_selectMode(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
  int i;

  myLCD_clearFull();
  for ( i = 0; i <= MAX_PAGE_COUNT; i++)
  {
    myLCD_displayBlock(1, i);
  }
  uiPageIdAddress = UI_PAGE_ID_ITEM_MODE;
  myLCD_str8x16(agr0 == 0&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                10, 1, "Enter RF Transmiter");
  myLCD_str8x16(agr0 == 1&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                10, 2, "Enter RF Receiver");
  myLCD_str8x16(agr0 == 2&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                10, 3, "Enter RF Continuous");
  myLCD_str8x16(agr0 == 3&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                10, 4, "Enter RF Setting");
  myLCD_str8x16(agr0 == 4&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                10, 5, "Enter Device Infor");
}
void myDisplay_ui_rf_tx(uint8_t mode)
{
  int i;

  myLCD_clearFull();
  for ( i = 0; i <= MAX_PAGE_COUNT; i++)
  {
    myLCD_displayBlock(1, i);
  }
  myLCD_str8x16(mode == 0 ? IM_INVERSE : IM_NOMALE, 
                10, 1, "Packet TX");
  myLCD_str8x16(mode == 1 ? IM_INVERSE : IM_NOMALE, 
                10, 2, "Continuous TX");
}
void myDisplay_ui_rf_tx_packet(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
    int i;
    if (uiPageIdAddress != UI_PAGE_ID_TX_PACKET)
    {
        myLCD_clearFull();
        for ( i = 0; i <= MAX_PAGE_COUNT; i++)
        {
            myLCD_displayBlock(1, i);
        }
        myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 1, "Packet TX");
        myLCD_str8x16(IM_NOMALE, 5, 2, "Packet:");
    }
    uiPageIdAddress = UI_PAGE_ID_TX_PACKET;

    if (uiPageIdAddress == UI_PAGE_ID_TX_PACKET)
    {
        myLCD_str8x16(agr0 == 0&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                    HORIZONTAL_RIGHT, 4, "START");
        myLCD_str8x16(agr0 == 1&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                    HORIZONTAL_RIGHT, 5, "TXWA");
        myLCD_str8x16(agr0 == 2&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                    HORIZONTAL_RIGHT, 6, "TXCC");
    }
}
void myDisplay_ui_rf_tx_packet_buffer(uint8_t *buffer)
{
    if (uiPageIdAddress == UI_PAGE_ID_TX_PACKET)
    {
        myLCD_str8x16(IM_NOMALE, 10, 3, (char *)buffer);
    }
}
void myDisplay_ui_rf_tx_packet_counts(uint32_t count)
{
    if (uiPageIdAddress == UI_PAGE_ID_TX_PACKET)
    {
        myLCD_str8x16(IM_NOMALE, HORIZONTAL_RIGHT, 2, "%5d",count);
    }
}
void myDisplay_ui_rf_tx_packet_rate(float rate)
{
    if (uiPageIdAddress == UI_PAGE_ID_TX_PACKET)
    {
        myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 2, "%5.1f%%",(float)rate);
    }
}
void myDisplay_ui_rf_tx_packet_consumeTime(uint32_t time)
{
    if (uiPageIdAddress == UI_PAGE_ID_TX_PACKET)
    {
        if (time == ~(uint32_t)0)
        {
            myLCD_str8x16(IM_NOMALE, HORIZONTAL_RIGHT, 1, "   OUT");
        }
        else
        {
            myLCD_str8x16(IM_NOMALE, HORIZONTAL_RIGHT, 1, "%5dms", time);
        }
        
    }
}
void myDisplay_ui_rf_tx_packet_ackRssi(int16_t rssi)
{
    if (uiPageIdAddress == UI_PAGE_ID_TX_PACKET)
    {
        if (rssi > -150 && (rssi < 10))
        {
            myLCD_str8x16(IM_NOMALE, HORIZONTAL_RIGHT, 3, "%04ddBm", rssi%10000);
        }
    }
}
void myDisplay_ui_rf_setting_freq(uint32_t freq)
{
    buffer_freq = freq;
}
void myDisplay_ui_rf_setting_rfPower(int8_t rfPower)
{
    buffer_rfPower = rfPower;
}
void myDisplay_ui_rf_setting_channelStep(uint32_t channelStep)
{
    buffer_channelStep = channelStep;
}
void myDisplay_ui_rf_setting_type(char *type)
{
    buffer_type = type;
}
void myDisplay_ui_rf_setting_rfBr(uint32_t br)
{
    rfBr = br;
}
char settingDisplayBuffer[SET_ITEM_INDEX_MAX_COUNT][20] = 
{
  "Type:%s",
  "Chl:%s",
  "Step:%s",
  "TxPower:%s",
  "RfBr:%s",
  "SF:%s",
  "BW:%s",
  "CR:%s",
  "RGMode:%s",
  "PkLen:%s",
};
// char settingDisplayBuffer[SET_ITEM_INDEX_MAX_COUNT][20] = 
// {
//   "Type:%s",
//   "Chl:%02d->%6.2fMHz",
//   "Step:%4.2fMHz",
//   "TxPower:%04ddBm",
//   "RfBr:%7.3fkbps",
//   "SF:%d",
//   "BW:%d",
//   "CR:%d",
//   "PkLen:%3d",
// };
char settingCuDisplayBuffer[5][20] = {0};
void myDisplay_setting_loadParams(int index, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsnprintf(uiPageParams[UI_PAGE_ID_SETTING].itemStringTab[index], (int)sizeof(uiPageParams[UI_PAGE_ID_SETTING].itemStringTab[index]), fmt, va);
    va_end(va);
}
void myDisplay_ui_rf_setting(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
  int i;
  int index = agr0;
  static int oldIndex = 255;

  if (uiPageIdAddress != UI_PAGE_ID_SETTING)
  {    
    myLCD_clearFull();
    for ( i = 0; i <= MAX_PAGE_COUNT; i++)
    {
      myLCD_displayBlock(1, i);
    }
    myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 1, "Setting");
    myLCD_str8x16(IM_NOMALE, 10, 2, settingDisplayBuffer[0]);
    myLCD_str8x16(IM_NOMALE, 10, 3, settingDisplayBuffer[1]);
    myLCD_str8x16(IM_NOMALE, 10, 4, settingDisplayBuffer[2]);
    myLCD_str8x16(IM_NOMALE, 10, 5, settingDisplayBuffer[3]);
    myLCD_str8x16(IM_NOMALE, 10, 6, settingDisplayBuffer[4]);
  }
  uiPageIdAddress = UI_PAGE_ID_SETTING;
  if (uiPageIdAddress == UI_PAGE_ID_SETTING)
  {    
    int indexCount = 0;
    //max rows is 5
    if (index >= 5)
    {
      indexCount = index - 4;
      index = 4;
    }
    for (int i = 0; i < 5; i++)
    {
      for (int j = 0; j < SET_ITEM_INDEX_MAX_COUNT; j++)
      {
        if (indexCount == j)
          snprintf(settingCuDisplayBuffer[i], 20, settingDisplayBuffer[indexCount], uiPageParams[UI_PAGE_ID_SETTING].itemStringTab[j]);
      }
      indexCount ++;
    }
    
    if (oldIndex != agr0)
    {
      myLCD_str8x16(IM_NOMALE, 10, 2, "                      ");
      myLCD_str8x16(IM_NOMALE, 10, 3, "                      ");
      myLCD_str8x16(IM_NOMALE, 10, 4, "                      ");
      myLCD_str8x16(IM_NOMALE, 10, 5, "                      ");
      myLCD_str8x16(IM_NOMALE, 10, 6, "                      ");

      myLCD_str8x16(index == 0&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 2, settingCuDisplayBuffer[0]);
      myLCD_str8x16(index == 1&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 3, settingCuDisplayBuffer[1]);
      myLCD_str8x16(index == 2&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 4, settingCuDisplayBuffer[2]);
      myLCD_str8x16(index == 3&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 5, settingCuDisplayBuffer[3]);
      myLCD_str8x16(index == 4&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 6, settingCuDisplayBuffer[4]);
    }
    else
    {
      switch (index)
      {
      case 0:
        myLCD_str8x16(index == 0&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 2, settingCuDisplayBuffer[0]);
        break;
      case 1:
        myLCD_str8x16(index == 1&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 3, settingCuDisplayBuffer[1]);
        break;
      case 2:
        myLCD_str8x16(index == 2&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 4, settingCuDisplayBuffer[2]);
        break;
      case 3:
        myLCD_str8x16(index == 3&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 5, settingCuDisplayBuffer[3]);
        break;
      case 4:
        myLCD_str8x16(index == 4&& flashFlag ? IM_INVERSE : IM_NOMALE, 10, 6, settingCuDisplayBuffer[4]);
        break;
      
      default:
        break;
      }
    }
    oldIndex = agr0;
  }
}
uint8_t ver_buffer;
char *mod_buffer;
void myDisplay_ui_device_infor(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
  int i;

  if (uiPageIdAddress != UI_PAGE_ID_DEVICE_INFOR)
  {    
    myLCD_clearFull();
    for ( i = 0; i <= MAX_PAGE_COUNT; i++)
    {
      myLCD_displayBlock(1, i);
    }
    myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 1, "Infor");
    myLCD_str8x16(IM_NOMALE, 10, 2, "TP: %s", mod_buffer);
    myLCD_str8x16(IM_NOMALE, 
                  10, 3, "VER: V%02x", ver_buffer);
    myLCD_str8x16(IM_NOMALE, 10, 4, "MD: VOLLGO");
  }
  uiPageIdAddress = UI_PAGE_ID_DEVICE_INFOR;
}
void myDisplay_ui_deviceInfor_setVer(uint8_t ver)
{
    ver_buffer = ver;
}
void myDisplay_ui_deviceInfor_setModule(char *moduleName)
{
    mod_buffer = moduleName;
}
void myDisplay_ui_rf_rx(uint8_t mode)
{
  int i;

  myLCD_clearFull();
  for ( i = 0; i <= MAX_PAGE_COUNT; i++)
  {
    myLCD_displayBlock(1, i);
  }
  myLCD_str8x16(mode == 0 ? IM_INVERSE : IM_NOMALE, 
                10, 1, "Packet RX");
  myLCD_str8x16(mode == 1 ? IM_INVERSE : IM_NOMALE, 
                10, 2, "Continuous RX");
}
void myDisplay_ui_rf_continuos(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
  int i;
  if (uiPageIdAddress != UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_clearFull();
    for ( i = 0; i <= MAX_PAGE_COUNT; i++)
    {
      myLCD_displayBlock(1, i);
    }
    myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 1, "Continuous");
    myDisplay_ui_rf_continuos_rfFreq();
    myDisplay_ui_rf_continuos_rfBr();
    myDisplay_ui_rf_continuos_rfPwr();
  }
  uiPageIdAddress = UI_PAGE_ID_RF_CONTINUOUS;
  if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(agr0 == CNT_ITEM_INDEX_TX&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                  HORIZONTAL_CENTER, 2, "TX");
    myLCD_str8x16(agr0 == CNT_ITEM_INDEX_RX&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                  HORIZONTAL_CENTER, 3, "RX");
    myLCD_str8x16(agr0 == CNT_ITEM_INDEX_TX_MD&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                  110+3, 2, "TX_MD");
  }
}
void myDisplay_ui_rf_continuos_txCurrent(float Current)
{
  if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  10, 4, "C=%6.2fmA", Current);
  }
}
void myDisplay_ui_rf_continuos_rxLen(float rate, uint16_t len)
{
  if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  HORIZONTAL_RIGHT, 4, "%5.1f%% %4d", rate, len);
  }
}
void myDisplay_ui_rf_continuos_rfFreq(void)
{
//   if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  5, 2, 
                  "%6.2fMHz", (float)buffer_freq / 1000000);
  }
}
void myDisplay_ui_rf_continuos_rfBr(void)
{
//   if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  HORIZONTAL_RIGHT, 3, 
                  "%7.3fkbps", (float)rfBr / 1000);
  }
}
void myDisplay_ui_rf_continuos_rfPwr(void)
{
//   if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  5, 3, 
                  "%ddBm", buffer_rfPower);
  }
}
void myDisplay_ui_rf_continuos_rxErrorRate(float rate)
{
  if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  10, 5, "rate=%6.2f%%", rate);
  }
}
void myDisplay_ui_rf_continuos_rxRssi(int16_t rssi)
{
  if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  HORIZONTAL_RIGHT, 2, "%4d", rssi);
  }
}
void myDisplay_ui_rf_continuos_rxContinuousFreq(uint32_t freq)
{
  if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
  {
    myLCD_str8x16(IM_NOMALE, 
                  10, 6, "freq=%6d", freq);
  }
}
void myDisplay_ui_rf_continuos_rxPacketCount(uint8_t status, uint16_t count)
{
    if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
    {
        myLCD_str8x16(IM_NOMALE, 
                    10, 5, "count=%d sta=%d", count, status);
    }
}

void myDisplay_ui_rf_continuos_rxPacket(uint8_t *buf, uint16_t len)
{
    int i = 0;
    if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
    {
        myLCD_str8x16(IM_NOMALE, 
                    10, 6, "                        ");
        if (len)
        {
            myLCD_str8x16(IM_NOMALE, 
                        10, 6, "packet=");
            for ( i = 0; i < len; i++)
            {
                myLCD_str8x16(IM_NOMALE, 
                            10, 6 + strlen("packet=") + i,
                             "%x", *buf);
                buf ++;
            }
            
        }
        else
        {
            myLCD_str8x16(IM_NOMALE, 
                        10, 6, "packet=%s", buf);
        }
    }
}
void myDisplay_ui_rf_continuos_rxPacketGetCount(uint32_t count)
{
    if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
    {
        myLCD_str8x16(IM_NOMALE, 
                    10, 6, "rx count=%d", count);
    }
}

void myDisplay_ui_rf_rx_packet(bool flashFlag, int agr0, void *agr1_ptr, void *agr2_ptr)
{
    int i;
    if (uiPageIdAddress != UI_PAGE_ID_RX_PACKET)
    {
        myLCD_clearFull();
        for ( i = 0; i <= MAX_PAGE_COUNT; i++)
        {
            myLCD_displayBlock(1, i);
        }
        myLCD_str8x16(IM_NOMALE, HORIZONTAL_CENTER, 1, "Packet RX");
        myLCD_str8x16(IM_NOMALE, 
                    5, 3, "Packet:");
    }
    uiPageIdAddress = UI_PAGE_ID_RX_PACKET;

    if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
    {
        myLCD_str8x16(agr0 == 0&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                    HORIZONTAL_RIGHT, 5, "START");
        myLCD_str8x16(agr0 == 1&& flashFlag ? IM_INVERSE : IM_NOMALE, 
                    HORIZONTAL_RIGHT, 6, "RXAA");
    }
}
void myDisplay_ui_rf_rxPacket_rxCurrent(float current)
{
  if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
  {
    myLCD_str8x16(IM_NOMALE, 
                  5, 2, "%-6.2fmA", current);
  }
}
void myDisplay_ui_rf_rxPacket_rssi(int16_t rssi)
{
    if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
    {
        if (rssi > -150 && (rssi < 10))
        {
            myLCD_str8x16(IM_NOMALE, 
                        HORIZONTAL_RIGHT, 2, "%04ddBm", rssi%10000);
        }
    }
}
void myDisplay_ui_rf_rxPacket_count(uint32_t count)
{
    if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
    {
        myLCD_str8x16(IM_NOMALE, 
                    HORIZONTAL_CENTER, 3, "%06d", count);
    }
}
void myDisplay_ui_rf_rxPacket_rate(float rate)
{
    if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
    {
        myLCD_str8x16(IM_NOMALE, 
                    HORIZONTAL_RIGHT, 3, "%05.1f%%", rate);
    }
}
void myDisplay_ui_rf_rxPacket_buffer(uint8_t *buf, uint16_t len)
{
    int i = 0;
    if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
    {
        myLCD_str8x16(IM_NOMALE, 
                    10, 5, "                        ");
        if (len)
        {
            myLCD_str8x16(IM_NOMALE, 
                        10, 5, "packet=");
            for ( i = 0; i < len; i++)
            {
                myLCD_str8x16(IM_NOMALE, 
                            10, 5 + strlen("packet=") + i,
                             "%x", *buf);
                buf ++;
            }
            
        }
        else
        {
            myLCD_str8x16(IM_NOMALE, 
                        10, 5, "packet=%-15s", buf);
        }
    }
}
uint8_t displayBuffer[3][20];
void loadDisplayBuffer(uint8_t *buf, uint16_t len)
{
    
    memcpy((char *)displayBuffer[0], (char *)displayBuffer[1], 20);
    memcpy((char *)displayBuffer[1], (char *)displayBuffer[2], 20);
    memset(displayBuffer[2], 0, 20);
    memcpy((char *)displayBuffer[2], (char *)buf, 20);
}
void myDisplay_ui_rf_rxPacket_scroll_buffer(uint8_t *buf, uint16_t len)
{
    int i = 0;
    if (uiPageIdAddress == UI_PAGE_ID_RX_PACKET)
    {

        if (len)
        {
            for ( i = 0; i < len; i++)
            {
                myLCD_str8x16(IM_NOMALE, 
                            10 + i, 5,
                             "%x", *buf);
                buf ++;
            }
            loadDisplayBuffer(buf, len);
        }
        else
        {
        }
        loadDisplayBuffer(buf, len);
        myLCD_str8x16(IM_NOMALE, 18, 4, 
                        "%-15s", displayBuffer[0]);
        myLCD_str8x16(IM_NOMALE, 18, 5, 
                        "%-15s", displayBuffer[1]);
        myLCD_str8x16(IM_NOMALE, 18, 6, 
                        "%-15s", displayBuffer[2]);
    }
}
void loadDisplayBufferContinue(uint8_t *buf, uint16_t len)
{
    
    memcpy((char *)displayBuffer[0], (char *)displayBuffer[1], 20);
    memset(displayBuffer[1], 0, 20);
    memcpy((char *)displayBuffer[1], (char *)buf, 20);
}
void myDisplay_ui_rf_rxContinue_scroll_buffer(uint8_t *buf, uint16_t len)
{
    int i = 0;
    if (uiPageIdAddress == UI_PAGE_ID_RF_CONTINUOUS)
    {
        loadDisplayBufferContinue(buf, len);
        myLCD_str8x16(IM_NOMALE, 18, 5, 
                        "%-15s", displayBuffer[0]);
        myLCD_str8x16(IM_NOMALE, 18, 6, 
                        "%-15s", displayBuffer[1]);
    }
}
/**
 * 光标控制
 * direct: 方向控制
 *          =1，向上移动
 *          =0，向下移动
 * **/
void myDisplay_change(uint8_t direct)
{
  uiPageUnit_ts *page = &uiPageParams[uiPageCount - 1];
    if (page->cursorCount)
    {
        //设置加减控制
      if (page->writeStaTab[page->cursorCounting])
      {
        if (direct)
        {
          page->itemValueTab[page->cursorCounting] += page->itemStepValueTab[page->cursorCounting];
          if (page->itemValueTab[page->cursorCounting] >=
              page->itemMaxValueTab[page->cursorCounting])
          {
            page->itemValueTab[page->cursorCounting] = page->itemMinValueTab[page->cursorCounting];
          }
        }
        else
        {
          page->itemValueTab[page->cursorCounting] -= page->itemStepValueTab[page->cursorCounting];
          if (page->itemValueTab[page->cursorCounting] < page->itemMinValueTab[page->cursorCounting])
          {
            page->itemValueTab[page->cursorCounting] = 
            page->itemMaxValueTab[page->cursorCounting] - 1;
          }
        }
        if (enterCb)
        {
          enterCb(uiPageCount, 
                page->cursorCounting, 
                page->writeStaTab[page->cursorCounting],
                page->itemValueTab[page->cursorCounting],
                page->itemStringTab[page->cursorCounting]
                );
        }
      }
      else
      {
          //光标移动控制
        if (direct)
        {
          page->cursorCounting --;
          if (page->cursorCounting < 0)
          {
            page->cursorCounting = page->cursorCount - 1;
          }
        }
        else
        {
          page->cursorCounting ++;
          if (page->cursorCounting >= page->cursorCount)
          {
            page->cursorCounting = 0;
          }
        }
      }
      
      page->uiDriver(true, page->cursorCounting, 
                                            page->itemValueTab, page->itemStringTab[page->cursorCounting]);
      
    }
}
/***
 * 按下确定键
 *  进入下一个界面或者进入设置状态
*/
void myDisplay_enter(uint8_t direct)
{
  if (direct == ENTER_NEXT_PAGE)
  {
      //进入设置状态
    if (uiPageParams[uiPageCount - 1].itemTypeTab[uiPageParams[uiPageCount - 1].cursorCounting] == TYPE_WRITE_TEXT)
    {
        //切换设置状态
      uiPageParams[uiPageCount - 1].writeStaTab[uiPageParams[uiPageCount - 1].cursorCounting] = 
          !uiPageParams[uiPageCount - 1].writeStaTab[uiPageParams[uiPageCount - 1].cursorCounting];
      uiPageParams[uiPageCount - 1].uiDriver(true, uiPageParams[uiPageCount - 1].cursorCounting, 
                                            uiPageParams[uiPageCount - 1].itemValueTab, uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]);
      if (enterCb)
      {
        enterCb(uiPageParams[uiPageCount - 1].id, 
              uiPageParams[uiPageCount - 1].cursorCounting, 
              uiPageParams[uiPageCount - 1].writeStaTab[uiPageParams[uiPageCount - 1].cursorCounting],
              uiPageParams[uiPageCount - 1].itemValueTab[uiPageParams[uiPageCount - 1].cursorCounting],
              uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]
              );
      }
      
    }
    //切换到下一个界面
    if (uiPageParams[uiPageCount - 1].itemTypeTab[uiPageParams[uiPageCount - 1].cursorCounting] == TYPE_NEXT_LINK)
    {
      uiPageCount = uiPageParams[uiPageCount - 1].nextPageIdTab[uiPageParams[uiPageCount - 1].cursorCounting];
      uiPageParams[uiPageCount - 1].uiDriver(true, uiPageParams[uiPageCount - 1].cursorCounting, 
                                            uiPageParams[uiPageCount - 1].itemValueTab, uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]);
    }
  }
  //切换到上一个界面
  if (direct == ENTER_LAST_PAGE)
  {
    if (uiPageParams[uiPageCount - 1].lastPageIdTab[0])
    {    
      memset(uiPageParams[uiPageCount - 1].writeStaTab, 0, sizeof(uiPageParams[uiPageCount - 1].writeStaTab));
      if (enterCb)
      {
        enterCb(uiPageParams[uiPageCount - 1].id, 
              uiPageParams[uiPageCount - 1].cursorCounting, 
              uiPageParams[uiPageCount - 1].writeStaTab[uiPageParams[uiPageCount - 1].cursorCounting],
              uiPageParams[uiPageCount - 1].itemValueTab[uiPageParams[uiPageCount - 1].cursorCounting],
              uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]
              );
      }
      uiPageCount = uiPageParams[uiPageCount - 1].lastPageIdTab[0];
      uiPageParams[uiPageCount - 1].uiDriver(true, uiPageParams[uiPageCount - 1].cursorCounting, 
                                            uiPageParams[uiPageCount - 1].itemValueTab, uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]);
    }
  }
}
int8_t myDisplay_getPageId(void)
{
  return uiPageIdAddress;
}
void uiTimerFlash_callBack(void)
{
    if (uiPageParams[uiPageCount - 1].writeStaTab[uiPageParams[uiPageCount - 1].cursorCounting])
    {
        uiPageParams[uiPageCount - 1].writeStatusTab[uiPageParams[uiPageCount - 1].cursorCounting] = 
            !uiPageParams[uiPageCount - 1].writeStatusTab[uiPageParams[uiPageCount - 1].cursorCounting];
        if (uiPageParams[uiPageCount - 1].writeStatusTab[uiPageParams[uiPageCount - 1].cursorCounting])
        {
            uiPageParams[uiPageCount - 1].uiDriver(true, uiPageParams[uiPageCount - 1].cursorCounting, 
                                                    uiPageParams[uiPageCount - 1].itemValueTab, uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]);
        }
        else
        {
            uiPageParams[uiPageCount - 1].uiDriver(false, uiPageParams[uiPageCount - 1].cursorCounting, uiPageParams[uiPageCount - 1].itemValueTab, uiPageParams[uiPageCount - 1].itemStringTab[uiPageParams[uiPageCount - 1].cursorCounting]);
        }
    }
}
void myDisplay_init(enterCallback cb)
{
    myLCD_init();

  uiPageParams[UI_PAGE_ID_FIRST_UI].id = UI_PAGE_ID_FIRST_UI + 1;
  uiPageParams[UI_PAGE_ID_FIRST_UI].uiDriver = myDisplay_ui_firstUi;
  uiPageParams[UI_PAGE_ID_FIRST_UI].lastPageIdTab[0] = 0;
  uiPageParams[UI_PAGE_ID_FIRST_UI].nextPageIdTab[0] = 2;
  uiPageParams[UI_PAGE_ID_FIRST_UI].itemTypeTab[0] = TYPE_NEXT_LINK;
  uiPageParams[UI_PAGE_ID_FIRST_UI].writeStaTab[0] = 1;
  uiPageParams[UI_PAGE_ID_FIRST_UI].cursorCounting = 0;
  uiPageParams[UI_PAGE_ID_FIRST_UI].cursorCount = 2;

  uiPageParams[UI_PAGE_ID_ITEM_MODE].id = UI_PAGE_ID_ITEM_MODE + 1;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].uiDriver = myDisplay_ui_selectMode;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].lastPageIdTab[0] = 0;

  uiPageParams[UI_PAGE_ID_ITEM_MODE].nextPageIdTab[0] = UI_PAGE_ID_TX_PACKET + 1;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].itemTypeTab[0] = TYPE_NEXT_LINK;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].nextPageIdTab[1] = UI_PAGE_ID_RX_PACKET + 1;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].itemTypeTab[1] = TYPE_NEXT_LINK;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].nextPageIdTab[2] = UI_PAGE_ID_RF_CONTINUOUS + 1;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].itemTypeTab[2] = TYPE_NEXT_LINK;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].nextPageIdTab[3] = UI_PAGE_ID_SETTING + 1;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].itemTypeTab[3] = TYPE_NEXT_LINK;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].nextPageIdTab[4] = UI_PAGE_ID_DEVICE_INFOR + 1;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].itemTypeTab[4] = TYPE_NEXT_LINK;

  uiPageParams[UI_PAGE_ID_ITEM_MODE].cursorCounting = 0;
  uiPageParams[UI_PAGE_ID_ITEM_MODE].cursorCount = 5;

  uiPageParams[UI_PAGE_ID_TX_PACKET].id = UI_PAGE_ID_TX_PACKET + 1;
  uiPageParams[UI_PAGE_ID_TX_PACKET].uiDriver = myDisplay_ui_rf_tx_packet;
  uiPageParams[UI_PAGE_ID_TX_PACKET].lastPageIdTab[0] = UI_PAGE_ID_ITEM_MODE + 1;
  uiPageParams[UI_PAGE_ID_TX_PACKET].nextPageIdTab[0] = 0;
  uiPageParams[UI_PAGE_ID_TX_PACKET].itemTypeTab[0] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_TX_PACKET].writeStaTab[0] = 0;
  uiPageParams[UI_PAGE_ID_TX_PACKET].itemTypeTab[1] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_TX_PACKET].writeStaTab[1] = 0;
  uiPageParams[UI_PAGE_ID_TX_PACKET].itemTypeTab[2] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_TX_PACKET].writeStaTab[2] = 0;
  uiPageParams[UI_PAGE_ID_TX_PACKET].cursorCounting = 0;
  uiPageParams[UI_PAGE_ID_TX_PACKET].cursorCount = 3;

  uiPageParams[UI_PAGE_ID_DEVICE_INFOR].id = UI_PAGE_ID_DEVICE_INFOR + 1;
  uiPageParams[UI_PAGE_ID_DEVICE_INFOR].uiDriver = myDisplay_ui_device_infor;
  uiPageParams[UI_PAGE_ID_DEVICE_INFOR].lastPageIdTab[0] = UI_PAGE_ID_ITEM_MODE + 1;
  uiPageParams[UI_PAGE_ID_DEVICE_INFOR].nextPageIdTab[0] = 0;
  uiPageParams[UI_PAGE_ID_DEVICE_INFOR].cursorCount = 0;

  uiPageParams[UI_PAGE_ID_RX_PACKET].id = UI_PAGE_ID_RX_PACKET + 1;
  uiPageParams[UI_PAGE_ID_RX_PACKET].uiDriver = myDisplay_ui_rf_rx_packet;
  uiPageParams[UI_PAGE_ID_RX_PACKET].lastPageIdTab[0] = UI_PAGE_ID_ITEM_MODE + 1;
  uiPageParams[UI_PAGE_ID_RX_PACKET].nextPageIdTab[0] = 0;
  uiPageParams[UI_PAGE_ID_RX_PACKET].itemTypeTab[0] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_RX_PACKET].writeStaTab[0] = 0;
  uiPageParams[UI_PAGE_ID_RX_PACKET].itemTypeTab[1] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_RX_PACKET].writeStaTab[1] = 0;
  uiPageParams[UI_PAGE_ID_RX_PACKET].cursorCounting = 0;
  uiPageParams[UI_PAGE_ID_RX_PACKET].cursorCount = 2;

  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].id = UI_PAGE_ID_RF_CONTINUOUS + 1;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].uiDriver = myDisplay_ui_rf_continuos;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].lastPageIdTab[CNT_ITEM_INDEX_TX] = UI_PAGE_ID_ITEM_MODE + 1;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].itemTypeTab[CNT_ITEM_INDEX_TX] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].writeStaTab[CNT_ITEM_INDEX_TX] = 0;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].itemTypeTab[CNT_ITEM_INDEX_RX] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].writeStaTab[CNT_ITEM_INDEX_RX] = 0;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].itemTypeTab[CNT_ITEM_INDEX_TX_MD] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].writeStaTab[CNT_ITEM_INDEX_TX_MD] = 0;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].cursorCounting = 0;
  uiPageParams[UI_PAGE_ID_RF_CONTINUOUS].cursorCount = CNT_ITEM_INDEX_MAX_COUNT;

  uiPageParams[UI_PAGE_ID_SETTING].id = UI_PAGE_ID_SETTING + 1;
  uiPageParams[UI_PAGE_ID_SETTING].uiDriver = myDisplay_ui_rf_setting;
  uiPageParams[UI_PAGE_ID_SETTING].lastPageIdTab[SET_ITEM_INDEX_TYPE] = UI_PAGE_ID_ITEM_MODE + 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_TYPE] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_TYPE] = 4;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_TYPE] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_FREQ] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_FREQ] = 32;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_FREQ] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_STEP] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_STEP] = 200;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_STEP] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_TXPOWER] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_TXPOWER] = -9;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_TXPOWER] = 20+1;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_TXPOWER] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_RFBAUDRATE] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_RFBAUDRATE] = 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_RFBAUDRATE] = 7;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_RFBAUDRATE] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_SF] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_SF] = 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_SF] = 7;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_SF] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_BW] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_BW] = 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_BW] = 7;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_BW] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_CR] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_CR] = 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_CR] = 7;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_CR] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_RGM] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_RGM] = 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_RGM] = 7;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_RGM] = 1;

  uiPageParams[UI_PAGE_ID_SETTING].itemTypeTab[SET_ITEM_INDEX_PACKET_LEN] = TYPE_WRITE_TEXT;
  uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[SET_ITEM_INDEX_PACKET_LEN] = 1;
  uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[SET_ITEM_INDEX_PACKET_LEN] = 256;
  uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[SET_ITEM_INDEX_PACKET_LEN] = 5;

  uiPageParams[UI_PAGE_ID_SETTING].cursorCounting = SET_ITEM_INDEX_TYPE;//默认设置光标在第一个
  uiPageParams[UI_PAGE_ID_SETTING].cursorCount = SET_ITEM_INDEX_MAX_COUNT;

  enterCb = cb;
  uiPageParams[uiPageCount - 1].uiDriver(true, 0, 0, 0);
}
void myDisplay_setSettingParamsProfile(uint8_t index, int value, int min, int max, int step)
{
    uiPageParams[UI_PAGE_ID_SETTING].itemValueTab[index] = value;
    uiPageParams[UI_PAGE_ID_SETTING].itemMinValueTab[index] = min;
    uiPageParams[UI_PAGE_ID_SETTING].itemMaxValueTab[index] = max + 1;
    uiPageParams[UI_PAGE_ID_SETTING].itemStepValueTab[index] = step;
}