#ifndef JDQ_H
#define JDQ_H

#include "stm32f1xx_hal.h"
#include "main.h"

/**
 * @file JDQ.h
 * @brief 继电器控制库头文件
 * 
 * 本库提供对继电器的初始化、控制和状态管理功能。
 * 使用前需确保对应GPIO端口的时钟已开启。
 * 默认高电平为触发电平。
 * 
 * 功能包括：
 * - 继电器初始化
 * - 设置继电器状态
 * - 切换继电器状态
 * - 安全控制继电器
 * - 获取继电器状态
 */

// 继电器状态枚举
typedef enum {
    RELAY_OFF = 0,  // 继电器关闭
    RELAY_ON = 1    // 继电器开启
} RelayState;

// 继电器对象结构体
typedef struct {
    GPIO_TypeDef *GPIO_Port;  // GPIO 端口
    uint16_t GPIO_Pin;        // GPIO 引脚
    RelayState State;         // 当前继电器状态
    uint32_t LastToggleTime;  // 上次切换时间（用于防抖）
} Relay;




// 函数声明
/**
 * @brief 初始化继电器
 * @param relay 继电器对象指针
 * @param GPIO_Port GPIO端口
 * @param GPIO_Pin GPIO引脚
 */
void Relay_Init(Relay *relay, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin);

/**
 * @brief 设置继电器状态
 * @param relay 继电器对象指针
 * @param state 要设置的状态（RELAY_ON/RELAY_OFF）
 */
void Relay_SetState(Relay *relay, RelayState state);

/**
 * @brief 切换继电器状态
 * @param relay 继电器对象指针
 */
void Relay_Toggle(Relay *relay);

/**
 * @brief 安全控制继电器（带状态检查）
 * @param relay 继电器对象指针
 * @param state 要设置的状态
 */
void HAL_Relay_Control(Relay *relay, RelayState state);

/**
 * @brief 获取继电器当前状态
 * @param relay 继电器对象指针
 * @return 当前继电器状态
 */
RelayState Relay_GetState(Relay *relay);

/**
 * @brief 检查继电器是否处于开启状态
 * @param relay 继电器对象指针
 * @return 1-开启，0-关闭
 */
uint8_t Relay_IsOn(Relay *relay);

/**
 * @brief 检查继电器是否处于关闭状态
 * @param relay 继电器对象指针
 * @return 1-关闭，0-开启
 */
uint8_t Relay_IsOff(Relay *relay);

#endif // JDQ_H
