#include "led.h"
#include "board.h"

ledParams_ts ledParams = 
{
    .list[0].ledOn = LED1_ON,
    .list[1].ledOn = LED2_ON,
    .list[0].ledOff = LED1_OFF,
    .list[1].ledOff = LED2_OFF,
};

uint16_t beepOnTimeOut;
uint8_t beepFrequence = 1;
//蜂鸣器 IO初始化
void beep_init(void)
{
 
    GPIO_InitTypeDef  GPIO_InitStructure;
 	
    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_BEEP;				
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(BOARD_PORT_BEEP, &GPIO_InitStructure);
			 
    GPIO_WriteBit(BOARD_PORT_BEEP, BOARD_PIN_BEEP, BEEP_OFF);			 
}

void beep_onDriver(void)
{
    static uint8_t freqCount = 0;
    freqCount ++;
    if (freqCount >= beepFrequence)
    {
        freqCount = 0;
        if (beepOnTimeOut)
        {
            GPIO_WriteBit(BOARD_PORT_BEEP, BOARD_PIN_BEEP, 
                        (BitAction)!GPIO_ReadOutputDataBit(BOARD_PORT_BEEP, BOARD_PIN_BEEP));
        }
    }
    
    if (beepOnTimeOut)
    {
        beepOnTimeOut --;
        if (beepOnTimeOut == 0)
        {
            GPIO_WriteBit(BOARD_PORT_BEEP, BOARD_PIN_BEEP, BEEP_OFF);
        }
    }
    for (int i = 0; i < MAX_LED_COUNT; i++)
    {        
        if (ledParams.list[i].count)
        {
            ledParams.list[i].count --;
            if (ledParams.list[i].count == 0)
            {
                if (ledParams.list[i].flashCount)
                {
                    ledParams.list[i].ledOff();
                    ledParams.list[i].flashCount --;
                    if (ledParams.list[i].flashCount == 0)
                    {                    
                        ledParams.sta.unit.led1 &= (~(1 << i));
                        ledParams.list[i].intervalCnting = 0;
                    }
                    else
                    {
                        ledParams.list[i].intervalCnting = ledParams.list[i].intervalSet;
                    }
                }
            }
        }
        else if(ledParams.list[i].intervalCnting)
        {
            ledParams.list[i].intervalCnting --;
            if (ledParams.list[i].intervalCnting == 0)
            {
                ledParams.list[i].count = ledParams.list[i].countSet;
                ledParams.list[i].ledOff();
            }
        }
    }
}
// void BEEP_ON()
// {
//     GPIO_WriteBit(BOARD_GPIO_BEEP, BEEP_ON);
// }
// void BEEP_OFF()
// {
//     GPIO_WriteBit(BOARD_GPIO_BEEP, BEEP_OFF);
// }
void beep_setFreq(uint8_t freq)
{
    beepFrequence = freq;
}
void beep_longBeep(void)
{
    beepOnTimeOut = 200;
}
void beep_shortBeep(void)
{
    beepOnTimeOut = 60;
}
//LED IO初始化
void LED_Init(void)
{
 
    GPIO_InitTypeDef  GPIO_InitStructure;
 	
    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_LED1;				
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(BOARD_PORT_LED1, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_LED2;				
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(BOARD_PORT_LED2, &GPIO_InitStructure);

    GPIO_WriteBit(BOARD_GPIO_LED1, LED_OFF);				 
    GPIO_WriteBit(BOARD_GPIO_LED2, LED_OFF);		 
}
void LED1_ON()
{
    GPIO_WriteBit(BOARD_GPIO_LED1, LED_ON);
}
void LED1_OFF()
{
    GPIO_WriteBit(BOARD_GPIO_LED1, LED_OFF);
}
void LED1_TOGGLE(void)
{
    GPIO_WriteBit(BOARD_GPIO_LED1, (BitAction)!GPIO_ReadOutputDataBit(BOARD_GPIO_LED1));
}
void LED1_ON_ONE(void)
{
    LED1_ON();
    ledParams.sta.unit.led1 = 1;
    ledParams.list[0].flashCount = 1;
    ledParams.list[0].countSet = 5 * 5;
    if(ledParams.list[0].count == 0)ledParams.list[0].count = ledParams.list[0].countSet;
    ledParams.list[0].intervalCnting = 0;
    ledParams.list[0].intervalSet = 0;
}
void LED2_ON()
{
    GPIO_WriteBit(BOARD_GPIO_LED2, LED_ON);
}
void LED2_OFF()
{
    GPIO_WriteBit(BOARD_GPIO_LED2, LED_OFF);
}
void LED2_TOGGLE(void)
{
    GPIO_WriteBit(BOARD_GPIO_LED2, (BitAction)!GPIO_ReadOutputDataBit(BOARD_GPIO_LED2));
}
void LED2_ON_ONE(void)
{
    LED2_ON();
    ledParams.sta.unit.led2 = 1;
    ledParams.list[1].flashCount = 1;
    ledParams.list[1].countSet = 5 * 5;
    if(ledParams.list[1].count == 0)ledParams.list[1].count = ledParams.list[1].countSet;
    ledParams.list[1].intervalCnting = 0;
    ledParams.list[1].intervalSet = 0;
}
void testAllLed(void)
{
    static uint8_t ledSta = 1;

    GPIO_WriteBit(BOARD_GPIO_LED1, LED_OFF);				 
    GPIO_WriteBit(BOARD_GPIO_LED2, LED_OFF);
    switch (ledSta)
    {
    case 1:
        GPIO_WriteBit(BOARD_GPIO_LED1, LED_ON);
        break;
    case 2:
        GPIO_WriteBit(BOARD_GPIO_LED2, LED_ON);
        break;
    
    default:
        break;
    }
    ledSta ++; 
    if (ledSta == 3)
    {
        ledSta = 0;
    }
    
}

