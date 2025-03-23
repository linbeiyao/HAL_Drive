#include "taskmanager.h"
#include <stdlib.h>

/* 全局任务管理器实例 */
TaskManager_t g_task_manager = {0};

/* 内部使用的空闲任务 */
static void idle_task(void* param) {
    g_task_manager.idle_count++;
}

/**
 * @brief  初始化任务管理器
 * @param  max_tasks: 最大任务数量
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_Init(uint8_t max_tasks) {
    // 参数检查
    if (max_tasks == 0) {
        return 1;
    }

    // 如果已经初始化过，先清理现有资源
    if (g_task_manager.is_initialized) {
        if (g_task_manager.tasks != NULL) {
            free(g_task_manager.tasks);
            g_task_manager.tasks = NULL;
        }
    }

    // 分配任务数组内存
    g_task_manager.tasks = (TaskHandle_t*)malloc(max_tasks * sizeof(TaskHandle_t));
    if (g_task_manager.tasks == NULL) {
        return 2;  // 内存分配失败
    }

    // 初始化任务管理器属性
    g_task_manager.max_tasks = max_tasks;
    g_task_manager.task_count = 0;
    g_task_manager.current_task_index = 0;
    g_task_manager.task_switch_count = 0;
    g_task_manager.idle_count = 0;
    g_task_manager.is_scheduling = 0;
    g_task_manager.is_initialized = 1;

    // 创建空闲任务（最低优先级）
    TaskManager_CreateTask("IDLE", idle_task, NULL, 0xFFFFFFFF, 0);

    return 0;
}

/**
 * @brief  创建新任务
 * @param  name: 任务名称
 * @param  function: 任务函数
 * @param  param: 任务参数
 * @param  priority: 任务优先级
 * @param  period: 周期时间（ms，0表示非周期任务）
 * @retval 任务句柄（NULL表示失败）
 */
TaskHandle_t* TaskManager_CreateTask(const char* name, TaskFunction_t function, void* param, uint32_t priority, uint32_t period) {
    // 参数检查
    if (!g_task_manager.is_initialized || function == NULL) {
        return NULL;
    }

    // 检查是否已达到最大任务数
    if (g_task_manager.task_count >= g_task_manager.max_tasks) {
        return NULL;
    }

    // 获取新任务的索引
    uint8_t task_index = g_task_manager.task_count;
    TaskHandle_t* task = &g_task_manager.tasks[task_index];

    // 初始化任务控制块
    strncpy(task->name, name, sizeof(task->name) - 1);
    task->name[sizeof(task->name) - 1] = '\0';  // 确保字符串结束
    task->function = function;
    task->param = param;
    task->priority = priority;
    task->period = period;
    task->next_run_time = HAL_GetTick();  // 立即可运行
    task->run_count = 0;
    task->timeout = 0;
    task->status = TASK_READY;
    task->queue = NULL;
    task->stack = NULL;
    task->stack_size = 0;
    task->user_data = NULL;

    // 更新任务计数
    g_task_manager.task_count++;

    return task;
}

/**
 * @brief  删除任务
 * @param  task: 任务句柄
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_DeleteTask(TaskHandle_t* task) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL) {
        return 1;
    }

    // 查找任务在数组中的索引
    int task_index = -1;
    for (uint8_t i = 0; i < g_task_manager.task_count; i++) {
        if (&g_task_manager.tasks[i] == task) {
            task_index = i;
            break;
        }
    }

    // 任务未找到
    if (task_index == -1) {
        return 2;
    }

    // 释放任务消息队列（如果有）
    if (task->queue != NULL) {
        MSGQUEUE_Deinit(task->queue);
        free(task->queue);
        task->queue = NULL;
    }

    // 将删除的任务标记为已删除状态
    task->status = TASK_DELETED;

    // 如果是最后一个任务，直接减少计数
    if (task_index == g_task_manager.task_count - 1) {
        g_task_manager.task_count--;
        return 0;
    }

    // 否则移动其他任务填补空缺（保持数组紧凑）
    for (uint8_t i = task_index; i < g_task_manager.task_count - 1; i++) {
        g_task_manager.tasks[i] = g_task_manager.tasks[i + 1];
    }

    // 更新任务计数
    g_task_manager.task_count--;

    // 如果当前正在执行的任务索引大于删除的索引，需要调整
    if (g_task_manager.current_task_index > task_index) {
        g_task_manager.current_task_index--;
    }

    return 0;
}

/**
 * @brief  挂起任务
 * @param  task: 任务句柄
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SuspendTask(TaskHandle_t* task) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL) {
        return 1;
    }

    // 仅处理非删除状态的任务
    if (task->status != TASK_DELETED) {
        task->status = TASK_SUSPENDED;
        return 0;
    }

    return 2;  // 任务已删除
}

/**
 * @brief  恢复任务
 * @param  task: 任务句柄
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_ResumeTask(TaskHandle_t* task) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL) {
        return 1;
    }

    // 仅处理挂起状态的任务
    if (task->status == TASK_SUSPENDED) {
        task->status = TASK_READY;
        task->next_run_time = HAL_GetTick();  // 立即可运行
        return 0;
    }

    return 2;  // 任务非挂起状态
}

/**
 * @brief  设置任务周期
 * @param  task: 任务句柄
 * @param  period: 周期时间（ms）
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SetTaskPeriod(TaskHandle_t* task, uint32_t period) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL) {
        return 1;
    }

    // 设置周期
    task->period = period;
    return 0;
}

/**
 * @brief  设置任务优先级
 * @param  task: 任务句柄
 * @param  priority: 优先级（数值越小优先级越高）
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SetTaskPriority(TaskHandle_t* task, uint32_t priority) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL) {
        return 1;
    }

    // 设置优先级
    task->priority = priority;
    return 0;
}

/**
 * @brief  任务延时
 * @param  delay_ms: 延时时间（ms）
 * @retval 无
 * @note   仅在任务内部调用有效
 */
