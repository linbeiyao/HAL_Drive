#include "button.h"
#include <stdio.h>   // printf等函数

// 回调函数 
// // 示例回调函数
// void OnButtonCallback(Button_ID_t button_id, Button_Event_t event)
// { 
//     if (g_KeyBoard_Occupy_Flag == 0) {
//         switch (event)
//         {
//         case BUTTON_EVENT_PRESSED:
//             printf("Button %d Pressed\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_RELEASED:
//             printf("Button %d Released\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_SINGLE_CLICK:
//             printf("Button %d Single Click\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_DOUBLE_CLICK:
//             printf("Button %d Double Click\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_LONG_PRESS:
//             printf("Button %d Long Press\r\n", button_id + 1);
//             break;
//         default:
//             break;
//         }
//     }
// }

// 另外一种回调函数
/* 按钮回调函数 */
// void OnButtonCallback(Button_ID_t button_id, Button_Event_t event)
// {
//     if (g_KeyBoard_Occupy_Flag == 0) {
//         switch (event)
//         {
//         case BUTTON_EVENT_PRESSED:
//             printf("Button %d Pressed!\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_RELEASED:
//             printf("Button %d Released!\r\n", button_id + 1);
//             break;
//         case BUTTON_EVENT_SINGLE_CLICK:
//             printf("Button %d Single Click!\r\n", button_id + 1);
//             // 根据按钮ID执行不同的单击逻辑
//             switch (button_id)
//             {
//             case BUTTON_1:
//                 printf("Button 1 Single Click Action\r\n");
//                 // 添加按钮1单击时需要执行的逻辑
//                 break;
//             case BUTTON_2:
//                 printf("Button 2 Single Click Action\r\n");
//                 // 添加按钮2单击时需要执行的逻辑
//                 break;
//             case BUTTON_3:
//                 printf("Button 3 Single Click Action\r\n");
//                 // 添加按钮3单击时需要执行的逻辑
//                 break;
//             case BUTTON_4:
//                 printf("Button 4 Single Click Action\r\n");
//                 // 添加按钮4单击时需要执行的逻辑
//                 break;
//             default:
//                 break;
//             }
//             break;
//         case BUTTON_EVENT_DOUBLE_CLICK:
//             printf("Button %d Double Click!\r\n", button_id + 1);
//             // 根据按钮ID执行不同的双击逻辑
//             switch (button_id)
//             {
//             case BUTTON_1:
//                 printf("Button 1 Double Click Action\r\n");
//                 // 添加按钮1双击时需要执行的逻辑
//                 break;
//             case BUTTON_2:
//                 printf("Button 2 Double Click Action\r\n");
//                 // 添加按钮2双击时需要执行的逻辑
//                 break;
//             case BUTTON_3:
//                 printf("Button 3 Double Click Action\r\n");
//                 // 添加按钮3双击时需要执行的逻辑
//                 break;
//             case BUTTON_4:
//                 printf("Button 4 Double Click Action\r\n");
//                 // 添加按钮4双击时需要执行的逻辑
//                 break;
//             default:
//                 break;
//             }
//             break;
//         case BUTTON_EVENT_LONG_PRESS:
//             printf("Button %d Long Press!\r\n", button_id + 1);
//             // 根据按钮ID执行不同的长按逻辑
//             switch (button_id)
//             {
//             case BUTTON_1:
//                 printf("Button 1 Long Press Action\r\n");
//                 // 添加按钮1长按时需要执行的逻辑
//                 break;
//             case BUTTON_2:
//                 printf("Button 2 Long Press Action\r\n");
//                 // 添加按钮2长按时需要执行的逻辑
//                 break;
//             case BUTTON_3:
//                 printf("Button 3 Long Press Action\r\n");
//                 // 添加按钮3长按时需要执行的逻辑
//                 break;
//             case BUTTON_4:
//                 printf("Button 4 Long Press Action\r\n");
//                 // 添加按钮4长按时需要执行的逻辑
//                 break;
//             default:
//                 break;
//             }
//             break;
//         default:
//             break;
//         }
//     }
// }



