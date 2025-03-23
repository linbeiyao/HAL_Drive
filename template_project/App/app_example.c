/**
  ******************************************************************************
  * @file           : app_example.c
  * @brief          : 应用示例
  * @author         : HAL_Drive团队
  * @version        : v1.0.0
  * @date           : 2023-01-01
  ******************************************************************************
  */

#include "app_includes.h"

/* 私有类型定义 ---------------------------------------------------------*/
/* 应用状态枚举 */
typedef enum {
    STATE_IDLE = 0,       /* 空闲状态 */
    STATE_MENU,           /* 菜单状态 */
    STATE_SETTING,        /* 设置状态 */
    STATE_RUNNING,        /* 运行状态 */
    STATE_MAX
} AppState_t;

/* 应用事件枚举 */
typedef enum {
    EVENT_NONE = 0,       /* 无事件 */
    EVENT_KEY_UP,         /* 向上按键事件 */
    EVENT_KEY_DOWN,       /* 向下按键事件 */
    EVENT_KEY_ENTER,      /* 确认按键事件 */
    EVENT_KEY_BACK,       /* 返回按键事件 */
    EVENT_TIMEOUT,        /* 超时事件 */
    EVENT_MAX
} AppEvent_t;

/* 应用数据结构 */
typedef struct {
    uint8_t menu_index;           /* 菜单索引 */
    uint8_t menu_count;           /* 菜单项数量 */
    uint8_t setting_value;        /* 设置值 */
    uint32_t run_time;            /* 运行时间 */
    uint8_t task_ids[5];          /* 任务ID数组 */
    BTN_Handle_t buttons[2];      /* 按钮句柄数组 */
    FSM_Handle_t fsm;             /* 状态机句柄 */
    uint8_t screen_update_flag;   /* 屏幕更新标志 */
} AppData_t;

/* 私有变量 -------------------------------------------------------------*/
static AppData_t app_data;        /* 应用数据 */
static uint32_t system_time = 0;  /* 系统时间(ms) */

/* 私有函数原型 ---------------------------------------------------------*/
static void App_Init(void);
static void App_ButtonInit(void);
static void App_FSMInit(void);
static void App_TaskInit(void);
static void App_UIInit(void);

static void App_UpdateUI(void);
static void App_ProcessEvents(void);

/* 任务回调函数 */
static void Task_ButtonScan(void *param);
static void Task_UIUpdate(void *param);
static void Task_Sensor(void *param);
static void Task_StatusReport(void *param);

/* FSM状态回调函数 */
static void State_IdleEnter(void *user_data, uint32_t state_id);
static void State_IdleExit(void *user_data, uint32_t state_id);
static void State_IdleUpdate(void *user_data, uint32_t state_id);

static void State_MenuEnter(void *user_data, uint32_t state_id);
static void State_MenuExit(void *user_data, uint32_t state_id);
static void State_MenuUpdate(void *user_data, uint32_t state_id);

static void State_SettingEnter(void *user_data, uint32_t state_id);
static void State_SettingExit(void *user_data, uint32_t state_id);
static void State_SettingUpdate(void *user_data, uint32_t state_id);

static void State_RunningEnter(void *user_data, uint32_t state_id);
static void State_RunningExit(void *user_data, uint32_t state_id);
static void State_RunningUpdate(void *user_data, uint32_t state_id);

/* 按钮回调函数 */
static void Button_Callback(BTN_Handle_t *handle, BTN_Event_t event);
static void Button_ComboCallback(uint32_t combo_mask, BTN_Event_t event);

/* 功能函数 -------------------------------------------------------------*/
/**
 * @brief  获取系统时间(ms)
 * @retval 系统时间(ms)
 */
uint32_t App_GetSystemTime(void)
{
    return system_time;
}

/**
 * @brief  系统时钟更新(在SysTick中断中调用)
 * @param  None
 * @retval None
 */
void App_SysTickUpdate(void)
{
    system_time++;
}

/**
 * @brief  应用程序初始化
 * @param  None
 * @retval None
 */
