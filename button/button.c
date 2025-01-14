/**
 * @file button.c
 * 
 * 本文件支持按钮相关的功能，包括初始化、注册回调函数、获取按钮状态、检查按钮是否被按下等。
 * 
 * 实现了按钮的单击、双击、长按等事件。
 * 
 * 
 * 使用方法如下：
 */

/* 更改引脚定义 */


/* main.c 文件中需要重写回调函数 模板如下： */
// /* 按钮回调函数 */
// void OnButtonCallback(Button_ID_t button_id, Button_Event_t event) {
//     switch (event) {
//         case BUTTON_EVENT_PRESSED:
//             printf("Button %d Pressed!\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_RELEASED:
//             printf("Button %d Released!\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_SINGLE_CLICK:
//             printf("Button %d Single Click!\r\n", button_id + 1);
//             // 根据按钮ID执行不同的单击逻辑
//             switch (button_id) {
//                 case BUTTON_1:
//                     printf("Button 1 Single Click Action\r\n");
//                     // 添加按钮1单击时需要执行的逻辑
//                     break;
//                 case BUTTON_2:
//                     printf("Button 2 Single Click Action\r\n");
//                     // 添加按钮2单击时需要执行的逻辑
//                     break;
//                 case BUTTON_3:
//                     printf("Button 3 Single Click Action\r\n");
//                     // 添加按钮3单击时需要执行的逻辑
//                     break;
//                 case BUTTON_4:
//                     printf("Button 4 Single Click Action\r\n");
//                     // 添加按钮4单击时需要执行的逻辑
//                     break;
//                 default:
//                     break;
//             }
//             break;
//         case BUTTON_EVENT_DOUBLE_CLICK:
//             printf("Button %d Double Click!\r\n", button_id + 1);
//             // 根据按钮ID执行不同的双击逻辑
//             switch (button_id) {
//                 case BUTTON_1:
//                     printf("Button 1 Double Click Action\r\n");
//                     // 添加按钮1双击时需要执行的逻辑
//                     break;
//                 case BUTTON_2:
//                     printf("Button 2 Double Click Action\r\n");
//                     // 添加按钮2双击时需要执行的逻辑
//                     break;
//                 case BUTTON_3:
//                     printf("Button 3 Double Click Action\r\n");
//                     // 添加按钮3双击时需要执行的逻辑
//                     break;
//                 case BUTTON_4:
//                     printf("Button 4 Double Click Action\r\n");
//                     // 添加按钮4双击时需要执行的逻辑
//                     break;
//                 default:
//                     break;
//             }
//             break;
//         case BUTTON_EVENT_LONG_PRESS:
//             printf("Button %d Long Press!\r\n", button_id + 1);
//             // 根据按钮ID执行不同的长按逻辑
//             switch (button_id) {
//                 case BUTTON_1:
//                     printf("Button 1 Long Press Action\r\n");
//                     // 添加按钮1长按时需要执行的逻辑
//                     break;
//                 case BUTTON_2:
//                     printf("Button 2 Long Press Action\r\n");
//                     // 添加按钮2长按时需要执行的逻辑
//                     break;
//                 case BUTTON_3:
//                     printf("Button 3 Long Press Action\r\n");
//                     // 添加按钮3长按时需要执行的逻辑
//                     break;
//                 case BUTTON_4:
//                     printf("Button 4 Long Press Action\r\n");
//                     // 添加按钮4长按时需要执行的逻辑
//                     break;
//                 default:
//                     break;
//             }
//             break;
//         default:
//             break;
//     }
// }


/* 在初始化部分 初始化按钮 以及 注册回调函数 */
// /* 初始化按钮 */
// Button_Init();

// /* 注册按钮回调函数 */
// for (int i = 0; i < NUM_BUTTONS; i++) {
//     Button_RegisterCallback(i, OnButtonCallback);
// }

/* 在循环部分 调用按钮处理函数 */
// /* 按钮处理 */
// Button_Process(); 



#include "button.h"
#include "stdio.h"

/* 宏定义 */
#define DEBOUNCE_DELAY_MS         10  // 去抖延时（毫秒）
#define LONG_PRESS_THRESHOLD_MS   6000 // 长按阈值（毫秒）
#define DOUBLE_CLICK_THRESHOLD_MS 100  // 双击阈值（毫秒）

