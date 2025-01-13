#ifndef JDQ_H
#define JDQ_H



#include "stm32f1xx_hal.h"

/**
 * 本文件是继电器的库的头文件
 * 结构体变量初始化之前应该先开启对应引脚的时钟
 * 默认高电平为触发电平
 */
 

// 继电器状态枚举
typedef enum {
    RELAY_OFF = 0,
    RELAY_ON = 1
} RelayState;

// 继电器对象结构体
typedef struct {
    GPIO_TypeDef *GPIO_Port;  // GPIO 端口
    uint16_t GPIO_Pin;        // GPIO 引脚
    RelayState State;         // 当前继电器状态
} Relay;
#include "main.h"

// 函数声明
void Relay_Init(Relay *relay, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin);
void Relay_SetState(Relay *relay, RelayState state);
void Relay_Toggle(Relay *relay);

#endif // JDQ_H
