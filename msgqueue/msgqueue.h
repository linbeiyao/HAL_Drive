#ifndef __MSGQUEUE_H
#define __MSGQUEUE_H

#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * 消息队列状态枚举
 */
typedef enum {
    MSGQUEUE_OK = 0,       // 操作成功
    MSGQUEUE_EMPTY,        // 队列为空
    MSGQUEUE_FULL,         // 队列已满
    MSGQUEUE_ERROR         // 一般错误
} MSGQUEUE_Status;

/**
 * 消息结构体定义
 */
typedef struct {
    uint8_t *data;         // 消息数据指针
    uint16_t size;         // 消息大小
    uint32_t priority;     // 消息优先级（可选）
    uint32_t timestamp;    // 消息时间戳（可选）
} MSGQUEUE_Message;

/**
 * 消息队列结构体定义
 */
typedef struct {
    MSGQUEUE_Message *messages;    // 消息数组
    uint16_t capacity;             // 队列容量
    uint16_t count;                // 当前消息数量
    uint16_t front;                // 队首索引
    uint16_t rear;                 // 队尾索引
    uint16_t max_msg_size;         // 最大消息大小
    uint8_t is_initialized;        // 初始化标志
    void *mutex;                   // 互斥锁（预留，可用于RTOS集成）
    void *user_data;               // 用户自定义数据
} MSGQUEUE_HandleTypeDef;

/**
 * @brief  初始化消息队列
 * @param  hqueue: 消息队列句柄
 * @param  capacity: 队列容量（最大消息数量）
 * @param  max_msg_size: 单个消息的最大大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Init(MSGQUEUE_HandleTypeDef *hqueue, uint16_t capacity, uint16_t max_msg_size);

/**
 * @brief  向消息队列发送消息
 * @param  hqueue: 消息队列句柄
 * @param  data: 消息数据指针
 * @param  size: 消息大小（字节）
 * @param  priority: 消息优先级（可选）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Send(MSGQUEUE_HandleTypeDef *hqueue, const uint8_t *data, uint16_t size, uint32_t priority);

/**
 * @brief  从消息队列接收消息
 * @param  hqueue: 消息队列句柄
 * @param  data: 接收数据的缓冲区指针
 * @param  size: 接收缓冲区大小
 * @param  received_size: 实际接收的数据大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Receive(MSGQUEUE_HandleTypeDef *hqueue, uint8_t *data, uint16_t size, uint16_t *received_size);

/**
 * @brief  接收并删除队首消息
 * @param  hqueue: 消息队列句柄
 * @param  data: 接收数据的缓冲区指针
 * @param  size: 接收缓冲区大小
 * @param  received_size: 实际接收的数据大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Pop(MSGQUEUE_HandleTypeDef *hqueue, uint8_t *data, uint16_t size, uint16_t *received_size);

/**
 * @brief  查看队首消息但不删除
 * @param  hqueue: 消息队列句柄
 * @param  data: 接收数据的缓冲区指针
 * @param  size: 接收缓冲区大小
 * @param  received_size: 实际接收的数据大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Peek(MSGQUEUE_HandleTypeDef *hqueue, uint8_t *data, uint16_t size, uint16_t *received_size);

/**
 * @brief  清空消息队列
 * @param  hqueue: 消息队列句柄
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Clear(MSGQUEUE_HandleTypeDef *hqueue);

/**
 * @brief  销毁消息队列并释放资源
 * @param  hqueue: 消息队列句柄
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Deinit(MSGQUEUE_HandleTypeDef *hqueue);

/**
 * @brief  获取消息队列中的消息数量
 * @param  hqueue: 消息队列句柄
 * @retval 消息数量
 */
uint16_t MSGQUEUE_GetCount(MSGQUEUE_HandleTypeDef *hqueue);

/**
 * @brief  检查消息队列是否为空
 * @param  hqueue: 消息队列句柄
 * @retval 1:空 0:非空
 */
uint8_t MSGQUEUE_IsEmpty(MSGQUEUE_HandleTypeDef *hqueue);

/**
 * @brief  检查消息队列是否已满
 * @param  hqueue: 消息队列句柄
 * @retval 1:满 0:未满
 */
uint8_t MSGQUEUE_IsFull(MSGQUEUE_HandleTypeDef *hqueue);

/**
 * @brief  设置用户自定义数据
 * @param  hqueue: 消息队列句柄
 * @param  user_data: 用户数据指针
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_SetUserData(MSGQUEUE_HandleTypeDef *hqueue, void *user_data);

/**
 * @brief  获取用户自定义数据
 * @param  hqueue: 消息队列句柄
 * @retval 用户数据指针
 */
void* MSGQUEUE_GetUserData(MSGQUEUE_HandleTypeDef *hqueue);

#endif /* __MSGQUEUE_H */ 