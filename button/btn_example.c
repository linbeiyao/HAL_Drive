/**
 * @file btn_example.c
 * @brief 按钮库使用示例，展示与FSM库和OLED库的集成
 * @author HAL_Drive项目组
 * @version 1.0
 * @date 2023-03-23
 */

#include "button.h"
#include "../fsm/fsm.h"
#include "../oled/UIManager.h"
#include "../oled/UIDrawer.h"
#include <stdio.h>

/* 屏幕ID定义 */
#define SCREEN_MAIN      0
#define SCREEN_MENU      1
#define SCREEN_SETTINGS  2

/* 状态ID定义 */
enum {
    STATE_IDLE = 0,
    STATE_MENU_NAVIGATION,
    STATE_SETTING_ADJUSTMENT,
    STATE_CONFIRMATION,
    STATE_MAX
};

/* 事件ID定义 */
enum {
    EVENT_BUTTON = 0,
    EVENT_TIMEOUT,
    EVENT_CONFIRM,
    EVENT_CANCEL,
    EVENT_MAX
};

/* 用户数据结构 */
typedef struct {
    int current_menu_item;
    int setting_value;
    int menu_count;
    int setting_min;
    int setting_max;
} AppData_t;

/* FSM实例 */
FSM_Instance_t* g_fsm = NULL;
AppData_t g_app_data = {0};

/**
 * @brief 状态进入回调 - 空闲状态
 */
static void OnIdleEnter(FSM_StateID_t from_state, void* user_data)
{
    AppData_t* app_data = (AppData_t*)user_data;
    
    /* 切换到主屏幕 */
    UIManager_SwitchScreen(SCREEN_MAIN);
    printf("进入空闲状态\r\n");
}

/**
 * @brief 状态进入回调 - 菜单导航状态
 */
static void OnMenuNavigationEnter(FSM_StateID_t from_state, void* user_data)
{
    AppData_t* app_data = (AppData_t*)user_data;
    
    /* 切换到菜单屏幕 */
    UIManager_SwitchScreen(SCREEN_MENU);
    printf("进入菜单导航状态，当前菜单项：%d\r\n", app_data->current_menu_item);
}

/**
 * @brief 状态进入回调 - 设置调整状态
 */
static void OnSettingAdjustmentEnter(FSM_StateID_t from_state, void* user_data)
{
    AppData_t* app_data = (AppData_t*)user_data;
    
    /* 切换到设置屏幕 */
    UIManager_SwitchScreen(SCREEN_SETTINGS);
    printf("进入设置调整状态，当前值：%d\r\n", app_data->setting_value);
}

/**
 * @brief 状态进入回调 - 确认状态
 */
static void OnConfirmationEnter(FSM_StateID_t from_state, void* user_data)
{
    /* 显示确认对话框 */
    UIManager_ShowDialog(DIALOG_TYPE_CONFIRM, "确认", "是否保存设置?", NULL);
    printf("进入确认状态\r\n");
}

/**
 * @brief 按钮事件处理 - 空闲状态
 */
static int OnIdleButtonEvent(FSM_EventID_t event_id, void* event_param, FSM_StateID_t* next_state, void* user_data)
{
    BTN_ID_t button_id = (BTN_ID_t)event_param;
    BTN_Event_t btn_event = *(BTN_Event_t*)((char*)event_param + sizeof(BTN_ID_t));
    AppData_t* app_data = (AppData_t*)user_data;
    
    if (btn_event == BTN_EVENT_SINGLE_CLICK) {
        /* 根据按钮决定下一个状态 */
        switch (button_id) {
            case 0: /* 确认键 */
                *next_state = STATE_MENU_NAVIGATION;
                return 1;
                
            case 1: /* 取消键 */
                /* 保持当前状态 */
                UIManager_ShowToast(TOAST_TYPE_INFO, "取消操作", 1000);
                return 0;
                
            default:
                return 0;
        }
    } else if (btn_event == BTN_EVENT_LONG_PRESS) {
        /* 长按可能进入特殊模式 */
        if (button_id == 0) {
            UIManager_ShowToast(TOAST_TYPE_WARNING, "进入特殊模式", 1500);
        }
    }
    
    return 0;
}

