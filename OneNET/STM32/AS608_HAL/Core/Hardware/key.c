#include "main.h"
#include "key.h"

uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;
	
	// KEY0 PE4 按下为低电平
	if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == 0)
	{	
		// 消抖
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == 0);
		HAL_Delay(20);
		KeyNum = 1;
	}
	// KEY1 PE3 按下为低电平
	if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == 0)
	{
		// 消抖
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == 0);
		HAL_Delay(20);
		KeyNum = 2;
	}
	// KEY_UP PA0 按下为高电平
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1)
	{
		// 消抖
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1);
		HAL_Delay(20);
		KeyNum = 3;
	}
	
	return KeyNum;
}


