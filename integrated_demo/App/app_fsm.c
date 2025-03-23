/**
 * @file app_fsm.c
 * @brief 应用状态机实现
 * @details 实现应用层状态机的各种回调函数和转换逻辑
 */

#include "app_includes.h"
#include <stdint.h>

/* 全局变量 */
FSM_Machine_t* AppFSM = NULL;

/* 辅助函数定义 */
static void UpdateScreenBasedOnState(AppState_t state);

/**
 * @brief 创建并初始化应用状态机
 * @return 0成功，非0失败
 */
int AppFSM_Init(void)
{
    FSM_Error_t result;
    
    // 创建状态机
    AppFSM = FSM_CreateMachine("AppFSM", NULL);
    if (AppFSM == NULL) {
        return -1;
    }
    
    // 添加状态
    result = FSM_AddState(AppFSM, STATE_INIT, "初始化", AppFSM_InitEnter, 
                         AppFSM_InitExit, AppFSM_InitUpdate, 5000);
    if (result != FSM_OK) return -2;
    
    result = FSM_AddState(AppFSM, STATE_IDLE, "空闲", AppFSM_IdleEnter, 
                         AppFSM_IdleExit, AppFSM_IdleUpdate, 10000);
    if (result != FSM_OK) return -2;
    
    result = FSM_AddState(AppFSM, STATE_MENU, "菜单", AppFSM_MenuEnter, 
                         AppFSM_MenuExit, AppFSM_MenuUpdate, 0);
    if (result != FSM_OK) return -2;
    
    result = FSM_AddState(AppFSM, STATE_DATA_DISPLAY, "数据显示", AppFSM_DataDisplayEnter, 
                         AppFSM_DataDisplayExit, AppFSM_DataDisplayUpdate, 0);
    if (result != FSM_OK) return -2;
    
    result = FSM_AddState(AppFSM, STATE_SETTINGS, "设置", AppFSM_SettingsEnter, 
                         AppFSM_SettingsExit, AppFSM_SettingsUpdate, 0);
    if (result != FSM_OK) return -2;
    
    result = FSM_AddState(AppFSM, STATE_ERROR, "错误", AppFSM_ErrorEnter, 
                         AppFSM_ErrorExit, AppFSM_ErrorUpdate, 0);
    if (result != FSM_OK) return -2;
    
    // 添加转换规则
    // 从初始化到空闲
    result = FSM_AddTransition(AppFSM, STATE_INIT, STATE_IDLE, EVENT_INIT_DONE, 
                              AppFSM_CanInitToIdle, NULL);
    if (result != FSM_OK) return -3;
    
    // 从空闲到菜单
    result = FSM_AddTransition(AppFSM, STATE_IDLE, STATE_MENU, EVENT_KEY_PRESS, 
                              NULL, NULL);
    if (result != FSM_OK) return -3;
    
    // 从菜单到数据显示
    result = FSM_AddTransition(AppFSM, STATE_MENU, STATE_DATA_DISPLAY, EVENT_KEY_PRESS, 
                              AppFSM_CanMenuToDataDisplay, NULL);
    if (result != FSM_OK) return -3;
    
    // 从菜单到设置
    result = FSM_AddTransition(AppFSM, STATE_MENU, STATE_SETTINGS, EVENT_KEY_PRESS, 
                              NULL, NULL);
    if (result != FSM_OK) return -3;
    
    // 从数据显示回到菜单
    result = FSM_AddTransition(AppFSM, STATE_DATA_DISPLAY, STATE_MENU, EVENT_KEY_PRESS, 
                              NULL, NULL);
    if (result != FSM_OK) return -3;
    
    // 从设置回到菜单
    result = FSM_AddTransition(AppFSM, STATE_SETTINGS, STATE_MENU, EVENT_KEY_PRESS, 
                              NULL, NULL);
    if (result != FSM_OK) return -3;
    
    // 从空闲到错误
    result = FSM_AddTransition(AppFSM, STATE_IDLE, STATE_ERROR, EVENT_ERROR, 
                              NULL, NULL);
    if (result != FSM_OK) return -3;
    
    // 从错误回到空闲
    result = FSM_AddTransition(AppFSM, STATE_ERROR, STATE_IDLE, EVENT_KEY_PRESS, 
                              NULL, AppFSM_ErrorReset);
    if (result != FSM_OK) return -3;
    
    // 超时转换设置
    FSM_SetTimeoutState(AppFSM, STATE_INIT, STATE_IDLE);
    FSM_SetTimeoutState(AppFSM, STATE_IDLE, STATE_SLEEP);
    
    // 设置初始状态并启动
    FSM_SetInitialState(AppFSM, STATE_INIT);
    FSM_Start(AppFSM);
    
    return 0;
}

/**
 * @brief 更新状态机时间和状态
 * @param current_time 当前系统时间(ms)
 */
void AppFSM_Update(uint32_t current_time)
{
    if (AppFSM) {
        FSM_Update(AppFSM, current_time);
    }
}

