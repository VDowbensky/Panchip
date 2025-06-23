
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "tft_lcd_128x160.h"
#include "PanSeries.h"
#include "pan_gpio.h"
#include "pan_sys.h"

#include "sample_config.h"

extern void delay_us(uint32_t nus);

/**
 *******************************************************************************
 ** \addtogroup TFT_LCD_128x160
 ******************************************************************************/
//@{

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/



/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
 #if CONFIG_BOARD_VERSION == 0
#define LCD_CLK_HIGH() 		(P15 = 1)//CLK
#define LCD_CLK_LOW() 		(P15 = 0)

#define LCD_DO_HIGH() 		(P11 = 1)//DOUT
#define LCD_DO_LOW() 		(P11 = 0)

#define LCD_RST_HIGH()  	(P20 = 1)//RST
#define LCD_RST_LOW()  		(P20 = 0)

#define LCD_DC_HIGH()  		(P05 = 1)//DC
#define LCD_DC_LOW()  		(P05 = 0)
 		     
#define LCD_CS_HIGH()  		(P04 = 1)//CS
#define LCD_CS_LOW()  		(P04 = 0)
#endif

 #if CONFIG_BOARD_VERSION == 1
#define LCD_CLK_HIGH() 		(P04 = 1)//CLK
#define LCD_CLK_LOW() 		(P04 = 0)

#define LCD_DO_HIGH() 		(P11 = 1)//DOUT
#define LCD_DO_LOW() 		(P11 = 0)

#define LCD_RST_HIGH()  	(P20 = 1)//RST
#define LCD_RST_LOW()  		(P20 = 0)

#define LCD_DC_HIGH()  		(P05 = 1)//DC
#define LCD_DC_LOW()  		(P05 = 0)
 		     
#define LCD_CS_HIGH()  		(P15 = 1)//CS
#define LCD_CS_LOW()  		(P15 = 0)
#endif

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 *******************************************************************************
 ** \brief ????? LCD DC ?? RST PIN ?? 
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void InitLcdDcAndRstPin(void)
{
	SYS_SET_MFP(P0, 4, GPIO);
    SYS_SET_MFP(P0, 5, GPIO);
	SYS_SET_MFP(P1, 1, GPIO);
	SYS_SET_MFP(P1, 5, GPIO);
    SYS_SET_MFP(P2, 0, GPIO);

    GPIO_SetMode(P0, BIT4|BIT5, GPIO_MODE_OUTPUT);
    GPIO_SetMode(P1, BIT1|BIT5, GPIO_MODE_OUTPUT);
    GPIO_SetMode(P2, BIT0, GPIO_MODE_OUTPUT);

	LCD_CLK_LOW();
	LCD_DO_LOW();
	LCD_CS_HIGH();
	LCD_RST_HIGH();
}


/**
 *******************************************************************************
 ** \brief ????? TFT LCD 128x160 
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void ResetLcd(void)
{
	/* LCD RST Low */
	LCD_RST_LOW();
	/* Wait 100 ms */
	delay_us(100000);
	/* LCD RST Hight */
	LCD_RST_HIGH(); 
	/* Wait 100 ms */
	delay_us(100000);
}

void SpiWriteReadByte(uint8_t dat)
{	
	uint8_t i;			  
	  
	for(i=0;i<8;i++)
	{			  
		if(dat&0x80)
		   LCD_DO_HIGH();
		else 
		   LCD_DO_LOW();
		LCD_CLK_HIGH();
		LCD_CLK_LOW();
		dat<<=1;   
	}				 		  
} 

/**
 *******************************************************************************
 ** \brief TFT LCD ????????
 **
 ** \param [in] u8Cmd ????????
 **
 ** \retval None
 **
 ******************************************************************************/
void TftLcdWriteCmd(uint8_t u8Cmd)
{
	/* LCD DC Low */
	LCD_DC_LOW();
	/* LCD CS Enable */
	LCD_CS_LOW();
	/* LCD Send Cmd Data */
	SpiWriteReadByte(u8Cmd);
	/* LCD CS Disable */
	LCD_CS_HIGH();
}

