 /**
 * @file Buzzer.h
 * @brief 蜂鸣器驱动头文件
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// 蜂鸣器结构体
typedef struct {
    GPIO_TypeDef* port;        // 蜂鸣器端口
    uint16_t pin;              // 蜂鸣器引脚
    bool is_active;            // 是否激活
    bool is_active_high;       // 是否高电平有效
} Buzzer_t;

// 函数声明
void Buzzer_Init(Buzzer_t* buzzer, GPIO_TypeDef* port, uint16_t pin, bool is_active_high);
void Buzzer_On(Buzzer_t* buzzer);
void Buzzer_Off(Buzzer_t* buzzer);
void Buzzer_Toggle(Buzzer_t* buzzer);
void Buzzer_Beep(Buzzer_t* buzzer, uint16_t duration_ms);

#endif /* BUZZER_H */