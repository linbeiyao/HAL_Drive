/**
 * @file fsm_example.c
 * @brief 有限状态机(FSM)使用示例
 * @author HAL_Drive项目组
 * @version 1.0
 * @date 2023-03-23
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fsm.h"

/* 定义状态ID */
#define STATE_IDLE      1
#define STATE_WORKING   2
#define STATE_ERROR     3
#define STATE_PAUSE     4

/* 定义事件ID */
#define EVENT_START     1
#define EVENT_STOP      2
#define EVENT_ERROR     3
#define EVENT_RESET     4
#define EVENT_PAUSE     5
#define EVENT_RESUME    6

/* 用户数据结构 */
typedef struct {
    int device_id;
    int error_code;
    int work_progress;
} Device_Data_t;

/* 模拟系统时间 */
static uint32_t system_time = 0;

/**
 * @brief 获取系统时间（毫秒）
 */
uint32_t get_system_time(void)
{
    return system_time;
}

/**
 * @brief 模拟时间流逝
 */
void simulate_time_passing(uint32_t ms)
{
    system_time += ms;
}

/* 状态回调函数 */

/**
 * @brief 空闲状态进入回调
 */
FSM_Error_t on_enter_idle(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 进入空闲状态\n", get_system_time(), data->device_id);
    data->work_progress = 0;
    return FSM_OK;
}

/**
 * @brief 空闲状态退出回调
 */
FSM_Error_t on_exit_idle(FSM_Machine_t* machine, FSM_State_t* state, 
                         FSM_State_t* next_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 退出空闲状态\n", get_system_time(), data->device_id);
    return FSM_OK;
}

/**
 * @brief 工作状态进入回调
 */
FSM_Error_t on_enter_working(FSM_Machine_t* machine, FSM_State_t* state, 
                             FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 开始工作\n", get_system_time(), data->device_id);
    return FSM_OK;
}

/**
 * @brief 工作状态退出回调
 */
FSM_Error_t on_exit_working(FSM_Machine_t* machine, FSM_State_t* state, 
                            FSM_State_t* next_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 停止工作, 进度: %d%%\n", 
           get_system_time(), data->device_id, data->work_progress);
    return FSM_OK;
}

/**
 * @brief 工作状态更新回调
 */
FSM_Error_t on_update_working(FSM_Machine_t* machine, FSM_State_t* state, 
                              FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    
    // 模拟工作进度
    if (data->work_progress < 100) {
        data->work_progress += 5;
        if (data->work_progress > 100) {
            data->work_progress = 100;
        }
        
        printf("[%u] 设备 %d 工作进度: %d%%\n", 
               get_system_time(), data->device_id, data->work_progress);
        
        // 模拟随机错误
        if (data->work_progress == 50 && (rand() % 10) == 0) {
            data->error_code = 100;
            printf("[%u] 设备 %d 检测到错误，准备发送错误事件\n", 
                   get_system_time(), data->device_id);
            FSM_SendEvent(machine, EVENT_ERROR, NULL);
        }
        
        // 如果完成工作，自动停止
        if (data->work_progress == 100) {
            printf("[%u] 设备 %d 工作完成，自动停止\n", 
                   get_system_time(), data->device_id);
            FSM_SendEvent(machine, EVENT_STOP, NULL);
        }
    }
    
    return FSM_OK;
}

/**
 * @brief 错误状态进入回调
 */
FSM_Error_t on_enter_error(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 进入错误状态，错误码: %d\n", 
           get_system_time(), data->device_id, data->error_code);
    return FSM_OK;
}

/**
 * @brief 错误状态退出回调
 */
FSM_Error_t on_exit_error(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* next_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 错误已清除\n", get_system_time(), data->device_id);
    data->error_code = 0;
    return FSM_OK;
}

/**
 * @brief 暂停状态进入回调
 */
FSM_Error_t on_enter_pause(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* prev_state, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 设备 %d 已暂停，当前进度: %d%%\n", 
           get_system_time(), data->device_id, data->work_progress);
    return FSM_OK;
}

/* 转换条件函数 */

/**
 * @brief 检查是否可以重置
 */
int check_can_reset(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                    FSM_Event_t* event, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 检查设备 %d 是否可以重置...\n", get_system_time(), data->device_id);
    
    // 模拟某些错误不能直接重置
    if (data->error_code > 500) {
        printf("[%u] 错误过于严重，不能重置\n", get_system_time());
        return 0;
    }
    
    return 1;
}

/* 转换动作函数 */

/**
 * @brief 执行重置操作
 */
FSM_Error_t do_reset(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                     FSM_Event_t* event, FSM_UserData_t user_data)
{
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("[%u] 执行设备 %d 重置操作\n", get_system_time(), data->device_id);
    return FSM_OK;
}

/**
 * @brief 主函数
 */
