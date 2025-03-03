#include "uart_user.h"
#include "stdio.h"
#include "usart.h"
#include <string.h>

/* PS: ʹ��printf��ӡ��Ҫ��ѡħ�����е�Use MicroLIB*/
/* ��дprintf uart1���� */
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
void Usart_SendString(UART_HandleTypeDef *huart, unsigned char *str, unsigned short len)
{
	unsigned short count = 0;
	for(; count < len; count++)
	{
		HAL_UART_Transmit(huart, str, strlen((const char *)str), 0xffff);				// ��������
		while(HAL_UART_GetState(huart) == HAL_UART_STATE_BUSY_TX);  // �ȴ��������
	}
}
