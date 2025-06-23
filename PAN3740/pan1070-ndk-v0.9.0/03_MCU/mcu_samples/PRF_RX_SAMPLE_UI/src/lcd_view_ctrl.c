

/*******************************************************************************
 * Include files 编码格式必须为: GB 2312 否则 PLINK LCD 乱码显示中文
 ******************************************************************************/
#include "lcd_view_ctrl.h"
#include "lcd_matrix.h"
#include "tft_lcd_128x160.h"


/**
 *******************************************************************************
 ** \addtogroup TFT_LCD_128x160
 ******************************************************************************/
//@{

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define START_FACE_VIEW_WAIT			1000

/* 注：该文件的编码格式必须与 lcd_matrix.c 编码格式一致 */
/* LCD 开始界面定义 */
#define PLINK_STR						"PAN-LINK "
#define EN_FOR_STR						"For"
#define CN_FOR_STR						"为"
#define EN_VERSION_STR					"Version"
#define CN_VERSION_STR					"版本"
#define EN_COLON_STR					":"
#define EN_CUT_STR						" "

#define CHIP_TYPE_STR					"PAN1080"

#define PLINK_STR_LEN					(sizeof(PLINK_STR) - 1)
#define PLINK_EN_FOR_STR_LEN			(sizeof(EN_FOR_STR) - 1)
#define PLINK_CN_FOR_STR_LEN			(sizeof(CN_FOR_STR) / 2)
#define EN_VERSION_STR_LEN				(sizeof(EN_VERSION_STR) - 1)
#define CN_VERSION_STR_LEN				(sizeof(CN_VERSION_STR) / 2)
#define CHIP_TYPE_STR_LEN				(sizeof(CHIP_TYPE_STR) - 1)

#define PROJECT_EN_NUMS_STR				" PJ NUMS"
#define PROJECT_CN_NUMS_STR				"程序总数"
#define PROJECT_EN_SEL_STR				"   PJ ID"
#define PROJECT_CN_SEL_STR				"当前程序"
#define PROJECT_ST_STR_LEN				8

#define PROJECT_EN_STR					" PJ "
#define PROJECT_CN_STR					"程序"
#define PROJECT_CRC_STR					"CRC"
#define PROJECT_STR_LEN					4
#define PROJECT_CRC_STR_LEN				3

#define LIMIT_EN_REMAIN_STR				"  Remain"
#define LIMIT_CN_REMAIN_STR				"剩余次数"
#define LIMIT_EN_USED_STR				"    Used"
#define LIMIT_CN_USED_STR				"已用次数"
#define LIMIT_EN_ALL_STR				"   Total"
#define LIMIT_CN_ALL_STR				"离线总数"
#define LIMIT_ST_STR_LEN				8
#define LIMIT_NUM_STR_LEN				7

#define TEST_MODE_EN_STR				"Burn mode"
#define SEL_PJ_MODE_EN_STR				"Select Project"
#define SEL_CON_MODE_EN_STR				"Setting Connect"
#define SEL_LANGUAGE_EN_STR				"Setting Language"
#define TESTING_EN_STR					"Burning ..."
#define TEST_SUCCESS_EN_STR				"Burn Success"
#define TEST_FAIL_EN_STR				"Burn Fail"
#define OFF_PJ_LACK_EN_STR				"Offline Use Up"
#define OFF_CFG_LACK_EN_STR				"Config Use Up"
#define OFF_CFG_REUSE_EN_STR			"Config Repeat"

#define TEST_MODE_CN_STR				"烧录模式"
#define SEL_PJ_MODE_CN_STR				"选择程序模式"
#define SEL_CON_MODE_CN_STR				"设置连接模式"
#define SEL_LANGUAGE_CN_STR				"设置语言模式"
#define TESTING_CN_STR					"烧录中"
#define TEST_SUCCESS_CN_STR				"烧录成功"
#define TEST_FAIL_CN_STR				"烧录失败"
#define OFF_PJ_LACK_CN_STR				"烧录次数不足"
#define OFF_CFG_LACK_CN_STR				"烧录配置不足"
#define OFF_CFG_REUSE_CN_STR			"烧录配置重复"

#define MAIN_FACE_MODE_MAX_LEN			((TFT_LCD_DEF_X_MAX - 2) / ENGLISH_8X16_TEXT_W)
#define MAIN_FACE_CFG_STR_LEN			7


/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/* LCD 主界面显示定义 */
typedef struct lcdMainFaceParam{
	MAIN_FACE_MODE mode;		// 模式显示
	uint8_t state;				// 设备连接状态
								//     [3:0]: 通讯模式
								//     [7:4]: 连接状态
	uint8_t progress;			// 进度显示百分比，0~100
	uint8_t name[26];			// 显示项目名
	uint8_t cfgState;			// 显示离线配置控制:
								//     0: 不显示
								//     cfgState[0]: 1:显示cfgNum, 0: 不显示
								//     cfgState[1]: 1:显示cfgNum1, 0: 不显示
	uint16_t num;				// 显示项目程序数
	uint16_t sel;				// 显示选中项目程序号
	uint32_t crc;				// 显示项目程序crc
	uint32_t times;				// 显示离线下载总数
	uint32_t used;				// 显示离线下载已使用次数
	uint32_t cfgNum;			// 显示离线配置个数
	uint32_t cfg1Num;			// 显示离线配置1个数
}LCD_MAIN_FACE_PARAM;

/* LCD 主界面芯片连接通讯模式定义 */
/*
#define CHIP_CON_MD_STR_LEN				4
#define CHIP_CON_MD_STR_NUM				3
char chipConModeStr[CHIP_CON_MD_STR_NUM][CHIP_CON_MD_STR_LEN + 1] = {
	{" TWI"},
	{"UART"},
	{" SPI"}
};*/
#define CHIP_CON_MD_STR_LEN				4
#define CHIP_CON_MD_STR_NUM				1
char chipConModeStr[CHIP_CON_MD_STR_NUM][CHIP_CON_MD_STR_LEN + 1] = {
	{"chip"}
};


/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
/* LCD 显示界面定义 */
static LCD_VIEW_FACE lcdViewFace = LCD_START_FACE;
/* LCD 刷新显示定义 */
static LCD_VIEW_REFLASH lcdViewReflash = LCD_REFLASH_ALL;
/* LCD 起始界面显示计时 */
static __IO uint16_t lcdStartViewCont = 0;
/* LCD 开始界面显示定义 */
static char plinkVerion[12] = {'2','1','1','0','2','6','1','5','5','2','\0'};

/* LCD 主界面显示参数 */
LCD_MAIN_FACE_PARAM mainFaceParam = {
	MAIN_FACE_TEST_MODE,		// default mode
	0,							// default state
	0,							// default progress
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // default name
	0,							// default cfgState
	0,							// default num
	0,							// default sel
	0x00000000,					// default crc
	0,							// default times
	0,							// default used
	0,							// default cfgNum
	0							// default cfg1Num
};

/* LCD 显示语言定义 */
static uint8_t languageView = 0; // 0: Chinese, 1: English,



/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/**
 *******************************************************************************
 ** \brief  计时跳过起始面显示,在 1ms 定时器回调函数中调用
 **
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void LcdStartFaceAliveCont(void)
{
	if(lcdStartViewCont){
		lcdStartViewCont --;
		if(lcdStartViewCont == 0){
			lcdViewFace = LCD_MAIN_FACE;
			lcdViewReflash = LCD_REFLASH_ALL;
		}
	}
}


#if (ENGLISH_6X12_TEXT == DDL_ON)
/**
 *******************************************************************************
 ** \brief  输出 6x12 的英文字符串显示
 **
 ** \param  u8x [in] 起始横向坐标
 **
 ** \param  u8y [in] 起始纵向坐标
 **
 ** \param  pStr [in] 显示字符串指针
 **
 ** \param  strColor [in] 显示字符串颜色
 **
 ** \param  bkColor [in] 显示背景颜色
 **
 ** \retval 无
 **
 ******************************************************************************/
