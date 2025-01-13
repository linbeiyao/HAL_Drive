#include "ASRPRO.h"
#include "string.h"
#include "SmartCup.h"
#include "stdio.h"


// 示例功能实现



// 接收缓存
uint8_t AsrPro_Temp[ASRPRO_REC_LEN];
uint8_t TempTEST = 0;
uint8_t LevelTEST = 0;


// 发送数据给语音模块
void SendAsrpro(uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t header[] = {0xAA, 0x00, cmd}; // 包头 + 命令
    HAL_UART_Transmit(&ASRPRO_UARTX, header, sizeof(header), 100);
    HAL_UART_Transmit(&huart1, header, sizeof(header), 100);

    if (data != NULL && len > 0)
    {
        HAL_UART_Transmit(&ASRPRO_UARTX, data, len, 100);
        HAL_UART_Transmit(&huart1, data, len, 100);
    }
}

// 解析接收到的数据
void ParseAsrproData(uint8_t *data, uint8_t len)
{
    if (len < 3)
    {
        uint8_t error[] = "Invalid Length";
        SendAsrpro(ASRPRO_CMD_Null, (uint8_t*)error, sizeof(error) - 1);
        return;
    }

    // 验证包头
    if (data[0] != 0x00 || data[1] != 0xAA)
    {
        uint8_t error[] = "Invalid Header";
        SendAsrpro(ASRPRO_CMD_Null, (uint8_t*)error, sizeof(error) - 1);
        return;
    }

    uint8_t command = data[2];
    uint8_t *payload = &data[3];
    uint8_t payload_len = len - 3;

    switch (command)
    {
        case ASRPRO_CMD_Broadcast_information: // 播报信息
        {
            printf("bobao info\r\n");
            uint8_t temperature = smart_cup.temperature.current_temperature;
            uint8_t water_level_high = smart_cup.water_level.current_weight;


            uint8_t response[4] = {temperature, water_level_high};
            SendAsrpro(command, (uint8_t*)response, 2);
            break;
        }

        case ASRPRO_CMD_Query_Temperature: // 查询水温
        {
            printf("Query Temperature\r\n");
            uint8_t temperature = smart_cup.temperature.current_temperature;
            SendAsrpro(command, &temperature, 1);
            break;
      
        default: // 未知命令
        {
            uint8_t error[] = "Unknown Command";
            SendAsrpro(ASRPRO_CMD_Null, (uint8_t*)error, sizeof(error) - 1);
            break;
        }
    }
}

// 接收中断回调函数
void ASRPRO_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &ASRPRO_UARTX)
    {
        // 解析接收的数据
        ParseAsrproData(AsrPro_Temp, ASRPRO_REC_LEN);
    }

    // 重新启动接收中断
    HAL_UART_Receive_IT(&ASRPRO_UARTX, AsrPro_Temp, ASRPRO_REC_LEN);
}



