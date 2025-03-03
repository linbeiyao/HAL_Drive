#ifndef __UART_USER_H__
#define __UART_USER_H__

#include "stm32f1xx_hal.h"

void Usart_SendString(UART_HandleTypeDef *huart, unsigned char *str, unsigned short len);

#endif
