#include "key.h"
#include <stdio.h>

/* 按键列表 */
key_gpio_t key_gpio[] = {
	{GPIOE, GPIO_PIN_4, 0},		// KEY0 按下为低电平
	{GPIOE, GPIO_PIN_3, 0},		// KEY1 按下为低电平
	{GPIOA, GPIO_PIN_0, 1},		// KEY_WP_UO 按下为高电平
};

#define KEY_NUM_MAX (sizeof(key_gpio)/sizeof(sizeof(key_gpio[0])))	// 按键数量

#define CONFIRM_TIME	20 		// 消抖时间20ms
#define LONG_PRESS_TIME 500		// 长按时间500ms
#define SHORT_RELEASE_VALID	1	// 1：短按松开时有效，0：短按按下时有效
#define LONG_RELEASE_VALID	1	// 1：长按松开时有效，0：长按按下时有效

key_param_t key_param[KEY_NUM_MAX];		/* 存放所有按键的属性 */

/**
  * @brief  读取按键是否按下
  * @param	index：key_param结构体对应按键下标
  * @retval 1：按下，0：未按下
  */
static uint8_t read_key_state(uint8_t index)
{
	if(HAL_GPIO_ReadPin(key_gpio[index].port, key_gpio[index].pin) == key_gpio[index].pressed_state)
		return 1;
	return 0;
}

/**
  * @brief  依次扫描每个按键状态
  * @param	index：key_param结构体对应按键下标
  * @retval void
  */
void key_scan(void)
{
	static uint8_t index, isKeyPress;
	static uint32_t pressed_time;
	static key_state_t current_state;
	static key_event_t key_event;
	for(index = 0; index < KEY_NUM_MAX; index++)
	{
		isKeyPress = read_key_state(index);				// 获取按键是否按下
		current_state = key_param[index].current_state;	// 获取按键状态机所处状态
		pressed_time  = key_param[index].pressed_time;	// 获取按键按下的时间
		key_event = key_param[index].key_event;			// 获取按键的事件
		switch(current_state)
		{
			/* 状态一：检测状态 */
			case KEY_CHECK:		
				if(isKeyPress)	// 按键按下
				{
					current_state = KEY_CONFIRM; 	// 进入消抖确认
				}
				else	// 按键松开
				{
					pressed_time = 0;
				}				
				break;
			/* 状态二：消抖确认 */
			case KEY_CONFIRM:		
				if(isKeyPress)	// 仍然按下
				{
					if(++pressed_time > CONFIRM_TIME)	// 消抖时间 
					{
						current_state = KEY_SHORT_PRESSED; 	// 进入短按状态
						#if (!SHORT_RELEASE_VALID)	// 短按按下立即生效
							key_event = EVENT_SHORT_PRESSED;	// 短按事件
						#endif						
					}
				}
				else	// 按键消抖前松开了(未消抖 按键无效)
				{
					current_state = KEY_CHECK;	// 返回上一个状态
				}		
				break;
			/* 状态三：短按状态 */
			case KEY_SHORT_PRESSED:	
				if(isKeyPress)	// 仍然按下
				{
					if(++pressed_time > LONG_PRESS_TIME)	// 长按时间
					{
						current_state = KEY_LONG_PRESSED;
						#if (!SHORT_RELEASE_VALID)	// 长按按下立即生效
							key_event = EVENT_LONG_PRESSED;	// 长按事件
						#endif		
					}
				}
				else	// 按键松开了
				{
					current_state = KEY_CHECK;			// 返回检测状态
					#if (SHORT_RELEASE_VALID)	// 短按松开有效
						key_event = EVENT_SHORT_PRESSED;	// 短按事件
					#endif	
				}
				break;
			/* 状态四：长按状态 */
			case KEY_LONG_PRESSED:	
				if(!isKeyPress)	// 按键松开了
				{
					current_state = KEY_CHECK;			// 返回检测状态
					#if (SHORT_RELEASE_VALID)	// 长按松开生效
						key_event = EVENT_LONG_PRESSED;	// 长按事件
					#endif	
				}					
				break;
			default:
				current_state = KEY_CHECK;			// 默认检测状态
				break;	
		}
		key_param[index].current_state = current_state;	// 存入结构体
		key_param[index].pressed_time = pressed_time;	// 存入结构体
		key_param[index].key_event = key_event;			// 存入结构体
	}
}
	
/**
  * @brief  按键测试函数
  * @param	void
  * @retval void
  */
void key_handle(void)
{
	key_scan();
	HAL_Delay(1);
	
	static uint8_t index;
	static key_event_t key_event;
	for(index = 0; index < KEY_NUM_MAX; index++)
	{
		key_event = key_param[index].key_event;
		//printf("key_event:%d\r\n", key_event);
		if(key_event != 0)	// 有事件发生
		{
			switch(index)
			{
				case 0:		// 按键KEY0
					if(key_event == EVENT_SHORT_PRESSED)
					{
						printf("KEY0 SHORT PRESSED\r\n");
					}
					else if(key_event == EVENT_LONG_PRESSED)
					{
						printf("KEY0 LONG PRESSED\r\n");
					}
					break;
				case 1:		// 按键KEY1
					if(key_event == EVENT_SHORT_PRESSED)
					{
						printf("KEY1 SHORT PRESSED\r\n");
					}
					else if(key_event == EVENT_LONG_PRESSED)
					{
						printf("KEY1 LONG PRESSED\r\n");
					}
					break;
				case 2:		// 按键KEY_WP_UP
					if(key_event == EVENT_SHORT_PRESSED)
					{
						printf("KEY_WP_UP SHORT PRESSED\r\n");
					}
					else if(key_event == EVENT_LONG_PRESSED)
					{
						printf("KEY_WP_UP LONG PRESSED\r\n");
					}
					break;
				default:			
					break;
			}
			key_param[index].key_event = EVENT_NULL; 	// 清除事件
		}
	}
}