/* 初始化阶段 */
// Button_Init(); // 初始化
// // 注册回调
// for (int i = 0; i < NUM_BUTTONS; i++)
// {
//     Button_RegisterCallback(i, OnButtonCallback);
// }


/* 运行阶段 */
// Button_Process(); // 周期性调用



/* 逻辑 */
// // 也可通过标志位判断某些逻辑
// if (Button_IsLongPressed(BUTTON_1))
// {
//     // do something
// }




/* 配置：去抖延时、长按判定时间、双击判定时间 */
#define DEBOUNCE_DELAY_MS         10   // 去抖延时
#define LONG_PRESS_THRESHOLD_MS   1000 // 长按阈值
#define DOUBLE_CLICK_THRESHOLD_MS 30  // 双击阈值

/* 全局按钮数组 */
Button_t g_Buttons[NUM_BUTTONS] =
{
    {BUTTON1_PORT, BUTTON1_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
    {BUTTON2_PORT, BUTTON2_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
    {BUTTON3_PORT, BUTTON3_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0},
    {BUTTON4_PORT, BUTTON4_PIN, BUTTON_RELEASED, 0, NULL, 0, 0, 0, 0},
};

uint8_t g_KeyBoard_Occupy_Flag;     // 键盘占用标识符

/* 全局标志位数组：index 对应每个按钮 */
uint8_t g_ButtonIsPressed[NUM_BUTTONS]     = {0};
uint8_t g_ButtonIsLongPress[NUM_BUTTONS]   = {0};
uint8_t g_ButtonIsDoubleClick[NUM_BUTTONS] = {0};
uint8_t g_ButtonIsSingleClick[NUM_BUTTONS] = {0};

/* 初始化函数：配置GPIO、复位变量 */
void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        GPIO_InitStruct.Pin   = g_Buttons[i].GPIO_Pin;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(g_Buttons[i].GPIO_Port, &GPIO_InitStruct);

        g_Buttons[i].State            = BUTTON_RELEASED;
        g_Buttons[i].LastDebounceTime = HAL_GetTick();
        g_Buttons[i].Callback         = NULL;

        g_Buttons[i].PressTime   = 0;
        g_Buttons[i].ReleaseTime = 0;
        g_Buttons[i].ClickCount  = 0;
        g_Buttons[i].IsLongPress = 0;

        g_ButtonIsPressed[i]     = 0;
        g_ButtonIsLongPress[i]   = 0;
        g_ButtonIsDoubleClick[i] = 0;
        g_ButtonIsSingleClick[i] = 0;
    }

    // 占用标识符
    g_KeyBoard_Occupy_Flag = 0;

    printf("All Buttons Initialized\r\n");
}

/* 注册回调函数 */
void Button_RegisterCallback(Button_ID_t button_id, void (*callback)(Button_ID_t, Button_Event_t))
{
    if (button_id >= NUM_BUTTONS) return;
    g_Buttons[button_id].Callback = callback;
}

/* 获取物理状态 */
Button_State_t Button_GetState(Button_ID_t button_id)
{
    if (button_id >= NUM_BUTTONS) return BUTTON_RELEASED;
    return g_Buttons[button_id].State;
}

