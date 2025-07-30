#ifndef __MYLCD_H
#define __MYLCD_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define MAX_COLUMN_COUNT  192
#define MAX_PAGE_COUNT  12

typedef enum{
    IM_NOMALE = 0,    //
    IM_INVERSE = 1,   //
}inverseMode_te;
typedef enum{
    IMG_SRC_VOLLGO = 0,    //
}imageSrc_te;
typedef enum{
    CMDT_EXT_CMD1 = 0x30,    //
    CMDT_EXT_CMD2 = 0x31,   //
    CMDT_EXT_CMD3 = 0x38,   //
    CMDT_EXT_CMD4 = 0x39,   //
}commandType_te;
typedef enum{
    HORIZONTAL_LEFT = 0x101,    //
    HORIZONTAL_CENTER = 0x102,    //
    HORIZONTAL_RIGHT = 0x103,    //
}displayHorizontalMode_te;
typedef enum{
    VERTICAL_LEFT = 0x101,    //
    VERTICAL_CENTER = 0x102,    //
    VERTICAL_RIGHT = 0x103,    //
}displayVerticalMode_te;
typedef struct 
{
    uint16_t width;
    uint16_t heigth;
    uint8_t *imageSrc;
}imageParams_ts;

/**
 *  扩屏指令 0 一定要调用 @CMDT_EXT_CMD1 才能用扩展指令 0
*/
#define LCD_CMD_DISPLAY_ON  0XAF
#define LCD_CMD_DISPLAY_OFF  0XAE
#define LCD_CMD_INVERSE_DISPLAY_ON  0XA7
#define LCD_CMD_INVERSE_DISPLAY_OFF  0XA6
#define LCD_CMD_ALL_PIXEL_ON  0X23
#define LCD_CMD_ALL_PIXEL_OFF  0X22
#define LCD_CMD_DISPLAY_CONTROL  0XCA
            //BYTE1，设置 CL 驱动频率： CLD=0
            //BYTE2，点空比：Duty=128
            //BYTE3，帧周期
#define LCD_CMD_POWER_SAVE_OFF  0X94
#define LCD_CMD_POWER_SAVE_ON  0X95
#define LCD_CMD_PAGE_ADDR_SETTING  0X75
            //BYTE1，起始页地址,00h≦ YS≦ 28h
            //BYTE2，结束页地址,YS≦ YE≦ 28h每 4 行为 1 页
#define LCD_CMD_CLUMN_ADDR_SETTING  0X15
            //BYTE1，起始列地址 00h≦ XS≦ FFh
            //BYTE2，结束列地址 XS≦ XE≦ FFh
#define LCD_CMD_SCAN_DIRECTION  0XBC
            //BYTE1，0X00：MX、MY=Normal
#define LCD_CMD_WRITE_DISPLAY_DATA  0X5C
            //BYTE1，8 位显示数据
#define LCD_CMD_READ_DISPLAY_DATA  0X5D
            //BYTE1，8 位显示数据
#define LCD_CMD_PARTIAL_DISPLAY  0XA8
            //BYTE1，起始区域地址：00h≤PTS≥A1h
            //BYTE2，结束区域地址：00h≤PTE≥A1h
#define LCD_CMD_PARTIAL_DISPLAY_OFF  0XA9
#define LCD_CMD_READ_CHANGE_WRITE_ENABLE  0XE0
#define LCD_CMD_READ_CHANGE_WRITE_DISABLE  0XEE
#define LCD_CMD_SCROLL_AREA_DISPLAY  0XAA   //滚动区域设置
            //BYTE1，TL[7:0]:起始区域地址
            //BYTE2，BL[7:0]:结束区域地址
            //BYTE3，NSL[7:0]:指定行数
            //BYTE4，SCM[1:0]:显示模式
typedef enum{
    SDM_MODE_CENTER = 0,    //
    SDM_MODE_TOP = 1,    //
    SDM_MODE_BOTTOM = 2,    //
    SDM_MODE_WHOLE = 3,    //
}scrollDisplayMode_te;
#define LCD_CMD_SCROLL_AREA_DISPLAY_INIT  0XAB  //滚动开始初始行设置
            //BYTE1，00h≤SL≥A1h
#define LCD_CMD_OSCIL_CIRCUL_ON  0XD1   //开内部振荡电路
#define LCD_CMD_OSCIL_CIRCUL_OFF  0XD1  //关内部振荡电路
#define LCD_CMD_POWER_CONTROL  0X20  //电源控制
            //BYTE1，0X0B： VB、VF、VR=1
