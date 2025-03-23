# HAL_Drive 调试库

这是一个基于HAL库的STM32调试工具库，提供了便捷的日志输出、计时和数据监控功能。

## 特性

- 通过串口1（UART1）实现调试输出
- 多级别日志系统（ERROR, WARN, INFO, DEBUG, VERBOSE）
- 性能计时功能，便于分析代码执行时间
- 十六进制数据打印，方便查看内存数据
- 断言功能支持，便于捕获异常情况

## 使用方法

### 1. 初始化

在项目初始化代码中添加：

```c
#include "Debug/debug.h"

void main(void)
{
    // 系统初始化...
    
    // 初始化调试串口，设置波特率为115200
    Debug_Init(115200);
    
    // 其他初始化...
}
```

### 2. 日志输出

根据不同的日志级别使用对应的宏：

```c
// 错误日志
DEBUG_ERROR("系统错误: %d", error_code);

// 警告日志
DEBUG_WARN("电池电量低: %d%%", battery_level);

// 信息日志
DEBUG_INFO("系统启动完成");

// 调试日志
DEBUG_DEBUG("变量值: x=%d, y=%f", x, y);

// 详细日志
DEBUG_VERBOSE("详细信息: %s", detailed_info);
```

### 3. 性能计时

```c
// 定义计时器
Debug_Timer_t timer;

// 开始计时
Debug_TimerStart(&timer, "任务执行");

// 执行任务...
HAL_Delay(1000);

// 停止计时
Debug_TimerStop(&timer);

// 打印计时结果
Debug_TimerPrint(&timer);  // 输出: [TIMER] '任务执行' elapsed: 1000 ms
```

### 4. 十六进制数据打印

```c
uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
DEBUG_HEX_DUMP("接收数据", data, sizeof(data));
```

输出示例：
```
接收数据 [8 bytes]:
  0000  01 02 03 04 05 06 07 08  ........
```

### 5. 代码跟踪

```c
void some_function(void)
{
    DEBUG_TRACE();  // 输出: [DEBUG] TRACE: some_function:123
    
    // 函数代码...
}
```

### 6. 断言

```c
// 如果条件不满足，将输出错误信息并进入死循环
DEBUG_ASSERT(value > 0);
```

## 配置

在debug.h中可以设置当前的调试级别：

```c
// 修改此处可以更改日志级别
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_INFO  // 只显示INFO及以上级别的日志
#endif
```

可用的日志级别：
- `DEBUG_LEVEL_NONE`：关闭所有日志
- `DEBUG_LEVEL_ERROR`：只显示错误日志
- `DEBUG_LEVEL_WARN`：显示警告和错误日志
- `DEBUG_LEVEL_INFO`：显示信息、警告和错误日志（默认）
- `DEBUG_LEVEL_DEBUG`：显示调试及以上级别的日志
- `DEBUG_LEVEL_VERBOSE`：显示所有日志

## 硬件连接

UART1默认引脚：
- PA9: TX (STM32发送)
- PA10: RX (STM32接收)

可通过USB转串口模块连接到电脑，使用串口调试助手查看输出信息。