void LcdDrawEnglish6x12Str(uint8_t u8x, uint8_t u8y, const char *pStr,
	uint16_t strColor, uint16_t bkColor)
{
	uint16_t u16StrIdx = 0, u16StrDataIdx = 0, u16w = 0, u16h = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pStr){
		/* 获取 LCD 屏幕显示尺寸 */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		/* 显示字符串 */
		u16w = u8x;
		u16h = u8y;
		for(u16StrIdx = 0; pStr[u16StrIdx]; u16StrIdx++){
			/* 计算并校验字符显示位置 */
			u8x = u16w;
			u8y = u16h;

			u16w += ENGLISH_6X12_TEXT_W;
			if(u16w > u16ScreenW){
				/* 换行显示 */
				u16w = 0;
				u8x = u16w;
				u16h += ENGLISH_6X12_TEXT_H;
				if(u16h > u16ScreenH){
					/* 超出屏幕显示 */
					break;
				}
				u8y = u16h;
			}
			if(u8y + ENGLISH_6X12_TEXT_H > u16ScreenH){
				/* 超出屏幕显示 */
				break;
			}

			/* 查询字符串数据位置 */
			if(pStr[u16StrIdx] >= '0' && pStr[u16StrIdx] <= '9'){
				u16StrDataIdx = pStr[u16StrIdx] - '0';
			}
			else if(pStr[u16StrIdx] >= 'A' && pStr[u16StrIdx] <= 'Z'){
				u16StrDataIdx = pStr[u16StrIdx] - 'A';
				u16StrDataIdx += 10;
			}
			else if(pStr[u16StrIdx] >= 'a' && pStr[u16StrIdx] <= 'z'){
				u16StrDataIdx = pStr[u16StrIdx] - 'a';
				u16StrDataIdx += 36;
			}
			else{
				switch(pStr[u16StrIdx]){
				case '.':
					u16StrDataIdx = 62;
					break;
				case '-':
					u16StrDataIdx = 63;
					break;
				case '_':
					u16StrDataIdx = 64;
					break;
				case ' ':
					u16StrDataIdx = 65;
					break;
				case ':':
					u16StrDataIdx = 66;
					break;
				case '[':
					u16StrDataIdx = 67;
					break;
				case ']':
					u16StrDataIdx = 68;
					break;
				default:
					u16StrDataIdx = 69;
					break;
				}
			}

			u16StrDataIdx *= ENGLISH_6X12_TEXT_SIZE;
			/* 显示查询到的字符 */
 			TftLcdDrawOnceText(u8x, u8y,
				ENGLISH_6X12_TEXT_W, ENGLISH_6X12_TEXT_H,
				english_6x12_text + u16StrDataIdx,
				strColor, bkColor
				);
		}
	}
}
#endif



#if (ENGLISH_8X16_TEXT == DDL_ON)
/**
 *******************************************************************************
 ** \brief  输出 8x16 的英文字符串显示
 **
 ** \param  u8x [in] 起始横向坐标
 **
 ** \param  u8y [in] 起始纵向坐标
 **
 ** \param  pStr [in] 显示字符串指针
 **
 ** \param  strColor [in] 显示字符串颜色
 **
 ** \param  bkColor [in] 显示背景颜色
 **
 ** \retval 无
 **
 ******************************************************************************/
void LcdDrawEnglish8x16Str(uint8_t u8x, uint8_t u8y, const char *pStr,
	uint16_t strColor, uint16_t bkColor)
{
	uint16_t u16StrIdx = 0, u16StrDataIdx = 0, u16w = 0, u16h = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pStr){
		/* 获取 LCD 屏幕显示尺寸 */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		/* 显示字符串 */
		u16w = u8x;
		u16h = u8y;
		for(u16StrIdx = 0; pStr[u16StrIdx]; u16StrIdx++){
			/* 计算并校验字符显示位置 */
			u8x = u16w;
			u8y = u16h;

			u16w += ENGLISH_8X16_TEXT_W;
			if(u16w > u16ScreenW){
				/* 换行显示 */
				u16w = 0;
				u8x = u16w;
				u16h += ENGLISH_8X16_TEXT_H;
				if(u16h > u16ScreenH){
					/* 超出屏幕显示 */
					break;
				}
				u8y = u16h;
			}
			if(u8y + ENGLISH_8X16_TEXT_H > u16ScreenH){
				/* 超出屏幕显示 */
				break;
			}

			if(pStr[u16StrIdx] >= '0' && pStr[u16StrIdx] <= '9'){
				u16StrDataIdx = pStr[u16StrIdx] - '0';
			}
			else if(pStr[u16StrIdx] >= 'A' && pStr[u16StrIdx] <= 'Z'){
				u16StrDataIdx = pStr[u16StrIdx] - 'A';
				u16StrDataIdx += 10;
			}
			else if(pStr[u16StrIdx] >= 'a' && pStr[u16StrIdx] <= 'z'){
				u16StrDataIdx = pStr[u16StrIdx] - 'a';
				u16StrDataIdx += 36;
			}
			else{
				switch(pStr[u16StrIdx]){
				case '.':
					u16StrDataIdx = 62;
					break;
				case '-':
					u16StrDataIdx = 63;
					break;
				case '_':
					u16StrDataIdx = 64;
					break;
				case ' ':
					u16StrDataIdx = 65;
					break;
				case ':':
					u16StrDataIdx = 66;
					break;
				case '[':
					u16StrDataIdx = 67;
					break;
				case ']':
					u16StrDataIdx = 68;
					break;
				default:
					u16StrDataIdx = 69;
					break;
				}
			}
			u16StrDataIdx *= ENGLISH_8X16_TEXT_SIZE;
			/* 显示字符 */
 			TftLcdDrawOnceText(u8x, u8y,
				ENGLISH_8X16_TEXT_W, ENGLISH_8X16_TEXT_H,
				english_8x16_text + u16StrDataIdx,
				strColor, bkColor
				);
		}
	}
}
#endif



#if (CHINESE_12X12_TEXT == DDL_ON)
/**
 *******************************************************************************
 ** \brief  输出 12x12 的中文字符串显示
 **
 ** \param  u8x [in] 起始横向坐标
 **
 ** \param  u8y [in] 起始纵向坐标
 **
 ** \param  pStr [in] 显示字符串指针
 **
 ** \param  strColor [in] 显示字符串颜色
 **
 ** \param  bkColor [in] 显示背景颜色
 **
 ** \retval 无
 **
 ******************************************************************************/
void LcdDrawChinese12x12Str(uint8_t u8x, uint8_t u8y, const char *pStr,
	uint16_t strColor, uint16_t bkColor)
{
	uint16_t u16StrIdx = 0, u16StrDataIdx = 0, u16w = 0, u16h = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;
	uint16_t u16TextIdx = 0;

	if(pStr){
		/* 获取 LCD 屏幕显示尺寸 */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		/* 显示字符串 */
		u16w = u8x;
		u16h = u8y;
		for(u16StrIdx = 0; pStr[u16StrIdx]; u16StrIdx += 2){
			/* 计算并校验字符显示位置 */
			u8x = u16w;
			u8y = u16h;

			u16w += CHINESE_12X12_TEXT_W;
			if(u16w > u16ScreenW){
				/* 换行显示 */
				u16w = 0;
				u8x = u16w;
				u16h += CHINESE_12X12_TEXT_H;
				if(u16h > u16ScreenH){
					/* 超出屏幕显示 */
					break;
				}
				u8y = u16h;
			}
			if(u8y + CHINESE_12X12_TEXT_H > u16ScreenH){
				/* 超出屏幕显示 */
				break;
			}

			/* 查询字符串数据位置 */
			u16StrDataIdx = 0;
			for(u16TextIdx = 0; chinese_12x12_text_str[u16TextIdx]; u16TextIdx += 2){
				if(pStr[u16StrIdx] == chinese_12x12_text_str[u16TextIdx] &&
					pStr[u16StrIdx + 1] == chinese_12x12_text_str[u16TextIdx + 1]){
					break;
				}
				u16StrDataIdx ++;
			}

			u16StrDataIdx *= CHINESE_12X12_TEXT_SIZE;
			/* 显示查询到的字符 */
 			TftLcdDrawOnceText(u8x, u8y,
				CHINESE_12X12_TEXT_W, CHINESE_12X12_TEXT_H,
				chinese_12x12_text + u16StrDataIdx,
				strColor, bkColor
				);
		}
	}
}
#endif


#if (CHINESE_16X16_TEXT == DDL_ON)
	//extern const uint8_t chinese_16x16_text[];
/**
 *******************************************************************************
 ** \brief  输出 16x16 的中文字符串显示
 **
 ** \param  u8x [in] 起始横向坐标
 **
 ** \param  u8y [in] 起始纵向坐标
 **
 ** \param  pStr [in] 显示字符串指针
 **
 ** \param  strColor [in] 显示字符串颜色
 **
 ** \param  bkColor [in] 显示背景颜色
 **
 ** \retval 无
 **
 ******************************************************************************/
void LcdDrawChinese16x16Str(uint8_t u8x, uint8_t u8y, const char *pStr,
	uint16_t strColor, uint16_t bkColor)
{
	uint16_t u16StrIdx = 0, u16StrDataIdx = 0, u16w = 0, u16h = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;
	uint16_t u16TextIdx = 0;

	if(pStr){
		/* 获取 LCD 屏幕显示尺寸 */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		/* 显示字符串 */
		u16w = u8x;
		u16h = u8y;
		for(u16StrIdx = 0; pStr[u16StrIdx]; u16StrIdx += 2){
			/* 计算并校验字符显示位置 */
			u8x = u16w;
			u8y = u16h;

			u16w += CHINESE_16X16_TEXT_W;
			if(u16w > u16ScreenW){
				/* 换行显示 */
				u16w = 0;
				u8x = u16w;
				u16h += CHINESE_16X16_TEXT_H;
				if(u16h > u16ScreenH){
					/* 超出屏幕显示 */
					break;
				}
				u8y = u16h;
			}
			if(u8y + CHINESE_16X16_TEXT_H > u16ScreenH){
				/* 超出屏幕显示 */
				break;
			}

			/* 查询字符串数据位置 */
			u16StrDataIdx = 0;
			for(u16TextIdx = 0; chinese_16x16_text_str[u16TextIdx]; u16TextIdx += 2){
				if(pStr[u16StrIdx] == chinese_16x16_text_str[u16TextIdx] &&
					pStr[u16StrIdx + 1] == chinese_16x16_text_str[u16TextIdx + 1]){
					break;
				}
				u16StrDataIdx ++;
			}

			u16StrDataIdx *= CHINESE_16X16_TEXT_SIZE;
			/* 显示查询到的字符 */
 			TftLcdDrawOnceText(u8x, u8y,
				CHINESE_16X16_TEXT_W, CHINESE_16X16_TEXT_H,
				chinese_16x16_text + u16StrDataIdx,
				strColor, bkColor
				);
		}
	}
}

