#ifndef __KEY_HANDLE_H__
#define __KEY_HANDLE_H__

#include "hc32_ddl.h"

#define BSP_KEY_ROW_NUM         (6U)

#define BSP_KEY1_PORT         (PortB)
#define BSP_KEY1_PIN          (Pin04)

#define BSP_KEY2_PORT         (PortB)
#define BSP_KEY2_PIN          (Pin05)

#define BSP_KEY3_PORT         (PortB)
#define BSP_KEY3_PIN          (Pin06)

#define BSP_KEY4_PORT         (PortB)
#define BSP_KEY4_PIN          (Pin07)

#define BSP_KEY5_PORT         (PortB)
#define BSP_KEY5_PIN          (Pin08)

#define BSP_KEY6_PORT         (PortB)
#define BSP_KEY6_PIN          (Pin09)

void KEY_SetSfRoutine(void);
void KEY_SetBwRoutine(void);
void KEY_SetPlRoutine(void);
void KEY_SetModeRoutine(void);
void KEY_SetCrRoutine(void);
void KEY_SetTxpowerRoutine(void);
void KEY_ClrCnt(void);
void KEY_StartTxRoutine(void);
void KEY_SetFreqRoutine(void);
void KEY_SetTestMode(void);
void KEY_Scan(void);
void KEY_Process(void);
void RF_Process(void);
void KEY_Init(void);

#endif // ! __KEY_HANDLE_H__
