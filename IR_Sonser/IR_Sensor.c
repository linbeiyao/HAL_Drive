 /**
 * @file IR_Sensor.c
 * @brief 红外传感器驱动实现文件
 */

#include "IR_Sensor.h"

/**
 * @brief 初始化红外传感器
 * @param sensor 传感器结构体指针
 * @param port GPIO端口
 * @param pin GPIO引脚
 * @param inverted 信号是否反转
 * @param filter_samples 滤波采样次数
 */
void IR_Sensor_Init(IR_Sensor_t* sensor, GPIO_TypeDef* port, uint16_t pin, bool inverted, uint8_t filter_samples) {
    // 初始化结构体
    sensor->port = port;
    sensor->pin = pin;
    sensor->inverted = inverted;
    sensor->filter_samples = filter_samples > 0 ? filter_samples : 1;
    
    // 初始化GPIO为输入模式
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
 * @brief 读取红外传感器状态
 * @param sensor 传感器结构体指针
 * @return 传感器是否检测到物体
 */
bool IR_Sensor_Read(IR_Sensor_t* sensor) {
    GPIO_PinState state = HAL_GPIO_ReadPin(sensor->port, sensor->pin);
    
    // 根据是否反转信号进行处理
    if (sensor->inverted) {
        return (state == GPIO_PIN_RESET);
    } else {
        return (state == GPIO_PIN_SET);
    }
}

/**
 * @brief 带滤波功能的读取红外传感器状态
 * @param sensor 传感器结构体指针
 * @return 0-255之间的值，代表检测到物体的可能性
 */
uint8_t IR_Sensor_ReadFiltered(IR_Sensor_t* sensor) {
    uint8_t count = 0;
    
    // 采样多次以滤波
    for (uint8_t i = 0; i < sensor->filter_samples; i++) {
        if (IR_Sensor_Read(sensor)) {
            count++;
        }
        HAL_Delay(1); // 短暂延时
    }
    
    // 返回检测到物体的可能性 (0-255)
    return (count * 255) / sensor->filter_samples;
}