#endif


/**
 *******************************************************************************
 ** \brief  开始界面显示
 **
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void StartFaceView(void)
{
	uint8_t u8x = 0, u8ty = 0, u8y = 0, u8w = 0, u8h = 0;
	uint8_t u8TextH = ENGLISH_6X12_TEXT_H + 2;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	/* LCD 刷新背景显示 */
	TftLcdViewAllColor(TFT_COLOR_WHITE);
	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	u16ScreenH = TftLcdScreenHight();
	/* LCD 显示起始界面 logo 图标 */
	u8w = START_FACE_IMAGE_W;
	u8h = START_FACE_IMAGE_H;
	u8x = (u16ScreenW - u8w) / 2;
	u8y = (u16ScreenH - u8h - 2 * u8TextH) / 2;
	TftLcdDrawU16ColorImage(u8x, u8y, u8w, u8h, start_face_image);
	/* LCD 显示 PAN-LINK 对应项目 */
	/* 计算起始坐标 */
	if(languageView){ // English
		u8w = (PLINK_STR_LEN + PLINK_EN_FOR_STR_LEN + 1 + CHIP_TYPE_STR_LEN) * ENGLISH_6X12_TEXT_W;
	}
	else{ // Chinese
		u8w = (PLINK_STR_LEN + 1 + CHIP_TYPE_STR_LEN) * ENGLISH_6X12_TEXT_W;
		u8w += (PLINK_CN_FOR_STR_LEN * CHINESE_12X12_TEXT_W);
	}
	u8x = (u16ScreenW - u8w) / 2;
	u8ty = u8x;
	u8y += (u8h + 1);
	/* 显示   PAN-LINK  */
	LcdDrawEnglish6x12Str(u8x, u8y, PLINK_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	/* 显示 为    */
	u8x += (PLINK_STR_LEN * ENGLISH_6X12_TEXT_W);
	if(languageView){ // English
		LcdDrawEnglish6x12Str(u8x, u8y, EN_FOR_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
		u8x += (PLINK_EN_FOR_STR_LEN * ENGLISH_6X12_TEXT_W);
	}
	else{ // Chinese
		LcdDrawChinese12x12Str(u8x, u8y, CN_FOR_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
		u8x += (PLINK_CN_FOR_STR_LEN * CHINESE_12X12_TEXT_W);
	}
	/* 显示空格     */
	LcdDrawEnglish6x12Str(u8x, u8y, EN_CUT_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	/* 显示 芯片新号       */
	u8x += ENGLISH_6X12_TEXT_W;
	LcdDrawEnglish6x12Str(u8x, u8y, CHIP_TYPE_STR, TFT_COLOR_PURPLE, TFT_COLOR_WHITE);
	/* LCD 显示 PAN-LINK 固件版本号 */
	u8x = u8ty;
	u8y += u8TextH;
	if(languageView){ // English
		LcdDrawEnglish6x12Str(u8x, u8y, EN_VERSION_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
		u8x += (EN_VERSION_STR_LEN * ENGLISH_6X12_TEXT_W);
	}
	else{ // Chinese
		LcdDrawChinese12x12Str(u8x, u8y, CN_VERSION_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
		u8x += (CN_VERSION_STR_LEN * CHINESE_12X12_TEXT_W);
	}
	LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	u8x += ENGLISH_6X12_TEXT_W;
	LcdDrawEnglish6x12Str(u8x, u8y, plinkVerion, TFT_COLOR_BLUE, TFT_COLOR_WHITE);
}


/**
 *******************************************************************************
 ** \brief  主界面背景绘制显示
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                        |    |PANCHIP                   |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |                               |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceDrawBk(void)
{
	uint8_t u8x = 0, u8tx = 0, u8y = 0, u8w = 0, u8h = 0;
	uint16_t u16ScreenW = 0;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	/* LCD 刷新背景显示 */
	TftLcdViewAllColor(TFT_COLOR_BLACK);
	/* LCD 头部显示 列：ST:0, 1 + 12 + 1*/
	u8x = 1; u8y = 0; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 0; u8y += u8h; u8w = u16ScreenW; u8h = 12;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* LCD 头部显示芯片型号 */
	u8x = 2; u8y = 1;
	LcdDrawEnglish6x12Str(u8x, u8y, CHIP_TYPE_STR, TFT_TITLE_COLOR, TFT_COLOR_BLUE);

	// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 程序总数，当前程序， crc ST: 14, 1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 14; u8w = 1; u8h = 27;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 项目程序名，ST: x:0, y:42, 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 42; u8w = 1; u8h = 14;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 模式显示，ST: x:0, y:57, 25 */
	u8x = 0; u8y = 57; u8w = 1; u8h = 25;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 离线次数，ST: x:0, y:83, 1 + 12 + 1 + 12 + 1 + 12 + 1 + 1 */
	u8x = 0; u8y = 83; u8w = 1; u8h = 40;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W) - 2;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 进度条，ST: x:0, y:124, 3 + 1 */
	u8x = 0; u8y = 124; u8w = 1; u8h = 3;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	if(languageView){ // English
		/* 程序总数 ST: x:2 y:15, h:12 */
		u8x = 2; u8y = 15;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 当前程序 ST: x:(u16ScreenW / 2) y:15, h:12 */
		u8x = (u16ScreenW / 2);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x:2 y:28, h:12 */
		u8x = 2; u8y = 28;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:84, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 84;
		u8tx = u8x;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:97, 12 */
		u8x = u8tx;
		u8y = 97;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:110, 12 */
		u8x = u8tx;
		u8y = 110;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
	else{ // Chinese
		/* 程序总数 ST: x:2 y:15, h:12 */
		u8x = 2; u8y = 15;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 当前程序 ST: x:(u16ScreenW / 2) y:15, h:12 */
		u8x = (u16ScreenW / 2);
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x:2 y:28, h:12 */
		u8x = 2; u8y = 28;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:84, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 84;
		u8tx = u8x;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:97, 12 */
		u8x = u8tx;
		u8y = 97;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:110, 12 */
		u8x = u8tx;
		u8y = 110;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 程序总数，当前程序， crc ST: 14, 1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 14; u8w = 1; u8h = 27;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 项目程序名，ST: x:0, y:42, 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 42; u8w = 1; u8h = 14;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 模式显示，ST: x:0, y:57, 25 */
	u8x = 0; u8y = 57; u8w = 1; u8h = 25;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 离线次数，ST: x:0, y:83, 1 + 12 + 1 + 12 + 1 + 12 + 1 + 1 */
	u8x = 0; u8y = 83; u8w = 1; u8h = 40;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W) - 2;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 进度条，ST: x:0, y:124, 3 + 1 */
	u8x = 0; u8y = 124; u8w = 1; u8h = 3;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	if(languageView){ // English
		/* 程序总数 ST: x:2 y:15, h:12 */
		u8x = 2; u8y = 15;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 当前程序 ST: x:(u16ScreenW / 2) y:15, h:12 */
		u8x = (u16ScreenW / 2);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x:2 y:28, h:12 */
		u8x = 2; u8y = 28;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:84, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 84;
		u8tx = u8x;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:97, 12 */
		u8x = u8tx;
		u8y = 97;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:110, 12 */
		u8x = u8tx;
		u8y = 110;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
	else{ // Chinese
		/* 程序总数 ST: x:2 y:15, h:12 */
		u8x = 2; u8y = 15;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 当前程序 ST: x:(u16ScreenW / 2) y:15, h:12 */
		u8x = (u16ScreenW / 2);
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x:2 y:28, h:12 */
		u8x = 2; u8y = 28;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:84, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 84;
		u8tx = u8x;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:97, 12 */
		u8x = u8tx;
		u8y = 97;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:110, 12 */
		u8x = u8tx;
		u8y = 110;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 程序总数，当前程序， crc ST: x:0, y:14, 14, 1 + 12 + 1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 14; u8w = 1; u8h = 40;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 项目程序名，ST: x:0, y:55, 1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 55; u8w = 1; u8h = 27;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 模式显示，ST: x:0, y:83, 31 */
	u8x = 0; u8y = 83; u8w = 1; u8h = 31;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 离线次数，ST: x:0, y:115, 1 + 12 + 1 + 12 + 1 + 12 + 1 + 1 */
	u8x = 0; u8y = 115; u8w = 1; u8h = 40;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W) - 2;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 进度条，ST: x:0, y:156, 3 + 1 */
	u8x = 0; u8y = 156; u8w = 1; u8h = 3;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);

	if(languageView){ // English
		/* 程序总数 ST: x = 2, y:15, 12 */
		u8x = 2; u8y = 15;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 程序总数，当前程序 ST: x = 2, y:28, h = 12 */
		u8x = 2; u8y = 28;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x = 2, y:41, h = 12 */
		u8x = 2; u8y = 41;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:116, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 116;
		u8tx = u8x;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:129, 12 */
		u8x = u8tx;
		u8y = 129;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:142, 12 */
		u8x = u8tx;
		u8y = 142;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
	else{ // Chinese
		/* 程序总数，当前程序 x = 2, y:15, h:12  */
		u8x = 2; u8y = 15;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 程序总数，当前程序 ST: x = 2, y:28, h = 12 */
		u8x = 2; u8y = 28;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x = 2, y:41, h = 12 */
		u8x = 2; u8y = 41;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:116, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 116;
		u8tx = u8x;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:129, 12 */
		u8x = u8tx;
		u8y = 129;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:142, 12 */
		u8x = u8tx;
		u8y = 142;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 程序总数，当前程序， crc ST: x:0, y:14, 14, 1 + 12 + 1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 14; u8w = 1; u8h = 40;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 项目程序名，ST: x:0, y:55, 1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 0; u8y = 55; u8w = 1; u8h = 27;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 模式显示，ST: x:0, y:83, 31 */
	u8x = 0; u8y = 83; u8w = 1; u8h = 31;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 离线次数，ST: x:0, y:115, 1 + 12 + 1 + 12 + 1 + 12 + 1 + 1 */
	u8x = 0; u8y = 115; u8w = 1; u8h = 40;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W) - 2;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	/* 进度条，ST: x:0, y:156, 3 + 1 */
	u8x = 0; u8y = 156; u8w = 1; u8h = 3;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = u16ScreenW - 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);
	u8x = 1; u8y += u8h; u8w = u16ScreenW - 2; u8h = 1;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLUE);

	if(languageView){ // English
		/* 程序总数 ST: x = 2, y:15, 12 */
		u8x = 2; u8y = 15;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 程序总数，当前程序 ST: x = 2, y:28, h = 12 */
		u8x = 2; u8y = 28;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x = 2, y:41, h = 12 */
		u8x = 2; u8y = 41;
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_EN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:116, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 116;
		u8tx = u8x;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:129, 12 */
		u8x = u8tx;
		u8y = 129;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:142, 12 */
		u8x = u8tx;
		u8y = 142;
		LcdDrawEnglish6x12Str(u8x, u8y, LIMIT_EN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
	else{ // Chinese
		/* 程序总数，当前程序 x = 2, y:15, h:12  */
		u8x = 2; u8y = 15;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_NUMS_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 程序总数，当前程序 ST: x = 2, y:28, h = 12 */
		u8x = 2; u8y = 28;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_SEL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* CRC ST: x = 2, y:41, h = 12 */
		u8x = 2; u8y = 41;
		LcdDrawChinese12x12Str(u8x, u8y, PROJECT_CN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, PROJECT_CRC_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (PROJECT_CRC_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 剩余次数，ST: x: <-, y:116, 12 */
		u8x = u16ScreenW - 2 - ((LIMIT_NUM_STR_LEN + 1 + LIMIT_ST_STR_LEN) * ENGLISH_6X12_TEXT_W);
		u8y = 116;
		u8tx = u8x;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_REMAIN_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 已用次数，ST: x:<-, y:129, 12 */
		u8x = u8tx;
		u8y = 129;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_USED_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		/* 离线次数 离线总数，ST: x:<-, y:142, 12 */
		u8x = u8tx;
		u8y = 142;
		LcdDrawChinese12x12Str(u8x, u8y, LIMIT_CN_ALL_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
		u8x += (LIMIT_ST_STR_LEN * ENGLISH_6X12_TEXT_W);
		LcdDrawEnglish6x12Str(u8x, u8y, EN_COLON_STR, TFT_STATIC_TEXT_COLOR, TFT_COLOR_BLACK);
	}
#endif


}


/**
 *******************************************************************************
 ** \brief  主界面芯片连接状态
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                   chip |    |PANCHIP              chip |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |                               |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceChipConnectState(void)
{
	char str[CHIP_CON_MD_STR_LEN + 2] = "chip";
	uint8_t u8x = 0, u8y = 0, u8Idx = 0;
	uint16_t u16ScreenW = 0, u16Color = TFT_COLOR_RED;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	/* 芯片连接状态 ST: x: w - 4*6 - 1, y:1, h:12 */
	u8x = u16ScreenW - (4 * ENGLISH_6X12_TEXT_W) - 1;
	u8y = 1;

	u8Idx = (mainFaceParam.state & 0x0F);
	if(u8Idx < CHIP_CON_MD_STR_NUM){
		strcpy(str, chipConModeStr[u8Idx]);
	}

	if(mainFaceParam.state >> 4){
		u16Color = TFT_COLOR_GREEN;
	}
	LcdDrawEnglish6x12Str(u8x, u8y, str, u16Color, TFT_COLOR_BLUE);
}


/**
 *******************************************************************************
 ** \brief  主界面绘制显示十进制数字显示
 **
 ** \param  [in] u8x 显示数据起始横向坐标
 **
 ** \param  [in] u8y 显示数据起始纵向坐标
 **
 ** \param  [in] u8Num 显示数据字符个数
 **
 ** \param  [in] u16Color 显示数据字符颜色
 **
 ** \param  [in] u16BkColor 显示数据字符背景颜色
 **
 ** \param  [in] u32Data 显示数据
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceViewDecData(uint8_t u8x, uint8_t u8y, uint8_t u8Num,
	uint16_t u16Color, uint16_t u16BkColor,
	uint32_t u32Data)
{
	char str[4];
	uint8_t u8i = 0;
	uint32_t u32Td = 0, u32Mul = 1;

	for(u8i = 0; u8i + 1 < u8Num; u8i++){
		u32Mul *= 10;
	}

	for(u8i = 0; u8i < u8Num; u8i++){
		if(u32Data >= u32Mul){
			u32Td = u32Data / u32Mul;
			u32Td %= 10;
			sprintk(str, "%.1d", u32Td);
		}
		else{
			if(u8i + 1 == u8Num){
				str[0] = '0';
			}
			else{
				str[0] = ' ';
			}
			str[1] = 0;
		}
		LcdDrawEnglish6x12Str(u8x, u8y, str, u16Color, u16BkColor);
		u8x += ENGLISH_6X12_TEXT_W;
		u32Mul /= 10;
	}
}


/**
 *******************************************************************************
 ** \brief  主界面背景绘制显示
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                        |    |PANCHIP                   |
 ** ---------------------------------    ----------------------------
 ** | 程序总数: 000        当前程序:        |    | 程序总数: 000                |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |                               |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceProjectNums(void)
{
	uint8_t u8x = 0, u8y = 0;

	/* 程序总数 ST: x = 2, y:15, 12 */
	u8x = 2 + (PROJECT_ST_STR_LEN * ENGLISH_6X12_TEXT_W) + ENGLISH_6X12_TEXT_W;
	u8y = 15;

	MainFaceViewDecData(u8x, u8y, 3, TFT_COLOR_YELLOW, TFT_COLOR_BLACK, mainFaceParam.num);
}


/**
 *******************************************************************************
 ** \brief  主界面显示当前程序
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                        |    |PANCHIP                   |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:000     |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序: 000                |
 ** ---------------------------------    | CRC:                     |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |                               |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceProjectSel(void)
{
	uint8_t u8x = 0, u8y = 0;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	uint16_t u16ScreenW = 0;
	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	/* 当前程序 ST: x:(u16ScreenW / 2) y:15, h:12 */
	u8x = (u16ScreenW / 2);
	u8y = 15;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	uint16_t u16ScreenW = 0;
	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	/* 当前程序 ST: x:(u16ScreenW / 2) y:15, h:12 */
	u8x = (u16ScreenW / 2);
	u8y = 15;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 程序总数，当前程序 ST: x = 2, y:28, h = 12 */
	u8x = 2;
	u8y = 28;
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 程序总数，当前程序 ST: x = 2, y:28, h = 12 */
	u8x = 2;
	u8y = 28;
#endif

	u8x += ((PROJECT_ST_STR_LEN + 1) * ENGLISH_6X12_TEXT_W);
	MainFaceViewDecData(u8x, u8y, 3, TFT_COLOR_YELLOW, TFT_COLOR_BLACK, mainFaceParam.sel);
}


/**
 *******************************************************************************
 ** \brief  主界面显示程序CRC
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                        |    |PANCHIP                   |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:0x00000000                |    | 当前程序:                    |
 ** ---------------------------------    | CRC:0x00000000           |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |                               |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceProjectCrc(void)
{
	char str[12];
	uint8_t u8x = 0, u8y = 0;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* CRC ST: x:2 y:28, h:12 */
	u8x = 2; u8y = 28;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* CRC ST: x:2 y:28, h:12 */
	u8x = 2; u8y = 28;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* CRC ST: x:2 y:41, h:12 */
	u8x = 2; u8y = 41;
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* CRC ST: x:2 y:41, h:12 */
	u8x = 2; u8y = 41;
#endif
	u8x += ((PROJECT_STR_LEN + PROJECT_CRC_STR_LEN + 1) * ENGLISH_6X12_TEXT_W);
	/* CRC data */
	sprintk(str, "0x%.8X", mainFaceParam.crc);
	LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
}


/**
 *******************************************************************************
 ** \brief  主界面显示显示项目程序名
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                        |    |PANCHIP                   |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** | Project name                  |    ----------------------------
 ** ---------------------------------    | Project name             |
 ** |                               |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceProjectName(void)
{
	char str[28];
	uint8_t u8x = 0, u8y = 0, u8w = 0, u8h = 0;
	uint16_t u16ScreenW = 0;
	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();

	memcpy(str, mainFaceParam.name, 26);
	str[26] = 0;
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 项目程序名，ST: x:2, y:43, w:156 h:1 + 12 + 1 + 1 */
	u8x = 2; u8y = 43;
	/* 先清除显示 */
	u8w = u16ScreenW - 4;
	u8h = 12;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLACK);
	/* 显示项目名 */
	LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 项目程序名，ST: x:2, y:43, w:156 h:1 + 12 + 1 + 1 */
	u8x = 2; u8y = 43;
	/* 先清除显示 */
	u8w = u16ScreenW - 4;
	u8h = 12;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLACK);
	/* 显示项目名 */
	LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	uint8_t u8OnceLineNum = 0, u8Len = 0;

	/* 项目程序名，ST: x:2, y:56, w:124 h:1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 2; u8y = 56;
	/* 先清除显示 */
	u8w = u16ScreenW - 4;
	u8h = 25;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLACK);
	/* 显示项目名 */
	u8Len = strlen(str);
	u8OnceLineNum = u8w / ENGLISH_6X12_TEXT_W;
	if(u8OnceLineNum >= u8Len){
		LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
	}
	else{
		memcpy(str, mainFaceParam.name, u8OnceLineNum);
		str[u8OnceLineNum] = 0;
		LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
		u8y += 13;
		u8Len -= u8OnceLineNum;
		memcpy(str, mainFaceParam.name + u8OnceLineNum, u8Len);
		str[u8Len] = 0;
		LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
	}
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	uint8_t u8OnceLineNum = 0, u8Len = 0;

	/* 项目程序名，ST: x:2, y:56, w:124 h:1 + 12 + 1 + 12 + 1 + 1*/
	u8x = 2; u8y = 56;
	/* 先清除显示 */
	u8w = u16ScreenW - 4;
	u8h = 25;
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLACK);
	/* 显示项目名 */
	u8Len = strlen(str);
	u8OnceLineNum = u8w / ENGLISH_6X12_TEXT_W;
	if(u8OnceLineNum >= u8Len){
		LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
	}
	else{
		memcpy(str, mainFaceParam.name, u8OnceLineNum);
		str[u8OnceLineNum] = 0;
		LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
		u8y += 13;
		u8Len -= u8OnceLineNum;
		memcpy(str, mainFaceParam.name + u8OnceLineNum, u8Len);
		str[u8Len] = 0;
		LcdDrawEnglish6x12Str(u8x, u8y, str, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
	}
#endif

}


