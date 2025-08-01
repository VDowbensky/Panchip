#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "board.h"

								    
//按键初始化函数
void key_init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin  = BOARD_PIN_KEY1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(BOARD_PORT_KEY1, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.GPIO_Pin  = BOARD_PIN_KEY2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(BOARD_PORT_KEY2, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.GPIO_Pin  = BOARD_PIN_KEY3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(BOARD_PORT_KEY3, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.GPIO_Pin  = BOARD_PIN_KEY4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(BOARD_PORT_KEY4, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.GPIO_Pin  = BOARD_PIN_KEY5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(BOARD_PORT_KEY5, &GPIO_InitStructure);//初始化
}

key_value_te keyScan(void)
{	 
	key_value_te keyReturn = KEY_VALUE_NULL;

	if (GPIO_ReadInputDataBit(BOARD_GPIO_KEY1) == 0)
	{
		keyReturn = KEY_VALUE_KEY1;
	}
	else if (GPIO_ReadInputDataBit(BOARD_GPIO_KEY2) == 0)
	{
		keyReturn = KEY_VALUE_KEY2;
	}
	else if (GPIO_ReadInputDataBit(BOARD_GPIO_KEY3) == 0)
	{
		keyReturn = KEY_VALUE_KEY3;
	}
	else if (GPIO_ReadInputDataBit(BOARD_GPIO_KEY4) == 0)
	{
		keyReturn = KEY_VALUE_KEY4;
	}
	else if (GPIO_ReadInputDataBit(BOARD_GPIO_KEY5) == 0)
	{
		keyReturn = KEY_VALUE_KEY5;
	}
		    
 	return keyReturn;// 无按键按下
}
