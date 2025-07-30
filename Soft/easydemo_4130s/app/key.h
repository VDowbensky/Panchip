#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

typedef enum{
    KEY_VALUE_NULL,
    KEY_VALUE_KEY1,
    KEY_VALUE_KEY2,
    KEY_VALUE_KEY3,
    KEY_VALUE_KEY4,
    KEY_VALUE_KEY5,
    KEY_VALUE_KEY6,
    KEY_VALUE_KEY7,
    KEY_VALUE_MAX,
}key_value_te;

#define LEFT_KEY KEY_VALUE_KEY2
#define RIGHT_KEY KEY_VALUE_KEY4
#define TOP_KEY KEY_VALUE_KEY1
#define BOTTOM_KEY KEY_VALUE_KEY3
#define OK_KEY KEY_VALUE_KEY5
void key_init(void);//IO≥ı ºªØ
key_value_te keyScan(void);


#endif
