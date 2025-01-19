#ifndef __BUZZER_H
#define __BUZZER_H

#include "main.h"
#include <stdint.h>  // 添加此头文件以支持 uint8_t 和 uint16_t

// 蜂鸣器结构体定义
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t Pin;
} Buzzer_TypeDef;

// 蜂鸣器控制宏定义
#define BUZZER_ON(buzzer)     HAL_GPIO_WritePin((buzzer).GPIO_Port, (buzzer).Pin, GPIO_PIN_RESET)   // 低电平触发
#define BUZZER_OFF(buzzer)    HAL_GPIO_WritePin((buzzer).GPIO_Port, (buzzer).Pin, GPIO_PIN_SET)
#define BUZZER_TOGGLE(buzzer) HAL_GPIO_TogglePin((buzzer).GPIO_Port, (buzzer).Pin)

// 函数声明
void Buzzer_Init(Buzzer_TypeDef* buzzer);                    // 蜂鸣器初始化
void Buzzer_Beep(Buzzer_TypeDef* buzzer, uint16_t time_ms);  // 蜂鸣器响指定时间
void Buzzer_BeepTimes(Buzzer_TypeDef* buzzer, uint8_t times);// 蜂鸣器响指定次数
void Buzzer_Alarm(Buzzer_TypeDef* buzzer);                   // 报警模式

#endif