/**
 * @brief 按钮事件处理 - 菜单导航状态
 */
static int OnMenuNavigationButtonEvent(FSM_EventID_t event_id, void* event_param, FSM_StateID_t* next_state, void* user_data)
{
    BTN_ID_t button_id = (BTN_ID_t)event_param;
    BTN_Event_t btn_event = *(BTN_Event_t*)((char*)event_param + sizeof(BTN_ID_t));
    AppData_t* app_data = (AppData_t*)user_data;
    
    if (btn_event == BTN_EVENT_SINGLE_CLICK) {
        /* 根据按钮决定行为 */
        switch (button_id) {
            case 0: /* 上一个 */
                app_data->current_menu_item = (app_data->current_menu_item - 1 + app_data->menu_count) % app_data->menu_count;
                UIManager_UpdateCurrentScreen();
                return 0;
                
            case 1: /* 下一个 */
                app_data->current_menu_item = (app_data->current_menu_item + 1) % app_data->menu_count;
                UIManager_UpdateCurrentScreen();
                return 0;
                
            case 2: /* 确认 */
                *next_state = STATE_SETTING_ADJUSTMENT;
                return 1;
                
            case 3: /* 取消 */
                *next_state = STATE_IDLE;
                return 1;
                
            default:
                return 0;
        }
    }
    
    return 0;
}

/**
 * @brief 按钮事件处理 - 设置调整状态
 */
static int OnSettingAdjustmentButtonEvent(FSM_EventID_t event_id, void* event_param, FSM_StateID_t* next_state, void* user_data)
{
    BTN_ID_t button_id = (BTN_ID_t)event_param;
    BTN_Event_t btn_event = *(BTN_Event_t*)((char*)event_param + sizeof(BTN_ID_t));
    AppData_t* app_data = (AppData_t*)user_data;
    
    if (btn_event == BTN_EVENT_SINGLE_CLICK) {
        /* 根据按钮决定行为 */
        switch (button_id) {
            case 0: /* 减少 */
                if (app_data->setting_value > app_data->setting_min) {
                    app_data->setting_value--;
                    UIManager_UpdateCurrentScreen();
                } else {
                    UIManager_ShowToast(TOAST_TYPE_WARNING, "已达最小值", 1000);
                }
                return 0;
                
            case 1: /* 增加 */
                if (app_data->setting_value < app_data->setting_max) {
                    app_data->setting_value++;
                    UIManager_UpdateCurrentScreen();
                } else {
                    UIManager_ShowToast(TOAST_TYPE_WARNING, "已达最大值", 1000);
                }
                return 0;
                
            case 2: /* 确认 */
                *next_state = STATE_CONFIRMATION;
                return 1;
                
            case 3: /* 取消 */
                *next_state = STATE_MENU_NAVIGATION;
                return 1;
                
            default:
                return 0;
        }
    } else if (btn_event == BTN_EVENT_REPEAT) {
        /* 重复触发可以快速调整值 */
        switch (button_id) {
            case 0: /* 快速减少 */
                if (app_data->setting_value > app_data->setting_min) {
                    app_data->setting_value--;
                    UIManager_UpdateCurrentScreen();
                }
                return 0;
                
            case 1: /* 快速增加 */
                if (app_data->setting_value < app_data->setting_max) {
                    app_data->setting_value++;
                    UIManager_UpdateCurrentScreen();
                }
                return 0;
                
            default:
                return 0;
        }
    }
    
    return 0;
}

/**
 * @brief 按钮事件处理 - 确认状态
 */
