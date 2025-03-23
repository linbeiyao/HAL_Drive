# 高级按钮处理库

此库提供了高级按钮处理功能，包括多种按键事件检测、状态机集成和组合键支持。设计用于嵌入式系统，尤其适合与OLED显示和有限状态机结合使用的场景。

## 主要特性

- **灵活的按钮配置**：支持自定义去抖时间、点击时间、长按时间等参数
- **多种按键事件检测**：支持单击、双击、三击、长按、释放等事件
- **多种按键模式**：
  - 标准模式：支持全部按键事件
  - 即时模式：仅处理简单的按下/释放事件
  - 重复模式：支持按键重复触发，适用于快速调整值
  - FSM模式：与有限状态机集成，实现更复杂的逻辑控制
- **多种检测方式**：
  - 轮询检测：简单可靠，适合大多数应用
  - 中断检测：响应更快，节省CPU资源
- **组合键支持**：检测多个按键的组合按下，并触发特定事件
- **状态机集成**：直接与FSM库配合，实现事件驱动的复杂控制逻辑
- **UI界面支持**：与OLED/UI库协同工作，实现丰富的交互体验

## 使用方法

### 基本初始化

```c
// 初始化按钮库
BTN_Init();

// 获取默认配置
BTN_Config_t btn_config;
BTN_GetDefaultConfig(&btn_config);

// 自定义配置参数
btn_config.debounce_time = 20;      // 去抖时间20ms
btn_config.long_press_time = 1500;  // 长按时间1.5秒
btn_config.mode = BTN_MODE_STANDARD; // 标准模式

// 创建GPIO按钮
BTN_Handle_t* button = BTN_CreateGPIO(0, "OK", GPIOA, GPIO_PIN_0, 0, &btn_config, ButtonCallback, NULL);

// 主循环中处理按钮
void MainLoop(void) {
    while(1) {
        BTN_Process();
        // 其他任务...
        HAL_Delay(10);
    }
}
```

### 按钮回调函数

```c
void ButtonCallback(BTN_ID_t button_id, BTN_Event_t event, void* user_data)
{
    switch(event) {
        case BTN_EVENT_PRESSED:
            printf("按钮 %d 被按下\n", button_id);
            break;
        case BTN_EVENT_RELEASED:
            printf("按钮 %d 被释放\n", button_id);
            break;
        case BTN_EVENT_SINGLE_CLICK:
            printf("按钮 %d 单击\n", button_id);
            break;
        case BTN_EVENT_DOUBLE_CLICK:
            printf("按钮 %d 双击\n", button_id);
            break;
        case BTN_EVENT_TRIPLE_CLICK:
            printf("按钮 %d 三击\n", button_id);
            break;
        case BTN_EVENT_LONG_PRESS:
            printf("按钮 %d 长按\n", button_id);
            break;
        case BTN_EVENT_REPEAT:
            printf("按钮 %d 重复触发\n", button_id);
            break;
    }
}
```

### 使用重复模式

适用于需要频繁重复触发的场景，如调整数值、滚动菜单等。

```c
// 配置为重复模式
btn_config.mode = BTN_MODE_REPEAT;
btn_config.repeat_time = 100;       // 重复间隔100ms
btn_config.repeat_start_time = 500; // 首次重复前等待500ms

// 创建按钮
BTN_Handle_t* btn = BTN_CreateGPIO(0, "UP", GPIOA, GPIO_PIN_0, 0, &btn_config, ButtonCallback, NULL);
```

### 使用中断模式

适用于需要快速响应按键事件或希望节省CPU资源的场景。

```c
// 配置为中断模式
btn_config.detect_mode = BTN_DETECT_INTERRUPT;

// 创建按钮
BTN_Handle_t* btn = BTN_CreateGPIO(0, "OK", GPIOA, GPIO_PIN_0, 0, &btn_config, ButtonCallback, NULL);

// 在GPIO中断处理函数中调用
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BTN_HandleInterrupt(GPIO_Pin);
}
```

