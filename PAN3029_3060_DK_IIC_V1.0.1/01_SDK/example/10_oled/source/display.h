#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <stdint.h>
#include "oled.h"

extern bool g_UserTxMode; /* 用户 TX 模式 */

void UI_ShowError(char *str);
void UI_SetSF(uint32_t val);
void UI_SetBW(uint32_t val);
void UI_SetPayloadLen(uint32_t val);
void UI_SetTxMode(uint32_t val);
void UI_SetTestMode(uint32_t val);
void UI_SetTRxCnt(uint32_t val);
void UI_SetFreq(uint32_t val);
void UI_SetCR(uint32_t val);
void UI_SetTxPower(uint32_t val);
void UI_SetCWMode(uint32_t val);
void UI_RefreshScreen(void);
void UI_Init(void);

#endif
