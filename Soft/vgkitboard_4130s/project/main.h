#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#pragma pack(1) 
typedef struct 
{
    char projectModel[20];
    uint32_t deviceId;
    uint8_t rfChannel;
    uint8_t channelStep;
    int8_t txPower;
    uint8_t rfBaudrate;
    uint8_t rf_sf;
    uint8_t rf_bw;
    uint8_t rf_cr;
    uint8_t rf_regulatorMode;
    uint8_t chipType;
    uint8_t beepNumb;
    uint8_t packetLen;
    uint32_t sendPacketCounts;
    uint8_t checkSum;
}userParams_ts;
#pragma pack()
#endif
