/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#define DMA_BUFFER_SIZE  512 // DMA 缓冲区大小

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

// UART初始化完成标志，0表示初始化阶段，1表示正常运行阶段
extern uint8_t g_uart_initialization_complete;

extern volatile uint16_t dma_rx_index;      	// 当前接收索引
extern volatile uint8_t dma_rx_complete;    	// 接收完成标志

void MX_USART1_UART_Init(void);
void UART_ProcessReceivedData(void);  		// 在主循环中处理UART接收到的数据
void UART_InitializationComplete(void);  	// 设置初始化阶段结束


#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