#define LCD_CMD_SET_VOP  0X81  //设置对比度
            //BYTE1，OX26:微调对比度,范围 OX00-OXFF
            //BYTE2，OX04:粗调对比度,范围 OX00-0X07先微调再粗调，顺序不能变
#define LCD_CMD_VOP_CONTROL_INC0_04  0XD6  //液晶内部电压控制,VOP 每格增加 0.04V
#define LCD_CMD_VOP_CONTROL_DEC0_04  0XD7  //液晶内部电压控制,VOP 每格减少 0.04V
#define LCD_CMD_READ_REG_VALUE_05  0X7C  //读寄存器值 Vop[5:0]
#define LCD_CMD_READ_REG_VALUE_68  0X7D  //读寄存器值 Vop[8:6]
#define LCD_CMD_NOP  0X25  //空操作
// #define LCD_CMD_READ_STATUS_REG  0X--  //读状态字节
// #define LCD_CMD_READ_STATUS_REG  0X--  //读状态字节
#define LCD_CMD_DATA_FORMAT_D7_D0  0X08  //数据格式选择,数据 D7→D0
#define LCD_CMD_DATA_FORMAT_D0_D7  0X0C  //数据格式选择,数据 D0→D7
#define LCD_CMD_DISPLAY_MODE  0XF0  //显示模式
typedef enum{
    DM_MONO_MODE = 0x10,    //黑白模式
    DM_4GRAY_MODE = 0x11,   //灰级度模式
}displayMode_te;

#define LCD_CMD_ICON_RAM_ENABLE  0X77  //启用 ICON RAM
#define LCD_CMD_ICON_RAM_DISABLE  0X76  //禁用 ICON RAM
#define LCD_CMD_SET_DIVCE_ROLE_MASTER  0X6E  //主模式(使用主模式)
#define LCD_CMD_SET_DIVCE_ROLE_SLAVER  0X6F  // 从模式
/**
 *  扩屏指令 1 一定要调用 @CMDT_EXT_CMD2 才能用扩展指令 1
*/
#define LCD_CMD_SET_GRAY_LEVEL  0X20
            //BYTE1，
            //...
            //BYTE16，
#define LCD_CMD_SET_LCD_OFFSET_VOL  0X32
            //BYTE1，
            //BYTE2，BE0,BE1,0X01： 升压电容频率
            //BYTE3，BS0,BS1,BS1,0X02： 偏压比，BIAS=1/12
#define LCD_CMD_BOOSTER_LEVEL  0X51 //内建升压倍数设置
            //BYTE1，0X7B:10 倍
#define LCD_CMD_VOLTAGE_SELECT_INT  0X41 //LCD 内部升压
// #define LCD_CMD_VOLTAGE_SELECT_XXX  0X40 //未知
// #define LCD_CMD_AUTO_READ  0XD7 //未知
#define LCD_CMD_SET_OTP_RW  0XE0 //控制OTP读写
            //BYTE1，0x00,WR/RD=0；使能 OTP 读
            //       0x20,WR/RD=1；使能 OTP 写
#define LCD_CMD_SET_OTP_OUT  0XE1 //控制 OTP 出
#define LCD_CMD_SET_OTP_WRITE  0XE2 //写 OTP
#define LCD_CMD_SET_OTP_READ  0XE3 //读 OTP
#define LCD_CMD_SET_OTP_SELECT  0XE4 //OTP 选择控制
            //BYTE1，Ctrl=1： 0xc9， 不使能 OTP
            //       Ctrl=0： 0x89， 使能 OTP

void myLCD_init(void);
void myLCD_setSdaMode(GPIOMode_TypeDef GPIO_Mode);
void myLCD_resetLcd(void);
void myLCD_setGrayLevel(uint8_t level_b, uint8_t level_d);
void myLCD_setVop(uint16_t value);

void myLCD_displayDot(int x, int y);
void myLCD_clearFull(void);
void myLCD_displayBlock(int x,int y);
void myLCD_8x16(inverseMode_te inverseMode, int column, int page, char *text);
void myLCD_str8x16(inverseMode_te inverseMode, int column, int page, char *fmt, ...);
void myLCD_16x16(inverseMode_te inverseMode, uint8_t column, uint8_t page, char *text);
void myLCD_displayImage(inverseMode_te inverseMode, int x, int y, imageSrc_te index);
void myLCD_scroll(scrollDisplayMode_te mode, uint8_t startAddr, uint8_t endAddr, uint8_t pages);
void myLCD_scrollLine(uint8_t pages);

int SHT3X_getPresentValue(int *pTemp, int *pHumit, unsigned char decimalPlaces);
#endif

