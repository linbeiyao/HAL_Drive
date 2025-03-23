/**
 * @file fsm.c
 * @brief 通用有限状态机(FSM)库的实现文件
 * @author HAL_Drive项目组
 * @version 1.0
 * @date 2023-03-23
 */

#include "fsm.h"

// 错误字符串数组
static const char* fsm_error_strings[] = {
    "操作成功",                 // FSM_OK
    "参数无效",                 // FSM_ERR_PARAM_INVALID
    "状态未找到",               // FSM_ERR_STATE_NOT_FOUND
    "状态转换失败",             // FSM_ERR_TRANSITION_FAILED
    "内存不足",                 // FSM_ERR_NO_MEMORY
    "状态已经注册",             // FSM_ERR_ALREADY_REGISTERED
    "状态处于活动状态",         // FSM_ERR_STATE_ACTIVE
    "事件未处理",               // FSM_ERR_EVENT_NOT_HANDLED
    "超时错误",                 // FSM_ERR_TIMEOUT
    "未知错误"                  // FSM_ERR_UNKNOWN
};

/**
 * @brief 查找状态
 * @param machine 状态机实例
 * @param state_id 状态ID
 * @return 状态实例，NULL表示未找到
 */
static FSM_State_t* FSM_FindState(FSM_Machine_t* machine, FSM_StateID_t state_id)
{
    if (!machine) return NULL;
    
    FSM_State_t* state = machine->states;
    while (state) {
        if (state->id == state_id) {
            return state;
        }
        state = state->next;
    }
    return NULL;
}

/**
 * @brief 执行状态转换
 * @param machine 状态机实例
 * @param transition 转换规则
 * @param event 触发事件
 * @return FSM_OK表示成功，其他表示错误
 */
