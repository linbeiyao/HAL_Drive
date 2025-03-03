#ifndef __KEY_H
#define __KEY_H

#include "gpio.h"

/* 按键引脚定义 */
typedef struct {
	GPIO_TypeDef* port;				// 端口号
	uint16_t 	  pin;				// 引脚号
	uint16_t	  pressed_state;	// 按键按下时的状态，0为低电平有效，1为高电平有效
}key_gpio_t;

/* 按键状态 */
typedef enum {
	KEY_CHECK = 0,		// 释放松开
	KEY_CONFIRM,		// 消抖确认
	KEY_SHORT_PRESSED,	// 短按	
	KEY_LONG_PRESSED,	// 长按			
}key_state_t;

/* 按键事件 */
typedef enum {
	EVENT_NULL = 0,				// 无事件
	EVENT_SHORT_PRESSED,		// 短按事件
	EVENT_LONG_PRESSED,			// 长按事件
}key_event_t;

/* 每个按键的属性 */
typedef struct {
	key_state_t current_state;		// 按键当前状态
	uint32_t pressed_time;			// 按下时间
	key_event_t key_event;			// 按键事件
}key_param_t;

/* 外部声明 */


/* 函数声明 */
void key_scan(void);
void key_handle(void);






#endif