static void App_Init(void)
{
    /* 初始化OLED显示 */
    SSD1306_Init(&hi2c1);
    UI_Init();
    
    /* 清空应用数据 */
    memset(&app_data, 0, sizeof(AppData_t));
    app_data.menu_count = 4;
    
    /* 初始化任务管理器 */
    TaskMan_Init();
    
    /* 初始化按钮 */
    App_ButtonInit();
    
    /* 初始化状态机 */
    App_FSMInit();
    
    /* 初始化任务 */
    App_TaskInit();
    
    /* 初始化UI */
    App_UIInit();
    
    /* 显示启动信息 */
    OLED_Clear();
    OLED_ShowString(30, 20, "HAL_Drive", 16);
    OLED_ShowString(20, 40, "Template App", 16);
    OLED_Refresh();
    
    /* 延时2秒 */
    HAL_Delay(2000);
}

/**
 * @brief  按钮初始化
 * @param  None
 * @retval None
 */
static void App_ButtonInit(void)
{
    BTN_Config_t btn_config;
    
    /* 初始化按钮模块 */
    BTN_Init();
    
    /* 获取默认配置 */
    BTN_GetDefaultConfig(&btn_config);
    
    /* 配置按钮1 */
    btn_config.long_time = 1000;
    btn_config.repeat_time = 150;
    btn_config.mode = BTN_MODE_STANDARD;
    
    /* 创建按钮1 */
    app_data.buttons[0] = BTN_CreateGPIO(0, "KEY1", KEY1_GPIO_PORT, KEY1_GPIO_PIN, KEY1_ACTIVE_LEVEL, &btn_config);
    
    /* 配置按钮2 */
    btn_config.long_time = 800;
    btn_config.mode = BTN_MODE_STANDARD;
    
    /* 创建按钮2 */
    app_data.buttons[1] = BTN_CreateGPIO(1, "KEY2", KEY2_GPIO_PORT, KEY2_GPIO_PIN, KEY2_ACTIVE_LEVEL, &btn_config);
    
    /* 设置按钮回调函数 */
    BTN_SetCallback(app_data.buttons[0], Button_Callback);
    BTN_SetCallback(app_data.buttons[1], Button_Callback);
    
    /* 注册组合按键回调 */
    BTN_RegisterCombo(BTN_MASK(0) | BTN_MASK(1), Button_ComboCallback);
    
    /* 绑定FSM */
    if (app_data.fsm != NULL) {
        BTN_BindFSM(app_data.buttons[0], app_data.fsm);
        BTN_BindFSM(app_data.buttons[1], app_data.fsm);
    }
}

/**
 * @brief  FSM状态机初始化
 * @param  None
 * @retval None
 */
static void App_FSMInit(void)
{
    /* 创建FSM实例 */
    app_data.fsm = FSM_Create("AppFSM");
    if (app_data.fsm == NULL) {
        Error_Handler();
    }
    
    /* 添加状态 */
    FSM_AddState(app_data.fsm, STATE_IDLE, "IDLE", State_IdleEnter, State_IdleUpdate, State_IdleExit);
    FSM_AddState(app_data.fsm, STATE_MENU, "MENU", State_MenuEnter, State_MenuUpdate, State_MenuExit);
    FSM_AddState(app_data.fsm, STATE_SETTING, "SETTING", State_SettingEnter, State_SettingUpdate, State_SettingExit);
    FSM_AddState(app_data.fsm, STATE_RUNNING, "RUNNING", State_RunningEnter, State_RunningUpdate, State_RunningExit);
    
    /* 添加状态转换 */
    FSM_AddTransition(app_data.fsm, STATE_IDLE, STATE_MENU, EVENT_KEY_ENTER);
    FSM_AddTransition(app_data.fsm, STATE_MENU, STATE_IDLE, EVENT_KEY_BACK);
    FSM_AddTransition(app_data.fsm, STATE_MENU, STATE_SETTING, EVENT_KEY_ENTER);
    FSM_AddTransition(app_data.fsm, STATE_SETTING, STATE_MENU, EVENT_KEY_BACK);
    FSM_AddTransition(app_data.fsm, STATE_MENU, STATE_RUNNING, EVENT_KEY_ENTER);
    FSM_AddTransition(app_data.fsm, STATE_RUNNING, STATE_MENU, EVENT_KEY_BACK);
    FSM_AddTransition(app_data.fsm, STATE_IDLE, STATE_IDLE, EVENT_TIMEOUT);
    
    /* 设置初始状态 */
    FSM_SetInitialState(app_data.fsm, STATE_IDLE);
    
    /* 设置用户数据 */
    FSM_SetUserData(app_data.fsm, &app_data);
    
    /* 启动状态机 */
    FSM_Start(app_data.fsm);
}

