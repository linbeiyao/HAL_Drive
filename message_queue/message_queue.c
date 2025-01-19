#include "message_queue.h"
#include <string.h> // 用于 memcpy

void MessageQueue_Init(MessageQueue *queue, uint16_t max_message_size, uint16_t depth) {
    queue->front = 0;
    queue->rear = 0;
    queue->max_message_size = max_message_size;
    queue->depth = depth;
}

bool MessageQueue_IsEmpty(const MessageQueue *queue) {
    return queue->front == queue->rear;
}

bool MessageQueue_IsFull(const MessageQueue *queue) {
    return ((queue->rear + 1) % queue->depth) == queue->front;
}

bool MessageQueue_Push(MessageQueue *queue, const uint8_t *message, uint16_t size) {
    if (MessageQueue_IsFull(queue) || size > queue->max_message_size) {
        return false; // 队列已满或消息超出最大大小
    }

    // 将消息复制到队列的当前尾部
    memcpy(queue->data[queue->rear], message, size);
    queue->message_size[queue->rear] = size;

    // 更新队列尾部索引
    queue->rear = (queue->rear + 1) % queue->depth;
    return true;
}

bool MessageQueue_Pop(MessageQueue *queue, uint8_t *message, uint16_t *size) {
    if (MessageQueue_IsEmpty(queue)) {
        return false; // 队列为空
    }

    // 取出消息
    memcpy(message, queue->data[queue->front], queue->message_size[queue->front]);
    *size = queue->message_size[queue->front];

    // 更新队列头部索引
    queue->front = (queue->front + 1) % queue->depth;
    return true;
}