static FSM_Error_t FSM_DoTransition(
    FSM_Machine_t* machine,
    FSM_Transition_t* transition,
    FSM_Event_t* event)
{
    if (!machine || !transition) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 找到目标状态
    FSM_State_t* target_state = FSM_FindState(machine, transition->target);
    if (!target_state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    // 保存当前状态作为上一个状态
    machine->previous_state = machine->current_state;
    
    // 如果当前状态存在，执行退出回调
    if (machine->current_state && machine->current_state->on_exit) {
        FSM_Error_t exit_result = machine->current_state->on_exit(
            machine, machine->current_state, target_state, machine->user_data);
        if (exit_result != FSM_OK) {
            return exit_result;
        }
    }
    
    // 如果转换有动作函数，执行动作
    if (transition->action) {
        FSM_Error_t action_result = transition->action(
            machine, transition, event, machine->user_data);
        if (action_result != FSM_OK) {
            return action_result;
        }
    }
    
    // 切换到目标状态
    machine->current_state = target_state;
    target_state->enter_time = machine->time_now;
    
    // 执行进入回调
    if (target_state->on_enter) {
        FSM_Error_t enter_result = target_state->on_enter(
            machine, target_state, machine->previous_state, machine->user_data);
        if (enter_result != FSM_OK) {
            return enter_result;
        }
    }
    
    // 增加转换计数
    machine->transition_count++;
    
    return FSM_OK;
}

/**
 * @brief 创建状态机
 */
FSM_Machine_t* FSM_Create(const char* name, FSM_UserData_t user_data)
{
    if (!name) return NULL;
    
    // 分配内存
    FSM_Machine_t* machine = (FSM_Machine_t*)malloc(sizeof(FSM_Machine_t));
    if (!machine) return NULL;
    
    // 初始化状态机
    memset(machine, 0, sizeof(FSM_Machine_t));
    strncpy(machine->name, name, sizeof(machine->name) - 1);
    machine->user_data = user_data;
    
    return machine;
}

/**
 * @brief 销毁状态机
 */
void FSM_Destroy(FSM_Machine_t* machine)
{
    if (!machine) return;
    
    // 释放所有状态和转换
    FSM_State_t* state = machine->states;
    while (state) {
        FSM_State_t* next_state = state->next;
        
        // 释放所有转换
        FSM_Transition_t* transition = state->transitions;
        while (transition) {
            FSM_Transition_t* next_transition = transition->next;
            free(transition);
            transition = next_transition;
        }
        
        free(state);
        state = next_state;
    }
    
    // 释放状态机
    free(machine);
}

/**
 * @brief 添加状态
 */
FSM_Error_t FSM_AddState(
    FSM_Machine_t* machine,
    FSM_StateID_t id,
    const char* name,
    FSM_StateEnterCallback_t on_enter,
    FSM_StateExitCallback_t on_exit,
    FSM_StateUpdateCallback_t on_update,
    uint32_t timeout)
{
    if (!machine || !name || id == 0) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 检查状态是否已经存在
    if (FSM_FindState(machine, id)) {
        return FSM_ERR_ALREADY_REGISTERED;
    }
    
    // 创建新状态
    FSM_State_t* state = (FSM_State_t*)malloc(sizeof(FSM_State_t));
    if (!state) {
        return FSM_ERR_NO_MEMORY;
    }
    
    // 初始化状态
    memset(state, 0, sizeof(FSM_State_t));
    state->id = id;
    strncpy(state->name, name, sizeof(state->name) - 1);
    state->on_enter = on_enter;
    state->on_exit = on_exit;
    state->on_update = on_update;
    state->timeout = timeout;
    
    // 添加到状态列表
    if (!machine->states) {
        machine->states = state;
    } else {
        FSM_State_t* last = machine->states;
        while (last->next) {
            last = last->next;
        }
        last->next = state;
    }
    
    machine->state_count++;
    
    return FSM_OK;
}

/**
 * @brief 添加状态转换规则
 */
FSM_Error_t FSM_AddTransition(
    FSM_Machine_t* machine,
    FSM_StateID_t source,
    FSM_StateID_t target,
    FSM_EventID_t event_id,
    FSM_ConditionCallback_t guard,
    FSM_ActionCallback_t action)
{
    if (!machine || source == 0 || target == 0) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 检查源状态和目标状态是否存在
    FSM_State_t* source_state = FSM_FindState(machine, source);
    if (!source_state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    FSM_State_t* target_state = FSM_FindState(machine, target);
    if (!target_state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    // 创建新转换
    FSM_Transition_t* transition = (FSM_Transition_t*)malloc(sizeof(FSM_Transition_t));
    if (!transition) {
        return FSM_ERR_NO_MEMORY;
    }
    
    // 初始化转换
    memset(transition, 0, sizeof(FSM_Transition_t));
    transition->source = source;
    transition->target = target;
    transition->event = event_id;
    transition->guard = guard;
    transition->action = action;
    
    // 添加到转换列表
    if (!source_state->transitions) {
        source_state->transitions = transition;
    } else {
        FSM_Transition_t* last = source_state->transitions;
        while (last->next) {
            last = last->next;
        }
        last->next = transition;
    }
    
    return FSM_OK;
}

/**
 * @brief 设置初始状态
 */
FSM_Error_t FSM_SetInitialState(FSM_Machine_t* machine, FSM_StateID_t state_id)
{
    if (!machine || state_id == 0) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 检查状态是否存在
    FSM_State_t* state = FSM_FindState(machine, state_id);
    if (!state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    // 设置初始状态
    machine->initial_state = state;
    
    return FSM_OK;
}

/**
 * @brief 启动状态机
 */
FSM_Error_t FSM_Start(FSM_Machine_t* machine)
{
    if (!machine) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 检查是否设置了初始状态
    if (!machine->initial_state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    // 如果状态机已经启动，不执行任何操作
    if (machine->current_state) {
        return FSM_OK;
    }
    
    // 设置当前状态为初始状态
    machine->current_state = machine->initial_state;
    machine->initial_state->enter_time = machine->time_now;
    
    // 执行进入回调
    if (machine->initial_state->on_enter) {
        return machine->initial_state->on_enter(
            machine, machine->initial_state, NULL, machine->user_data);
    }
    
    return FSM_OK;
}

/**
 * @brief 停止状态机
 */
FSM_Error_t FSM_Stop(FSM_Machine_t* machine)
{
    if (!machine) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 如果当前状态存在，执行退出回调
    if (machine->current_state && machine->current_state->on_exit) {
        FSM_Error_t exit_result = machine->current_state->on_exit(
            machine, machine->current_state, NULL, machine->user_data);
        if (exit_result != FSM_OK) {
            return exit_result;
        }
    }
    
    // 清除当前状态
    machine->previous_state = machine->current_state;
    machine->current_state = NULL;
    
    return FSM_OK;
}

/**
 * @brief 发送事件到状态机
 */
FSM_Error_t FSM_SendEvent(
    FSM_Machine_t* machine,
    FSM_EventID_t event_id,
    FSM_UserData_t event_data)
{
    if (!machine || !machine->current_state) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 创建事件
    FSM_Event_t event;
    event.id = event_id;
    event.data = event_data;
    
    // 查找匹配的转换
    FSM_Transition_t* transition = machine->current_state->transitions;
    while (transition) {
        // 检查事件是否匹配
        if (transition->event == event_id) {
            // 检查条件是否满足
            if (!transition->guard || transition->guard(machine, transition, &event, machine->user_data)) {
                // 执行转换
                return FSM_DoTransition(machine, transition, &event);
            }
        }
        transition = transition->next;
    }
    
    // 没有匹配的转换
    return FSM_ERR_EVENT_NOT_HANDLED;
}

/**
 * @brief 更新状态机，处理超时等
 */
FSM_Error_t FSM_Update(FSM_Machine_t* machine, uint32_t time_ms)
{
    if (!machine || !machine->current_state) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 更新当前时间
    machine->time_now = time_ms;
    
    // 检查超时
    if (machine->current_state->timeout > 0 && 
        machine->current_state->timeout_state &&
        time_ms - machine->current_state->enter_time >= machine->current_state->timeout) {
        
        // 创建超时转换
        FSM_Transition_t timeout_transition;
        memset(&timeout_transition, 0, sizeof(FSM_Transition_t));
        timeout_transition.source = machine->current_state->id;
        timeout_transition.target = machine->current_state->timeout_state->id;
        
        // 创建空事件
        FSM_Event_t timeout_event;
        memset(&timeout_event, 0, sizeof(FSM_Event_t));
        
        // 执行超时转换
        return FSM_DoTransition(machine, &timeout_transition, &timeout_event);
    }
    
    // 调用当前状态的更新回调
    if (machine->current_state->on_update) {
        return machine->current_state->on_update(
            machine, machine->current_state, machine->user_data);
    }
    
    return FSM_OK;
}

/**
 * @brief 获取当前状态ID
 */
FSM_StateID_t FSM_GetCurrentState(FSM_Machine_t* machine)
{
    if (!machine || !machine->current_state) {
        return 0;
    }
    
    return machine->current_state->id;
}

/**
 * @brief 获取当前状态名称
 */
const char* FSM_GetCurrentStateName(FSM_Machine_t* machine)
{
    if (!machine || !machine->current_state) {
        return NULL;
    }
    
    return machine->current_state->name;
}

/**
 * @brief 获取上一个状态ID
 */
FSM_StateID_t FSM_GetPreviousState(FSM_Machine_t* machine)
{
    if (!machine || !machine->previous_state) {
        return 0;
    }
    
    return machine->previous_state->id;
}

/**
 * @brief 检查是否处于指定状态
 */
int FSM_IsInState(FSM_Machine_t* machine, FSM_StateID_t state_id)
{
    if (!machine || !machine->current_state) {
        return 0;
    }
    
    return machine->current_state->id == state_id ? 1 : 0;
}

/**
 * @brief 获取状态在当前状态的持续时间
 */
uint32_t FSM_GetStateTime(FSM_Machine_t* machine)
{
    if (!machine || !machine->current_state) {
        return 0;
    }
    
    return machine->time_now - machine->current_state->enter_time;
}

/**
 * @brief 设置超时状态
 */
FSM_Error_t FSM_SetTimeoutState(
    FSM_Machine_t* machine,
    FSM_StateID_t state_id,
    FSM_StateID_t timeout_state_id)
{
    if (!machine || state_id == 0 || timeout_state_id == 0) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    // 检查状态是否存在
    FSM_State_t* state = FSM_FindState(machine, state_id);
    if (!state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    FSM_State_t* timeout_state = FSM_FindState(machine, timeout_state_id);
    if (!timeout_state) {
        return FSM_ERR_STATE_NOT_FOUND;
    }
    
    // 设置超时状态
    state->timeout_state = timeout_state;
    
    return FSM_OK;
}

/**
 * @brief 设置用户数据
 */
FSM_Error_t FSM_SetUserData(FSM_Machine_t* machine, FSM_UserData_t user_data)
{
    if (!machine) {
        return FSM_ERR_PARAM_INVALID;
    }
    
    machine->user_data = user_data;
    
    return FSM_OK;
}

/**
 * @brief 获取用户数据
 */
FSM_UserData_t FSM_GetUserData(FSM_Machine_t* machine)
{
    if (!machine) {
        return NULL;
    }
    
    return machine->user_data;
}

/**
 * @brief 获取状态转换次数
 */
uint32_t FSM_GetTransitionCount(FSM_Machine_t* machine)
{
    if (!machine) {
        return 0;
    }
    
    return machine->transition_count;
}

/**
 * @brief 获取错误信息
 */
const char* FSM_GetErrorString(FSM_Error_t error)
{
    if (error < 0 || error >= FSM_ERR_UNKNOWN) {
        return fsm_error_strings[FSM_ERR_UNKNOWN];
    }
    
    return fsm_error_strings[error];
} 