/**
 *******************************************************************************
 ** \brief  主界面显示提示
 ** ---------------------------------    ----------------------------
 ** |PANCHIP                        |    |PANCHIP                   |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** |                               |    ----------------------------
 ** ---------------------------------    |                          |
 ** |   烧录模式                        |    |                          |
 ** |                               |    ----------------------------
 ** ---------------------------------    |  烧录模式                    |
 ** |       | 剩余数量:                 |    |                          |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |                               |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      |                          |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceViewMode(void)
{
	char buff[MAIN_FACE_MODE_MAX_LEN + 1] = {0};
	uint8_t u8x = 0, u8y = 0, u8w = 0, u8h = 0, u8Len = 0;
	uint16_t u16ScreenW = 0, u16Color = TFT_COLOR_GRAPE;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	u8x = 2;
	u8w = u16ScreenW - 4;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 模式显示，ST: x:0, y:57, 25 */
	u8y = 57;
	u8h = 25;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 模式显示，ST: x:0, y:57, 25 */
	u8y = 57;
	u8h = 25;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 模式显示，ST: x:0, y:83, 31 */
	u8y = 83;
	u8h = 31;
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 模式显示，ST: x:0, y:83, 31 */
	u8y = 83;
	u8h = 31;
#endif
	u8x = 2;

	/* 先清除显示 */
	TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLACK);
	/* 设置显示 */
	switch(mainFaceParam.mode){
	case MAIN_FACE_TEST_MODE:		// 测试模式
		if(languageView){ // English
			strcpy(buff, TEST_MODE_EN_STR);
		}
		else{ // Chinese

			strcpy(buff, TEST_MODE_CN_STR);
		}
		break;
	case MAIN_FACE_SEL_PJ_MODE:		// 选择项目程序模式
		if(languageView){ // English
			strcpy(buff, SEL_PJ_MODE_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, SEL_PJ_MODE_CN_STR);
		}
		break;
	case MAIN_FACE_SEL_CON_MODE:	// 设置连接模式模式
		if(languageView){ // English
			strcpy(buff, SEL_CON_MODE_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, SEL_CON_MODE_CN_STR);
		}
		break;
	case MAIN_FACE_SEL_LANGUAGE_MODE: // 语言模式
		if(languageView){ // English
			strcpy(buff, SEL_LANGUAGE_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, SEL_LANGUAGE_CN_STR);
		}
		break;
	case MAIN_FACE_TESTING:			// 测试中
		if(languageView){ // English
			strcpy(buff, TESTING_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, TESTING_CN_STR);
		}
		u16Color = TFT_COLOR_YELLOW;
		break;
	case MAIN_FACE_TEST_SUCCESS:	// 测试成功
		if(languageView){ // English
			strcpy(buff, TEST_SUCCESS_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, TEST_SUCCESS_CN_STR);
		}
		u16Color = TFT_COLOR_GREEN;
		break;
	case MAIN_FACE_TEST_FAIL:		// 测试失败
		if(languageView){ // English
			strcpy(buff, TEST_FAIL_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, TEST_FAIL_CN_STR);
		}
		u16Color = TFT_COLOR_RED;
		break;
	case MAIN_FACE_OFF_PJ_LACK:		// 离线下载次数不足提示
		if(languageView){ // English
			strcpy(buff, OFF_PJ_LACK_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, OFF_PJ_LACK_CN_STR);
		}
		u16Color = TFT_COLOR_RED;
		break;
	case MAIN_FACE_OFF_CFG_LACK:	// 离线下载配置不足提示
		if(languageView){ // English
			strcpy(buff, OFF_CFG_LACK_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, OFF_CFG_LACK_CN_STR);
		}
		u16Color = TFT_COLOR_RED;
		break;
	case MAIN_FACE_OFF_CFG_REUSE:	// 离线下载配置重复提示
		if(languageView){ // English
			strcpy(buff, OFF_CFG_REUSE_EN_STR);
		}
		else{ // Chinese
			strcpy(buff, OFF_CFG_REUSE_CN_STR);
		}
		u16Color = TFT_COLOR_RED;
		break;
	}

	u8Len = strlen(buff);
	if(languageView){ // English
		u8x += ((u8w - u8Len * ENGLISH_8X16_TEXT_W) / 2);
		u8y += ((u8h - ENGLISH_8X16_TEXT_H) / 2);
		if(u8h % 2){
			u8y += 1;
		}
		LcdDrawEnglish8x16Str(u8x, u8y, buff, u16Color, TFT_COLOR_BLACK);
	}
	else{ // Chinese
		u8Len /= 2;
		if(mainFaceParam.mode == MAIN_FACE_TESTING){
			u8x += ((u8w - u8Len * CHINESE_16X16_TEXT_W - 4 * ENGLISH_8X16_TEXT_W) / 2);
		}
		else{
			u8x += ((u8w - u8Len * CHINESE_16X16_TEXT_W) / 2);
		}
		u8y += ((u8h - CHINESE_16X16_TEXT_H) / 2);
		if(u8h % 2){
			u8y += 1;
		}
		LcdDrawChinese16x16Str(u8x, u8y, buff, u16Color, TFT_COLOR_BLACK);
		if(mainFaceParam.mode == MAIN_FACE_TESTING){
			u8x += (u8Len * CHINESE_16X16_TEXT_W);
			LcdDrawEnglish8x16Str(u8x, u8y, " ...", u16Color, TFT_COLOR_BLACK);
		}
	}
}


