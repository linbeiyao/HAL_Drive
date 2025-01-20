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
#include <stdint.h>


#define ASRPRO_UARTX huart3
#define ASRPRO_REC_LEN 3

// 命令定义
#define ASRPRO_CMD_Null                  0x00  // 空命令
#define ASRPRO_CMD_Broadcast_information 0x01  // 播报信息
#define ASRPRO_CMD_Query_Temperature     0x02  // 查询温度
#define ASRPRO_CMD_Query_Light          0x03  // 查询光照
#define ASRPRO_CMD_Set_Light            0x04  // 设置灯光

// 四种测试模式
#define TESTFLAGS_1 0xf1
#define TESTFLAGS_2 0xf2
#define TESTFLAGS_3 0xf3
#define TESTFLAGS_4 0xf4

// ASRPRO设备结构体
typedef struct {
    UART_HandleTypeDef* huart;       // UART句柄
    uint8_t rxBuffer[ASRPRO_REC_LEN];// 接收缓冲区
    uint8_t isInitialized;           // 初始化标志
    void (*onDataReceived)(uint8_t cmd, uint8_t* data, uint8_t len); // 数据接收回调
} ASRPRO_Device_t;

/** 
 * 使用流程：
 * 1. 定义回调函数：
 *    void OnAsrproDataReceived(uint8_t cmd, uint8_t* data, uint8_t len) {
 *        // 处理接收到的数据
 *    }
 * 
 * 2. 初始化设备：
 *    ASRPRO_Init(&g_ASRPRO_Device, &huart3, OnAsrproDataReceived);
 * 
 * 3. 发送命令：
 *    uint8_t data[] = {0x01, 0x02};
 *    ASRPRO_SendCommand(&g_ASRPRO_Device, ASRPRO_CMD_Set_Light, data, sizeof(data));
 */

// 函数声明
void ASRPRO_Init(ASRPRO_Device_t* dev, UART_HandleTypeDef* huart, 
                 void (*callback)(uint8_t, uint8_t*, uint8_t));
void ASRPRO_SendCommand(ASRPRO_Device_t* dev, uint8_t cmd, uint8_t* data, uint8_t len);
void ASRPRO_RxCpltCallback(ASRPRO_Device_t* dev);

// 外部变量声明
extern ASRPRO_Device_t g_ASRPRO_Device;

#endif

