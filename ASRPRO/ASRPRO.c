#include "ASRPRO.h"
#include "string.h"
#include "stdio.h"

// 全局设备实例
ASRPRO_Device_t g_ASRPRO_Device = {
    .huart = NULL,
    .isInitialized = 0,
    .onDataReceived = NULL
};

// 示例功能实现



// 接收缓存
uint8_t AsrPro_Temp[ASRPRO_REC_LEN];
uint8_t TempTEST = 0;
uint8_t LevelTEST = 0;

// 初始化ASRPRO设备
void ASRPRO_Init(ASRPRO_Device_t* dev, UART_HandleTypeDef* huart, 
                 void (*callback)(uint8_t, uint8_t*, uint8_t))
{
    dev->huart = huart;
    dev->onDataReceived = callback;
    dev->isInitialized = 1;

    // 启动接收
    HAL_UART_Receive_IT(dev->huart, dev->rxBuffer, ASRPRO_REC_LEN);
}

// 发送命令给语音模块
void ASRPRO_SendCommand(ASRPRO_Device_t* dev, uint8_t cmd, uint8_t* data, uint8_t len)
{
    if (!dev->isInitialized) {
        return;
    }

    uint8_t header[] = {0xAA, 0x00, cmd}; // 包头 + 命令
    
    // 发送包头
    HAL_UART_Transmit(dev->huart, header, sizeof(header), HAL_MAX_DELAY);

    // 发送数据（如果有）
    if (data != NULL && len > 0) {
        HAL_UART_Transmit(dev->huart, data, len, HAL_MAX_DELAY);
    }
}

// 解析接收到的数据
static void ASRPRO_ParseData(ASRPRO_Device_t* dev)
{
    uint8_t* data = dev->rxBuffer;
    
    // 验证包头
    if (data[0] != 0x00 || data[1] != 0xAA) {
        uint8_t error[] = "Invalid Header";
        ASRPRO_SendCommand(dev, ASRPRO_CMD_Null, error, sizeof(error) - 1);
        return;
    }

    // 提取命令和数据
    uint8_t command = data[2];
    uint8_t* payload = &data[3];
    uint8_t payload_len = ASRPRO_REC_LEN - 3;

    // 调用回调函数处理数据
    if (dev->onDataReceived) {
        dev->onDataReceived(command, payload, payload_len);
    }
}

// 接收中断回调函数
void ASRPRO_RxCpltCallback(ASRPRO_Device_t* dev)
{
    if (!dev->isInitialized) {
        return;
    }

    // 解析接收到的数据
    ASRPRO_ParseData(dev);

    // 重新启动接收
    HAL_UART_Receive_IT(dev->huart, dev->rxBuffer, ASRPRO_REC_LEN);
}

// 在UART中断回调中调用此函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == g_ASRPRO_Device.huart) {
        ASRPRO_RxCpltCallback(&g_ASRPRO_Device);
    }
}



