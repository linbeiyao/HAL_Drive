#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 按钮引脚定义 */
#define BUTTON1_PIN GPIO_PIN_12
#define BUTTON1_PORT GPIOB
#define BUTTON2_PIN GPIO_PIN_13
#define BUTTON2_PORT GPIOB
#define BUTTON3_PIN GPIO_PIN_14
#define BUTTON3_PORT GPIOB
#define BUTTON4_PIN GPIO_PIN_15
#define BUTTON4_PORT GPIOB

/* 按钮数量定义 */
#define NUM_BUTTONS 4

/* 按钮编号枚举 */
typedef enum {
    BUTTON_1 = 0,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4
} Button_ID_t;

/* 按钮状态枚举 */
typedef enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED
} Button_State_t;

/* 按钮事件类型 */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_SINGLE_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_PRESS
} Button_Event_t;

/* 按钮结构体 */
typedef struct {
    GPIO_TypeDef* GPIO_Port;               // 按钮所在GPIO端口
    uint16_t GPIO_Pin;                     // 按钮所在GPIO引脚
    Button_State_t State;                  // 当前按钮状态
    uint32_t LastDebounceTime;             // 最后一次状态变化的时间（毫秒）
    void (*Callback)(Button_ID_t button_id, Button_Event_t event); // 按钮事件回调函数

    // 新增成员用于长按和双击检测
    uint32_t PressTime;                    // 按钮按下的时间（毫秒）
    uint32_t ReleaseTime;                  // 按钮释放的时间（毫秒）
    uint8_t ClickCount;                    // 点击次数
    uint8_t IsLongPress;                   // 是否已经触发长按事件
} Button_t;

/* 按钮数组声明 */
extern Button_t buttons[NUM_BUTTONS];

/* 函数原型声明 */
void Button_Init(void);
void Button_RegisterCallback(Button_ID_t button_id, void (*callback)(Button_ID_t, Button_Event_t));
Button_State_t Button_GetState(Button_ID_t button_id);
uint8_t Button_IsPressed(Button_ID_t button_id);
void Button_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H__ */

