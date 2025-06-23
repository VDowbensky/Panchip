

/******************************************************************************/
/** \file lcd_view_ctrl.h
 ******************************************************************************/
#ifndef __LCD_VIEW_CTRL_H
#define __LCD_VIEW_CTRL_H

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "PanSeries.h"

#ifdef __cplusplus
 extern "C" {
#endif


/**
 *******************************************************************************
 ** \defgroup DdlUtilityGroup Device Driver Library Gpio (GPIO CONTROL)
 **
 ******************************************************************************/
//@{

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define ENGLISH_6X12_TEXT			(DDL_ON) // DDL_ON DDL_OFF
#define ENGLISH_8X16_TEXT			(DDL_ON)
#define CHINESE_12X12_TEXT			(DDL_ON)
#define CHINESE_16X16_TEXT			(DDL_ON)
#define ENGLISH_8X16_TEXT			(DDL_ON)

#define START_FACE_IMAGE			(DDL_ON)

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/* LCD 显示界面定义 */
typedef enum{
	LCD_START_FACE,
	LCD_MAIN_FACE,
}LCD_VIEW_FACE;

/* LCD 刷新界面显示定义 */
typedef enum{
	LCD_NO_REFLASH					= (0), // 不刷新
	LCD_MAIN_FACE_BKG				= (1 << 1), // 刷新主界面背景显示
	LCD_MAIN_FACE_CON_STATE			= (1 << 2), // 刷新主界面显示设备连接状态
	LCD_MAIN_FACE_PG_NUM			= (1 << 3), // 刷新主界面显示项目程序个数
	LCD_MAIN_FACE_PG_SEL			= (1 << 4), // 刷新主界面显示选择项目号
	LCD_MAIN_FACE_PG_CRC			= (1 << 5), // 刷新主界面显示项目程序 CRC
	LCD_MAIN_FACE_PG_NAME			= (1 << 6), // 刷新主界面显示项目名
	LCD_MAIN_FACE_T_MODE			= (1 << 7), // 刷新主界面显示测试模式与测试状态
	LCD_MAIN_FACE_OFF_NUM			= (1 << 8), // 刷新主界面显示离线下载程序总数
	LCD_MAIN_FACE_OFF_USE			= (1 << 9), // 刷新主界面显示离线下载程序剩余与使用个数
	LCD_MAIN_FACE_PROGRESS			= (1 << 10), // 刷新主界面显示进度条
	LCD_MAIN_FACE_CFG_NUM			= (1 << 11), // 刷新主界面显示离线配置剩余个数
	
	LCD_REFLASH_ALL					= (0xFFFF), // 刷新全部
}LCD_VIEW_REFLASH;

/* LCD 主界面模式定义 */
typedef enum{
	MAIN_FACE_TEST_MODE,		// 测试模式
	MAIN_FACE_SEL_PJ_MODE,		// 选择项目程序模式
	MAIN_FACE_SEL_CON_MODE,		// 设置连接模式模式
	MAIN_FACE_SEL_LANGUAGE_MODE,// 设置显示语言模式
	MAIN_FACE_TESTING,			// 测试中
	MAIN_FACE_TEST_SUCCESS,		// 测试成功
	MAIN_FACE_TEST_FAIL,		// 测试失败
	MAIN_FACE_OFF_PJ_LACK,		// 离线下载次数不足提示
	MAIN_FACE_OFF_CFG_LACK,		// 离线下载配置不足提示
	MAIN_FACE_OFF_CFG_REUSE		// 离线下载配置重复提示
}MAIN_FACE_MODE;


/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
void LcdStartFaceAliveCont(void);
void LcdDrawEnglish8x16Str(uint8_t u8x, uint8_t u8y, const char *pStr, 
	uint16_t strColor, uint16_t bkColor);
void LcdDrawEnglish6x12Str(uint8_t u8x, uint8_t u8y, const char *pStr, 
	uint16_t strColor, uint16_t bkColor);

void DoLcdView(void);

void SetViewLanguage(uint8_t u8Lg);
void ChangeLanguageView(void);

void SetViewConnect(uint8_t u8Md, uint8_t u8Param);
void SetVersionView(uint8_t *pU8Version);
void SetToStartFaceView(void);
void SetChipConnectView(uint8_t u8State);
void SetChipConnectStateView(uint8_t u8State);
void ChangeChipConnectModeView(uint8_t u8Next);
void SetProjectNumsView(uint16_t u16Num);
void SetProjectSelView(uint16_t u16Sel);
void SetProjectCrcView(uint32_t u32Crc);
void SetProjectNameView(uint8_t *pU8Name);
void SetModeView(MAIN_FACE_MODE mode);
void SetOfflineTimesView(uint32_t u32Times);
void SetOfflineUsedView(uint32_t u32Used);
void SetProgressView(uint8_t u8Percent);
void SetViewProgress(uint32_t u32Range, uint32_t u32Idx);
void SetViewProgressRealTime(uint8_t u8Percent);
void SetConfigView(uint8_t u8State, uint32_t u32Num, uint32_t u32Num1);

uint8_t CheckMainFaceFree(void);

uint8_t GetCurrentViewLanguage(void);
uint8_t GetViewChipConnectMode(void);
uint8_t GetLcdViewMode(void);
uint16_t GetViewProjectNums(void);
uint16_t GetViewProjectSel(void);


#ifdef __cplusplus
}
#endif

#endif /* __LCD_VIEW_CTRL_H */


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/



