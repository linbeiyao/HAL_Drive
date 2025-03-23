/**
 * @file app_example.c
 * @brief 应用示例程序
 * @details 展示如何使用OLED、按键、状态机和任务管理器库
 */

#include "app_includes.h"

/* 全局变量定义 */
// 传感器模拟数据
static struct {
    float temperature;        // 温度(°C)
    float humidity;           // 湿度(%)
    float pressure;           // 气压(hPa)
    float voltage;            // 电池电压(V)
    uint32_t counter;         // 计数器
} SensorData = {
    .temperature = 25.0f,
    .humidity = 60.0f,
    .pressure = 1013.25f,
    .voltage = 3.3f,
    .counter = 0
};

// 任务句柄
static TaskHandle_t* UITask = NULL;
static TaskHandle_t* KeyTask = NULL;
static TaskHandle_t* DataTask = NULL;
static TaskHandle_t* LEDTask = NULL;

// 按键句柄
static BTN_Handle_t* UpButton = NULL;
static BTN_Handle_t* DownButton = NULL;
static BTN_Handle_t* OkButton = NULL;
static BTN_Handle_t* BackButton = NULL;

/* 函数定义 */

// 按键回调函数
void ButtonCallback(BTN_ID_t button_id, BTN_Event_t event, void* user_data);

// 任务函数
void UITaskFunction(void* param);
void KeyTaskFunction(void* param);
void DataTaskFunction(void* param);
void LEDTaskFunction(void* param);

/**
 * @brief 应用程序初始化
 * @return 0成功，非0失败
 */
int App_Init(void)
{
    int result = 0;
    
    // 初始化OLED
    if (OLED_Init() != OLED_OK) {
        return -1;
    }
    
    // 初始化UI管理器
    UIManager_Init();
    
    // 初始化按键库
    if (BTN_Init() != BTN_OK) {
        return -2;
    }
    
    // 创建按键
    BTN_Config_t btn_config;
    BTN_GetDefaultConfig(&btn_config);
    
    // 设置按键参数
    btn_config.debounce_time = BTN_DEBOUNCE_TIME;
    btn_config.long_press_time = BTN_LONG_PRESS_TIME;
    btn_config.double_click_time = BTN_DOUBLE_CLICK_TIME;
    btn_config.mode = BTN_MODE_STANDARD;
    
    // 创建按键
    UpButton = BTN_CreateGPIO(0, "UP", BTN_UP_GPIO, BTN_UP_PIN, 0, &btn_config, ButtonCallback, NULL);
    DownButton = BTN_CreateGPIO(1, "DOWN", BTN_DOWN_GPIO, BTN_DOWN_PIN, 0, &btn_config, ButtonCallback, NULL);
    OkButton = BTN_CreateGPIO(2, "OK", BTN_OK_GPIO, BTN_OK_PIN, 0, &btn_config, ButtonCallback, NULL);
    BackButton = BTN_CreateGPIO(3, "BACK", BTN_BACK_GPIO, BTN_BACK_PIN, 0, &btn_config, ButtonCallback, NULL);

    if (!UpButton || !DownButton || !OkButton || !BackButton) {
        return -3;
    }
    
    // 初始化状态机
    result = AppFSM_Init();
    if (result != 0) {
        return -4;
    }
    
    // 初始化任务管理器
    if (TaskManager_Init(TASK_MAX_COUNT) != 0) {
        return -5;
    }
    
    // 创建任务
    UITask = TaskManager_CreateTask("UI", UITaskFunction, NULL, TASK_PRIO_MEDIUM, TASK_PERIOD_UI);
    KeyTask = TaskManager_CreateTask("KEY", KeyTaskFunction, NULL, TASK_PRIO_HIGH, TASK_PERIOD_KEY);
    DataTask = TaskManager_CreateTask("DATA", DataTaskFunction, NULL, TASK_PRIO_LOW, TASK_PERIOD_DATA);
    LEDTask = TaskManager_CreateTask("LED", LEDTaskFunction, NULL, TASK_PRIO_LOW, TASK_PERIOD_LED);
    
    if (!UITask || !KeyTask || !DataTask || !LEDTask) {
        return -6;
    }
    
    // 初始化完成，显示欢迎界面
    UIManager_SwitchScreen(SCREEN_INIT);
    UIManager_ShowToast("初始化完成", TOAST_SUCCESS, 2000);
    
    return 0;
}

/**
 * @brief 应用程序主循环
 */
