#ifndef _ReadKey_H
#define _ReadKey_H
#include <stdbool.h>

#define baseTime 10		//KeyValueChange()调用的基准时间

typedef struct{
	bool enReleaseKey;	//使能弹起有效功能
	bool enDoubleKey;		//使能双击有效功能
	bool enLongKey;			//使能长按有效功能
	bool enCyclicKey;		//使能按下有效，周期性触发功能
	
	bool stReleaseKey;	//弹起状态
	bool stDoubleKey;		//双击状态
	bool stLongKey;			//长按状态
	bool stCyclicKey;		//按下有效，周期性触发功能状态
	
	bool isDown;				//按键按下=true，按键弹起=false
	unsigned int downTime; //按键按下时间
	unsigned int upTime; 	 //按键弹起时间
	
	unsigned char oldValue; //上一次按键
	unsigned char newValue; //当前按键
	
}KeyParam_ts;
typedef struct{
	bool haveKey;				 //产生了新按键
	unsigned char value; //按键键值
}KeyParamExt_ts;
/**
 * --------按键功能转换
 * @since 2018-7-25
 * @Param1 PreKeyValue：需要转换实现功能的按键键值
 * @Addition
 * 		可以产生长按有效，弹起有效，长按循环有效，双击有效功能
 *		长按有效，长按循环有效，双击有效功能响应时间受该函数的调用
 *		基准时间有关，本例基准时间为10ms，请知悉！
 * @Return 
 *		KeyParamExt_ts：haveKey=true才可以处理按键，value取得键值
 */
KeyParamExt_ts *KeyValueChange(unsigned char PreKeyValue);
/**
 * --------使能双击有效功能
 * @since 2018-7-25
 * @Param1 void
 * @Addition	Keys.upTime：基准时间10ms
 *						使用方法见.c文件头
 * @Return 
 *
 */
void 	EnableDoubleKey(void);
bool getDoubleKeySt(void);
void clearDoubleKey(void);
/**
 * --------使能长按有效功能，单次有效
 * @since 2018-7-25
 * @Param1 void
 * @Addition	Keys.downTime：基准时间10ms
 *						使用方法见.c文件头
 * @Return 
 *
 */
void 	EnableLongKey(unsigned int Xms);
bool getLongKeySt(void);
void clearLongKey(void);
/**
 * --------使能循环长按有效功能，循环有效
 * @since 2018-7-25
 * @Param1 void
 * @Addition	Keys.downTime：基准时间10ms
 *						使用方法见.c文件头
 * @Return 
 *
 */
void 	EnableCyclicKey(unsigned int Xms);
bool getCyclicKeySt(void);
void clearCyclicKey(void);
/**
 * --------使能弹起有效功能
 * @since 2018-7-25
 * @Param1 void
 * @Addition	
 *						使用方法见.c文件头
 * @Return 
 *
 */
void 	EnableReleaseKey(void);
bool getReleaseKeySt(void);
void clearReleaseKey(void);
#endif