/**
 *******************************************************************************
 ** \brief TFT LCD ???? 16 ��????
 **
 ** \param [in] u16Data ????? 16 ��????
 **
 ** \retval None
 **
 ******************************************************************************/
void TftLcdWriteData(uint16_t u16Data)
{
	/* LCD DC HIGH */
	LCD_DC_HIGH();
	/* LCD CS Enable */
	LCD_CS_LOW();
	/* LCD Send [15:8] Data */
	SpiWriteReadByte(u16Data >> 8);
	/* LCD Send [7:0] Data */
	SpiWriteReadByte(u16Data);
	/* LCD CS Disable */
	LCD_CS_HIGH();
}


/**
 *******************************************************************************
 ** \brief TFT LCD ????????
 **
 ** \param [in] pU8Datas ??????????????
 **
 ** \param [in] len ???????????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t TftLcdWriteDatas(uint8_t *pU8Datas, uint16_t len)
{
	uint8_t u8Ret = 0;
	uint16_t i = 0;

	if(pU8Datas && len > 0){
		/* LCD DC HIGH */
		LCD_DC_HIGH();
		/* LCD CS Enable */
		LCD_CS_LOW();
		for(i = 0; i < len; i++){
			/* LCD Send Data */
			SpiWriteReadByte(pU8Datas[i]);
		}
		/* LCD CS Disable */
		LCD_CS_HIGH();

		u8Ret = 1;
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ??????????
 **
 ** \param [in] pU8Datas ??????????????
 **
 ** \param [in] len ???????????
 **
 ** \retval None
 **
 ******************************************************************************/
