 /**
 * @file IR_Sensor.h
 * @brief 红外传感器驱动头文件
 */

#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// 红外传感器结构体
typedef struct {
    GPIO_TypeDef* port;        // 传感器端口
    uint16_t pin;              // 传感器引脚
    bool inverted;             // 信号是否反转
    uint8_t filter_samples;    // 滤波采样次数
} IR_Sensor_t;

// 函数声明
void IR_Sensor_Init(IR_Sensor_t* sensor, GPIO_TypeDef* port, uint16_t pin, bool inverted, uint8_t filter_samples);
bool IR_Sensor_Read(IR_Sensor_t* sensor);
uint8_t IR_Sensor_ReadFiltered(IR_Sensor_t* sensor);

#endif /* IR_SENSOR_H */