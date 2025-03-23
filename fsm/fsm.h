/**
 * @file fsm.h
 * @brief 通用有限状态机(FSM)库的头文件
 * @author HAL_Drive项目组
 * @version 1.0
 * @date 2023-03-23
 * 
 * @details
 * 该库提供一个轻量级、通用的有限状态机框架，简化复杂逻辑的处理。
 * 特点：
 * - 支持状态转换、事件触发和条件判断
 * - 支持状态进入/退出动作
 * - 支持异步状态转换
 * - 支持状态超时处理
 * - 支持状态历史记录
 */

#ifndef __FSM_H
#define __FSM_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FSM错误码定义
 */
typedef enum {
    FSM_OK = 0,                  ///< 操作成功
    FSM_ERR_PARAM_INVALID,       ///< 参数无效
    FSM_ERR_STATE_NOT_FOUND,     ///< 状态未找到
    FSM_ERR_TRANSITION_FAILED,   ///< 状态转换失败
    FSM_ERR_NO_MEMORY,           ///< 内存不足
    FSM_ERR_ALREADY_REGISTERED,  ///< 状态已经注册
    FSM_ERR_STATE_ACTIVE,        ///< 状态处于活动状态
    FSM_ERR_EVENT_NOT_HANDLED,   ///< 事件未处理
    FSM_ERR_TIMEOUT,             ///< 超时错误
    FSM_ERR_UNKNOWN              ///< 未知错误
} FSM_Error_t;

/**
 * @brief FSM状态ID类型
 */
typedef uint32_t FSM_StateID_t;

/**
 * @brief FSM事件ID类型
 */
typedef uint32_t FSM_EventID_t;

/**
 * @brief FSM用户数据类型
 */
typedef void* FSM_UserData_t;

// 前置声明
struct FSM_Machine;
struct FSM_State;
struct FSM_Transition;
struct FSM_Event;

/**
 * @brief 状态机实例的结构体
 */
typedef struct FSM_Machine {
    char name[32];                   ///< 状态机名称
    struct FSM_State *states;        ///< 状态列表
    uint32_t state_count;            ///< 状态数量
    struct FSM_State *current_state; ///< 当前状态
    struct FSM_State *previous_state;///< 上一个状态
    struct FSM_State *initial_state; ///< 初始状态
    FSM_UserData_t user_data;        ///< 用户数据
    uint32_t time_now;               ///< 当前时间
    uint32_t transition_count;       ///< 状态转换计数
} FSM_Machine_t;

/**
 * @brief 状态进入回调函数类型
 * @param machine 状态机实例
 * @param state 当前状态
 * @param prev_state 上一个状态
 * @param user_data 用户数据
 * @return FSM_OK表示成功，其他表示错误
 */
typedef FSM_Error_t (*FSM_StateEnterCallback_t)(
    struct FSM_Machine *machine,
    struct FSM_State *state,
    struct FSM_State *prev_state,
    FSM_UserData_t user_data);

/**
 * @brief 状态退出回调函数类型
 * @param machine 状态机实例
 * @param state 当前状态
 * @param next_state 下一个状态
 * @param user_data 用户数据
 * @return FSM_OK表示成功，其他表示错误
 */
typedef FSM_Error_t (*FSM_StateExitCallback_t)(
    struct FSM_Machine *machine,
    struct FSM_State *state,
    struct FSM_State *next_state,
    FSM_UserData_t user_data);

/**
 * @brief 状态更新回调函数类型
 * @param machine 状态机实例
 * @param state 当前状态
 * @param user_data 用户数据
 * @return FSM_OK表示成功，其他表示错误
 */
typedef FSM_Error_t (*FSM_StateUpdateCallback_t)(
    struct FSM_Machine *machine,
    struct FSM_State *state,
    FSM_UserData_t user_data);

/**
 * @brief 条件判断回调函数类型
 * @param machine 状态机实例
 * @param transition 转换规则
 * @param event 触发事件
 * @param user_data 用户数据
 * @return 1表示条件满足，0表示条件不满足
 */
typedef int (*FSM_ConditionCallback_t)(
    struct FSM_Machine *machine,
    struct FSM_Transition *transition,
    struct FSM_Event *event,
    FSM_UserData_t user_data);

/**
 * @brief 动作执行回调函数类型
 * @param machine 状态机实例
 * @param transition 转换规则
 * @param event 触发事件
 * @param user_data 用户数据
 * @return FSM_OK表示成功，其他表示错误
 */
typedef FSM_Error_t (*FSM_ActionCallback_t)(
    struct FSM_Machine *machine,
    struct FSM_Transition *transition,
    struct FSM_Event *event,
    FSM_UserData_t user_data);

/**
 * @brief 状态转换定义
 */
typedef struct FSM_Transition {
    FSM_StateID_t source;            ///< 源状态ID
    FSM_StateID_t target;            ///< 目标状态ID
    FSM_EventID_t event;             ///< 触发事件ID
    FSM_ConditionCallback_t guard;   ///< 条件函数
    FSM_ActionCallback_t action;     ///< 动作函数
    struct FSM_Transition *next;     ///< 下一个转换规则
} FSM_Transition_t;

/**
 * @brief 状态定义
 */
