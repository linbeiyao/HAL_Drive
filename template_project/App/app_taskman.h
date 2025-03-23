/**
  ******************************************************************************
  * @file           : app_taskman.h
  * @brief          : 应用层任务管理器头文件
  * @author         : HAL_Drive团队
  * @version        : v1.0.0
  * @date           : 2023-01-01
  ******************************************************************************
  */

#ifndef __APP_TASKMAN_H
#define __APP_TASKMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"
#include <stdint.h>

/* 任务状态定义 ---------------------------------------------------------*/
typedef enum {
    TASK_STATE_INIT = 0,      /* 初始化状态 */
    TASK_STATE_READY,         /* 就绪状态 */
    TASK_STATE_RUNNING,       /* 运行状态 */
    TASK_STATE_BLOCKED,       /* 阻塞状态 */
    TASK_STATE_SUSPENDED,     /* 挂起状态 */
    TASK_STATE_DELETED        /* 已删除状态 */
} TaskState_t;

/* 任务回调函数定义 -----------------------------------------------------*/
typedef void (*TaskCallback_t)(void *param);

/* 任务统计信息 ---------------------------------------------------------*/
typedef struct {
    uint32_t execute_count;   /* 执行次数 */
    uint32_t execute_time;    /* 执行时间(us) */
    uint32_t max_time;        /* 最长执行时间(us) */
    uint32_t min_time;        /* 最短执行时间(us) */
    uint32_t avg_time;        /* 平均执行时间(us) */
    uint32_t missed_count;    /* 错过执行次数 */
    uint32_t last_exec_time;  /* 上次执行时间(ms) */
} TaskStats_t;

/* 任务控制块定义 -------------------------------------------------------*/
typedef struct {
    char name[TASK_NAME_MAX_LEN];  /* 任务名称 */
    TaskCallback_t callback;       /* 任务回调函数 */
    void *param;                   /* 任务参数 */
    uint32_t period;               /* 任务周期(ms) */
    uint32_t delay;                /* 初始延迟(ms) */
    uint32_t last_run_time;        /* 上次运行时间(ms) */
    uint32_t next_run_time;        /* 下次运行时间(ms) */
    TaskPriority_t priority;       /* 任务优先级 */
    TaskState_t state;             /* 任务状态 */
    uint8_t id;                    /* 任务ID */
    uint8_t is_periodic;           /* 是否周期性任务 */
    uint8_t auto_reload;           /* 是否自动重新加载 */
    TaskStats_t stats;             /* 任务统计信息 */
} Task_t;

/* 任务管理器错误码定义 -------------------------------------------------*/
typedef enum {
    TASKMAN_OK = 0,               /* 成功 */
    TASKMAN_ERROR_PARAM,          /* 参数错误 */
    TASKMAN_ERROR_FULL,           /* 任务队列已满 */
    TASKMAN_ERROR_NOT_FOUND,      /* 任务未找到 */
    TASKMAN_ERROR_ALREADY_EXIST,  /* 任务已存在 */
    TASKMAN_ERROR_TIMEOUT,        /* 超时错误 */
    TASKMAN_ERROR_NOT_READY       /* 未就绪错误 */
} TaskManError_t;

/* 任务管理器函数定义 ---------------------------------------------------*/
/**
 * @brief  初始化任务管理器
 * @param  None
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_Init(void);

/**
 * @brief  反初始化任务管理器
 * @param  None
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_DeInit(void);

/**
 * @brief  创建一个任务
 * @param  name: 任务名称
 * @param  callback: 任务回调函数
 * @param  param: 任务参数
 * @param  period: 任务周期(ms)
 * @param  delay: 初始延迟(ms)
 * @param  priority: 任务优先级
 * @param  is_periodic: 是否周期性任务
 * @param  task_id: 返回的任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_CreateTask(const char *name, TaskCallback_t callback, void *param, 
                                uint32_t period, uint32_t delay, TaskPriority_t priority, 
                                uint8_t is_periodic, uint8_t *task_id);

/**
 * @brief  删除一个任务
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_DeleteTask(uint8_t task_id);

/**
 * @brief  启动一个任务
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_StartTask(uint8_t task_id);

/**
 * @brief  停止一个任务
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_StopTask(uint8_t task_id);

/**
 * @brief  暂停一个任务
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_SuspendTask(uint8_t task_id);

/**
 * @brief  恢复一个任务
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_ResumeTask(uint8_t task_id);

/**
 * @brief  修改任务周期
 * @param  task_id: 任务ID
 * @param  period: 新周期(ms)
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_SetTaskPeriod(uint8_t task_id, uint32_t period);

/**
 * @brief  获取任务信息
 * @param  task_id: 任务ID
 * @param  task: 任务信息结构体指针
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_GetTaskInfo(uint8_t task_id, Task_t *task);

/**
 * @brief  获取任务统计信息
 * @param  task_id: 任务ID
 * @param  stats: 任务统计信息结构体指针
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_GetTaskStats(uint8_t task_id, TaskStats_t *stats);

/**
 * @brief  清除任务统计信息
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_ClearTaskStats(uint8_t task_id);

/**
 * @brief  执行所有到达时间的任务(在主循环中调用)
 * @param  current_time: 当前时间(ms)
 * @retval 执行的任务数量
 */
uint8_t TaskMan_Execute(uint32_t current_time);

/**
 * @brief  获取当前任务数量
 * @param  None
 * @retval 当前任务数量
 */
uint8_t TaskMan_GetTaskCount(void);

/**
 * @brief  获取任务管理器状态
 * @param  None
 * @retval 1: 就绪
 *         0: 未就绪
 */
uint8_t TaskMan_IsReady(void);

/**
 * @brief  重置任务管理器
 * @param  None
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_Reset(void);

/**
 * @brief  任务看门狗喂狗
 * @param  task_id: 任务ID
 * @retval TASKMAN_OK: 成功
 *         其他: 错误码
 */
TaskManError_t TaskMan_FeedWatchdog(uint8_t task_id);

#ifdef __cplusplus
}
#endif

#endif /* __APP_TASKMAN_H */ 