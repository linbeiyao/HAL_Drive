#include "uart_user.h"
#include "stdio.h"
#include "usart.h"
#include <string.h>

/**
 * @file uart_user.c
 * @brief 串口通信用户接口实现
 * 
 * @功能说明:
 * 1. printf重定向:
 *    - 实现fputc函数，将printf输出重定向到UART1
 *    - 可以直接使用printf函数输出调试信息
 * 
 * 2. 串口发送函数:
 *    - Usart_SendString函数用于发送字符串到指定串口
 *    - 支持任意串口，只需传入对应的串口句柄
 * 
 * @配置要求:
 * 1. 在CubeMX中正确配置串口:
 *    - UART1: 用于调试输出
 *    - UART2: 连接ESP8266模块
 * 
 * 2. 确保串口时钟已使能:
 *    - __HAL_RCC_USART1_CLK_ENABLE()
 *    - __HAL_RCC_USART2_CLK_ENABLE()
 * 
 * 3. 确保GPIO已正确配置:
 *    - UART1_TX: PA9
 *    - UART1_RX: PA10
 *    - UART2_TX: PA2
 *    - UART2_RX: PA3
 */

/* PS: ʹprintfӡҪ��ѡħ�����е�Use MicroLIB*/
/* 重写printf uart1输出 */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xffff);
	return ch;
}

/*
************************************************************
*	�������ƣ�	Usart_SendString
*
*	�������ܣ�	�������ݷ���
*
*	��ڲ�����	huart��������
*				str��Ҫ���͵�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
/**
 * @brief 发送字符串到指定串口
 * 
 * @param huart 串口句柄
 * @param str 要发送的字符串
 * @param len 字符串长度
 * 
 * @使用示例:
 * // 发送字符串到UART1
 * Usart_SendString(&huart1, (unsigned char*)"Hello", 5);
 * 
 * // 发送字符串到UART2
 * Usart_SendString(&huart2, (unsigned char*)"AT\r\n", 4);
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
