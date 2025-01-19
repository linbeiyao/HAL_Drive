#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

// 最大消息大小和队列深度，可以根据实际需求调整
#define MESSAGE_QUEUE_MAX_SIZE 256
#define MESSAGE_QUEUE_DEPTH 16

typedef struct {
    uint8_t data[MESSAGE_QUEUE_DEPTH][MESSAGE_QUEUE_MAX_SIZE]; // 消息存储区
    uint16_t message_size[MESSAGE_QUEUE_DEPTH];               // 每条消息的大小
    uint16_t front;                                           // 队列头部索引
    uint16_t rear;                                            // 队列尾部索引
    uint16_t max_message_size;                                // 单条消息的最大大小
    uint16_t depth;                                           // 队列深度
} MessageQueue;

// 初始化消息队列
void MessageQueue_Init(MessageQueue *queue, uint16_t max_message_size, uint16_t depth);

// 判断队列是否为空
bool MessageQueue_IsEmpty(const MessageQueue *queue);

// 判断队列是否为满
bool MessageQueue_IsFull(const MessageQueue *queue);

// 向队列中添加消息
bool MessageQueue_Push(MessageQueue *queue, const uint8_t *message, uint16_t size);

// 从队列中提取消息
bool MessageQueue_Pop(MessageQueue *queue, uint8_t *message, uint16_t *size);

#endif // MESSAGE_QUEUE_H
