#include "led.h"

/* LED1£ºPB5	
*  status: 1ÁÁ 0Ãð
*/
void led1_set(uint8_t status)
{
	if(status)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);	// PB5ÁÁ
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);		// PB5Ãð
}

/* LED1£ºPB5	
*  ·´×ª
*/
void led1_reverse()
{
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
}

/* LED2£ºPE5	
*  status: 1ÁÁ 0Ãð
*/
void led2_set(uint8_t status)
{
	if(status)
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);	// PE5ÁÁ
	else
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);		// PE5Ãð
}

/* LED2£ºPE5	
*  ·´×ª
*/
void led2_reverse()
{
	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
}
