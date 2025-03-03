#ifndef __KEY_H
#define __KEY_H

#include "gpio.h"


/* 对应按键值 */
typedef enum {
	KEY_NULL = 0,
	KEY_0,
	KEY_1,
	KEY_WK_UP,
}KEY_VALUE;

/* 按键状态 */
typedef enum {
	KEY_CHECK = 0,			// 按键检测
	KEY_CONFIRM,			// 消抖确认
	KEY_RELEASE,			// 按键松开
}KEY_STATE;

/* 按键类型 */
typedef enum {
	NULL_KEY = 0,		// 无按键
	SHORT_KEY,			// 短按
	LONG_KEY,			// 长按
}KEY_TYPE;

/* 声明 */
extern uint8_t g_KeyFlag; 
extern KEY_TYPE KeyType;
extern KEY_VALUE KeyValue;
/* 函数 */
void Key_Scan(void);

//uint8_t Key_GetNum(void);

#endif