void App_MainLoop(void)
{
    // 启动任务调度器
    TaskManager_StartScheduler();
    
    // 正常情况下不会执行到这里
    while (1) {
        HAL_Delay(1000);
    }
}

/**
 * @brief 按键回调函数
 * @param button_id 按键ID
 * @param event 按键事件
 * @param user_data 用户数据
 */
void ButtonCallback(BTN_ID_t button_id, BTN_Event_t event, void* user_data)
{
    // 检查是否有弹窗显示
    if (UIManager_HasOverlay()) {
        // 弹窗显示时的按键处理
        if (event == BTN_EVENT_SINGLE_CLICK) {
            switch (button_id) {
                case 2: // OK按钮
                    UIManager_DialogConfirm();
                    break;
                    
                case 3: // BACK按钮
                    UIManager_DialogCancel();
                    break;
                    
                default:
                    break;
            }
        }
        return;
    }
    
    // 发送按键事件到状态机
    AppFSM_SendButtonEvent(button_id, event, NULL);
    
    // 根据不同界面处理按键
    UIScreen_t current_screen = UIManager_GetCurrentScreen();
    
    if (event == BTN_EVENT_SINGLE_CLICK) {
        switch (button_id) {
            case 0: // UP按钮
                UIManager_TriggerPreviousScreen();
                break;
                
            case 1: // DOWN按钮
                UIManager_TriggerNextScreen();
                break;
                
            case 2: // OK按钮
                // 根据当前界面处理确认操作
                switch (current_screen) {
                    case SCREEN_MAIN:
                        UIManager_SwitchToSubScreen(SCREEN_MAIN, 0);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case 3: // BACK按钮
                UIManager_ReturnToParentScreen();
                break;
        }
    } else if (event == BTN_EVENT_DOUBLE_CLICK) {
        // 处理双击事件
        switch (button_id) {
            case 2: // OK按钮双击
                UIManager_SwitchScreen(SCREEN_MAIN);
                break;
                
            default:
                break;
        }
    } else if (event == BTN_EVENT_LONG_PRESS) {
        // 处理长按事件
        switch (button_id) {
            case 3: // BACK按钮长按
                // 显示系统信息
                UIManager_ShowDialog("系统信息", "HAL_Drive示例\n版本: 1.0\n按OK继续", DIALOG_INFO, NULL);
                break;
                
            default:
                break;
        }
    }
}

/**
 * @brief UI任务函数
 * @param param 任务参数
 */
void UITaskFunction(void* param)
{
    // 更新UI显示
    UIManager_Update();
    
    // 模拟系统状态变化
    static uint32_t idle_counter = 0;
    idle_counter++;
    
    if (idle_counter > 100) {
        idle_counter = 0;
        
        // 每10秒切换回主界面
        UIScreen_t current_screen = UIManager_GetCurrentScreen();
        if (current_screen != SCREEN_MAIN && current_screen != SCREEN_INIT) {
            UIManager_SwitchScreen(SCREEN_MAIN);
        }
    }
}

/**
 * @brief 按键任务函数
 * @param param 任务参数
 */
void KeyTaskFunction(void* param)
{
    // 处理按键
    BTN_Process();
}

/**
 * @brief 数据任务函数
 * @param param 任务参数
 */
void DataTaskFunction(void* param)
{
    // 更新模拟传感器数据
    SensorData.counter++;
    
    // 温度在20-30°C之间变化
    SensorData.temperature = 25.0f + 5.0f * sinf(SensorData.counter * 0.01f);
    
    // 湿度在50-70%之间变化
    SensorData.humidity = 60.0f + 10.0f * sinf(SensorData.counter * 0.005f);
    
    // 气压在1010-1015hPa之间变化
    SensorData.pressure = 1013.0f + 2.5f * sinf(SensorData.counter * 0.002f);
    
    // 电池电压在3.0-4.2V之间变化
    SensorData.voltage = 3.6f + 0.3f * sinf(SensorData.counter * 0.003f);
    
    // 发送数据更新事件到状态机
    FSM_SendEvent(AppFSM, EVENT_DATA_UPDATE, &SensorData);
}

/**
 * @brief LED任务函数
 * @param param 任务参数
 */
void LEDTaskFunction(void* param)
{
    static uint8_t led_state = 0;
    led_state = !led_state;
    
    // 切换LED状态
    // 注意：需要定义LED引脚和初始化GPIO
    // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief 获取传感器数据
 * @return 传感器数据结构体指针
 */
void* GetSensorData(void)
{
    return &SensorData;
} 