/**
 *******************************************************************************
 ** \brief  主界面显示剩余数量
 ** ---------------------------------    ----------------------------
 ** |PANCHIP           TWI/UART/SPI |    |PANCHIP      TWI/UART/SPI |
 ** ---------------------------------    ----------------------------
 ** | 程序总数: 000    当前程序: 000|            | 程序总数: 000                |
 ** | CRC: 0x00000000               |    | 当前程序: 000         cfg    |
 ** ---------------------------------    | CRC: 0x00000000   cfg1   |
 ** | 项目程序名                         |    ----------------------------
 ** ---------------------------------    |  项目名                     |
 ** |                               |    |                          |
 ** |          测试模式                 |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:     0000000     |    |        测试模式              |
 ** | CFG   | 使用数量: 0000000         |    ----------------------------
 ** | CFG1  | 脱机总数:                 |    |    | 剩余数量:     0000000   |
 ** ---------------------------------    |    | 使用数量: 0000000       |
 ** |进度显示                           |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      | 进度条                      |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceOfflineRemainAndUsed(void)
{
	uint8_t u8x = 0, u8y = 0, u8w = 0, u8h = 0;
	uint16_t u16ScreenW = 0;
	uint32_t u32Remain = 0, u32Used = 0;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 离线次数 剩余次数，ST: x: <-, y:84, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 84;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 离线次数 剩余次数，ST: x: <-, y:84, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 84;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 离线次数 剩余次数，ST: x: <-, y:116, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 116;
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 离线次数 剩余次数，ST: x: <-, y:116, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 116;
#endif
	if(mainFaceParam.times > mainFaceParam.used){
		u32Remain = mainFaceParam.times - mainFaceParam.used;
		u32Used = mainFaceParam.used;
	}
	else{
		u32Used = mainFaceParam.times;
	}
	/* 离线次数 剩余次数 */
	MainFaceViewDecData(u8x, u8y, LIMIT_NUM_STR_LEN, TFT_COLOR_YELLOW, TFT_COLOR_BLACK, u32Remain);
	/* 离线次数 使用次数 */
	u8y += 13;
	MainFaceViewDecData(u8x, u8y, LIMIT_NUM_STR_LEN, TFT_COLOR_YELLOW, TFT_COLOR_BLACK, u32Used);
	/* 离线次数 剩余次数 */
	u8y -= 13; u8h = 38;
	u8w = u8x - 5 - (LIMIT_ST_STR_LEN + 1) * ENGLISH_6X12_TEXT_W;
	u8x = 2;
	if(mainFaceParam.times){
		TftLcdDrawPercent(u8x, u8y, u8w, u8h, 2,
			TFT_COLOR_GREEN, TFT_COLOR_BLACK,
			u32Remain, mainFaceParam.times);
	}
	else{
		TftLcdDrawRange(u8x, u8y, u8w, u8h, TFT_COLOR_BLACK);
	}
}