void TaskManager_Delay(uint32_t delay_ms) {
    // 获取当前任务
    TaskHandle_t* current_task = TaskManager_GetCurrentTask();
    if (current_task == NULL) {
        return;  // 不在任务上下文中
    }

    // 设置任务状态为阻塞状态
    current_task->status = TASK_BLOCKED;
    
    // 设置下次运行时间
    current_task->next_run_time = HAL_GetTick() + delay_ms;
    
    // 让出CPU时间
    TaskManager_Schedule();
}

/**
 * @brief  获取当前任务句柄
 * @retval 当前任务句柄（NULL表示无任务运行）
 */
TaskHandle_t* TaskManager_GetCurrentTask(void) {
    if (!g_task_manager.is_initialized || !g_task_manager.is_scheduling) {
        return NULL;
    }

    if (g_task_manager.current_task_index < g_task_manager.task_count) {
        return &g_task_manager.tasks[g_task_manager.current_task_index];
    }

    return NULL;
}

/**
 * @brief  根据名称查找任务
 * @param  name: 任务名称
 * @retval 任务句柄（NULL表示未找到）
 */
TaskHandle_t* TaskManager_FindTaskByName(const char* name) {
    // 参数检查
    if (!g_task_manager.is_initialized || name == NULL) {
        return NULL;
    }

    // 遍历任务数组
    for (uint8_t i = 0; i < g_task_manager.task_count; i++) {
        if (strcmp(g_task_manager.tasks[i].name, name) == 0) {
            return &g_task_manager.tasks[i];
        }
    }

    return NULL;  // 未找到任务
}

/**
 * @brief  创建任务消息队列
 * @param  task: 任务句柄
 * @param  queue_size: 队列大小
 * @param  msg_size: 消息大小
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_CreateTaskQueue(TaskHandle_t* task, uint16_t queue_size, uint16_t msg_size) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL) {
        return 1;
    }

    // 如果已有队列，先释放
    if (task->queue != NULL) {
        MSGQUEUE_Deinit(task->queue);
        free(task->queue);
        task->queue = NULL;
    }

    // 分配队列内存
    task->queue = (MSGQUEUE_HandleTypeDef*)malloc(sizeof(MSGQUEUE_HandleTypeDef));
    if (task->queue == NULL) {
        return 2;  // 内存分配失败
    }

    // 初始化队列
    MSGQUEUE_Status status = MSGQUEUE_Init(task->queue, queue_size, msg_size);
    if (status != MSGQUEUE_OK) {
        free(task->queue);
        task->queue = NULL;
        return 3;  // 队列初始化失败
    }

    return 0;
}

/**
 * @brief  向任务发送消息
 * @param  task: 目标任务句柄
 * @param  msg: 消息数据
 * @param  size: 消息大小
 * @param  priority: 消息优先级
 * @retval 0:成功 非0:失败
 */
uint8_t TaskManager_SendTaskMessage(TaskHandle_t* task, const void* msg, uint16_t size, uint32_t priority) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL || msg == NULL) {
        return 1;
    }

    // 检查任务是否有消息队列
    if (task->queue == NULL) {
        return 2;  // 任务无消息队列
    }

    // 发送消息
    MSGQUEUE_Status status = MSGQUEUE_Send(task->queue, (const uint8_t*)msg, size, priority);
    if (status != MSGQUEUE_OK) {
        return 3;  // 消息发送失败
    }

    return 0;
}

/**
 * @brief  从任务队列接收消息
 * @param  task: 任务句柄
 * @param  buffer: 接收缓冲区
 * @param  size: 缓冲区大小
 * @param  received_size: 实际接收大小
 * @retval 0:成功 1:队列为空 其他:错误
 */
