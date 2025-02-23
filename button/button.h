#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"  // 需要保证里面有 HAL 库及 GPIO 定义等

/* 按钮引脚定义，根据需要修改 */
#define BUTTON1_PIN   GPIO_PIN_15
#define BUTTON1_PORT  GPIOB
#define BUTTON2_PIN   GPIO_PIN_14
#define BUTTON2_PORT  GPIOB
#define BUTTON3_PIN   GPIO_PIN_13
#define BUTTON3_PORT  GPIOB
#define BUTTON4_PIN   GPIO_PIN_12
#define BUTTON4_PORT  GPIOB

/* 按钮数量 */
#define NUM_BUTTONS   4

/* 按钮枚举 */
typedef enum Button_ID_t{
    BUTTON_1 = 0,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4
} Button_ID_t;

/* 按钮物理状态 */
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
    GPIO_TypeDef*     GPIO_Port;    // 按钮GPIO端口
    uint16_t          GPIO_Pin;     // 按钮GPIO引脚
    Button_State_t    State;        // 当前物理状态
    uint32_t          LastDebounceTime; // 上一次抖动过滤的时间
    void (*Callback)(Button_ID_t button_id, Button_Event_t event); // 回调函数
    
    // 新增信息，用于区分单击、双击、长按
    uint32_t          PressTime;    // 按下时间(ms)
    uint32_t          ReleaseTime;  // 松开时间(ms)
    uint8_t           ClickCount;   // 点击次数（用于区分单/双击）
    uint8_t           IsLongPress;  // 是否已经判定为长按
} Button_t;

/* 全局按钮数组 */
extern Button_t g_Buttons[NUM_BUTTONS];
extern uint8_t g_KeyBoard_Occupy_Flag;
extern uint8_t isPerss;         // 确保
extern uint8_t isDouble;
extern uint8_t isLong;

/* 记录每个按钮当前是否处于“按下 / 长按 / 双击 / 单击”状态的标志位 */
extern uint8_t g_ButtonIsPressed[NUM_BUTTONS];
extern uint8_t g_ButtonIsLongPress[NUM_BUTTONS];
extern uint8_t g_ButtonIsDoubleClick[NUM_BUTTONS];
extern uint8_t g_ButtonIsSingleClick[NUM_BUTTONS];

/* 函数原型 */
void     Button_Init(void);
void     Button_RegisterCallback(Button_ID_t button_id, void (*callback)(Button_ID_t, Button_Event_t));
Button_State_t Button_GetState(Button_ID_t button_id);
void     Button_Process(void);

void 	   OnButtonCallback(Button_ID_t button_id, Button_Event_t event);

/* 提供一些查询接口（可选），从上面四个标志位数组里读出状态 */
uint8_t  Button_IsPressed(Button_ID_t button_id);
uint8_t  Button_IsLongPressed(Button_ID_t button_id);
uint8_t  Button_IsDoubleClicked(Button_ID_t button_id);
uint8_t  Button_IsSingleClicked(Button_ID_t button_id);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H__ */