static int OnConfirmationButtonEvent(FSM_EventID_t event_id, void* event_param, FSM_StateID_t* next_state, void* user_data)
{
    BTN_ID_t button_id = (BTN_ID_t)event_param;
    BTN_Event_t btn_event = *(BTN_Event_t*)((char*)event_param + sizeof(BTN_ID_t));
    
    if (btn_event == BTN_EVENT_SINGLE_CLICK) {
        /* 根据按钮决定行为 */
        switch (button_id) {
            case 2: /* 确认 */
                UIManager_HideDialog();
                UIManager_ShowToast(TOAST_TYPE_SUCCESS, "设置已保存", 1500);
                *next_state = STATE_IDLE;
                return 1;
                
            case 3: /* 取消 */
                UIManager_HideDialog();
                UIManager_ShowToast(TOAST_TYPE_INFO, "操作已取消", 1500);
                *next_state = STATE_SETTING_ADJUSTMENT;
                return 1;
                
            default:
                return 0;
        }
    }
    
    return 0;
}

/**
 * @brief 按键FSM事件处理函数
 */
static void ButtonFSMEventHandler(BTN_ID_t button_id, BTN_Event_t event, void* fsm_instance, void* user_data)
{
    /* 创建FSM事件参数，包含按钮ID和按钮事件 */
    uint32_t event_param[2] = {button_id, event};
    
    /* 发送事件到FSM */
    FSM_SendEvent((FSM_Instance_t*)fsm_instance, EVENT_BUTTON, event_param);
}

/**
 * @brief 组合键回调函数
 */
static void ComboKeyCallback(BTN_ID_t btn_id, BTN_Event_t event, void* user_data)
{
    if (event == BTN_EVENT_COMBO) {
        BTN_ComboMask_t mask = (BTN_ComboMask_t)user_data;
        
        /* 根据组合键掩码执行不同操作 */
        if (mask == (BTN_MASK(0) | BTN_MASK(1))) {
            UIManager_ShowToast(TOAST_TYPE_INFO, "组合键 0+1 触发", 2000);
        } else if (mask == (BTN_MASK(2) | BTN_MASK(3))) {
            UIManager_ShowToast(TOAST_TYPE_WARNING, "组合键 2+3 触发", 2000);
        }
    }
}

/**
 * @brief 初始化FSM
 */
static void InitFSM(void)
{
    /* 创建FSM实例 */
    g_fsm = FSM_Create(STATE_MAX, EVENT_MAX, "App FSM");
    if (!g_fsm) {
        printf("FSM创建失败\r\n");
        return;
    }
    
    /* 初始化应用数据 */
    g_app_data.current_menu_item = 0;
    g_app_data.setting_value = 50;
    g_app_data.menu_count = 5;
    g_app_data.setting_min = 0;
    g_app_data.setting_max = 100;
    
    /* 设置状态进入回调 */
    FSM_SetStateEnterCallback(g_fsm, STATE_IDLE, OnIdleEnter);
    FSM_SetStateEnterCallback(g_fsm, STATE_MENU_NAVIGATION, OnMenuNavigationEnter);
    FSM_SetStateEnterCallback(g_fsm, STATE_SETTING_ADJUSTMENT, OnSettingAdjustmentEnter);
    FSM_SetStateEnterCallback(g_fsm, STATE_CONFIRMATION, OnConfirmationEnter);
    
    /* 设置事件处理函数 */
    FSM_SetEventHandler(g_fsm, STATE_IDLE, EVENT_BUTTON, OnIdleButtonEvent);
    FSM_SetEventHandler(g_fsm, STATE_MENU_NAVIGATION, EVENT_BUTTON, OnMenuNavigationButtonEvent);
    FSM_SetEventHandler(g_fsm, STATE_SETTING_ADJUSTMENT, EVENT_BUTTON, OnSettingAdjustmentButtonEvent);
    FSM_SetEventHandler(g_fsm, STATE_CONFIRMATION, EVENT_BUTTON, OnConfirmationButtonEvent);
    
    /* 设置初始状态和用户数据 */
    FSM_SetInitialState(g_fsm, STATE_IDLE);
    FSM_SetUserData(g_fsm, &g_app_data);
    
    /* 启动FSM */
    FSM_Start(g_fsm);
}

