#ifndef __TASKMANAGER_H
#define __TASKMANAGER_H

#include "main.h"
#include <stdint.h>
#include <string.h>

/* 可选：使用消息队列进行任务间通信 */
#include "msgqueue.h"

/* 任务状态枚举 */
typedef enum {
    TASK_READY = 0,        // 就绪状态（可执行）
    TASK_RUNNING,          // 运行状态（当前正在执行）
    TASK_BLOCKED,          // 阻塞状态（等待事件/延时）
    TASK_SUSPENDED,        // 挂起状态（被暂停）
    TASK_DELETED           // 已删除状态
} TaskStatus;

/* 任务函数指针类型定义 */
typedef void (*TaskFunction_t)(void* param);

/* 任务控制块结构体 */
typedef struct {
    char name[16];                  // 任务名称
    TaskFunction_t function;        // 任务函数
    void* param;                    // 任务参数
    uint32_t priority;              // 任务优先级（数值越小优先级越高）
    uint32_t period;                // 周期性任务的周期时间（ms，0表示非周期任务）
    uint32_t next_run_time;         // 下次运行时间（系统滴答值）
    uint32_t run_count;             // 运行次数统计
    uint32_t timeout;               // 超时值（ms，用于阻塞时）
    TaskStatus status;              // 任务状态
    MSGQUEUE_HandleTypeDef* queue;  // 任务消息队列（可选）
    void* stack;                    // 任务栈（保留，用于将来扩展）
    uint32_t stack_size;            // 任务栈大小（保留，用于将来扩展）
    void* user_data;                // 用户自定义数据
} TaskHandle_t;

/* 任务管理器结构体 */
typedef struct {
    TaskHandle_t* tasks;            // 任务数组
    uint8_t max_tasks;              // 最大任务数
    uint8_t task_count;             // 当前任务数
    uint8_t current_task_index;     // 当前执行的任务索引
    uint8_t is_initialized;         // 初始化标志
    uint32_t task_switch_count;     // 任务切换计数
    uint32_t idle_count;            // 空闲计数
    uint8_t is_scheduling;          // 调度标志
} TaskManager_t;

/* 全局任务管理器实例 */
extern TaskManager_t g_task_manager;

/* 函数声明 */

/**
 * @brief  初始化任务管理器
 * @param  max_tasks: 最大任务数量
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_Init(uint8_t max_tasks);

/**
 * @brief  创建新任务
 * @param  name: 任务名称
 * @param  function: 任务函数
 * @param  param: 任务参数
 * @param  priority: 任务优先级
 * @param  period: 周期时间（ms，0表示非周期任务）
 * @retval 任务句柄（NULL表示失败）
 */
TaskHandle_t* TaskManager_CreateTask(const char* name, TaskFunction_t function, void* param, uint32_t priority, uint32_t period);

/**
 * @brief  删除任务
 * @param  task: 任务句柄
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_DeleteTask(TaskHandle_t* task);

/**
 * @brief  挂起任务
 * @param  task: 任务句柄
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SuspendTask(TaskHandle_t* task);

/**
 * @brief  恢复任务
 * @param  task: 任务句柄
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_ResumeTask(TaskHandle_t* task);

/**
 * @brief  设置任务周期
 * @param  task: 任务句柄
 * @param  period: 周期时间（ms）
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SetTaskPeriod(TaskHandle_t* task, uint32_t period);

/**
 * @brief  设置任务优先级
 * @param  task: 任务句柄
 * @param  priority: 优先级（数值越小优先级越高）
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SetTaskPriority(TaskHandle_t* task, uint32_t priority);

/**
 * @brief  任务延时
 * @param  delay_ms: 延时时间（ms）
 * @retval 无
 * @note   仅在任务内部调用有效
 */
void TaskManager_Delay(uint32_t delay_ms);

/**
 * @brief  获取当前任务句柄
 * @retval 当前任务句柄（NULL表示无任务运行）
 */
TaskHandle_t* TaskManager_GetCurrentTask(void);

/**
 * @brief  根据名称查找任务
 * @param  name: 任务名称
 * @retval 任务句柄（NULL表示未找到）
 */
TaskHandle_t* TaskManager_FindTaskByName(const char* name);

/**
 * @brief  创建任务消息队列
 * @param  task: 任务句柄
 * @param  queue_size: 队列大小
 * @param  msg_size: 消息大小
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_CreateTaskQueue(TaskHandle_t* task, uint16_t queue_size, uint16_t msg_size);

/**
 * @brief  向任务发送消息
 * @param  task: 目标任务句柄
 * @param  msg: 消息数据
 * @param  size: 消息大小
 * @param  priority: 消息优先级
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SendTaskMessage(TaskHandle_t* task, const void* msg, uint16_t size, uint32_t priority);

/**
 * @brief  从任务队列接收消息
 * @param  task: 任务句柄
 * @param  buffer: 接收缓冲区
 * @param  size: 缓冲区大小
 * @param  received_size: 实际接收大小
 * @retval 0:成功 1:队列为空 其他:错误
 */
uint8_t TaskManager_ReceiveTaskMessage(TaskHandle_t* task, void* buffer, uint16_t size, uint16_t* received_size);

/**
 * @brief  任务调度器
 * @retval 无
 */
void TaskManager_Schedule(void);

/**
 * @brief  开始任务调度
 * @retval 无
 */
void TaskManager_StartScheduler(void);

/**
 * @brief  停止任务调度
 * @retval 无
 */
void TaskManager_StopScheduler(void);

/**
 * @brief  获取任务统计信息
 * @param  task: 任务句柄
 * @param  run_count: 运行次数
 * @retval 无
 */
void TaskManager_GetTaskStats(TaskHandle_t* task, uint32_t* run_count);

/**
 * @brief  获取系统统计信息
 * @param  task_switch_count: 任务切换次数
 * @param  idle_count: 空闲计数
 * @retval 无
 */
void TaskManager_GetSystemStats(uint32_t* task_switch_count, uint32_t* idle_count);

/**
 * @brief  任务管理器周期性更新（在systick中断中调用）
 * @retval 无
 */
void TaskManager_Update(void);

#endif /* __TASKMANAGER_H */ 