/* 按钮处理函数，需在主循环中周期性调用 */
void Button_Process(void)
{
    uint32_t current_time = HAL_GetTick();

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        Button_t* btn = &g_Buttons[i];

        // 假设 低电平 = 按下
        Button_State_t physical_state = 
            (HAL_GPIO_ReadPin(btn->GPIO_Port, btn->GPIO_Pin) == GPIO_PIN_RESET) ? 
                BUTTON_PRESSED : BUTTON_RELEASED;

        // 去抖判断：只有物理状态与当前状态不同 且 超过去抖时间，才更新
        if ((physical_state != btn->State) && 
            ((current_time - btn->LastDebounceTime) > DEBOUNCE_DELAY_MS))
        {
            btn->State = physical_state;
            btn->LastDebounceTime = current_time;

            if (btn->State == BUTTON_PRESSED)
            {
                // 刚按下
                g_ButtonIsPressed[i] = 1;
                btn->PressTime       = current_time;
                btn->IsLongPress     = 0;  // 重置长按标志

                // 回调：按下事件
                if (btn->Callback) {
                    btn->Callback((Button_ID_t)i, BUTTON_EVENT_PRESSED);
                }
            }
            else
            {
                // 刚释放
                g_ButtonIsPressed[i] = 0;
                btn->ReleaseTime     = current_time;
                uint32_t press_duration = btn->ReleaseTime - btn->PressTime;

                if (press_duration >= LONG_PRESS_THRESHOLD_MS)
                {
                    // 长按
                    g_ButtonIsLongPress[i] = 1;
                    if (btn->Callback) {
                        btn->Callback((Button_ID_t)i, BUTTON_EVENT_LONG_PRESS);
                    }
                    // 长按后释放，不再统计单击、双击
                    btn->ClickCount = 0;
                }
                else
                {
                    // 非长按，则可能是单/双击
                    btn->ClickCount++;

                    if (btn->ClickCount == 2)
                    {
                        // 双击
                        g_ButtonIsDoubleClick[i] = 1;
                        if (btn->Callback) {
                            btn->Callback((Button_ID_t)i, BUTTON_EVENT_DOUBLE_CLICK);
                        }
                        // 触发双击后，清空
                        btn->ClickCount = 0;
                    }
                }

                // 回调：释放事件
                if (btn->Callback) {
                    btn->Callback((Button_ID_t)i, BUTTON_EVENT_RELEASED);
                }
            }
        }

        // 若一直按着且还没判定为长按，则继续判断长按
        if ((btn->State == BUTTON_PRESSED) && (!btn->IsLongPress))
        {
            uint32_t press_duration = current_time - btn->PressTime;
            if (press_duration >= LONG_PRESS_THRESHOLD_MS)
            {
                btn->IsLongPress         = 1;
                g_ButtonIsLongPress[i]   = 1;
                if (btn->Callback) {
                    btn->Callback((Button_ID_t)i, BUTTON_EVENT_LONG_PRESS);
                }
            }
        }

        // 单击识别：若只点击一次，超过阈值后仍未出现第二次点击，则判定为单击
        if (btn->ClickCount == 1)
        {
            if ((current_time - btn->ReleaseTime) > DOUBLE_CLICK_THRESHOLD_MS)
            {
                // 单击
                g_ButtonIsSingleClick[i] = 1;
                if (btn->Callback) {
                    btn->Callback((Button_ID_t)i, BUTTON_EVENT_SINGLE_CLICK);
                }
                btn->ClickCount = 0;  // 识别完成后清零
            }
        }

        // 这里根据需求，可在超过阈值后清除各种标志（防止一直置1）       
        if (g_ButtonIsDoubleClick[i] && 
            ((current_time - btn->ReleaseTime) > DOUBLE_CLICK_THRESHOLD_MS))
        {
            g_ButtonIsDoubleClick[i] = 0;
        }
        if (g_ButtonIsSingleClick[i] && 
            ((current_time - btn->ReleaseTime) > DOUBLE_CLICK_THRESHOLD_MS))
        {
            g_ButtonIsSingleClick[i] = 0;
        }
        if (g_ButtonIsLongPress[i] && 
            ((current_time - btn->ReleaseTime) > LONG_PRESS_THRESHOLD_MS))
        {
            g_ButtonIsLongPress[i] = 0;
        }
    }
}

/* 以下函数若不需要，可删除；若需要，可用来读取“是否按下/长按/双击/单击”状态 */
uint8_t Button_IsPressed(Button_ID_t button_id)
{
    if (button_id >= NUM_BUTTONS) return 0;
    return g_ButtonIsPressed[button_id];
}

uint8_t Button_IsLongPressed(Button_ID_t button_id)
{
    if (button_id >= NUM_BUTTONS) return 0;
    return g_ButtonIsLongPress[button_id];
}

uint8_t Button_IsDoubleClicked(Button_ID_t button_id)
{
    if (button_id >= NUM_BUTTONS) return 0;
    return g_ButtonIsDoubleClick[button_id];
}

uint8_t Button_IsSingleClicked(Button_ID_t button_id)
{
    if (button_id >= NUM_BUTTONS) return 0;
    return g_ButtonIsSingleClick[button_id];
}
