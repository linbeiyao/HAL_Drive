# STM32 HAL 任务管理器

这是一个轻量级的任务管理器库，为STM32单片机提供简单的任务调度功能。通过这个库，可以实现任务的创建、调度、挂起、恢复以及任务间通信，使程序更加模块化和易于维护。

## 特点

- 基于优先级的协作式任务调度
- 支持任务创建、删除、挂起和恢复
- 支持周期性任务和事件驱动任务
- 提供任务延时功能
- 支持任务间消息队列通信
- 提供任务统计信息
- 轻量级实现，内存占用小
- 无需操作系统支持

## 使用方法

### 1. 初始化任务管理器

```c
// 初始化任务管理器，最多支持10个任务
if (TaskManager_Init(10) != 0) {
    // 初始化失败处理
}
```

### 2. 创建任务

```c
// 创建任务函数
void LED_Task(void* param) {
    // 任务代码
    
    // 可选：使用延时函数
    TaskManager_Delay(100);
}

// 创建周期性任务（500ms执行一次，优先级3）
TaskHandle_t* task_handle = TaskManager_CreateTask("LED", LED_Task, NULL, 3, 500);
if (task_handle == NULL) {
    // 创建失败处理
}
```

### 3. 任务间通信

```c
// 创建任务消息队列
TaskManager_CreateTaskQueue(task_handle, 5, 32);  // 5条消息，每条最大32字节

// 发送消息
const char* msg = "COMMAND";
TaskManager_SendTaskMessage(task_handle, msg, strlen(msg) + 1, 0);

// 接收消息
uint8_t buffer[32];
uint16_t size = 0;
if (TaskManager_ReceiveTaskMessage(task_handle, buffer, sizeof(buffer), &size) == 0) {
    // 处理接收到的消息
}
```

### 4. 任务控制

```c
// 挂起任务
TaskManager_SuspendTask(task_handle);

// 恢复任务
TaskManager_ResumeTask(task_handle);

// 删除任务
TaskManager_DeleteTask(task_handle);
```

### 5. 启动任务调度器

```c
// 启动任务调度器（开始任务执行）
TaskManager_StartScheduler();

// 停止任务调度器
TaskManager_StopScheduler();
```

### 6. 系统时钟集成

```c
// 在systick中断处理函数中调用
void SysTick_Handler(void) {
    HAL_IncTick();
    TaskManager_Update();  // 更新任务状态
}
```

## 性能考虑

- 任务应该避免长时间占用CPU
- 对于耗时操作，应使用TaskManager_Delay函数或状态机设计
- 任务优先级值越小，优先级越高
- 空闲任务自动创建，具有最低优先级

## 示例

在`example`文件夹中提供了完整的使用示例：
1. 多任务协作
2. 任务间通信
3. 任务控制（挂起/恢复）
4. 系统监控

## 内存使用

任务管理器的内存使用计算公式：
`总内存 = sizeof(TaskManager_t) + max_tasks * sizeof(TaskHandle_t) + 任务队列内存`

例如，10个任务的管理器大约需要：
`20 + 10 * 64 = 660字节`（不包含消息队列）

## 限制

- 协作式调度，非抢占式
- 任务应主动让出CPU时间
- 无法处理实时性要求严格的场景

## 与消息队列库结合使用

本任务管理器库可以与消息队列库结合使用，提供任务间通信功能。请确保在使用任务间通信前，先包含消息队列库头文件。 