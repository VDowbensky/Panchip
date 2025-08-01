#ifndef __MY_INPUT_CAPTURE_H
#define __MY_INPUT_CAPTURE_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

typedef void (*CaptureCallback)(uint32_t captureValue1, uint32_t captureValue2, uint32_t freq);

void myInputCaptureTIM2_CH2_init(CaptureCallback cb);
void myInputCaptureTIM2_CH3_init(CaptureCallback cb);
void myInputCaptureTIM3_CH4_init(CaptureCallback cb);
#endif