/**
 * @brief 发送按键事件到状态机
 * @param button_id 按键ID
 * @param event 按键事件
 * @param user_data 用户数据
 */
void AppFSM_SendButtonEvent(BTN_ID_t button_id, BTN_Event_t event, void* user_data)
{
    if (!AppFSM) return;
    
    // 创建按键事件参数，高16位为按键ID，低16位为按键事件类型
    uint32_t event_param = (button_id << 16) | event;
    
    // 发送按键事件到状态机
    FSM_SendEvent(AppFSM, EVENT_KEY_PRESS, &event_param);
}

/* 状态回调函数实现 */

FSM_Error_t AppFSM_InitEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    // 初始化状态进入时，显示初始化界面
    UIManager_SwitchScreen(SCREEN_INIT);
    UIManager_ShowToast("系统启动中...", TOAST_INFO, 2000);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_InitUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_UserData_t user_data)
{
    static uint8_t init_progress = 0;
    
    // 模拟初始化进度
    init_progress += 5;
    if (init_progress >= 100) {
        // 初始化完成，发送事件
        FSM_SendEvent(machine, EVENT_INIT_DONE, NULL);
        init_progress = 0;
    }
    
    return FSM_OK;
}

FSM_Error_t AppFSM_InitExit(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data)
{
    // 初始化退出时，显示提示
    UIManager_ShowToast("初始化完成", TOAST_SUCCESS, 1000);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_IdleEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    // 空闲状态进入时，显示主界面
    UIManager_SwitchScreen(SCREEN_MAIN);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_IdleUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_UserData_t user_data)
{
    // 空闲状态更新，更新主界面显示内容
    return FSM_OK;
}

FSM_Error_t AppFSM_IdleExit(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_MenuEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    // 菜单状态进入，显示菜单界面
    UIManager_SwitchScreen(SCREEN_STATUS);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_MenuUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_MenuExit(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_DataDisplayEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                                  FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    // 数据显示状态进入，显示数据界面
    UIManager_SwitchScreen(SCREEN_DATA);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_DataDisplayUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                                   FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_DataDisplayExit(FSM_Machine_t* machine, FSM_State_t* state, 
                                 FSM_State_t* next_state, FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_SettingsEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                               FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    // 设置状态进入，显示设置界面
    UIManager_SwitchScreen(SCREEN_ENV);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_SettingsUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                                FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_SettingsExit(FSM_Machine_t* machine, FSM_State_t* state, 
                              FSM_State_t* next_state, FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_ErrorEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    // 错误状态进入，显示错误提示
    UIManager_ShowDialog("系统错误", "发生错误，请按OK重置", DIALOG_ERROR, NULL);
    
    return FSM_OK;
}

FSM_Error_t AppFSM_ErrorUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                             FSM_UserData_t user_data)
{
    return FSM_OK;
}

FSM_Error_t AppFSM_ErrorExit(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* next_state, FSM_UserData_t user_data)
{
    UIManager_CloseOverlay();
    return FSM_OK;
}

/* 转换条件和动作函数 */

int AppFSM_CanInitToIdle(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                        FSM_Event_t* event, FSM_UserData_t user_data)
{
    // 总是允许从初始化转到空闲状态
    return 1;
}

int AppFSM_CanMenuToDataDisplay(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                               FSM_Event_t* event, FSM_UserData_t user_data)
{
    // 检查是否是OK按键按下事件
    if (event && event->data) {
        uint32_t event_param = *(uint32_t*)event->data;
        uint16_t button_id = (event_param >> 16) & 0xFFFF;
        uint16_t button_event = event_param & 0xFFFF;
        
        if (button_id == 2 && button_event == BTN_EVENT_SINGLE_CLICK) {
            // OK按键单击时允许跳转
            return 1;
        }
    }
    
    return 0;
}

FSM_Error_t AppFSM_ErrorReset(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                            FSM_Event_t* event, FSM_UserData_t user_data)
{
    // 执行系统复位相关操作
    UIManager_ShowToast("系统已重置", TOAST_SUCCESS, 2000);
    
    return FSM_OK;
}

/**
 * @brief 根据当前状态更新显示界面
 * @param state 当前状态
 */
static void UpdateScreenBasedOnState(AppState_t state)
{
    switch (state) {
        case STATE_INIT:
            UIManager_SwitchScreen(SCREEN_INIT);
            break;
            
        case STATE_IDLE:
            UIManager_SwitchScreen(SCREEN_MAIN);
            break;
            
        case STATE_MENU:
            UIManager_SwitchScreen(SCREEN_STATUS);
            break;
            
        case STATE_DATA_DISPLAY:
            UIManager_SwitchScreen(SCREEN_DATA);
            break;
            
        case STATE_SETTINGS:
            UIManager_SwitchScreen(SCREEN_ENV);
            break;
            
        case STATE_ERROR:
            UIManager_SwitchScreen(SCREEN_STATUS);
            UIManager_ShowDialog("系统错误", "发生错误，请按OK重置", DIALOG_ERROR, NULL);
            break;
            
        default:
            break;
    }
} 