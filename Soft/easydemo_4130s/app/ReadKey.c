// Header:
// File Name: 
// Author:
// Date:
// 使用举例：
/*
		if(isTmFor10ms){
			isTmFor10ms = false;
			KeyIn = KeyValueChange(mKeyValue);
		}
			if(K1 == 0){

					mKeyValue =1;
			}	else if(K2 == 0){
			mKeyValue =2;
			}else if(K3 == 0){
			mKeyValue =3;
			}else if(K4 == 0){
			mKeyValue =4;
			}else{
			mKeyValue =0;
			}
			
			if(KeyIn->haveKey){
				KeyIn->haveKey = false;
				switch(KeyIn->value){
					case 1:
						EnableLongKey(100);	
						if(getLongKeySt() == true){
							clearLongKey();	
							lenth++;
						}			
					break;
					case 2:
						EnableDoubleKey();	
						if(getDoubleKeySt() == true){
							clearDoubleKey();				
							lenth++;
						}							
					break;
					case 3:
						EnableReleaseKey();	
						if(getReleaseKeySt() == true){
							clearReleaseKey();				
							lenth++;
						}							
					break;	
					case 4:
						EnableCyclicKey(100);	
						if(getCyclicKeySt() == true)
							{			
									lenth++;
						}							
					break;
					default:
						break;
				}
				
			}
*/
#include "ReadKey.h"

KeyParam_ts Keys;
KeyParamExt_ts KeysExt;
/**
 * --------按键功能转换
 * @since 2018-7-25
 * @Param1 PreKeyValue：需要转换实现功能的按键键值
 * @Addition
 * 		可以产生长按有效，弹起有效，长按循环有效，双击有效功能
 *		长按有效，长按循环有效，双击有效功能响应时间受该函数的调用
 *		基准时间有关，本例基准时间:baseTime=10ms，若有不同，请在.h文件中修改请知悉！
 * @Return 
 *		KeyParamExt_ts：haveKey=true才可以处理按键，value取得键值
 */
KeyParamExt_ts *KeyValueChange(unsigned char PreKeyValue){
	
	Keys.newValue = PreKeyValue;
	if(Keys.upTime){
		Keys.upTime--;
	}else{
		Keys.enDoubleKey = false;
	}
		
	if(Keys.newValue){
		if(Keys.downTime){
			Keys.downTime--;
		}		
		if(Keys.newValue == Keys.oldValue){//相等，表示按键长按状态下
//------------------------------长按键-----------------------------------					
			if(Keys.enLongKey){//如果使能了长按有效功能，单次触发
				if(Keys.downTime == 0){//如果长按时间到
					if(Keys.stLongKey == false){//如果未产生长按键
						KeysExt.haveKey = true;	//产生按键
					}
					Keys.stLongKey = true;
				}
			}
//------------------------------循环长按键-----------------------------------					
			if(Keys.enCyclicKey){//如果使能了长按循环有效功能，循环触发
				if(Keys.downTime == 0){//如果长按时间到
					KeysExt.haveKey = true;	//产生按键
					Keys.stCyclicKey = true;	//产生了循环长按键
				}
			}	
			
		}else{//不相等，表示新按键按下，检测双击键
//------------------------------双击按键-----------------------------------					
			if(Keys.enDoubleKey){//如果使能了双击有效功能
				if(Keys.upTime){//如果在按键弹起时间范围能
					KeysExt.haveKey = true;	//产生按键
					Keys.stDoubleKey = true;	//产生了双击按键
				}				
			}
		}
//------------------------------其他功能按键-----------------------------------				
		if(Keys.isDown == false){//第一次产生按键
			KeysExt.haveKey = true;	//产生按键
		}
		//更新按键
		Keys.oldValue = Keys.newValue;	
		Keys.stReleaseKey = false;	
		Keys.isDown = true;
	}else{//按键未按下，检测双击键和弹起有效功能
//------------------------------弹起按键-----------------------------------				
		if(Keys.enReleaseKey){//如果使能了弹起有效功能
			if(Keys.isDown){//表示按键弹起
				KeysExt.haveKey = true;	//产生按键
				Keys.stReleaseKey = true;	//产生了弹起按键
			}
		}else{
			//更新按键
			Keys.oldValue = Keys.newValue;		
		}
		Keys.enCyclicKey = false;
		Keys.enLongKey = false;
		Keys.stLongKey = false;
		Keys.stCyclicKey = false;
		Keys.stDoubleKey = false;
		Keys.isDown = false;
	}
	

	KeysExt.value = Keys.oldValue;
	return &KeysExt;
}
/**
 * --------使能双击有效功能
 * @since 2018-7-25
 * @Param1 void
 * @Addition	Keys.upTime：基准时间10ms
 *						使用方法见.c文件头
 * @Return 
 *
 */
void EnableDoubleKey(void)
{
	if(Keys.enDoubleKey==false)
		{
		Keys.enDoubleKey = true; 
		Keys.upTime= 500/baseTime;
		}	
}
bool getDoubleKeySt(void){
	return Keys.stDoubleKey;
}
void clearDoubleKey(void){
	Keys.enDoubleKey = false;
}
//-----------------------------------------
//
//按键长按键处理子程序
//
//-----------------------------------------
/**
 * --------使能长按有效功能，单次有效
 * @since 2018-7-25
 * @Param1 void
 * @Addition	Keys.downTime：基准时间10ms
 *						使用方法见.c文件头
 * @Return 
 *
 */
void EnableLongKey(unsigned int Xms)
{
	if(Keys.enLongKey==false)
		{
		Keys.enLongKey = true; 
		Keys.downTime=Xms/baseTime;
		}	
}
bool getLongKeySt(void){
	return Keys.stLongKey;
}
void clearLongKey(void){
	Keys.enLongKey = false;
}
//-----------------------------------------
//
//按键循环长按有效处理子程序
//
//-----------------------------------------
/**
 * --------使能循环长按有效功能，循环有效
 * @since 2018-7-25
 * @Param1 void
 * @Addition	Keys.downTime：基准时间10ms
 *						若需要按下首次有效，则无需调用getCyclicKeySt()
 *						使用方法见.c文件头
 * @Return 
 *
 */
void EnableCyclicKey(unsigned int Xms)
{
		Keys.enCyclicKey = true; 
		Keys.downTime = Xms/baseTime;	
}
bool getCyclicKeySt(void){
	return Keys.stCyclicKey;
}
//void clearCyclicKey(void){
//	Keys.enCyclicKey = false;
//}

//-----------------------------------------
//
//按键弹起有效处理子程序
//
//-----------------------------------------
//-----------------------------------------
/**
 * --------使能弹起有效功能
 * @since 2018-7-25
 * @Param1 void
 * @Addition	
 *						使用方法见.c文件头
 * @Return 
 *
 */
void EnableReleaseKey(void)
{
		Keys.enReleaseKey = true; 
}
bool getReleaseKeySt(void){
	return Keys.stReleaseKey;
}
void clearReleaseKey(void){
	Keys.enReleaseKey = false;
}
