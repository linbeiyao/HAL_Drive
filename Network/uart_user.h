#ifndef __UART_USER_H__
#define __UART_USER_H__

#include "stm32f1xx_hal.h"

/**
 * @file uart_user.h
 * @brief 串口通信用户接口头文件
 * 
 * @配置说明:
 * 1. 串口配置:
 *    - 默认使用UART1作为调试串口，UART2连接ESP8266
 *    - 波特率: 115200
 *    - 数据位: 8
 *    - 停止位: 1
 *    - 校验位: 无
 *    - 流控制: 无
 * 
 * 2. 中断配置:
 *    - UART2需要开启接收中断，用于接收ESP8266数据
 *    - 在CubeMX中配置NVIC，使能USART2_IRQn中断
 * 
 * 3. 重定向printf:
 *    - 本文件实现了fputc函数，将printf重定向到UART1
 *    - 可以直接使用printf函数输出调试信息
 * 
 * @使用示例:
 * // 发送字符串
 * Usart_SendString(&huart1, (unsigned char*)"Hello World", strlen("Hello World"));
 * 
 * // 使用printf输出调试信息
 * printf("Debug: %d\r\n", value);
 */

/**
 * @brief 发送字符串到指定串口
 * @param huart 串口句柄
 * @param str 要发送的字符串
 * @param len 字符串长度
 */
void Usart_SendString(UART_HandleTypeDef *huart, unsigned char *str, unsigned short len);

#endif
