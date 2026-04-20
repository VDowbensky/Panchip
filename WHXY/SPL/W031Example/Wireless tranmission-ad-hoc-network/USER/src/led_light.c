/*******************************************************************************      
 *******************************************************************************/
 
#include "cw32w031bios.h"
#include "LED_light.h"

uint16_t led3_value1;
uint16_t led3_value2;
uint16_t led4_value1;
uint16_t led4_value2;

uint16_t  led_show(uint16_t r, uint16_t g,uint16_t a,uint16_t c)
{
				led3_value1=r;
	   led3_value2=a;
				led4_value1=g;
				led4_value2=c;
	   return 0;

}
void led_on(void)
{ 
    GTIM_Cmd(CW_GTIM1, ENABLE);
	   GTIM_Cmd(CW_GTIM2, ENABLE);
}

void led_off(void)
{
    GTIM_Cmd(CW_GTIM1, DISABLE);
	   GTIM_Cmd(CW_GTIM2, DISABLE);
}


