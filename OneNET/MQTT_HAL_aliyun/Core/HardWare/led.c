#include "led.h"

/* LED1��PB5	
*  status: 1�� 0��
*/
void led1_set(uint8_t status)
{
	if(status)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);	// PB5��
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);		// PB5��
}

/* LED2��PE5	
*  status: 1�� 0��
*/
void led2_set(uint8_t status)
{
	if(status)
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);	// PE5��
	else
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);		// PE5��
}
