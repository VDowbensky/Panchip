

#include <stdint.h>

//#define LED_PERIOD          255

#define LED3_LED4_QKLIGHT()        led_show(500, 500,500,500)
#define LED3_LED4_ONLIGHT()        led_show(0, 500,0,500)
#define LED3_LED4_LIGHTON()        led_show(50, 0,50,0)
#define LED3_LED4_SLOWQK()         led_show(50, 500,50,500)
#define LED3_LED4_OFFON()          led_show(2000, 0,0,0)
#define LED3_LED4_OFF()            led_show(2000, 2000,0,0)
#define LED3_LED4_SLOWLIGHT()      led_show(50, 50,50,50)
#define LED3_LED4_ON()             led_show(0,0,0,0)
uint16_t  led_show(uint16_t r, uint16_t g,uint16_t a,uint16_t c);
void led_on(void);
void led_off(void);

