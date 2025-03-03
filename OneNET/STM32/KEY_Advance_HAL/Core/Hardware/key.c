#include "main.h"
#include "key.h"

/* 支持长短按 
   目前存在问题：无法检测多个按键同时按下
*/

/* 按键是否要有长按 不能同时注释或存在*/
#define LongKeyEvent 1
//#define ShortKeyEvent 1

#define KEY0   	HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)		// 按下低电平
#define KEY1 	HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)		// 按下低电平
#define KEY_UP 	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)		// 按下高电平

#define isKeyDown (!KEY0 || !KEY1 || KEY_UP)	// 1表示按下，0表示没有按下

KEY_VALUE KeyValue = KEY_NULL;		 // 初始化按键值
KEY_STATE KeyState = KEY_CHECK;		 // 初始化按键状态为检测状态
KEY_TYPE  KeyType = NULL_KEY;		 // 初始化按键类型为空
uint8_t g_KeyFlag = 0;               // 按键有效标志，0： 按键值无效； 1：按键值有效


/* 按键检测无长按 */
#if ShortKeyEvent
void Key_Scan(void)
{
	switch(KeyState)
	{
		case KEY_CHECK:
			if(isKeyDown)	// 是否有按键按下
			{
				KeyState = KEY_CONFIRM;	// 进入下一个状态
			}
			break;
		case KEY_CONFIRM:
			if(isKeyDown)	// 是否一直按着 消抖
			{
				KeyState = KEY_RELEASE;	// 进入下一个状态				
			}
			else
			{
				KeyState = KEY_CHECK;	// 返回上一个状态
			}			
			break;
		case KEY_RELEASE:
			if(!isKeyDown)	// 按键松开
			{
				KeyState = KEY_CHECK;	// 返回开始状态
				g_KeyFlag = 1;
			}				
			break;
		default:
			break;		
	}	
}
/* 按键检测有长按 */
#elif defined(LongKeyEvent)
void Key_Scan(void)
{
	static uint8_t count, isFirstEntry;
	switch(KeyState)
	{
		case KEY_CHECK:
			if(isKeyDown)	// 是否有按键按下
			{
				KeyState = KEY_CONFIRM;	// 进入下一个状态
				count = 0;
				isFirstEntry = 0;
			}
			break;
		case KEY_CONFIRM:
			if(isKeyDown)	// 是否一直按着 消抖
			{
				if(!isFirstEntry) 
					isFirstEntry = 1;
				count++;		
				/* 判断是哪个按键按下的 */
				if(!KEY0)
					KeyValue = KEY_0;
				else if(!KEY1)
					KeyValue = KEY_1;
				else if(KEY_UP)
					KeyValue = KEY_WK_UP;		
			}
			else
			{
				if(isFirstEntry)  	// 不是第一次进来 
				{
					KeyState = KEY_RELEASE;	// 进入下一个状态	
				}					
				else			
					KeyState = KEY_CHECK;	// 返回上一个状态
			}			
			break;
		case KEY_RELEASE:
			if(!isKeyDown)	// 按键松开
			{
				if(count > 80) // 计时超过80次判断为长按
				{
					KeyType = LONG_KEY;	// 按键类型为长按
					count = 0;
					isFirstEntry = 0;
				}	
				else
				{
					KeyType = SHORT_KEY;	// 按键类型为短按
				}	
				KeyState = KEY_CHECK;	// 返回开始状态
			}				
			break;
		default:
			break;		
	}	
}
#endif