/**
 *******************************************************************************
 ** \brief  主界面显示脱机总数
 ** ---------------------------------    ----------------------------
 ** |PANCHIP           TWI/UART/SPI |    |PANCHIP      TWI/UART/SPI |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** | 项目程序名                         |    ----------------------------
 ** ---------------------------------    |  项目名                     |
 ** |                               |    |                          |
 ** |          测试模式                 |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |        测试模式              |
 ** | CFG   | 使用数量:                 |    ----------------------------
 ** | CFG1  | 脱机总数: 0000000         |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |进度显示                           |    |    | 脱机总数: 0000000       |
 ** ---------------------------------    ----------------------------
 **                                      | 进度条                      |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceOfflineTotal(void)
{
	uint8_t u8x = 0, u8y = 0;
	uint16_t u16ScreenW = 0;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 离线次数 脱机总数，ST: x: <-, y:110, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 110;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 离线次数 脱机总数，ST: x: <-, y:110, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 110;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 离线次数 脱机总数，ST: x: <-, y:142, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 142;
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 离线次数 脱机总数，ST: x: <-, y:142, 12 */
	u8x = u16ScreenW - 2 - (LIMIT_NUM_STR_LEN * ENGLISH_6X12_TEXT_W);
	u8y = 142;
#endif

	/* 离线次数 脱机总数 */
	MainFaceViewDecData(u8x, u8y, LIMIT_NUM_STR_LEN, TFT_COLOR_YELLOW, TFT_COLOR_BLACK, mainFaceParam.times);
}


/**
 *******************************************************************************
 ** \brief  主界面显示进度条
 ** ---------------------------------    ----------------------------
 ** |PANCHIP           TWI/UART/SPI |    |PANCHIP      TWI/UART/SPI |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:                    |
 ** ---------------------------------    | CRC:                     |
 ** | 项目程序名                         |    ----------------------------
 ** ---------------------------------    |  项目名                     |
 ** |                               |    |                          |
 ** |          测试模式                 |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |        测试模式              |
 ** |       | 使用数量:                 |    ----------------------------
 ** |       | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |进度显示                           |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      | 进度条                      |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceProgress(void)
{
	uint8_t u8x = 0, u8y = 0, u8w = 0, u8h = 0;
	uint16_t u16ScreenW = 0;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();
	/* 进度条，ST: x:1, h:3, w: w - 2 */
	u8x = 1;
	u8w = u16ScreenW - 2;
	u8h = 3;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 进度条，ST: y:124 */
	u8y = 124;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 进度条，ST: y:124 */
	u8y = 124;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 进度条，ST: y:156 */
	u8y = 156;
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 进度条，ST: y:156 */
	u8y = 156;
#endif

	/* 进度条显示 */
	TftLcdDrawPercent(u8x, u8y, u8w, u8h, 1,
		TFT_COLOR_GREEN, TFT_COLOR_BLACK,
		mainFaceParam.progress, 100);
}


/**
 *******************************************************************************
 ** \brief  主界面显示配置
 ** ---------------------------------    ----------------------------
 ** |PANCHIP           TWI/UART/SPI |    |PANCHIP      TWI/UART/SPI |
 ** ---------------------------------    ----------------------------
 ** | 程序总数:            当前程序:        |    | 程序总数:                    |
 ** | CRC:                          |    | 当前程序:             cfg    |
 ** ---------------------------------    | CRC:              cfg1   |
 ** | 项目程序名                         |    ----------------------------
 ** ---------------------------------    |  项目名                     |
 ** |                               |    |                          |
 ** |          测试模式                 |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:                 |    |        测试模式              |
 ** | CFG   | 使用数量:                 |    ----------------------------
 ** | CFG1  | 脱机总数:                 |    |    | 剩余数量:               |
 ** ---------------------------------    |    | 使用数量:               |
 ** |进度显示                           |    |    | 脱机总数:               |
 ** ---------------------------------    ----------------------------
 **                                      | 进度条                      |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceConfig(void)
{
	uint8_t u8x = 0, u8y = 0, u8w = 0;
	uint16_t u16ScreenW = 0;
	uint16_t u16Color = TFT_COLOR_PURPLE, u16BkColor = TFT_COLOR_WHITE;

	/* 获取 LCD 屏幕显示尺寸 */
	u16ScreenW = TftLcdScreenWidth();

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	/* 配置 ，ST: x: 2, y:90 */
	u8w = u16ScreenW - 7 - (LIMIT_ST_STR_LEN + LIMIT_NUM_STR_LEN + 1) * ENGLISH_6X12_TEXT_W;
	u8x = MAIN_FACE_CFG_STR_LEN * ENGLISH_6X12_TEXT_W;
	if(u8w > u8x){
		u8x = (u8w - u8x) / 2;
	}
	else{
		u8x = 0;
	}
	u8x += 2;
	u8y = 90;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	/* 配置 ，ST: x: 2, y:90 */
	u8w = u16ScreenW - 7 - (LIMIT_ST_STR_LEN + LIMIT_NUM_STR_LEN + 1) * ENGLISH_6X12_TEXT_W;
	u8x = MAIN_FACE_CFG_STR_LEN * ENGLISH_6X12_TEXT_W;
	if(u8w > u8x){
		u8x = (u8w - u8x) / 2;
	}
	else{
		u8x = 0;
	}
	u8x += 2;
	u8y = 90;
#endif

// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	/* 配置 ST: w:MAIN_FACE_CFG_STR_LEN * ENGLISH_6X12_TEXT_W x = sw - w - 2, y:15 */
	u8w = MAIN_FACE_CFG_STR_LEN * ENGLISH_6X12_TEXT_W;
	u8x = u16ScreenW - u8w - 2;
	u8y = 15;
	/* 配置清除显示       */
	if(!(mainFaceParam.cfgState & 0x01)){
		TftLcdDrawRange(u8x, u8y, u8w, ENGLISH_6X12_TEXT_H, TFT_COLOR_BLACK);
	}
	/* 配置1清除显示       */
	if(!(mainFaceParam.cfgState & 0x02)){
		TftLcdDrawRange(u8x, u8y + ENGLISH_6X12_TEXT_H + 1, u8w, ENGLISH_6X12_TEXT_H, TFT_COLOR_BLACK);
	}
#endif

// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	/* 配置 ST: w:MAIN_FACE_CFG_STR_LEN * ENGLISH_6X12_TEXT_W x = sw - w - 2, y:15 */
	u8w = MAIN_FACE_CFG_STR_LEN * ENGLISH_6X12_TEXT_W;
	u8x = u16ScreenW - u8w - 2;
	u8y = 15;
	/* 配置清除显示       */
	if(!(mainFaceParam.cfgState & 0x01)){
		TftLcdDrawRange(u8x, u8y, u8w, ENGLISH_6X12_TEXT_H, TFT_COLOR_BLACK);
	}
	/* 配置1清除显示       */
	if(!(mainFaceParam.cfgState & 0x02)){
		TftLcdDrawRange(u8x, u8y + ENGLISH_6X12_TEXT_H + 1, u8w, ENGLISH_6X12_TEXT_H, TFT_COLOR_BLACK);
	}