/**
 * @brief  任务初始化
 * @param  None
 * @retval None
 */
static void App_TaskInit(void)
{
    TaskManError_t err;
    
    /* 创建按钮扫描任务 */
    err = TaskMan_CreateTask("BTN_SCAN", Task_ButtonScan, NULL, 10, 0, TASK_PRIO_HIGH, 1, &app_data.task_ids[0]);
    if (err != TASKMAN_OK) {
        Error_Handler();
    }
    
    /* 创建UI更新任务 */
    err = TaskMan_CreateTask("UI_UPDATE", Task_UIUpdate, NULL, 100, 10, TASK_PRIO_NORMAL, 1, &app_data.task_ids[1]);
    if (err != TASKMAN_OK) {
        Error_Handler();
    }
    
    /* 创建传感器读取任务 */
    err = TaskMan_CreateTask("SENSOR", Task_Sensor, NULL, 500, 20, TASK_PRIO_LOW, 1, &app_data.task_ids[2]);
    if (err != TASKMAN_OK) {
        Error_Handler();
    }
    
    /* 创建状态报告任务 */
    err = TaskMan_CreateTask("STATUS", Task_StatusReport, NULL, 1000, 30, TASK_PRIO_LOW, 1, &app_data.task_ids[3]);
    if (err != TASKMAN_OK) {
        Error_Handler();
    }
    
    /* 启动所有任务 */
    for (int i = 0; i < 4; i++) {
        TaskMan_StartTask(app_data.task_ids[i]);
    }
}

/**
 * @brief  UI初始化
 * @param  None
 * @retval None
 */
static void App_UIInit(void)
{
    /* 注册屏幕 */
    UI_RegisterScreen(SCREEN_MAIN, "主界面");
    UI_RegisterScreen(SCREEN_MENU, "菜单界面");
    UI_RegisterScreen(SCREEN_SETTING, "设置界面");
    UI_RegisterScreen(SCREEN_INFO, "信息界面");
    
    /* 设置当前屏幕 */
    UI_SetCurrentScreen(SCREEN_MAIN);
}

/**
 * @brief  应用程序主循环
 * @param  None
 * @retval None
 */
void App_Run(void)
{
    /* 初始化应用 */
    App_Init();
    
    /* 主循环 */
    while (1) {
        /* 执行任务 */
        TaskMan_Execute(system_time);
        
        /* 处理FSM事件 */
        FSM_Process(app_data.fsm);
        
        /* 更新UI */
        if (app_data.screen_update_flag) {
            App_UpdateUI();
            app_data.screen_update_flag = 0;
        }
        
        /* 空闲处理 */
        HAL_Delay(1);
    }
}

/* 任务回调函数实现 -----------------------------------------------------*/
/**
 * @brief  按钮扫描任务
 * @param  param: 任务参数
 * @retval None
 */
static void Task_ButtonScan(void *param)
{
    /* 处理按钮事件 */
    BTN_Process();
}

/**
 * @brief  UI更新任务
 * @param  param: 任务参数
 * @retval None
 */
static void Task_UIUpdate(void *param)
{
    /* 设置屏幕更新标志 */
    app_data.screen_update_flag = 1;
}

/**
 * @brief  传感器读取任务
 * @param  param: 任务参数
 * @retval None
 */