/**
 * @brief 初始化按钮
 */
static void InitButtons(void)
{
    /* 初始化按钮库 */
    BTN_Init();
    
    /* 按钮配置 */
    BTN_Config_t btn_config;
    BTN_GetDefaultConfig(&btn_config);
    btn_config.mode = BTN_MODE_FSM;
    btn_config.detect_mode = BTN_DETECT_POLLING;
    btn_config.enable_repeat = 1;
    
    /* 创建4个按钮 */
    BTN_Handle_t* btn0 = BTN_CreateGPIO(0, "UP", BUTTON0_GPIO_PORT, BUTTON0_GPIO_PIN, 0, &btn_config, NULL, NULL);
    BTN_Handle_t* btn1 = BTN_CreateGPIO(1, "DOWN", BUTTON1_GPIO_PORT, BUTTON1_GPIO_PIN, 0, &btn_config, NULL, NULL);
    BTN_Handle_t* btn2 = BTN_CreateGPIO(2, "OK", BUTTON2_GPIO_PORT, BUTTON2_GPIO_PIN, 0, &btn_config, NULL, NULL);
    BTN_Handle_t* btn3 = BTN_CreateGPIO(3, "CANCEL", BUTTON3_GPIO_PORT, BUTTON3_GPIO_PIN, 0, &btn_config, NULL, NULL);
    
    /* 绑定FSM事件处理函数 */
    BTN_BindFSM(btn0, g_fsm, ButtonFSMEventHandler);
    BTN_BindFSM(btn1, g_fsm, ButtonFSMEventHandler);
    BTN_BindFSM(btn2, g_fsm, ButtonFSMEventHandler);
    BTN_BindFSM(btn3, g_fsm, ButtonFSMEventHandler);
    
    /* 注册组合键 */
    BTN_ComboConfig_t combo1 = {
        .mask = BTN_MASK(0) | BTN_MASK(1),
        .callback = ComboKeyCallback,
        .user_data = (void*)(BTN_MASK(0) | BTN_MASK(1)),
        .time_window = 300
    };
    
    BTN_ComboConfig_t combo2 = {
        .mask = BTN_MASK(2) | BTN_MASK(3),
        .callback = ComboKeyCallback,
        .user_data = (void*)(BTN_MASK(2) | BTN_MASK(3)),
        .time_window = 300
    };
    
    BTN_RegisterCombo(&combo1);
    BTN_RegisterCombo(&combo2);
}

/**
 * @brief 初始化UI
 */
static void InitUI(void)
{
    /* 初始化OLED和UI管理器 */
    // OLED_Init();
    UIManager_Init();
    
    /* 注册屏幕 */
    // 这里应该注册各个屏幕的绘制函数
    // UIManager_RegisterScreen(SCREEN_MAIN, DrawMainScreen);
    // UIManager_RegisterScreen(SCREEN_MENU, DrawMenuScreen);
    // UIManager_RegisterScreen(SCREEN_SETTINGS, DrawSettingsScreen);
    
    /* 设置初始屏幕 */
    UIManager_SwitchScreen(SCREEN_MAIN);
}

/**
 * @brief 主函数循环
 */
void ButtonExample_Run(void)
{
    /* 初始化 */
    InitFSM();
    InitUI();
    InitButtons();
    
    printf("按钮示例启动，与FSM和OLED集成\r\n");
    
    /* 主循环 */
    while (1) {
        /* 处理按钮事件 */
        BTN_Process();
        
        /* 更新UI */
        UIManager_Process();
        
        /* 处理其他逻辑... */
        
        /* 延时 */
        HAL_Delay(10);
    }
}

/* 注：此示例代码需要根据实际硬件和工程结构进行适配 */ 