#endif
	/* 配置 ，h:25 */
	u8w = 25;
	//     1: 只显示cfgNum
	//     2: 只显示cfg1Num,
	//     3: 显示 cfgNum 与 cfg1Num

	/* 配置   */
	if(mainFaceParam.cfgState & 0x01){
		MainFaceViewDecData(u8x, u8y, MAIN_FACE_CFG_STR_LEN, u16Color, u16BkColor, mainFaceParam.cfgNum);
	}
	/* 配置1   */
	if(mainFaceParam.cfgState & 0x02){
		u8y += 13;
		MainFaceViewDecData(u8x, u8y, MAIN_FACE_CFG_STR_LEN, u16Color, u16BkColor, mainFaceParam.cfg1Num);
	}
}


/**
 *******************************************************************************
 ** \brief  主界面显示
 ** ---------------------------------    ----------------------------
 ** |PANCHIP           TWI/UART/SPI |    |PANCHIP      TWI/UART/SPI |
 ** ---------------------------------    ----------------------------
 ** | 程序总数: 000    当前程序: 000|            | 程序总数: 000                |
 ** | CRC: 0x00000000               |    | 当前程序: 000         cfg    |
 ** ---------------------------------    | CRC: 0x00000000   cfg1   |
 ** | 项目程序名                         |    ----------------------------
 ** ---------------------------------    |  项目名                     |
 ** |                               |    |                          |
 ** |          测试模式                 |    ----------------------------
 ** ---------------------------------    |                          |
 ** |       | 剩余数量:     0000000     |    |        测试模式              |
 ** | CFG   | 使用数量: 0000000         |    ----------------------------
 ** | CFG1  | 脱机总数: 0000000         |    |    | 剩余数量:     0000000   |
 ** ---------------------------------    |    | 使用数量: 0000000       |
 ** |进度显示                           |    |    | 脱机总数: 0000000       |
 ** ---------------------------------    ----------------------------
 **                                      | 进度条                      |
 **                                      ----------------------------
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void MainFaceView(void)
{
	/* LCD 显示主界面界面背景刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_BKG){
		MainFaceDrawBk();
	}

	/* LCD 显示主界面界面设备连接状态刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_CON_STATE){
		MainFaceChipConnectState();
	}

	/* LCD 显示主界面界面项目程序个数刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_PG_NUM){
		MainFaceProjectNums();
	}

	/* LCD 显示主界面界面选择项目号刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_PG_SEL){
		MainFaceProjectSel();
	}

	/* LCD 显示主界面界面项目程序 CRC 刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_PG_CRC){
		MainFaceProjectCrc();
	}

	/* LCD 显示主界面界面项目名刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_PG_NAME){
		MainFaceProjectName();
	}

	/* LCD 显示主界面界面测试模式与测试状态刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_T_MODE){
		MainFaceViewMode();
	}

	/* LCD 显示主界面界面离线下载程序剩余与使用个数刷新 */
	if(lcdViewReflash & (LCD_MAIN_FACE_OFF_USE | LCD_MAIN_FACE_OFF_NUM)){
		MainFaceOfflineRemainAndUsed();

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
		/* 界面离线配置剩余个数刷新   */
		lcdViewReflash |= LCD_MAIN_FACE_CFG_NUM;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
		/* 界面离线配置剩余个数刷新   */
		lcdViewReflash |= LCD_MAIN_FACE_CFG_NUM;
#endif
	}

	/* LCD 显示主界面界面离线下载程序总数刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_OFF_NUM){
		MainFaceOfflineTotal();
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
		/* 界面离线配置剩余个数刷新   */
		lcdViewReflash |= LCD_MAIN_FACE_CFG_NUM;
#endif

// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
		/* 界面离线配置剩余个数刷新   */
		lcdViewReflash |= LCD_MAIN_FACE_CFG_NUM;
#endif
	}

	/* LCD 显示主界面界面进度条刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_PROGRESS){
		MainFaceProgress();
	}

	/* LCD 显示主界面界面离线配置剩余个数刷新 */
	if(lcdViewReflash & LCD_MAIN_FACE_CFG_NUM){
		if(mainFaceParam.cfgState){
			MainFaceConfig();
		}
	}
}


/**
 *******************************************************************************
 ** \brief  控制Lcd显示
 **
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void DoLcdView(void)
{
	if(lcdViewReflash != LCD_NO_REFLASH){
		if(lcdViewFace == LCD_START_FACE){
			if(lcdViewReflash == LCD_REFLASH_ALL){
				StartFaceView();
				lcdStartViewCont = START_FACE_VIEW_WAIT;
			}
		}
		else{
			MainFaceView();
		}
		lcdViewReflash = LCD_NO_REFLASH;
	}
}


/**
 *******************************************************************************
 ** \brief  设置语言显示
 **
 ** \param  [in] u8Lg 语言模式，0: 中文，1: English
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetViewLanguage(uint8_t u8Lg)
{
	if(languageView != u8Lg){
		languageView = u8Lg;
		lcdViewReflash = LCD_REFLASH_ALL;
	}
}


/**
 *******************************************************************************
 ** \brief  设置语言显示
 **
 ** \param  [in] u8Lg 语言模式，0: 中文，1: English
 **
 ** \retval 无
 **
 ******************************************************************************/
void ChangeLanguageView(void)
{
	uint8_t u8Lg = languageView;

	u8Lg ++;
	if(u8Lg > 1){
		u8Lg = 0;
	}
	languageView = u8Lg;
	lcdViewReflash = LCD_REFLASH_ALL;
}


/**
 *******************************************************************************
 ** \brief  设置连接显示
 **
 ** \param  [in] u8Lg 语言模式，0: 中文，1: English
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetViewConnect(uint8_t u8Md, uint8_t u8Param)
{
	uint8_t u8Mode = 0;

	u8Mode = u8Param;
	if(mainFaceParam.state != u8Mode){
		mainFaceParam.state = u8Mode;
		lcdViewReflash |= LCD_MAIN_FACE_CON_STATE;
	}
}


/**
 *******************************************************************************
 ** \brief  设置本版显示
 **
 ** \param  [in] pU8Version 传入的 5 个字节版本数据
 **              [0]: ([0] >> 4) + '0', ([0] 0x0F) + '0'
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetVersionView(uint8_t *pU8Version)
{
	uint8_t u8i = 0;

	if(pU8Version){
		for(u8i = 0; u8i < 5; u8i++){
			plinkVerion[2 * u8i] = '0';
			plinkVerion[2 * u8i] += (pU8Version[u8i] >> 4);
			plinkVerion[2 * u8i + 1] = '0';
			plinkVerion[2 * u8i + 1] += (pU8Version[u8i] & 0x0F);
		}
		plinkVerion[2 * u8i] = 0;
	}
}


/**
 *******************************************************************************
 ** \brief  设置却换到开始界面显示
 **
 ** \param  无
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetToStartFaceView(void)
{
	if(lcdViewFace != LCD_START_FACE){
		lcdViewFace = LCD_START_FACE;
		lcdViewReflash = LCD_REFLASH_ALL;
	}
}


/**
 *******************************************************************************
 ** \brief  设置芯片连接显示
 **
 ** \param  [in] u8State [3:0]: 通讯模式, [7:4]: 连接状态
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetChipConnectView(uint8_t u8State)
{
	if(mainFaceParam.state != u8State){
		mainFaceParam.state = u8State;
		lcdViewReflash |= LCD_MAIN_FACE_CON_STATE;
	}
}


/**
 *******************************************************************************
 ** \brief  设置芯片连接状态显示
 **
 ** \param  [in] u8State 连接状态 1: 连接成功，0: 连接失败
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetChipConnectStateView(uint8_t u8State)
{
	uint8_t u8Status = 0;

	u8Status = (mainFaceParam.state >> 4);
	if(u8State){
		if(u8Status == 0){
			mainFaceParam.state += (1 << 4);
			lcdViewReflash |= LCD_MAIN_FACE_CON_STATE;
		}
	}
	else{
		if(u8Status){
			mainFaceParam.state &= 0x0F;
			lcdViewReflash |= LCD_MAIN_FACE_CON_STATE;
		}
	}
}


/**
 *******************************************************************************
 ** \brief  设置芯片模式显示
 **
 ** \param  [in] u8State [3:0]: 通讯模式, [7:4]: 连接状态
 **
 ** \retval 无
 **
 ******************************************************************************/
void ChangeChipConnectModeView(uint8_t u8Next)
{
	uint8_t u8State = 0;

	if(CHIP_CON_MD_STR_NUM > 1){
		u8State = mainFaceParam.state;
		u8State &= 0x0F;
		if(u8Next){
			/* 却换下一种模式 */
			if(u8State + 1 < CHIP_CON_MD_STR_NUM){
				u8State ++;
			}
			else{
				u8State = 0;
			}
		}
		else{
			/* 却换上一种模式 */
			if(u8State > 0){
				u8State --;
			}
			else{
				u8State = CHIP_CON_MD_STR_NUM - 1;
			}
		}
		SetChipConnectView(u8State);
	}
}