uint8_t TftLcdSetRegion(uint8_t u8X, uint8_t u8Y, uint8_t u8W, uint8_t u8H)
{
	uint8_t u8Ret = 0;
	uint8_t u8Buf[4];

	if(u8X < TFT_LCD_DEF_X_MAX){
		if(u8Y < TFT_LCD_DEF_Y_MAX){
			if(u8X + u8W > TFT_LCD_DEF_X_MAX){
				u8W = TFT_LCD_DEF_X_MAX - u8X;
			}
			if(u8Y + u8H > TFT_LCD_DEF_Y_MAX){
				u8H = TFT_LCD_DEF_Y_MAX - u8Y;
			}
			// Set X
			u8Buf[0] = 0x02;
			u8Buf[1] = u8X + 2;
			u8Buf[2] = 0x02;
			u8Buf[3] = u8X + u8W + 1;
			TftLcdWriteCmd(0x2A); // Set X Cmd
			TftLcdWriteDatas(u8Buf, 4); // Set X Param
			
			// Set Y
			u8Buf[0] = 0x01;
			u8Buf[1] = u8Y + 1;
			u8Buf[2] = 0x01;
			u8Buf[3] = u8Y + u8H;
			TftLcdWriteCmd(0x2B); // Set Y Cmd
			TftLcdWriteDatas(u8Buf, 4); // Set Y Param

			TftLcdWriteCmd(0x2C);

			u8Ret = 1;
		}
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ???????
 **
 ** \param [in] u16Color ???????
 **
 ** \retval None
 **
 ******************************************************************************/
void TftLcdViewAllColor(uint16_t u16Color)
{
	uint8_t u8x = 0, u8y = 0;
	
	TftLcdSetRegion(u8x, u8y, TFT_LCD_DEF_X_MAX, TFT_LCD_DEF_Y_MAX);
	for(u8y = 0; u8y < TFT_LCD_DEF_Y_MAX; u8y++){
		for(u8x = 0; u8x < TFT_LCD_DEF_X_MAX; u8x++){
			TftLcdWriteData(u16Color);
		}
	}
}


/**
 *******************************************************************************
 ** \brief ??? TFT LCD ??????????
 **
 ** \param ??
 **
 ** \retval ??????????????
 **
 ******************************************************************************/
uint16_t TftLcdScreenWidth(void)
{
	uint16_t width = TFT_LCD_DEF_X_MAX;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax 	
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	width = TFT_LCD_DEF_Y_MAX;
#endif
	
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	width = TFT_LCD_DEF_Y_MAX;
#endif
	
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	width = TFT_LCD_DEF_X_MAX;
#endif
	
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	width = TFT_LCD_DEF_X_MAX;
#endif

	return width;
}


/**
 *******************************************************************************
 ** \brief ??? TFT LCD ?????????
 **
 ** \param ??
 **
 ** \retval ?????????????
 **
 ******************************************************************************/
uint16_t TftLcdScreenHight(void)
{
	uint16_t hight = TFT_LCD_DEF_Y_MAX;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax 	
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	hight = TFT_LCD_DEF_X_MAX;
#endif
		
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	hight = TFT_LCD_DEF_X_MAX;
#endif
		
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	hight = TFT_LCD_DEF_Y_MAX;
#endif
		
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	hight = TFT_LCD_DEF_Y_MAX;
#endif

	return hight;
}



/**
 *******************************************************************************
 ** \brief ???? TFT LCD ????????????????
 **
 **---------------------------------------------
 **| TFT Screen  |   Txt View Scan Mode        |
 **|             |      x -->                  |
 **| LCD 160x128 |    0,0------ +1 ------>128  |
 **| 0,0->128    |   y | \                     |
 **|  | \  |	 |    +1  \                    |
 **|  |  \ |	 |    \|/  \                   |
 **| 160--->	 |     |                       |
 **|             |    160                      |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] pU8Data ????????????????????????? 
 **
 ** \param [in] u16Color ??????
 **
 ** \param [in] u16bkColor ???????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t TftLcdDrawOnceText(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	const uint8_t *pU8Data, 
	uint16_t u16Color, uint16_t u16BkColor)
{
	uint8_t u8Ret = 0;
	uint8_t u8WbyteNum = 0, u8WrmBit = 0, u8Bit = 0;
	uint8_t u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16DataIdx = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pU8Data && u8W > 0 && u8H > 0){
		u8WbyteNum = u8W / 8;
		u8WrmBit = u8W % 8;
		if(u8WrmBit){
			u8WbyteNum += 1;
		}
		/* ??? LCD ????????? */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		/* ?��????????????????? */
		if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
			u8Width = u8H;
			u8Hight = u8W;
			TftLcdSetRegion(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					if(u8Y < u8WrmBit){
						u8Bit = u8WrmBit - u8Y - 1;
					}
					else{
						u8Bit = 7 - ((u8Y - u8WrmBit) % 8);
					}
					u16DataIdx = (u8X * u8WbyteNum) + u8WbyteNum - 1;
					if(u8Y){
						if(u8WrmBit){
							u16DataIdx -= (u8Y + 8 - u8WrmBit) / 8;
						}
						else{
							u16DataIdx -= u8Y / 8;
						}
					}
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						TftLcdWriteData(u16Color);
					}
					else{
						TftLcdWriteData(u16BkColor);
					}
				}
			}
#endif
			
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
			u8Width = u8H;
			u8Hight = u8W;
			TftLcdSetRegion(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					u8Bit = u8Y % 8;
					u16DataIdx = u8Width - u8X - 1;
					u16DataIdx *= u8WbyteNum;
					if(u8Y){
						u16DataIdx += (u8Y / 8);
					}
					else{
						u16DataIdx += u8Y;
					}
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						TftLcdWriteData(u16Color);
					}
					else{
						TftLcdWriteData(u16BkColor);
					}
				}
			}