int main(void)
{
    FSM_Error_t result;
    
    // 创建设备数据
    Device_Data_t device_data = {
        .device_id = 1,
        .error_code = 0,
        .work_progress = 0
    };
    
    // 创建状态机
    FSM_Machine_t* machine = FSM_Create("设备控制", &device_data);
    if (!machine) {
        printf("创建状态机失败\n");
        return -1;
    }
    
    // 添加状态
    result = FSM_AddState(machine, STATE_IDLE, "空闲", 
                          on_enter_idle, on_exit_idle, NULL, 0);
    if (result != FSM_OK) {
        printf("添加空闲状态失败: %s\n", FSM_GetErrorString(result));
        FSM_Destroy(machine);
        return -1;
    }
    
    result = FSM_AddState(machine, STATE_WORKING, "工作中", 
                          on_enter_working, on_exit_working, on_update_working, 0);
    if (result != FSM_OK) {
        printf("添加工作状态失败: %s\n", FSM_GetErrorString(result));
        FSM_Destroy(machine);
        return -1;
    }
    
    result = FSM_AddState(machine, STATE_ERROR, "错误", 
                          on_enter_error, on_exit_error, NULL, 0);
    if (result != FSM_OK) {
        printf("添加错误状态失败: %s\n", FSM_GetErrorString(result));
        FSM_Destroy(machine);
        return -1;
    }
    
    result = FSM_AddState(machine, STATE_PAUSE, "暂停", 
                          on_enter_pause, NULL, NULL, 0);
    if (result != FSM_OK) {
        printf("添加暂停状态失败: %s\n", FSM_GetErrorString(result));
        FSM_Destroy(machine);
        return -1;
    }
    
    // 添加转换规则
    FSM_AddTransition(machine, STATE_IDLE, STATE_WORKING, EVENT_START, NULL, NULL);
    FSM_AddTransition(machine, STATE_WORKING, STATE_IDLE, EVENT_STOP, NULL, NULL);
    FSM_AddTransition(machine, STATE_WORKING, STATE_ERROR, EVENT_ERROR, NULL, NULL);
    FSM_AddTransition(machine, STATE_ERROR, STATE_IDLE, EVENT_RESET, check_can_reset, do_reset);
    FSM_AddTransition(machine, STATE_WORKING, STATE_PAUSE, EVENT_PAUSE, NULL, NULL);
    FSM_AddTransition(machine, STATE_PAUSE, STATE_WORKING, EVENT_RESUME, NULL, NULL);
    
    // 设置超时状态 (10秒超时自动从工作转为空闲)
    FSM_SetTimeoutState(machine, STATE_WORKING, STATE_IDLE);
    
    // 设置初始状态并启动
    FSM_SetInitialState(machine, STATE_IDLE);
    result = FSM_Start(machine);
    if (result != FSM_OK) {
        printf("启动状态机失败: %s\n", FSM_GetErrorString(result));
        FSM_Destroy(machine);
        return -1;
    }
    
    printf("=== 状态机示例运行 ===\n\n");
    
    // 模拟设备运行
    
    // 开始工作
    printf("\n--- 发送开始事件 ---\n");
    FSM_SendEvent(machine, EVENT_START, NULL);
    
    // 模拟时间流逝和状态更新
    for (int i = 0; i < 10; i++) {
        simulate_time_passing(500);  // 每次前进500毫秒
        FSM_Update(machine, get_system_time());
    }
    
    // 暂停工作
    printf("\n--- 发送暂停事件 ---\n");
    FSM_SendEvent(machine, EVENT_PAUSE, NULL);
    
    simulate_time_passing(1000);
    FSM_Update(machine, get_system_time());
    
    // 恢复工作
    printf("\n--- 发送恢复事件 ---\n");
    FSM_SendEvent(machine, EVENT_RESUME, NULL);
    
    // 继续模拟时间流逝和状态更新
    for (int i = 0; i < 15; i++) {
        simulate_time_passing(500);
        FSM_Update(machine, get_system_time());
    }
    
    // 手动触发错误
    if (FSM_IsInState(machine, STATE_WORKING)) {
        printf("\n--- 手动触发错误事件 ---\n");
        device_data.error_code = 200;
        FSM_SendEvent(machine, EVENT_ERROR, NULL);
        
        simulate_time_passing(1000);
        FSM_Update(machine, get_system_time());
        
        // 重置
        printf("\n--- 发送重置事件 ---\n");
        FSM_SendEvent(machine, EVENT_RESET, NULL);
    }
    
    // 再次开始工作并让其自动完成
    if (FSM_IsInState(machine, STATE_IDLE)) {
        printf("\n--- 再次发送开始事件 ---\n");
        FSM_SendEvent(machine, EVENT_START, NULL);
        
        // 等待工作完成
        while (FSM_IsInState(machine, STATE_WORKING)) {
            simulate_time_passing(500);
            FSM_Update(machine, get_system_time());
        }
    }
    
    // 输出最终状态
    printf("\n=== 状态机运行结束 ===\n");
    printf("最终状态: %s\n", FSM_GetCurrentStateName(machine));
    printf("状态转换次数: %u\n", FSM_GetTransitionCount(machine));
    printf("总运行时间: %u 毫秒\n", get_system_time());
    
    // 清理
    FSM_Destroy(machine);
    
    return 0;
} 