#include "msgqueue.h"

/**
 * @brief  初始化消息队列
 * @param  hqueue: 消息队列句柄
 * @param  capacity: 队列容量（最大消息数量）
 * @param  max_msg_size: 单个消息的最大大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Init(MSGQUEUE_HandleTypeDef *hqueue, uint16_t capacity, uint16_t max_msg_size)
{
    // 参数检查
    if (hqueue == NULL || capacity == 0 || max_msg_size == 0) {
        return MSGQUEUE_ERROR;
    }

    // 初始化队列属性
    hqueue->capacity = capacity;
    hqueue->count = 0;
    hqueue->front = 0;
    hqueue->rear = 0;
    hqueue->max_msg_size = max_msg_size;
    hqueue->mutex = NULL;
    hqueue->user_data = NULL;

    // 分配消息数组内存
    hqueue->messages = (MSGQUEUE_Message *)malloc(capacity * sizeof(MSGQUEUE_Message));
    if (hqueue->messages == NULL) {
        return MSGQUEUE_ERROR;
    }

    // 初始化每个消息项
    for (uint16_t i = 0; i < capacity; i++) {
        hqueue->messages[i].data = (uint8_t *)malloc(max_msg_size);
        if (hqueue->messages[i].data == NULL) {
            // 清理已分配的资源
            for (uint16_t j = 0; j < i; j++) {
                free(hqueue->messages[j].data);
            }
            free(hqueue->messages);
            hqueue->messages = NULL;
            return MSGQUEUE_ERROR;
        }
        hqueue->messages[i].size = 0;
        hqueue->messages[i].priority = 0;
        hqueue->messages[i].timestamp = 0;
    }

    hqueue->is_initialized = 1;
    return MSGQUEUE_OK;
}

/**
 * @brief  向消息队列发送消息
 * @param  hqueue: 消息队列句柄
 * @param  data: 消息数据指针
 * @param  size: 消息大小（字节）
 * @param  priority: 消息优先级（可选）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Send(MSGQUEUE_HandleTypeDef *hqueue, const uint8_t *data, uint16_t size, uint32_t priority)
{
    // 参数检查
    if (hqueue == NULL || data == NULL || !hqueue->is_initialized) {
        return MSGQUEUE_ERROR;
    }

    // 检查队列是否已满
    if (MSGQUEUE_IsFull(hqueue)) {
        return MSGQUEUE_FULL;
    }

    // 检查消息大小
    if (size > hqueue->max_msg_size) {
        size = hqueue->max_msg_size; // 截断消息
    }

    // 复制消息数据到队列
    memcpy(hqueue->messages[hqueue->rear].data, data, size);
    hqueue->messages[hqueue->rear].size = size;
    hqueue->messages[hqueue->rear].priority = priority;
    hqueue->messages[hqueue->rear].timestamp = HAL_GetTick(); // 记录当前时间戳

    // 更新队尾索引
    hqueue->rear = (hqueue->rear + 1) % hqueue->capacity;
    hqueue->count++;

    return MSGQUEUE_OK;
}

/**
 * @brief  从消息队列接收消息（不删除）
 * @param  hqueue: 消息队列句柄
 * @param  data: 接收数据的缓冲区指针
 * @param  size: 接收缓冲区大小
 * @param  received_size: 实际接收的数据大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Peek(MSGQUEUE_HandleTypeDef *hqueue, uint8_t *data, uint16_t size, uint16_t *received_size)
{
    // 参数检查
    if (hqueue == NULL || data == NULL || !hqueue->is_initialized) {
        return MSGQUEUE_ERROR;
    }

    // 检查队列是否为空
    if (MSGQUEUE_IsEmpty(hqueue)) {
        return MSGQUEUE_EMPTY;
    }

    // 获取队首消息的实际大小
    uint16_t msg_size = hqueue->messages[hqueue->front].size;
    if (received_size != NULL) {
        *received_size = msg_size;
    }

    // 复制数据到用户缓冲区，不超过缓冲区大小
    uint16_t copy_size = (size < msg_size) ? size : msg_size;
    memcpy(data, hqueue->messages[hqueue->front].data, copy_size);

    return MSGQUEUE_OK;
}

/**
 * @brief  接收并删除队首消息
 * @param  hqueue: 消息队列句柄
 * @param  data: 接收数据的缓冲区指针
 * @param  size: 接收缓冲区大小
 * @param  received_size: 实际接收的数据大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Pop(MSGQUEUE_HandleTypeDef *hqueue, uint8_t *data, uint16_t size, uint16_t *received_size)
{
    // 先查看消息
    MSGQUEUE_Status status = MSGQUEUE_Peek(hqueue, data, size, received_size);
    if (status != MSGQUEUE_OK) {
        return status;
    }

    // 更新队首索引，删除消息
    hqueue->front = (hqueue->front + 1) % hqueue->capacity;
    hqueue->count--;

    return MSGQUEUE_OK;
}

/**
 * @brief  从消息队列接收消息（同Pop操作）
 * @param  hqueue: 消息队列句柄
 * @param  data: 接收数据的缓冲区指针
 * @param  size: 接收缓冲区大小
 * @param  received_size: 实际接收的数据大小（字节）
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Receive(MSGQUEUE_HandleTypeDef *hqueue, uint8_t *data, uint16_t size, uint16_t *received_size)
{
    return MSGQUEUE_Pop(hqueue, data, size, received_size);
}

/**
 * @brief  清空消息队列
 * @param  hqueue: 消息队列句柄
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Clear(MSGQUEUE_HandleTypeDef *hqueue)
{
    // 参数检查
    if (hqueue == NULL || !hqueue->is_initialized) {
        return MSGQUEUE_ERROR;
    }

    // 重置队列状态
    hqueue->count = 0;
    hqueue->front = 0;
    hqueue->rear = 0;

    return MSGQUEUE_OK;
}

/**
 * @brief  销毁消息队列并释放资源
 * @param  hqueue: 消息队列句柄
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_Deinit(MSGQUEUE_HandleTypeDef *hqueue)
{
    // 参数检查
    if (hqueue == NULL || !hqueue->is_initialized) {
        return MSGQUEUE_ERROR;
    }

    // 释放每个消息的数据缓冲区
    for (uint16_t i = 0; i < hqueue->capacity; i++) {
        if (hqueue->messages[i].data != NULL) {
            free(hqueue->messages[i].data);
            hqueue->messages[i].data = NULL;
        }
    }

    // 释放消息数组
    if (hqueue->messages != NULL) {
        free(hqueue->messages);
        hqueue->messages = NULL;
    }

    // 重置队列属性
    hqueue->is_initialized = 0;
    hqueue->capacity = 0;
    hqueue->count = 0;
    hqueue->front = 0;
    hqueue->rear = 0;
    hqueue->max_msg_size = 0;
    hqueue->mutex = NULL;
    hqueue->user_data = NULL;

    return MSGQUEUE_OK;
}

/**
 * @brief  获取消息队列中的消息数量
 * @param  hqueue: 消息队列句柄
 * @retval 消息数量
 */
