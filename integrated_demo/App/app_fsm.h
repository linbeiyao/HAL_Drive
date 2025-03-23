/**
 * @file app_fsm.h
 * @brief 应用状态机定义
 * @details 包含应用状态机的状态定义和事件处理函数
 */
#ifndef __APP_FSM_H
#define __APP_FSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_config.h"
#include "fsm.h"
#include "button.h"

// 状态机实例指针
extern FSM_Machine_t* AppFSM;

/**
 * @brief 创建并初始化应用状态机
 * @return 0成功，非0失败
 */
int AppFSM_Init(void);

/**
 * @brief 更新状态机时间和状态
 * @param current_time 当前系统时间(ms)
 */
void AppFSM_Update(uint32_t current_time);

/**
 * @brief 发送按键事件到状态机
 * @param button_id 按键ID
 * @param event 按键事件
 * @param user_data 用户数据
 */
void AppFSM_SendButtonEvent(BTN_ID_t button_id, BTN_Event_t event, void* user_data);

/* 状态回调函数定义 */

/**
 * @brief 初始化状态进入回调
 */
FSM_Error_t AppFSM_InitEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data);

/**
 * @brief 初始化状态更新回调
 */
FSM_Error_t AppFSM_InitUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_UserData_t user_data);

/**
 * @brief 初始化状态退出回调
 */
FSM_Error_t AppFSM_InitExit(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data);

/**
 * @brief 空闲状态进入回调
 */
FSM_Error_t AppFSM_IdleEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data);

/**
 * @brief 空闲状态更新回调
 */
FSM_Error_t AppFSM_IdleUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_UserData_t user_data);

/**
 * @brief 空闲状态退出回调
 */
FSM_Error_t AppFSM_IdleExit(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data);

/**
 * @brief 菜单状态进入回调
 */
FSM_Error_t AppFSM_MenuEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data);

/**
 * @brief 菜单状态更新回调
 */
FSM_Error_t AppFSM_MenuUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_UserData_t user_data);

/**
 * @brief 菜单状态退出回调
 */
FSM_Error_t AppFSM_MenuExit(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data);

/**
 * @brief 数据显示状态进入回调
 */
FSM_Error_t AppFSM_DataDisplayEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                                  FSM_State_t* prev_state, FSM_UserData_t user_data);

/**
 * @brief 数据显示状态更新回调
 */
FSM_Error_t AppFSM_DataDisplayUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                                   FSM_UserData_t user_data);

/**
 * @brief 数据显示状态退出回调
 */
FSM_Error_t AppFSM_DataDisplayExit(FSM_Machine_t* machine, FSM_State_t* state, 
                                 FSM_State_t* next_state, FSM_UserData_t user_data);

/**
 * @brief 设置状态进入回调
 */
FSM_Error_t AppFSM_SettingsEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                               FSM_State_t* prev_state, FSM_UserData_t user_data);

/**
 * @brief 设置状态更新回调
 */
FSM_Error_t AppFSM_SettingsUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                                FSM_UserData_t user_data);

/**
 * @brief 设置状态退出回调
 */
FSM_Error_t AppFSM_SettingsExit(FSM_Machine_t* machine, FSM_State_t* state, 
                              FSM_State_t* next_state, FSM_UserData_t user_data);

/**
 * @brief 错误状态进入回调
 */
FSM_Error_t AppFSM_ErrorEnter(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_State_t* prev_state, FSM_UserData_t user_data);

/**
 * @brief 错误状态更新回调
 */
FSM_Error_t AppFSM_ErrorUpdate(FSM_Machine_t* machine, FSM_State_t* state, 
                             FSM_UserData_t user_data);

/**
 * @brief 错误状态退出回调
 */
FSM_Error_t AppFSM_ErrorExit(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* next_state, FSM_UserData_t user_data);

/* 转换条件和动作函数 */

/**
 * @brief 检查是否可以从初始化转到空闲状态
 */
int AppFSM_CanInitToIdle(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                        FSM_Event_t* event, FSM_UserData_t user_data);

/**
 * @brief 检查是否可以从菜单转到数据显示状态
 */
int AppFSM_CanMenuToDataDisplay(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                               FSM_Event_t* event, FSM_UserData_t user_data);

/**
 * @brief 在错误状态重置操作
 */
FSM_Error_t AppFSM_ErrorReset(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                            FSM_Event_t* event, FSM_UserData_t user_data);

#ifdef __cplusplus
}
#endif

#endif /* __APP_FSM_H */ 