static void Task_Sensor(void *param)
{
    /* 这里可以读取传感器数据 */
    /* 示例: 模拟增加运行时间 */
    app_data.run_time++;
}

/**
 * @brief  状态报告任务
 * @param  param: 任务参数
 * @retval None
 */
static void Task_StatusReport(void *param)
{
    /* 示例: LED指示系统运行 */
    LED_TOGGLE();
}

/* FSM状态回调函数实现 --------------------------------------------------*/
/**
 * @brief  空闲状态进入回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_IdleEnter(void *user_data, uint32_t state_id)
{
    AppData_t *data = (AppData_t *)user_data;
    
    /* 切换到主屏幕 */
    UI_SetCurrentScreen(SCREEN_MAIN);
    
    /* 立即更新UI */
    App_UpdateUI();
}

/**
 * @brief  空闲状态退出回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_IdleExit(void *user_data, uint32_t state_id)
{
    /* 空闲状态退出处理 */
}

/**
 * @brief  空闲状态更新回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_IdleUpdate(void *user_data, uint32_t state_id)
{
    /* 空闲状态下的定期处理 */
}

/**
 * @brief  菜单状态进入回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_MenuEnter(void *user_data, uint32_t state_id)
{
    AppData_t *data = (AppData_t *)user_data;
    
    /* 重置菜单索引 */
    data->menu_index = 0;
    
    /* 切换到菜单屏幕 */
    UI_SetCurrentScreen(SCREEN_MENU);
    
    /* 立即更新UI */
    App_UpdateUI();
}

/**
 * @brief  菜单状态退出回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_MenuExit(void *user_data, uint32_t state_id)
{
    /* 菜单状态退出处理 */
}

/**
 * @brief  菜单状态更新回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_MenuUpdate(void *user_data, uint32_t state_id)
{
    AppData_t *data = (AppData_t *)user_data;
    
    /* 菜单状态下的定期处理 */
}

/**
 * @brief  设置状态进入回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_SettingEnter(void *user_data, uint32_t state_id)
{
    AppData_t *data = (AppData_t *)user_data;
    
    /* 切换到设置屏幕 */
    UI_SetCurrentScreen(SCREEN_SETTING);
    
    /* 立即更新UI */
    App_UpdateUI();
}

/**
 * @brief  设置状态退出回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_SettingExit(void *user_data, uint32_t state_id)
{
    /* 设置状态退出处理 */
}

/**
 * @brief  设置状态更新回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_SettingUpdate(void *user_data, uint32_t state_id)
{
    /* 设置状态下的定期处理 */
}

/**
 * @brief  运行状态进入回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_RunningEnter(void *user_data, uint32_t state_id)
{
    AppData_t *data = (AppData_t *)user_data;
    
    /* 切换到信息屏幕 */
    UI_SetCurrentScreen(SCREEN_INFO);
    
    /* 立即更新UI */
    App_UpdateUI();
}

/**
 * @brief  运行状态退出回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_RunningExit(void *user_data, uint32_t state_id)
{
    /* 运行状态退出处理 */
}

/**
 * @brief  运行状态更新回调
 * @param  user_data: 用户数据
 * @param  state_id: 状态ID
 * @retval None
 */
static void State_RunningUpdate(void *user_data, uint32_t state_id)
{
    /* 运行状态下的定期处理 */
}

/* 按钮回调函数实现 -----------------------------------------------------*/
/**
 * @brief  按钮回调函数
 * @param  handle: 按钮句柄
 * @param  event: 按钮事件
 * @retval None
 */
