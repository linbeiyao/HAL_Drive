 /**
 * @file LED_Indicator.h
 * @brief LED指示灯驱动头文件
 */

#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// LED状态枚举
typedef enum {
    LED_OFF = 0,
    LED_ON,
    LED_BLINK_SLOW,
    LED_BLINK_FAST
} LED_State_t;

// LED指示灯结构体
typedef struct {
    GPIO_TypeDef* port;        // LED端口
    uint16_t pin;              // LED引脚
    LED_State_t state;         // LED状态
    bool is_active_high;       // 是否高电平有效
    uint32_t blink_timestamp;  // 闪烁时间戳
    uint16_t blink_interval;   // 闪烁间隔(毫秒)
} LED_t;

// 函数声明
void LED_Init(LED_t* led, GPIO_TypeDef* port, uint16_t pin, bool is_active_high);
void LED_SetState(LED_t* led, LED_State_t state);
void LED_Toggle(LED_t* led);
void LED_Process(LED_t* led);
void LED_Blink(LED_t* led, uint16_t interval_ms);

#endif /* LED_INDICATOR_H */