#endif
			
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
			u8Width = u8W;
			u8Hight = u8H;
			TftLcdSetRegion(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					u16DataIdx = (u8Hight - u8Y) * u8WbyteNum - 1;
					if(u8X){
						if(u8WrmBit){
							u16DataIdx -= (u8X + 8 - u8WrmBit) / 8;
						}
						else{
							u16DataIdx -= u8X / 8;
						}
					}
					if(u8X < u8WrmBit){
						u8Bit = u8WrmBit - u8X - 1;
					}
					else{
						u8Bit = 7 - ((u8X - u8WrmBit) % 8);
					}
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						TftLcdWriteData(u16Color);
					}
					else{
						TftLcdWriteData(u16BkColor);
					}
				}
			}
#endif
			
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
			u8Width = u8W;
			u8Hight = u8H;
			TftLcdSetRegion(u8Px, u8Py, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					u16DataIdx = u8Y * u8WbyteNum;
					if(u8X){
						u16DataIdx += u8X / 8;
					}
					u8Bit = u8X % 8;
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						TftLcdWriteData(u16Color);
					}
					else{
						TftLcdWriteData(u16BkColor);
					}
				}
			}
#endif
			u8Ret = 1;
		}

	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ??????????????????
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] pU8Data ????????????????????????? 
 **
 ** \param [in] u16Color ??????
 **
 ** \param [in] u16bkColor ???????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t TftLcdDrawLineText(uint8_t u8X, uint8_t u8Y, uint8_t u8Fw, uint8_t u8Fh,
	const uint8_t *pU8TextDatas, uint16_t u16TextNum, 
	uint16_t u16FtColor, uint16_t u16BkColor
	)
{
	uint8_t u8TextX = 0, u8TextY = 0, u8Ret = 0;
	uint16_t u16OnceTextDatasSize = 0, u16TextX = 0, u16TextY = 0;
	uint16_t u16TextIdx = 0, u16TextDatasIdx = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pU8TextDatas && u16TextNum > 0){
		/* ??? LCD ????????? */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		// calc once text datas size
		u16OnceTextDatasSize = (u8Fw / 8);
		if(u8Fw % 8){
			u16OnceTextDatasSize ++;
		}
		u16OnceTextDatasSize *= u8Fh;
		u16TextX = u8X;
		u16TextY = u8Y;
		for(u16TextIdx = 0; u16TextIdx < u16TextNum; u16TextIdx++){
			/* ????��????????��?? */
			u8TextX = u16TextX;
			u8TextY = u16TextY;
			
			u16TextX += u8Fw;
			if(u16TextX > u16ScreenW){
				/* ??????? */
				u16TextX = 0;
				u8TextX = u16TextX;
				u16TextY += u8Fh;
				if(u8TextY > u16ScreenH){
					/* ?????????? */
					break;
				}
				u8TextY = u16TextY;
			}
			if(u16TextY + u8Fh > u16ScreenH){
				/* ?????????? */
				break;
			}
			
			// calc datas start index
			u16TextDatasIdx = u16TextIdx * u16OnceTextDatasSize;
			TftLcdDrawOnceText(u8TextX, u8TextY, u8Fw, u8Fh, pU8TextDatas + u16TextDatasIdx, u16FtColor, u16BkColor);
		}
		if(u16TextIdx >= u16TextNum){
			u8Ret = 1;
		}
	}

	return u8Ret;
}	


/**
 *******************************************************************************
 ** \brief TFT LCD 16 16 ([7:0],[15:8]) 
 ** R:5 bit [15:11], G:6 bit[10:5] B:5 bit[4:0]
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8Px LCD ???????????��??
 **
 ** \param [in] u8Py LCD ???????????��??
 **
 ** \param [in] u8W ??????????????
 **
 ** \param [in] u8H ?????????????
 **
 ** \param [in] pImage ???????????????????  2 * (u8W * u8H) ???
 **             ?????????????? 16 ��??????????8��???????????8��???????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t TftLcdDrawU16ColorImage(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	const uint8_t *pImage)
{
	uint8_t u8Ret = 0;
	uint8_t u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16Idx = 0, u16Color = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pImage){
		/* ??? LCD ????????? */
		u16ScreenW = TftLcdScreenWidth();
		u16ScreenH = TftLcdScreenHight();
		/* ?��????????????????? */
		if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
			u8Width = u8H;
			u8Hight = u8W;
			TftLcdSetRegion(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = u8X;
					u16Idx *= u8Hight;
					u16Idx += (u8Hight - u8Y);
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					TftLcdWriteData(u16Color);
				}
			}
