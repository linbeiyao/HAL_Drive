#include "uart_user.h"
#include "stdio.h"
#include "usart.h"
#include <string.h>

/* PS: 使用printf打印需要勾选魔术棒中的Use MicroLIB*/
/* 重写printf uart1串口 */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xffff);
	return ch;
}

/*
************************************************************
*	函数名称：	Usart_SendString
*
*	函数功能：	串口数据发送
*
*	入口参数：	huart：串口组
*				str：要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Usart_SendString(UART_HandleTypeDef *huart, unsigned char *str, unsigned short len)
{
	unsigned short count = 0;
	for(; count < len; count++)
	{
		HAL_UART_Transmit(huart, str, strlen((const char *)str), 0xffff);				// 发送数据
		while(HAL_UART_GetState(huart) == HAL_UART_STATE_BUSY_TX);  // 等待发送完成
	}
}
