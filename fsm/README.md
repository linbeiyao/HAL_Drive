# HAL_Drive 有限状态机(FSM)库

这是一个轻量级但功能强大的有限状态机(Finite State Machine，FSM)库，设计用于简化嵌入式系统中的复杂逻辑处理。状态机是一个可以在不同状态之间转换的系统，基于当前状态和触发事件来决定转换，本库提供了完整的状态机框架实现。

## 主要特性

- **状态管理** - 支持多种状态定义和管理
- **事件驱动** - 基于事件驱动的状态转换
- **条件转换** - 支持转换条件(Guard)检查
- **动作执行** - 状态转换时可执行指定动作
- **超时转换** - 支持基于时间的自动状态转换
- **回调函数** - 状态进入/退出/更新时的回调
- **内存动态分配** - 自动管理状态和转换的内存
- **错误处理** - 完善的错误码和错误信息
- **完全中文注释** - 便于中文开发者学习和使用

## 基本概念

1. **状态(State)** - 系统在特定时刻的模式或情况
2. **事件(Event)** - 触发状态转换的外部信号
3. **转换(Transition)** - 从一个状态到另一个状态的变化规则
4. **条件(Guard)** - 转换执行的附加条件判断
5. **动作(Action)** - 转换过程中执行的操作

## 使用方法

### 基本用法

```c
// 1. 创建状态机
FSM_Machine_t* machine = FSM_Create("设备控制", NULL);

// 2. 添加状态
FSM_AddState(machine, STATE_IDLE, "空闲", on_enter_idle, on_exit_idle, on_update_idle, 0);
FSM_AddState(machine, STATE_WORKING, "工作中", on_enter_working, on_exit_working, on_update_working, 5000);
FSM_AddState(machine, STATE_ERROR, "错误", on_enter_error, on_exit_error, NULL, 0);

// 3. 添加转换规则
FSM_AddTransition(machine, STATE_IDLE, STATE_WORKING, EVENT_START, NULL, NULL);
FSM_AddTransition(machine, STATE_WORKING, STATE_IDLE, EVENT_STOP, NULL, NULL);
FSM_AddTransition(machine, STATE_WORKING, STATE_ERROR, EVENT_ERROR, NULL, NULL);
FSM_AddTransition(machine, STATE_ERROR, STATE_IDLE, EVENT_RESET, check_can_reset, do_reset);

// 4. 设置超时状态
FSM_SetTimeoutState(machine, STATE_WORKING, STATE_IDLE);

// 5. 设置初始状态并启动
FSM_SetInitialState(machine, STATE_IDLE);
FSM_Start(machine);

// 6. 使用状态机
// 定期更新状态机时间
FSM_Update(machine, get_system_time());

// 发送事件
FSM_SendEvent(machine, EVENT_START, NULL);

// 7. 清理
FSM_Destroy(machine);
```

### 回调函数示例

```c
// 状态进入回调
FSM_Error_t on_enter_idle(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* prev_state, FSM_UserData_t user_data) {
    printf("进入空闲状态\n");
    // 设备进入低功耗模式
    return FSM_OK;
}

// 状态退出回调
FSM_Error_t on_exit_working(FSM_Machine_t* machine, FSM_State_t* state, 
                           FSM_State_t* next_state, FSM_UserData_t user_data) {
    printf("退出工作状态\n");
    // 停止设备运行
    return FSM_OK;
}

// 状态更新回调
FSM_Error_t on_update_working(FSM_Machine_t* machine, FSM_State_t* state, 
                             FSM_UserData_t user_data) {
    printf("工作状态更新\n");
    // 检查工作进度
    return FSM_OK;
}

// 条件函数
int check_can_reset(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                    FSM_Event_t* event, FSM_UserData_t user_data) {
    // 检查是否可以重置
    return 1; // 返回1表示可以转换
}

// 动作函数
FSM_Error_t do_reset(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                     FSM_Event_t* event, FSM_UserData_t user_data) {
    printf("执行重置操作\n");
    // 执行重置逻辑
    return FSM_OK;
}
```

## 高级用法

### 带用户数据的状态机

```c
// 定义用户数据结构
typedef struct {
    int device_id;
    int error_code;
    void* device_handle;
} Device_Data_t;

// 创建用户数据
Device_Data_t device_data = {
    .device_id = 1,
    .error_code = 0,
    .device_handle = NULL
};

// 创建状态机时传入用户数据
FSM_Machine_t* machine = FSM_Create("设备状态机", &device_data);

// 在回调中使用用户数据
FSM_Error_t on_enter_error(FSM_Machine_t* machine, FSM_State_t* state, 
                          FSM_State_t* prev_state, FSM_UserData_t user_data) {
    Device_Data_t* data = (Device_Data_t*)user_data;
    printf("设备 %d 发生错误: %d\n", data->device_id, data->error_code);
    return FSM_OK;
}
```

### 使用事件数据

```c
// 事件数据结构
typedef struct {
    int param1;
    float param2;
} EventParams_t;

// 创建事件数据
EventParams_t params = {
    .param1 = 100,
    .param2 = 3.14f
};

// 发送事件时附带数据
FSM_SendEvent(machine, EVENT_CUSTOM, &params);

// 在动作函数中使用事件数据
FSM_Error_t custom_action(FSM_Machine_t* machine, FSM_Transition_t* transition, 
                          FSM_Event_t* event, FSM_UserData_t user_data) {
    if (event && event->data) {
        EventParams_t* params = (EventParams_t*)event->data;
        printf("参数1: %d, 参数2: %f\n", params->param1, params->param2);
    }
    return FSM_OK;
}
```

## 常见应用场景

- **设备状态管理** - 管理设备的开机、关机、运行、故障等状态
- **通信协议处理** - 实现通信协议的状态转换逻辑
- **用户界面流程** - 控制UI界面的流转逻辑
- **游戏AI逻辑** - 实现游戏AI的决策状态机
- **多步骤流程控制** - 如温控系统、工业控制等

## 注意事项

1. 确保正确初始化状态机并设置初始状态
2. 定期调用`FSM_Update`更新状态机时间
3. 所有回调函数应返回合适的错误码
4. 合理使用超时功能和条件函数
5. 使用完毕后调用`FSM_Destroy`释放内存

## 错误处理

所有API函数（除创建和销毁外）都返回错误码，可以通过`FSM_GetErrorString`获取对应的错误信息：

```c
FSM_Error_t result = FSM_AddState(machine, STATE_IDLE, "空闲", NULL, NULL, NULL, 0);
if (result != FSM_OK) {
    printf("错误: %s\n", FSM_GetErrorString(result));
}
```

## 扩展性

本库设计简洁但扩展性强，可以根据需要添加更多功能：

- 状态历史记录
- 层次化状态机
- 并行状态机
- 更复杂的事件队列
- 状态机持久化
- 调试和可视化支持

## 性能与内存

- 库使用动态内存分配，适用于资源充足的系统
- 对于资源极其受限的系统，可修改为静态内存分配
- 时间复杂度：状态查找和事件处理为O(n)
- 空间复杂度：与状态数和转换规则数成正比 