typedef struct FSM_State {
    FSM_StateID_t id;                  ///< 状态ID
    char name[32];                     ///< 状态名称
    FSM_StateEnterCallback_t on_enter; ///< 进入状态回调
    FSM_StateExitCallback_t on_exit;   ///< 退出状态回调
    FSM_StateUpdateCallback_t on_update; ///< 状态更新回调
    struct FSM_Transition *transitions; ///< 该状态的转换规则列表
    uint32_t timeout;                  ///< 状态超时时间(ms)，0表示不超时
    struct FSM_State *timeout_state;   ///< 超时后转到的状态
    uint32_t enter_time;               ///< 进入该状态的时间
    struct FSM_State *next;            ///< 下一个状态
} FSM_State_t;

/**
 * @brief 事件定义
 */
typedef struct FSM_Event {
    FSM_EventID_t id;                  ///< 事件ID
    FSM_UserData_t data;               ///< 事件数据
} FSM_Event_t;

/**
 * @brief 创建状态机
 * @param name 状态机名称
 * @param user_data 用户数据
 * @return 状态机实例指针，NULL表示创建失败
 */
FSM_Machine_t* FSM_Create(const char* name, FSM_UserData_t user_data);

/**
 * @brief 销毁状态机
 * @param machine 状态机实例
 */
void FSM_Destroy(FSM_Machine_t* machine);

/**
 * @brief 添加状态
 * @param machine 状态机实例
 * @param id 状态ID
 * @param name 状态名称
 * @param on_enter 进入状态回调
 * @param on_exit 退出状态回调
 * @param on_update 状态更新回调
 * @param timeout 状态超时时间(ms)，0表示不超时
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_AddState(
    FSM_Machine_t* machine,
    FSM_StateID_t id,
    const char* name,
    FSM_StateEnterCallback_t on_enter,
    FSM_StateExitCallback_t on_exit,
    FSM_StateUpdateCallback_t on_update,
    uint32_t timeout);

/**
 * @brief 添加状态转换规则
 * @param machine 状态机实例
 * @param source 源状态ID
 * @param target 目标状态ID
 * @param event_id 触发事件ID
 * @param guard 条件函数，NULL表示无条件
 * @param action 动作函数，NULL表示无动作
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_AddTransition(
    FSM_Machine_t* machine,
    FSM_StateID_t source,
    FSM_StateID_t target,
    FSM_EventID_t event_id,
    FSM_ConditionCallback_t guard,
    FSM_ActionCallback_t action);

/**
 * @brief 设置初始状态
 * @param machine 状态机实例
 * @param state_id 状态ID
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_SetInitialState(FSM_Machine_t* machine, FSM_StateID_t state_id);

/**
 * @brief 启动状态机
 * @param machine 状态机实例
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_Start(FSM_Machine_t* machine);

/**
 * @brief 停止状态机
 * @param machine 状态机实例
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_Stop(FSM_Machine_t* machine);

/**
 * @brief 发送事件到状态机
 * @param machine 状态机实例
 * @param event_id 事件ID
 * @param event_data 事件数据
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_SendEvent(
    FSM_Machine_t* machine,
    FSM_EventID_t event_id,
    FSM_UserData_t event_data);

/**
 * @brief 更新状态机，处理超时等
 * @param machine 状态机实例
 * @param time_ms 当前系统时间(ms)
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_Update(FSM_Machine_t* machine, uint32_t time_ms);

/**
 * @brief 获取当前状态ID
 * @param machine 状态机实例
 * @return 当前状态ID，如果状态机未启动或出错返回0
 */
FSM_StateID_t FSM_GetCurrentState(FSM_Machine_t* machine);

/**
 * @brief 获取当前状态名称
 * @param machine 状态机实例
 * @return 当前状态名称，如果状态机未启动或出错返回NULL
 */
const char* FSM_GetCurrentStateName(FSM_Machine_t* machine);

/**
 * @brief 获取上一个状态ID
 * @param machine 状态机实例
 * @return 上一个状态ID，如果没有上一个状态或出错返回0
 */
FSM_StateID_t FSM_GetPreviousState(FSM_Machine_t* machine);

/**
 * @brief 检查是否处于指定状态
 * @param machine 状态机实例
 * @param state_id 状态ID
 * @return 1表示是，0表示否
 */
int FSM_IsInState(FSM_Machine_t* machine, FSM_StateID_t state_id);

/**
 * @brief 获取状态在当前状态的持续时间
 * @param machine 状态机实例
 * @return 持续时间(ms)
 */
uint32_t FSM_GetStateTime(FSM_Machine_t* machine);

/**
 * @brief 设置超时状态
 * @param machine 状态机实例
 * @param state_id 当前状态ID
 * @param timeout_state_id 超时后要转换到的状态ID
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_SetTimeoutState(
    FSM_Machine_t* machine,
    FSM_StateID_t state_id,
    FSM_StateID_t timeout_state_id);

/**
 * @brief 设置用户数据
 * @param machine 状态机实例
 * @param user_data 用户数据
 * @return FSM_OK表示成功，其他表示错误
 */
FSM_Error_t FSM_SetUserData(FSM_Machine_t* machine, FSM_UserData_t user_data);

/**
 * @brief 获取用户数据
 * @param machine 状态机实例
 * @return 用户数据
 */
FSM_UserData_t FSM_GetUserData(FSM_Machine_t* machine);

/**
 * @brief 获取状态转换次数
 * @param machine 状态机实例
 * @return 状态转换次数
 */
uint32_t FSM_GetTransitionCount(FSM_Machine_t* machine);

/**
 * @brief 获取错误信息
 * @param error 错误码
 * @return 错误信息字符串
 */
const char* FSM_GetErrorString(FSM_Error_t error);

#ifdef __cplusplus
}
#endif

#endif /* __FSM_H */ 