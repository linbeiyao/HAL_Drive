 /**
 * @file LED_Indicator.c
 * @brief LED指示灯驱动实现文件
 */

#include "LED_Indicator.h"

/**
 * @brief 初始化LED指示灯
 * @param led LED结构体指针
 * @param port GPIO端口
 * @param pin GPIO引脚
 * @param is_active_high 是否高电平有效
 */
void LED_Init(LED_t* led, GPIO_TypeDef* port, uint16_t pin, bool is_active_high) {
    // 初始化结构体
    led->port = port;
    led->pin = pin;
    led->state = LED_OFF;
    led->is_active_high = is_active_high;
    led->blink_timestamp = 0;
    led->blink_interval = 500;  // 默认闪烁间隔500ms
    
    // 初始化GPIO为输出模式
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    // 初始状态设为关闭
    if (is_active_high) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    }
}

/**
 * @brief 设置LED状态
 * @param led LED结构体指针
 * @param state LED状态
 */
void LED_SetState(LED_t* led, LED_State_t state) {
    led->state = state;
    
    if (state == LED_ON) {
        if (led->is_active_high) {
            HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
        }
    } else if (state == LED_OFF) {
        if (led->is_active_high) {
            HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
        }
    } else if (state == LED_BLINK_SLOW) {
        led->blink_interval = 500;  // 慢闪：500ms
    } else if (state == LED_BLINK_FAST) {
        led->blink_interval = 200;  // 快闪：200ms
    }
}

/**
 * @brief 翻转LED状态
 * @param led LED结构体指针
 */
void LED_Toggle(LED_t* led) {
    HAL_GPIO_TogglePin(led->port, led->pin);
}

/**
 * @brief 处理LED闪烁
 * @param led LED结构体指针
 */
void LED_Process(LED_t* led) {
    // 处理闪烁状态
    if (led->state == LED_BLINK_SLOW || led->state == LED_BLINK_FAST) {
        uint32_t current_time = HAL_GetTick();
        
        if (current_time - led->blink_timestamp >= led->blink_interval) {
            LED_Toggle(led);
            led->blink_timestamp = current_time;
        }
    }
}

/**
 * @brief 设置LED闪烁
 * @param led LED结构体指针
 * @param interval_ms 闪烁间隔(毫秒)
 */
void LED_Blink(LED_t* led, uint16_t interval_ms) {
    led->blink_interval = interval_ms;
    
    if (interval_ms <= 200) {
        led->state = LED_BLINK_FAST;
    } else {
        led->state = LED_BLINK_SLOW;
    }
}