/**
 *******************************************************************************
 ** \brief  设置项目程序个数显示
 **
 ** \param  [in] u16Num 项目程序个数
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetProjectNumsView(uint16_t u16Num)
{
	if(mainFaceParam.num != u16Num){
		mainFaceParam.num = u16Num;
		lcdViewReflash |= LCD_MAIN_FACE_PG_NUM;
	}
}


/**
 *******************************************************************************
 ** \brief  设置选中项目程序 ID 显示
 **
 ** \param  [in] u16Sel 项目程序个数
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetProjectSelView(uint16_t u16Sel)
{
	if(mainFaceParam.sel != u16Sel){
		mainFaceParam.sel = u16Sel;
		lcdViewReflash |= LCD_MAIN_FACE_PG_SEL;
	}
}


/**
 *******************************************************************************
 ** \brief  设置项目程序 CRC 显示
 **
 ** \param  [in] u32Crc CRC 值
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetProjectCrcView(uint32_t u32Crc)
{
	if(mainFaceParam.crc != u32Crc){
		mainFaceParam.crc = u32Crc;
		lcdViewReflash |= LCD_MAIN_FACE_PG_CRC;
	}
}


/**
 *******************************************************************************
 ** \brief  设置项目程序名显示
 **
 ** \param  [in] pU8Name 传入项目名数据指针
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetProjectNameView(uint8_t *pU8Name)
{
	uint8_t u8i = 0;

	if(pU8Name){
		for(u8i = 0; u8i < 26; u8i++){
			if(pU8Name[u8i] != mainFaceParam.name[u8i]){
				memcpy(mainFaceParam.name, pU8Name, 26);
				lcdViewReflash |= LCD_MAIN_FACE_PG_NAME;
				break;
			}
			else{
				if(pU8Name[u8i] == 0){
					break;
				}
			}
		}
	}
}


/**
 *******************************************************************************
 ** \brief  设置显示模式显示
 **
 ** \param  [in] mode
 ** 				MAIN_FACE_TEST_MODE,		// 测试模式
 ** 				MAIN_FACE_SEL_PJ_MODE,		// 选择项目程序模式
 ** 				MAIN_FACE_SEL_CON_MODE,		// 设置连接模式模式
 ** 				MAIN_FACE_TESTING,			// 测试中
 ** 				MAIN_FACE_TEST_SUCCESS,		// 测试成功
 ** 				MAIN_FACE_TEST_FAIL,		// 测试失败
 ** 				MAIN_FACE_OFF_PJ_LACK,		// 离线下载次数不足提示
 ** 				MAIN_FACE_OFF_CFG_LACK,		// 离线下载配置不足提示
 ** 				MAIN_FACE_OFF_CFG_REUSE		// 离线下载配置重复提示
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetModeView(MAIN_FACE_MODE mode)
{
	if(mainFaceParam.mode != mode){
		mainFaceParam.mode = mode;
		lcdViewReflash |= LCD_MAIN_FACE_T_MODE;
	}
}


/**
 *******************************************************************************
 ** \brief  设置离线烧录总数显示
 **
 ** \param  [in] u32Times 离线总数
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetOfflineTimesView(uint32_t u32Times)
{
	if(mainFaceParam.times != u32Times){
		mainFaceParam.times = u32Times;
		lcdViewReflash |= LCD_MAIN_FACE_OFF_NUM;
	}
}


/**
 *******************************************************************************
 ** \brief  设置离线烧录使用显示
 **
 ** \param  [in] u32Used 使用次数
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetOfflineUsedView(uint32_t u32Used)
{
	if(mainFaceParam.used != u32Used){
		mainFaceParam.used = u32Used;
		lcdViewReflash |= LCD_MAIN_FACE_OFF_USE;
	}
}


/**
 *******************************************************************************
 ** \brief  设置进度条显示
 **
 ** \param  [in] u8Percent 进度参数 0 ~ 100
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetProgressView(uint8_t u8Percent)
{
	if(u8Percent > 100){
		u8Percent = 100;
	}
	if(mainFaceParam.progress != u8Percent){
		mainFaceParam.progress = u8Percent;
		lcdViewReflash |= LCD_MAIN_FACE_PROGRESS;
	}
}


/**
 *******************************************************************************
 ** \brief  设置进度条显示
 **
 ** \param  [in] u32Range 总大小值
 **
 ** \param  [in] u32Idx 索引值
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetViewProgress(uint32_t u32Range, uint32_t u32Idx)
{
	uint8_t u8Percent = 0;

	for(; u32Range > 0x2000000;){
		u32Range /= 10;
		if(u32Idx){
			u32Idx /= 10;
		}
	}
	if(u32Idx){
		u32Idx *= 100;
		u32Idx /= u32Range;
		u8Percent = u32Idx;
	}
	SetProgressView(u8Percent);
}


/**
 *******************************************************************************
 ** \brief  设置进度条马上刷新显示
 **
 ** \param  [in] u8Percent 百分比值
 **
 ** \retval 无
 **
 ******************************************************************************/
void SetViewProgressRealTime(uint8_t u8Percent)
{
	if(u8Percent > 100){
		u8Percent = 100;
	}
	if(mainFaceParam.progress != u8Percent){
		mainFaceParam.progress = u8Percent;
		MainFaceProgress();
	}
}


/**
 *******************************************************************************
 ** \brief  设置配置显示
 **
 ** \param  [in] u8State 配置显示状态
 **               0: 不显示
 **             [0]: 1:显示cfgNum, 0: 不显示
 **             [1]: 1:显示cfgNum1, 0: 不显示
 ** \retval 无
 **
 ******************************************************************************/
void SetConfigView(uint8_t u8State, uint32_t u32Num, uint32_t u32Num1)
{
	if(mainFaceParam.cfgState != u8State){
		mainFaceParam.cfgState = u8State;
		mainFaceParam.cfgNum = u32Num;
		mainFaceParam.cfg1Num = u32Num1;
		lcdViewReflash |= LCD_MAIN_FACE_CFG_NUM;
	}
	else{
		if(mainFaceParam.cfgNum != u32Num ||
			mainFaceParam.cfg1Num != u32Num1){
			mainFaceParam.cfgNum = u32Num;
			mainFaceParam.cfg1Num = u32Num1;
			lcdViewReflash |= LCD_MAIN_FACE_CFG_NUM;
		}
	}
}


/**
 *******************************************************************************
 ** \brief  检测主界面空闲
 **
 ** \param  无
 **
 ** \retval 0: 不是主界面，1: 主界面空闲可以切换模式，2: 主界面繁忙，不允许切换
 **
 ******************************************************************************/
uint8_t CheckMainFaceFree(void)
{
	uint8_t u8Ret = 0;

	if(lcdViewFace == LCD_MAIN_FACE){
		if(mainFaceParam.mode == MAIN_FACE_TEST_MODE ||
			mainFaceParam.mode == MAIN_FACE_TEST_SUCCESS ||
			mainFaceParam.mode == MAIN_FACE_TEST_FAIL ||
			mainFaceParam.mode == MAIN_FACE_OFF_PJ_LACK ||
			mainFaceParam.mode == MAIN_FACE_OFF_CFG_LACK ||
			mainFaceParam.mode == MAIN_FACE_OFF_CFG_REUSE){
			// free
			u8Ret = 1;
		}
		else if(mainFaceParam.mode == MAIN_FACE_TESTING){
			// busy
			u8Ret = 2;
		}
		else if(mainFaceParam.mode == MAIN_FACE_SEL_PJ_MODE){
			// Sellect Project mode
			u8Ret = 3;
		}
		else if(mainFaceParam.mode == MAIN_FACE_SEL_CON_MODE){
			// Sellect connect mode
			u8Ret = 4;
		}
		else if(mainFaceParam.mode == MAIN_FACE_SEL_LANGUAGE_MODE){
			// Sellect Language mode
			u8Ret = 5;
		}
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief  获取当前显示语言模式
 **
 ** \param  无
 **
 ** \retval 返回当前显示的芯片连接通讯模式
 **
 ******************************************************************************/
uint8_t GetCurrentViewLanguage(void)
{
	return languageView;
}


/**
 *******************************************************************************
 ** \brief  获取当前显示的芯片连接通讯模式
 **
 ** \param  无
 **
 ** \retval 返回当前显示的芯片连接通讯模式
 **
 ******************************************************************************/
uint8_t GetViewChipConnectMode(void)
{
	uint8_t u8Mode = 0;

    u8Mode = (mainFaceParam.state & 0x0F);

	return u8Mode;
}


/**
 *******************************************************************************
 ** \brief  获取当前 LCD 显示模式
 **
 ** \param  无
 **
 ** \retval 返回当前显示的模式
 **
 ******************************************************************************/
uint8_t GetLcdViewMode(void)
{
	return languageView;
}



/**
 *******************************************************************************
 ** \brief  获取当前显示的项目总数
 **
 ** \param  无
 **
 ** \retval 返回当前显示的项目总数
 **
 ******************************************************************************/
uint16_t GetViewProjectNums(void)
{
	return mainFaceParam.num;
}


/**
 *******************************************************************************
 ** \brief  获取当前显示的选中项目 ID
 **
 ** \param  无
 **
 ** \retval 返回当前显示的选中项目 ID
 **
 ******************************************************************************/
uint16_t GetViewProjectSel(void)
{
	return mainFaceParam.sel;
}


/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