uint8_t TaskManager_ReceiveTaskMessage(TaskHandle_t* task, void* buffer, uint16_t size, uint16_t* received_size) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL || buffer == NULL) {
        return 2;
    }

    // 检查任务是否有消息队列
    if (task->queue == NULL) {
        return 3;  // 任务无消息队列
    }

    // 接收消息
    MSGQUEUE_Status status = MSGQUEUE_Receive(task->queue, (uint8_t*)buffer, size, received_size);
    if (status == MSGQUEUE_EMPTY) {
        return 1;  // 队列为空
    } else if (status != MSGQUEUE_OK) {
        return 4;  // 消息接收失败
    }

    return 0;
}

/**
 * @brief  任务调度器
 * @retval 无
 */
void TaskManager_Schedule(void) {
    // 参数检查
    if (!g_task_manager.is_initialized || !g_task_manager.is_scheduling) {
        return;
    }

    TaskHandle_t* task = NULL;
    uint8_t task_found = 0;
    uint32_t current_time = HAL_GetTick();
    uint32_t lowest_priority = 0xFFFFFFFF;
    uint8_t next_task_index = 0;

    // 找到可运行的最高优先级（数值最小）任务
    for (uint8_t i = 0; i < g_task_manager.task_count; i++) {
        task = &g_task_manager.tasks[i];

        // 检查任务状态
        if (task->status == TASK_READY) {
            // 就绪状态，直接检查优先级
            if (task->priority < lowest_priority) {
                lowest_priority = task->priority;
                next_task_index = i;
                task_found = 1;
            }
        } else if (task->status == TASK_BLOCKED) {
            // 阻塞状态，检查是否已到运行时间
            if (current_time >= task->next_run_time) {
                task->status = TASK_READY;  // 恢复就绪状态
                if (task->priority < lowest_priority) {
                    lowest_priority = task->priority;
                    next_task_index = i;
                    task_found = 1;
                }
            }
        }
    }

    // 如果找到可运行任务，执行它
    if (task_found) {
        g_task_manager.current_task_index = next_task_index;
        task = &g_task_manager.tasks[next_task_index];
        
        // 更新任务状态和统计信息
        task->status = TASK_RUNNING;
        task->run_count++;
        g_task_manager.task_switch_count++;
        
        // 执行任务函数
        task->function(task->param);
        
        // 更新任务状态
        if (task->status == TASK_RUNNING) {  // 如果任务内部没有改变状态
            if (task->period > 0) {
                // 周期性任务，设置下次运行时间
                task->next_run_time = current_time + task->period;
                task->status = TASK_BLOCKED;
            } else {
                // 非周期性任务，任务运行结束后回到就绪状态
                task->status = TASK_READY;
            }
        }
    }
}

/**
 * @brief  开始任务调度
 * @retval 无
 */
void TaskManager_StartScheduler(void) {
    // 参数检查
    if (!g_task_manager.is_initialized) {
        return;
    }

    // 启动调度器
    g_task_manager.is_scheduling = 1;
    
    // 主循环调度任务
    while (g_task_manager.is_scheduling) {
        TaskManager_Schedule();
    }
}

/**
 * @brief  停止任务调度
 * @retval 无
 */
void TaskManager_StopScheduler(void) {
    g_task_manager.is_scheduling = 0;
}

/**
 * @brief  获取任务统计信息
 * @param  task: 任务句柄
 * @param  run_count: 运行次数
 * @retval 无
 */
void TaskManager_GetTaskStats(TaskHandle_t* task, uint32_t* run_count) {
    // 参数检查
    if (!g_task_manager.is_initialized || task == NULL || run_count == NULL) {
        return;
    }

    *run_count = task->run_count;
}

/**
 * @brief  获取系统统计信息
 * @param  task_switch_count: 任务切换次数
 * @param  idle_count: 空闲计数
 * @retval 无
 */
void TaskManager_GetSystemStats(uint32_t* task_switch_count, uint32_t* idle_count) {
    // 参数检查
    if (!g_task_manager.is_initialized) {
        return;
    }

    if (task_switch_count != NULL) {
        *task_switch_count = g_task_manager.task_switch_count;
    }
    
    if (idle_count != NULL) {
        *idle_count = g_task_manager.idle_count;
    }
}

/**
 * @brief  任务管理器周期性更新（在systick中断中调用）
 * @retval 无
 */
void TaskManager_Update(void) {
    // 参数检查
    if (!g_task_manager.is_initialized) {
        return;
    }

    // 遍历所有任务，更新阻塞任务状态
    uint32_t current_time = HAL_GetTick();
    for (uint8_t i = 0; i < g_task_manager.task_count; i++) {
        TaskHandle_t* task = &g_task_manager.tasks[i];
        
        // 检查阻塞任务是否到期
        if (task->status == TASK_BLOCKED && current_time >= task->next_run_time) {
            task->status = TASK_READY;
        }
    }
} 