static void Button_Callback(BTN_Handle_t *handle, BTN_Event_t event)
{
    AppEvent_t app_event = EVENT_NONE;
    uint8_t button_id = handle->id;
    
    /* 根据按钮ID和事件类型生成应用事件 */
    if (button_id == 0) {  /* KEY1 */
        if (event == BTN_EVENT_CLICK) {
            app_event = EVENT_KEY_UP;
        } else if (event == BTN_EVENT_LONG_PRESS) {
            app_event = EVENT_KEY_ENTER;
        } else if (event == BTN_EVENT_DOUBLE_CLICK) {
            app_event = EVENT_KEY_BACK;
        }
    } else if (button_id == 1) {  /* KEY2 */
        if (event == BTN_EVENT_CLICK) {
            app_event = EVENT_KEY_DOWN;
        } else if (event == BTN_EVENT_LONG_PRESS) {
            app_event = EVENT_KEY_BACK;
        }
    }
    
    /* 如果产生了应用事件，则发送到FSM */
    if (app_event != EVENT_NONE && app_data.fsm != NULL) {
        FSM_SendEvent(app_data.fsm, app_event, NULL, 0);
    }
}

/**
 * @brief  组合按键回调函数
 * @param  combo_mask: 组合按键掩码
 * @param  event: 按键事件
 * @retval None
 */
static void Button_ComboCallback(uint32_t combo_mask, BTN_Event_t event)
{
    /* 处理组合按键事件 */
    if (combo_mask == (BTN_MASK(0) | BTN_MASK(1)) && event == BTN_EVENT_LONG_PRESS) {
        /* 发送超时事件到FSM */
        if (app_data.fsm != NULL) {
            FSM_SendEvent(app_data.fsm, EVENT_TIMEOUT, NULL, 0);
        }
    }
}

/**
 * @brief  更新UI
 * @param  None
 * @retval None
 */
static void App_UpdateUI(void)
{
    UI_ScreenID_t current_screen = UI_GetCurrentScreen();
    
    /* 清屏 */
    OLED_Clear();
    
    /* 根据当前屏幕ID更新UI */
    switch (current_screen) {
        case SCREEN_MAIN:
            /* 绘制主界面 */
            OLED_ShowString(10, 0, "HAL_Drive Template", 12);
            OLED_DrawLine(0, 15, 127, 15);
            OLED_ShowString(10, 20, "Press KEY1 to enter", 12);
            OLED_ShowString(10, 35, "System Time:", 12);
            OLED_ShowNum(90, 35, system_time / 1000, 5, 12);
            OLED_ShowString(10, 50, "Status: Running", 12);
            break;
            
        case SCREEN_MENU:
            /* 绘制菜单界面 */
            OLED_ShowString(40, 0, "MENU", 16);
            OLED_DrawLine(0, 16, 127, 16);
            
            /* 绘制菜单项 */
            char *menu_items[] = {"Settings", "Information", "Run Demo", "About"};
            for (int i = 0; i < app_data.menu_count; i++) {
                if (i == app_data.menu_index) {
                    /* 选中项反色显示 */
                    OLED_ShowString(10, 20 + i * 12, ">", 12);
                }
                OLED_ShowString(20, 20 + i * 12, menu_items[i], 12);
            }
            break;
            
        case SCREEN_SETTING:
            /* 绘制设置界面 */
            OLED_ShowString(30, 0, "SETTINGS", 16);
            OLED_DrawLine(0, 16, 127, 16);
            OLED_ShowString(10, 20, "Value:", 12);
            OLED_ShowNum(60, 20, app_data.setting_value, 3, 12);
            OLED_ShowString(10, 35, "Press UP/DOWN to change", 12);
            OLED_ShowString(10, 50, "Press BACK to return", 12);
            break;
            
        case SCREEN_INFO:
            /* 绘制信息界面 */
            OLED_ShowString(30, 0, "INFO", 16);
            OLED_DrawLine(0, 16, 127, 16);
            OLED_ShowString(10, 20, "Run Time:", 12);
            OLED_ShowNum(80, 20, app_data.run_time, 5, 12);
            OLED_ShowString(10, 35, "Task Count:", 12);
            OLED_ShowNum(80, 35, TaskMan_GetTaskCount(), 2, 12);
            OLED_ShowString(10, 50, "State:", 12);
            OLED_ShowString(60, 50, FSM_GetCurrentStateName(app_data.fsm), 12);
            break;
            
        default:
            break;
    }
    
    /* 刷新显示 */
    OLED_Refresh();
} 