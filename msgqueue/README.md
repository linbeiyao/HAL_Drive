# STM32 HAL 消息队列库

这是一个适用于STM32 HAL库的通用消息队列实现，提供了多种消息传递和处理功能。

## 功能特点

- 支持可配置的队列容量和消息大小
- 支持消息优先级（可选）
- 提供多种消息操作（发送、接收、查看、删除）
- 支持队列状态查询（空/满/消息数量）
- 支持用户自定义数据
- 提供资源管理功能（初始化/销毁）
- 预留RTOS集成接口

## 使用方法

### 1. 初始化消息队列

```c
// 定义消息队列对象
MSGQUEUE_HandleTypeDef hMsgQueue;

// 初始化消息队列，容量为10，每条消息最大32字节
MSGQUEUE_Status status = MSGQUEUE_Init(&hMsgQueue, 10, 32);
if (status != MSGQUEUE_OK) {
    // 处理初始化失败
}
```

### 2. 发送消息

```c
// 准备消息数据
uint8_t data[] = "Hello Queue";
uint16_t size = strlen((char*)data) + 1;

// 发送消息到队列，优先级为0
MSGQUEUE_Status status = MSGQUEUE_Send(&hMsgQueue, data, size, 0);
if (status != MSGQUEUE_OK) {
    // 处理发送失败
}
```

### 3. 接收消息

```c
// 准备接收缓冲区
uint8_t buffer[32];
uint16_t received_size = 0;

// 从队列接收消息
MSGQUEUE_Status status = MSGQUEUE_Receive(&hMsgQueue, buffer, 32, &received_size);
if (status == MSGQUEUE_OK) {
    // 使用接收到的消息
    printf("接收到消息: %s, 大小: %d\r\n", (char*)buffer, received_size);
} else if (status == MSGQUEUE_EMPTY) {
    // 队列为空
}
```

### 4. 查看消息（不删除）

```c
uint8_t buffer[32];
uint16_t peek_size = 0;

MSGQUEUE_Status status = MSGQUEUE_Peek(&hMsgQueue, buffer, 32, &peek_size);
if (status == MSGQUEUE_OK) {
    // 查看消息内容
}
```

### 5. 消息队列管理

```c
// 获取消息数量
uint16_t count = MSGQUEUE_GetCount(&hMsgQueue);

// 检查队列是否为空
if (MSGQUEUE_IsEmpty(&hMsgQueue)) {
    // 队列为空
}

// 检查队列是否已满
if (MSGQUEUE_IsFull(&hMsgQueue)) {
    // 队列已满
}

// 清空队列
MSGQUEUE_Clear(&hMsgQueue);

// 销毁队列并释放资源
MSGQUEUE_Deinit(&hMsgQueue);
```

## 示例

在`example`文件夹中提供了完整的使用示例：

1. 基本消息队列操作示例
2. 传感器数据采集与处理场景示例

## 注意事项

1. 确保为队列分配足够的容量和消息大小
2. 释放不再使用的队列资源
3. 处理队列满/空状态
4. 如需与RTOS集成，可以扩展互斥锁相关功能

## 应用场景

- 传感器数据采集与处理
- 任务间通信
- 事件处理
- 命令队列
- 数据缓冲

## 内存使用

消息队列的内存使用计算公式：
`总内存 = sizeof(MSGQUEUE_HandleTypeDef) + capacity * (sizeof(MSGQUEUE_Message) + max_msg_size)`

例如，容量为10，每条消息32字节的队列大约需要：
`24 + 10 * (16 + 32) = 504字节` 