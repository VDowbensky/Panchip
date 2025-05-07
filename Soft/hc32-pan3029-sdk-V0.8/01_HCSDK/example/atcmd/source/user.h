#ifndef _USER_H
#define _USER_H



#include "userdef.h"


/* Micro Definition */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define START_TASK_STK_SIZE           128
#define START_TASK_PRIO               3

#define PT_TASK_STK_SIZE              128
#define PT_TASK_PRIO                  1

#define AT_TASK_STK_SIZE              256
#define AT_TASK_PRIO                  2

/* Enum */
enum
{
    UART_IDLE = 0,
    UART_RECEIVING,
    UART_SUCCESS,
    UART_TRANSMITTING,
};

typedef struct
{
    u32 freq;
    u16 cr;
    u16 bw;
    u16 sf;
    u16 pwr;
    u16 crc;
    u16 ldr;
    u16 modemMode;
    u16 preamble;
    u16 dcdc;
}rfConfig_t;

/* Variable Declaration */
extern rfConfig_t gRfConfig;

/* Function Declaration */
extern void rf_SetUserPara(rfConfig_t *p);
extern int MOD_SettingStore(void);
extern int MOD_SettingRestore(void);
extern int MOD_SettingDelete(void);
extern void initTask(void *pvParameters);

#endif

