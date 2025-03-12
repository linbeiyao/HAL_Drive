#ifndef __LED_H
#define __LED_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// LED引脚定义
#define LED_HEARTBEAT_PIN         GPIO_PIN_13
#define LED_HEARTBEAT_GPIO_PORT   GPIOC

// LED状态定义
#define LED_ON                    0  // 低电平点亮
#define LED_OFF                   1  // 高电平熄灭

// LED控制函数
void LED_Init(void);
void LED_HeartbeatToggle(void);
void LED_HeartbeatOn(void);
void LED_HeartbeatOff(void);
void LED_HeartbeatProcess(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */