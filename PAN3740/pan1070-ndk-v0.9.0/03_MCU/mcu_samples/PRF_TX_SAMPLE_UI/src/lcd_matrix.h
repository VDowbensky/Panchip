
/******************************************************************************/
/** \file lcd_matrix.h
 ******************************************************************************/
#ifndef __LCD_MATRIX_H
#define __LCD_MATRIX_H

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "lcd_view_ctrl.h"
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
#define START_FACE_IMAGE_W			80
#define START_FACE_IMAGE_H			72
#define START_FACE_IMAGE_SIZE		(2 * START_FACE_IMAGE_W * START_FACE_IMAGE_H)


#if (ENGLISH_6X12_TEXT == DDL_ON)
#define ENGLISH_6X12_TEXT_SIZE		12
#define ENGLISH_6X12_TEXT_W			6
#define ENGLISH_6X12_TEXT_H			12
#endif


#if (ENGLISH_8X16_TEXT == DDL_ON)
#define ENGLISH_8X16_TEXT_SIZE		16
#define ENGLISH_8X16_TEXT_W			8
#define ENGLISH_8X16_TEXT_H			16
#endif


#if (CHINESE_12X12_TEXT == DDL_ON)
#define CHINESE_12X12_TEXT_SIZE		24
#define CHINESE_12X12_TEXT_W		12
#define CHINESE_12X12_TEXT_H		12
#endif


#if (CHINESE_16X16_TEXT == DDL_ON)
#define CHINESE_16X16_TEXT_SIZE		32
#define CHINESE_16X16_TEXT_W		16
#define CHINESE_16X16_TEXT_H		16
#endif

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

#if (ENGLISH_6X12_TEXT == DDL_ON)
	extern const uint8_t english_6x12_text[];
#endif


#if (ENGLISH_8X16_TEXT == DDL_ON)
	extern const uint8_t english_8x16_text[];
#endif


#if (CHINESE_12X12_TEXT == DDL_ON)
	extern const char chinese_12x12_text_str[];
	extern const uint8_t chinese_12x12_text[];
#endif


#if (CHINESE_16X16_TEXT == DDL_ON)
	extern const char chinese_16x16_text_str[];
	extern const uint8_t chinese_16x16_text[];
#endif


#if (START_FACE_IMAGE == DDL_ON)
	extern const uint8_t start_face_image[START_FACE_IMAGE_SIZE];
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LCD_MATRIX_H */


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