uint16_t MSGQUEUE_GetCount(MSGQUEUE_HandleTypeDef *hqueue)
{
    if (hqueue == NULL || !hqueue->is_initialized) {
        return 0;
    }
    return hqueue->count;
}

/**
 * @brief  检查消息队列是否为空
 * @param  hqueue: 消息队列句柄
 * @retval 1:空 0:非空
 */
uint8_t MSGQUEUE_IsEmpty(MSGQUEUE_HandleTypeDef *hqueue)
{
    if (hqueue == NULL || !hqueue->is_initialized) {
        return 1;
    }
    return (hqueue->count == 0) ? 1 : 0;
}

/**
 * @brief  检查消息队列是否已满
 * @param  hqueue: 消息队列句柄
 * @retval 1:满 0:未满
 */
uint8_t MSGQUEUE_IsFull(MSGQUEUE_HandleTypeDef *hqueue)
{
    if (hqueue == NULL || !hqueue->is_initialized) {
        return 0;
    }
    return (hqueue->count >= hqueue->capacity) ? 1 : 0;
}

/**
 * @brief  设置用户自定义数据
 * @param  hqueue: 消息队列句柄
 * @param  user_data: 用户数据指针
 * @retval MSGQUEUE_Status: 操作状态
 */
MSGQUEUE_Status MSGQUEUE_SetUserData(MSGQUEUE_HandleTypeDef *hqueue, void *user_data)
{
    if (hqueue == NULL || !hqueue->is_initialized) {
        return MSGQUEUE_ERROR;
    }
    hqueue->user_data = user_data;
    return MSGQUEUE_OK;
}

/**
 * @brief  获取用户自定义数据
 * @param  hqueue: 消息队列句柄
 * @retval 用户数据指针
 */
void* MSGQUEUE_GetUserData(MSGQUEUE_HandleTypeDef *hqueue)
{
    if (hqueue == NULL || !hqueue->is_initialized) {
        return NULL;
    }
    return hqueue->user_data;
} 