#endif
		
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
			u8Width = u8H;
			u8Hight = u8W;
			TftLcdSetRegion(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = u8Width - u8X - 1;
					u16Idx *= u8Hight;
					u16Idx += u8Y;
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					TftLcdWriteData(u16Color);
				}
			}
#endif
		
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
			u8Width = u8W;
			u8Hight = u8H;
			TftLcdSetRegion(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = (u8Hight - u8Y - 1);
					u16Idx *= u8Width;
					u16Idx += (u8Width - u8X - 1);
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					TftLcdWriteData(u16Color);
				}
			}
#endif
		
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
			u8Width = u8W;
			u8Hight = u8H;
			TftLcdSetRegion(u8Px, u8Py, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = u8Y;
					u16Idx *= u8Width;
					u16Idx += u8X;
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					TftLcdWriteData(u16Color);
				}
			}
#endif
			u8Ret = 1;
		}
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ??????????
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] u16Color ??????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t TftLcdDrawRange(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	uint16_t u16Color)
{
	uint8_t u8Ret = 0, u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	/* ??? LCD ????????? */
	u16ScreenW = TftLcdScreenWidth();
	u16ScreenH = TftLcdScreenHight();
	/* ?��????????????????? */
	if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
		u8Width = u8H;
		u8Hight = u8W;
		TftLcdSetRegion(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				TftLcdWriteData(u16Color);
			}
		}
#endif
					
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
		u8Width = u8H;
		u8Hight = u8W;
		TftLcdSetRegion(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				TftLcdWriteData(u16Color);
			}
		}
#endif
					
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
		u8Width = u8W;
		u8Hight = u8H;
		TftLcdSetRegion(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				TftLcdWriteData(u16Color);
			}
		}
#endif
					
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
		u8Width = u8W;
		u8Hight = u8H;
		TftLcdSetRegion(u8Px, u8Py, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				TftLcdWriteData(u16Color);
			}
		}
#endif
		u8Ret = 1;
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ???????
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] u8Direction ???????????
 **             0: --> ????????????????????????????????????????
 **             1: --> ??????????????????????????????????
 **             2: | ?��??????????????????????????????????????
 **             3: | ?��????????????????????????????????
 **
 ** \param [in] u16Color ?????????
 **
 ** \param [in] u16EmptyColor ????????
 **
 ** \param [in] u16Value ????????
 **
 ** \param [in] u16AllValue ??????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/

uint8_t TftLcdDrawPercent(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	uint8_t u8Direction, 
	uint16_t u16Color, uint16_t u16EmptyColor,
	uint32_t u32Value, uint32_t u32AllValue
	)
{
	uint8_t u8Ret = 0, u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16PointNum = 0, u16ValuePoint = 0, u16Cont = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;
	uint32_t u32Percent = 0;

	/* ??? LCD ????????? */
	u16ScreenW = TftLcdScreenWidth();
	u16ScreenH = TftLcdScreenHight();
	/* ?��????????????????? */
	if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
		/* ?????????????? */
		u16PointNum = u8W;
		u16PointNum *= u8H;
		for(;;){
			if(u32AllValue > (0xFFFFFFFF / TFT_LCD_MAX_POINT)){
				/* ??��???? */
				u32AllValue /= 10;
				if(u32Value > 0){
					u32Value /= 10;
				}
			}
			else{
				/* ???????????????��?? */
				break;
			}
		}
		u32Percent = u32Value;
		u32Percent *= u16PointNum;
		u32Percent /= u32AllValue;
		u16ValuePoint = u32Percent;
		if(u8Direction == 1){
			u8Hight = u16ValuePoint % u8H;
			u16ValuePoint = (u16ValuePoint / u8H) * u8H;
			if(u8Hight > u8H / 2){
				u16ValuePoint += u8H;
			}
		}
		else if(u8Direction == 3){
			u8Width = u16ValuePoint % u8W;
			u16ValuePoint = (u16ValuePoint / u8W) * u8W;
			if(u8Width > u8W / 2){
				u16ValuePoint += u8W;
			}
		}
			
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
		u8Width = u8H;
		u8Hight = u8W;
		TftLcdSetRegion(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8X + 1);
				}
				else{ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Y + 1);
				}
				u16Cont = u16PointNum - u16Cont;
				if(u16ValuePoint > u16Cont){
					TftLcdWriteData(u16Color);
				}

				else{
					TftLcdWriteData(u16EmptyColor);
				}
			}
		}