/* 按钮数组定义并初始化 */
Button_t buttons[NUM_BUTTONS] = {
    {BUTTON1_PORT, BUTTON1_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
    {BUTTON2_PORT, BUTTON2_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
    {BUTTON3_PORT, BUTTON3_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
    {BUTTON4_PORT, BUTTON4_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
};

/* 按钮初始化函数 */
void Button_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    for (int i = 0; i < NUM_BUTTONS; i++) {
        GPIO_InitStruct.Pin = buttons[i].GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(buttons[i].GPIO_Port, &GPIO_InitStruct);

        buttons[i].State = BUTTON_RELEASED;
        buttons[i].LastDebounceTime = HAL_GetTick();
        buttons[i].Callback = NULL;

        // 初始化新增成员
        buttons[i].PressTime = 0;
        buttons[i].ReleaseTime = 0;
        buttons[i].ClickCount = 0;
        buttons[i].IsLongPress = 0;
    }

    printf("All Buttons Initialized\r\n");
}

/* 注册按钮回调函数 */
void Button_RegisterCallback(Button_ID_t button_id, void (*callback)(Button_ID_t, Button_Event_t)) {
    if (button_id >= NUM_BUTTONS)
        return;
    buttons[button_id].Callback = callback;
}

/* 获取按钮当前状态 */
Button_State_t Button_GetState(Button_ID_t button_id) {
    if (button_id >= NUM_BUTTONS)
        return BUTTON_RELEASED;
    return buttons[button_id].State;
}

/* 检查按钮是否被按下 */
uint8_t Button_IsPressed(Button_ID_t button_id) {
    if (button_id >= NUM_BUTTONS)
        return 0;
    return (buttons[button_id].State == BUTTON_PRESSED) ? 1 : 0;
}

/* 按钮处理函数，需在主循环中频繁调用 */
void Button_Process(void) {
    uint32_t current_time = HAL_GetTick();

    for (int i = 0; i < NUM_BUTTONS; i++) {
        Button_t* btn = &buttons[i];
        // 读取物理状态，假设低电平为按下
        Button_State_t physical_state = (HAL_GPIO_ReadPin(btn->GPIO_Port, btn->GPIO_Pin) == GPIO_PIN_RESET) ? BUTTON_PRESSED : BUTTON_RELEASED;

        // 去抖处理
        if (physical_state != btn->State && (current_time - btn->LastDebounceTime) > DEBOUNCE_DELAY_MS) {
            btn->State = physical_state;
            btn->LastDebounceTime = current_time;

            if (btn->State == BUTTON_PRESSED) {
                btn->PressTime = current_time;
                btn->IsLongPress = 0; // 重置长按标志
            } else { // BUTTON_RELEASED
                btn->ReleaseTime = current_time;
                uint32_t press_duration = btn->ReleaseTime - btn->PressTime;

                if (press_duration >= LONG_PRESS_THRESHOLD_MS) {
                    // 长按事件
                    if (btn->Callback) {
                        btn->Callback(i, BUTTON_EVENT_LONG_PRESS);
                    }
                    btn->ClickCount = 0; // 重置点击次数
                } else {
                    // 短按，可能是单击或双击
                    btn->ClickCount++;

                    if (btn->ClickCount == 1) {
                        // 记录第一次点击的时间
                        // 在下一次循环中检查是否有第二次点击
                    } else if (btn->ClickCount == 2) {
                        // 双击事件
                        if (btn->Callback) {
                            btn->Callback(i, BUTTON_EVENT_DOUBLE_CLICK);
                        }
                        btn->ClickCount = 0; // 重置点击次数
                    }
                }

                // 触发按下和释放事件
                if (btn->Callback) {
                    Button_Event_t event = (physical_state == BUTTON_PRESSED) ? BUTTON_EVENT_PRESSED : BUTTON_EVENT_RELEASED;
                    btn->Callback(i, event);
                }
            }
        }

        // 处理长按持续检测
        if (btn->State == BUTTON_PRESSED && !btn->IsLongPress) {
            uint32_t press_duration = current_time - btn->PressTime;
            if (press_duration >= LONG_PRESS_THRESHOLD_MS) {
                btn->IsLongPress = 1;
                if (btn->Callback) {
                    btn->Callback(i, BUTTON_EVENT_LONG_PRESS);
                }
            }
        }

        // 处理双击逻辑
        if (btn->ClickCount == 1) {
            if ((current_time - btn->ReleaseTime) > DOUBLE_CLICK_THRESHOLD_MS) {
                // 单击事件
                if (btn->Callback) {
                    btn->Callback(i, BUTTON_EVENT_SINGLE_CLICK);
                }
                btn->ClickCount = 0;
            }
        }
    }
}
