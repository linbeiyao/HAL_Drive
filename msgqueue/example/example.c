#include "msgqueue.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

// 定义消息队列对象
MSGQUEUE_HandleTypeDef hMsgQueue;

// 发送者任务的演示函数
void Sender_Task_Demo(void)
{
    char str_buffer[64];
    uint8_t data_buffer[32];
    uint16_t value = 0;
    
    // 构建消息数据
    sprintf(str_buffer, "传感器数据: %d", value);
    uint16_t size = strlen(str_buffer) + 1; // 包含结束符
    memcpy(data_buffer, str_buffer, size);
    
    // 发送消息到队列
    MSGQUEUE_Status status = MSGQUEUE_Send(&hMsgQueue, data_buffer, size, 0);
    
    // 检查发送结果
    if (status == MSGQUEUE_OK) {
        printf("消息发送成功: %s\r\n", str_buffer);
    } else if (status == MSGQUEUE_FULL) {
        printf("消息队列已满\r\n");
    } else {
        printf("消息发送失败\r\n");
    }
    
    // 增加消息计数
    value++;
}

// 接收者任务的演示函数
void Receiver_Task_Demo(void)
{
    uint8_t data_buffer[32];
    uint16_t received_size = 0;
    
    // 从队列接收消息
    MSGQUEUE_Status status = MSGQUEUE_Receive(&hMsgQueue, data_buffer, 32, &received_size);
    
    // 检查接收结果
    if (status == MSGQUEUE_OK) {
        printf("接收到消息: %s, 大小: %d\r\n", (char *)data_buffer, received_size);
    } else if (status == MSGQUEUE_EMPTY) {
        printf("消息队列为空\r\n");
    } else {
        printf("消息接收失败\r\n");
    }
}

// 队列使用示例
void MsgQueue_Example(void)
{
    // 初始化消息队列
    if (MSGQUEUE_Init(&hMsgQueue, 10, 32) != MSGQUEUE_OK) {
        printf("消息队列初始化失败\r\n");
        return;
    }
    
    printf("消息队列初始化成功，容量: 10, 单消息最大长度: 32\r\n");
    
    // 循环发送和接收消息
    for (int i = 0; i < 5; i++) {
        // 发送多个消息
        for (int j = 0; j < 3; j++) {
            Sender_Task_Demo();
            HAL_Delay(100);
        }
        
        // 接收消息
        while (!MSGQUEUE_IsEmpty(&hMsgQueue)) {
            Receiver_Task_Demo();
            HAL_Delay(100);
        }
        
        HAL_Delay(500);
    }
    
    // 清空消息队列
    MSGQUEUE_Clear(&hMsgQueue);
    printf("消息队列已清空\r\n");
    
    // 发送高优先级消息
    char priority_msg[] = "紧急消息";
    MSGQUEUE_Send(&hMsgQueue, (uint8_t *)priority_msg, strlen(priority_msg) + 1, 10);
    printf("发送高优先级消息\r\n");
    
    // 查看消息但不删除
    uint8_t peek_buffer[32];
    uint16_t peek_size = 0;
    if (MSGQUEUE_Peek(&hMsgQueue, peek_buffer, 32, &peek_size) == MSGQUEUE_OK) {
        printf("查看消息: %s, 大小: %d\r\n", (char *)peek_buffer, peek_size);
    }
    
    // 再次接收同一消息并删除
    uint8_t recv_buffer[32];
    uint16_t recv_size = 0;
    if (MSGQUEUE_Pop(&hMsgQueue, recv_buffer, 32, &recv_size) == MSGQUEUE_OK) {
        printf("删除消息: %s, 大小: %d\r\n", (char *)recv_buffer, recv_size);
    }
    
    // 销毁消息队列
    MSGQUEUE_Deinit(&hMsgQueue);
    printf("消息队列已销毁\r\n");
}

// 实际应用场景：传感器数据采集与处理示例
void Sensor_Data_Collection_Example(void)
{
    // 初始化消息队列，用于存储传感器数据
    MSGQUEUE_HandleTypeDef hSensorQueue;
    MSGQUEUE_Init(&hSensorQueue, 20, 64);
    
    printf("传感器数据采集队列初始化成功\r\n");
    
    // 模拟多个传感器数据采集
    for (int i = 0; i < 10; i++) {
        // 模拟温度传感器数据
        float temperature = 25.5f + (float)(i % 5);
        char temp_data[32];
        sprintf(temp_data, "T:%.1f", temperature);
        MSGQUEUE_Send(&hSensorQueue, (uint8_t *)temp_data, strlen(temp_data) + 1, 1);
        
        // 模拟湿度传感器数据
        float humidity = 60.0f + (float)(i % 10);
        char humid_data[32];
        sprintf(humid_data, "H:%.1f", humidity);
        MSGQUEUE_Send(&hSensorQueue, (uint8_t *)humid_data, strlen(humid_data) + 1, 1);
        
        HAL_Delay(100);
    }
    
    // 处理所有传感器数据
    printf("\n开始处理传感器数据:\r\n");
    float temp_sum = 0.0f, humid_sum = 0.0f;
    int temp_count = 0, humid_count = 0;
    
    while (!MSGQUEUE_IsEmpty(&hSensorQueue)) {
        uint8_t sensor_data[64];
        uint16_t data_size = 0;
        
        if (MSGQUEUE_Pop(&hSensorQueue, sensor_data, 64, &data_size) == MSGQUEUE_OK) {
            printf("处理数据: %s\r\n", (char *)sensor_data);
            
            // 解析数据类型和值
            char type = sensor_data[0];
            float value = 0.0f;
            sscanf((char *)sensor_data + 2, "%f", &value);
            
            // 根据数据类型累加
            if (type == 'T') {
                temp_sum += value;
                temp_count++;
            } else if (type == 'H') {
                humid_sum += value;
                humid_count++;
            }
        }
        
        HAL_Delay(50);
    }
    
    // 计算平均值
    float avg_temp = (temp_count > 0) ? (temp_sum / temp_count) : 0;
    float avg_humid = (humid_count > 0) ? (humid_sum / humid_count) : 0;
    
    printf("\n数据处理结果:\r\n");
    printf("平均温度: %.1f°C (样本数: %d)\r\n", avg_temp, temp_count);
    printf("平均湿度: %.1f%% (样本数: %d)\r\n", avg_humid, humid_count);
    
    // 销毁队列
    MSGQUEUE_Deinit(&hSensorQueue);
}

// 主示例函数，可以在主循环中调用
void MsgQueue_RunExample(void)
{
    printf("\n===== 消息队列基本示例 =====\r\n");
    MsgQueue_Example();
    
    HAL_Delay(1000);
    
    printf("\n===== 传感器数据采集示例 =====\r\n");
    Sensor_Data_Collection_Example();
} 