#endif
					
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
		u8Width = u8H;
		u8Hight = u8W;
		TftLcdSetRegion(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8Width - u8X - 1);
				}
				else{ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Hight - u8Y - 1);
				}
				if(u16ValuePoint >= u16Cont){
					TftLcdWriteData(u16Color);
				}

				else{
					TftLcdWriteData(u16EmptyColor);
				}
			}
		}
#endif
					
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
		u8Width = u8W;
		u8Hight = u8H;
		TftLcdSetRegion(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Hight - u8Y - 1);
					u16Cont = u16PointNum - u16Cont;
				}
				else{ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8Width - u8X - 1);
				}
				if(u16ValuePoint >= u16Cont){
					TftLcdWriteData(u16Color);
				}

				else{
					TftLcdWriteData(u16EmptyColor);
				}
			}
		}
#endif
					
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
		u8Width = u8W;
		u8Hight = u8H;
		TftLcdSetRegion(u8Px, u8Py, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Y + 1);
				}
				else{ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8Width - u8X - 1);
					u16Cont = u16PointNum - u16Cont;
				}
				if(u16ValuePoint >= u16Cont){
					TftLcdWriteData(u16Color);
				}

				else{
					TftLcdWriteData(u16EmptyColor);
				}
			}
		}
#endif
		u8Ret = 1;
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ????? TFT LCD 128x160 
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void InitTftLcd128x160(void)
{
	uint8_t i = 0, u8Buf[16];
	
	/* ????? TFT LCD DC RST PIN */
	InitLcdDcAndRstPin();

	/* ??�� TFT LCD DC RST PIN */
	ResetLcd();

	/* ???? TFT LCD ??? SLEEP ?? */
	TftLcdWriteCmd(0x11);

	/* ???? TFT LCD ?????? */
	i = 0;
	u8Buf[i++] = 0x01;
	u8Buf[i++] = 0x2C;
	u8Buf[i++] = 0x2D;
	u8Buf[i++] = 0x01;
	u8Buf[i++] = 0x2C;
	u8Buf[i++] = 0x2D;
	TftLcdWriteCmd(0xB1);
	TftLcdWriteDatas(u8Buf, 3);
	TftLcdWriteCmd(0xB2);
	TftLcdWriteDatas(u8Buf, 3);
	TftLcdWriteCmd(0xB3);
	TftLcdWriteDatas(u8Buf, 6);

	
	/* ???? TFT LCD ?��?? */
	i = 0;
	u8Buf[i++] = 0x07;
	TftLcdWriteCmd(0xB4);
	TftLcdWriteDatas(u8Buf, i);
	/* ???? TFT LCD Power Sequence */
	i = 0;
	u8Buf[i++] = 0xA2;
	u8Buf[i++] = 0x02;
	u8Buf[i++] = 0x84;
	TftLcdWriteCmd(0xC0);
	TftLcdWriteDatas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0xC5;
	TftLcdWriteCmd(0xC1);
	TftLcdWriteDatas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0x0A;
	u8Buf[i++] = 0x00;
	TftLcdWriteCmd(0xC2);
	TftLcdWriteDatas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0x8A;
	u8Buf[i++] = 0x2A;
	TftLcdWriteCmd(0xC3);
	TftLcdWriteDatas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0x8A;
	u8Buf[i++] = 0xEE;
	TftLcdWriteCmd(0xC4);
	TftLcdWriteDatas(u8Buf, i);

	// VCOM
	i = 0;
	u8Buf[i++] = 0x0E;
	TftLcdWriteCmd(0xC5);
	TftLcdWriteDatas(u8Buf, i);
	//MX, MY, RGB mode
	i = 0;
	u8Buf[i++] = 0xA0;
	u8Buf[i++] = 0xA8;
	TftLcdWriteCmd(0x36);
	TftLcdWriteDatas(u8Buf, i);
	//ST7735R Gamma Sequence
	i = 0;
	u8Buf[i++] = 0x0f; 
	u8Buf[i++] = 0x1a; 
	u8Buf[i++] = 0x0f; 
	u8Buf[i++] = 0x18; 
	u8Buf[i++] = 0x2f; 
	u8Buf[i++] = 0x28; 
	u8Buf[i++] = 0x20; 
	u8Buf[i++] = 0x22; 
	u8Buf[i++] = 0x1f; 
	u8Buf[i++] = 0x1b; 
	u8Buf[i++] = 0x23; 
	u8Buf[i++] = 0x37; 
	u8Buf[i++] = 0x00; 	
	u8Buf[i++] = 0x07; 
	u8Buf[i++] = 0x02; 
	u8Buf[i++] = 0x10;
	TftLcdWriteCmd(0xe0);
	TftLcdWriteDatas(u8Buf, i);

	i = 0;
	u8Buf[i++] = 0x0f; 
	u8Buf[i++] = 0x1b; 
	u8Buf[i++] = 0x17; 
	u8Buf[i++] = 0x33; 
	u8Buf[i++] = 0x2c; 
	u8Buf[i++] = 0x29; 
	u8Buf[i++] = 0x2e; 
	u8Buf[i++] = 0x30; 
	u8Buf[i++] = 0x30; 
	u8Buf[i++] = 0x39; 
	u8Buf[i++] = 0x3f;  
	u8Buf[i++] = 0x03;
	TftLcdWriteCmd(0xe1);
	TftLcdWriteDatas(u8Buf, i);
	
	i = 0;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x02;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x9f;
	TftLcdWriteCmd(0x2a);
	TftLcdWriteDatas(u8Buf, i);
	
	i = 0;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x03;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x7f;
	TftLcdWriteCmd(0x2b);
	TftLcdWriteDatas(u8Buf, i);
	//Enable test command
	i = 0;
	u8Buf[i++] = 0x01;
	TftLcdWriteCmd(0xF0);
	TftLcdWriteDatas(u8Buf, i);
	//Disable ram power save mode
	i = 0;
	u8Buf[i++] = 0x00;
	TftLcdWriteCmd(0xF6);
	TftLcdWriteDatas(u8Buf, i);
	//65k mode 
	i = 0;
	u8Buf[i++] = 0x05;
	TftLcdWriteCmd(0x3A);
	TftLcdWriteDatas(u8Buf, i);
	//Display on
	TftLcdWriteCmd(0x29);

	/* Set TFT LCD View */
	TftLcdViewAllColor(TFT_COLOR_WHITE);

	//TftLcdDrawOnceText(0, 0, 6, 12, u8English6x12, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
	//TftLcdDrawOnceText(6, 0, 6, 12, u8English6x12 + 12, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);

	//TftLcdDrawOnceText(0, 12, 8, 16, u8English8x16, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
	//TftLcdDrawOnceText(8, 12, 8, 16, u8English8x16 + 16, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);

	//TftLcdDrawLineText(0, 28, 6, 12, u8English6x12, 31, TFT_COLOR_RED, TFT_COLOR_BLACK);

	//TftLcdDrawU16ColorImage(10, 10, 100, 100, gImage_pan108);
	
}





/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/



