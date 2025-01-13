#ifndef ASRPRO_H
#define ASRPRO_H


/* * * * * * 
 * 作 者：王永成
 * 时 间: 2024/12/23
 * 邮 箱：2042789231@qq.com
 * 
 * V1     基于STM32单片机的智能水杯设计      2024/12/23
 *
 * */



#include "main.h"
#include "usart.h"

#define ASRPRO_UARTX huart3
#define ASRPRO_REC_LEN 3
extern uint8_t AsrPro_Temp[];

// 四种测试模式
#define TESTFLAGS_1 0xf1
#define TESTFLAGS_2 0xf2
#define TESTFLAGS_3 0xf3
#define TESTFLAGS_4 0xf4
extern uint8_t TempTEST;	// 温度测试
extern uint8_t LevelTEST;	// 水量测试


void SendAsrpro(uint8_t cmd, uint8_t *data, uint8_t len);
void ASRPRO_RxCpltCallback(UART_HandleTypeDef *huart);


#endif