### 组合键支持

用于检测多个按键同时按下的场景。

```c
// 定义组合键配置
BTN_ComboConfig_t combo = {
    .mask = BTN_MASK(0) | BTN_MASK(1),  // 按钮0和按钮1的组合
    .callback = ComboKeyCallback,
    .user_data = NULL,
    .time_window = 300                  // 300ms内按下视为组合键
};

// 注册组合键
BTN_RegisterCombo(&combo);

// 组合键回调函数
void ComboKeyCallback(BTN_ID_t btn_id, BTN_Event_t event, void* user_data)
{
    if (event == BTN_EVENT_COMBO) {
        printf("检测到组合键!\n");
        // 处理组合键事件...
    }
}
```

### 与FSM库集成

将按钮事件直接发送到状态机，实现复杂的控制逻辑。

```c
// 创建FSM实例
FSM_Instance_t* fsm = FSM_Create(4, 5, "Controller");

// 配置按钮为FSM模式
btn_config.mode = BTN_MODE_FSM;

// 创建按钮
BTN_Handle_t* btn = BTN_CreateGPIO(0, "OK", GPIOA, GPIO_PIN_0, 0, &btn_config, NULL, NULL);

// 绑定FSM处理函数
BTN_BindFSM(btn, fsm, ButtonFSMEventHandler);

// FSM事件处理函数
void ButtonFSMEventHandler(BTN_ID_t button_id, BTN_Event_t event, void* fsm_instance, void* user_data)
{
    // 创建FSM事件参数
    uint32_t event_param = (button_id << 16) | event;
    
    // 发送事件到FSM
    FSM_SendEvent((FSM_Instance_t*)fsm_instance, EVENT_BUTTON, &event_param);
}
```

## 高级功能

### 动态更新按钮配置

```c
// 获取按钮句柄
BTN_Handle_t* btn = BTN_GetHandleByID(0);

// 更新配置
BTN_Config_t new_config;
BTN_GetDefaultConfig(&new_config);
new_config.mode = BTN_MODE_REPEAT;
BTN_UpdateConfig(btn, &new_config);

// 更改模式
BTN_SetMode(btn, BTN_MODE_STANDARD);
```

### 按钮状态查询

```c
// 检查按钮是否被按下
if (BTN_IsPressed(btn)) {
    // 按钮当前被按下...
}

// 检查按钮是否处于长按状态
if (BTN_IsLongPressed(btn)) {
    // 按钮当前处于长按状态...
}

// 获取当前状态
BTN_State_t state = BTN_GetState(btn);
```

## 与UI界面集成示例

按钮库可以与OLED/UI库集成，实现丰富的交互体验。

```c
// 在按钮回调中更新UI
void ButtonCallback(BTN_ID_t button_id, BTN_Event_t event, void* user_data)
{
    if (event == BTN_EVENT_SINGLE_CLICK) {
        switch (button_id) {
            case 0: // UP按钮
                UIManager_NavigatePrevious();
                break;
            case 1: // DOWN按钮
                UIManager_NavigateNext();
                break;
            case 2: // OK按钮
                UIManager_Select();
                break;
            case 3: // BACK按钮
                UIManager_Back();
                break;
        }
        UIManager_UpdateScreen();
    }
}
```

完整的集成示例可参考 `btn_example.c` 文件。

## 注意事项

1. 按钮处理函数 `BTN_Process()` 需要周期性调用，建议间隔10ms左右
2. 中断模式下，需要确保在GPIO中断中调用 `BTN_HandleInterrupt()` 函数
3. 使用FSM模式时，需要先初始化FSM库并创建FSM实例
4. 组合键检测时，确保时间窗口合理设置，过长可能导致误触发，过短可能无法检测到
5. 按钮配置中的时间参数单位均为毫秒(ms)
6. 最大支持按钮数由 `BTN_CONFIG_MAX_BUTTONS` 定义，默认为16

## API参考

详细API参考请查看 `button.h` 文件中的函数声明和注释。 