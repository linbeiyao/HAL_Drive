# ESP8266 MQTT库

## 简介

这是一个基于STM32 HAL库开发的ESP8266 WiFi模块驱动，专注于实现MQTT通信功能。该库提供了完整的WiFi连接、MQTT通信以及JSON数据处理解析功能。

本库支持环形缓冲区、DMA传输和消息重发机制，提高了通信的可靠性和效率，特别适合于物联网设备的开发。

## 特点

- **丰富的AT指令支持**：封装ESP8266 AT指令集，简化WiFi和MQTT操作
- **标准MQTT协议支持**：兼容所有标准MQTT服务器（如Mosquitto、EMQX等）
- **DMA+环形缓冲区**：支持高效的数据接收和处理
- **异步处理机制**：非阻塞式设计，提高系统响应性
- **JSON数据处理**：内置JSON构建和解析功能
- **错误处理与重发**：自动检测发送失败的消息并重发
- **兼容多种STM32平台**：适用于STM32F1/F4等系列单片机

## 目录结构

```
esp8266/
├── esp8266.h           - 核心头文件，包含函数声明和数据结构
├── esp8266.c           - 基础功能实现
├── esp8266_uart2.c     - UART2+DMA扩展实现
├── esp8266callback.c   - 回调函数实现
├── NETConfig.h         - 网络配置文件
├── 示例.md             - 使用示例文档
└── example/            - 示例代码目录
```

## 功能模块

### 1. WiFi连接管理
- 连接/断开WiFi
- 自动重连机制
- WiFi状态监控

### 2. MQTT通信
- 配置MQTT客户端参数
- 连接MQTT服务器
- 订阅/发布主题
- 消息回调处理

### 3. 环形缓冲区
- 高效数据缓存
- 并发安全设计
- 动态数据处理

### 4. 消息重发机制
- 自动检测失败消息
- 定时重发队列
- 优先级管理

### 5. JSON数据处理
- 构建JSON格式数据
- 解析JSON数据

## 使用方法

### 初始化

```c
// 声明变量
ESP8266_UART2_HandleTypeDef esp8266_handle;
DMA_HandleTypeDef hdma_usart2_rx;

// 初始化DMA
MX_DMA_Init();

// 初始化串口
MX_USART2_UART_Init();

// 初始化ESP8266
ESP8266_UART2_Init(&esp8266_handle, &huart2, &hdma_usart2_rx);

// 设置MQTT消息回调
void mqtt_callback(const char *topic, const char *message) {
    // 处理接收到的MQTT消息
    printf("收到主题: %s, 消息: %s\r\n", topic, message);
}
ESP8266_SetMQTTMessageCallback(&esp8266_handle.esp_base, mqtt_callback);
```

### 连接WiFi和MQTT

```c
// 连接WiFi
ESP8266_ConnectWiFi(&esp8266_handle.esp_base, WIFI_SSID, WIFI_PASSWORD, 5000);

// 连接MQTT服务器
ESP8266_SetMQTTConfig(&esp8266_handle.esp_base, 0, 1, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, 0, 0, "", 1000);
ESP8266_ConnectMQTT(&esp8266_handle.esp_base, 0, MQTT_SERVER, MQTT_PORT, 1, 2000);

// 订阅主题
ESP8266_SubscribeMQTT(&esp8266_handle.esp_base, 0, MQTT_TOPIC_Subscribe, MQTT_QoS1, 1000);
```

### 发送MQTT消息

```c
// 发送普通消息
ESP8266_UART2_PublishMQTT(&esp8266_handle, MQTT_TOPIC_Publish, "Hello, World!", MQTT_QoS1, 0, 1000);

// 发送JSON格式消息
ESP8266_UART2_PublishMQTT_JSON(&esp8266_handle, MQTT_TOPIC_Publish, "temperature", "25.5", MQTT_QoS1, 0, 1000);
```

### 主循环处理

```c
while (1) {
    // 处理接收到的数据和重发队列
    ESP8266_UART2_Process(&esp8266_handle);
    
    // 其他任务...
    HAL_Delay(10);
}
```

### 中断处理

在USART2_IRQHandler中添加:

```c
void USART2_IRQHandler(void) {
    ESP8266_UART2_IRQHandler(&esp8266_handle);
    HAL_UART_IRQHandler(&huart2);
}
```

## 配置说明

在NETConfig.h中配置网络参数:

```c
// WiFi配置
#define WIFI_SSID "YourSSID"  
#define WIFI_PASSWORD "YourPassword"

// MQTT服务器配置
#define MQTT_SERVER "mqtt.example.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "user"
#define MQTT_PASSWORD "password"
#define MQTT_CLIENT_ID "device1"

// MQTT主题
#define MQTT_TOPIC_Subscribe "/device/control"
#define MQTT_TOPIC_Publish "/device/data"
```

## 注意事项

1. 使用前请确保正确配置UART和DMA
2. 确保在USART中断处理函数中调用ESP8266_UART2_IRQHandler
3. 主循环中需要定期调用ESP8266_UART2_Process处理数据
4. 如遇未定义标识符"USART_CR1_IDLEIE"等错误，请检查STM32 HAL库版本

## 错误处理

如遇到编译错误，可能需要根据不同的STM32型号调整中断和DMA相关代码：

```c
// 对于部分STM32型号，可能需要手动定义这些宏
#ifndef UART_IT_IDLE
#define UART_IT_IDLE                       ((uint32_t)0x00000010)
#endif

#ifndef UART_FLAG_IDLE
#define UART_FLAG_IDLE                     ((uint32_t)0x00000010)
#endif

// 清除IDLE标志位的替代方案
#ifndef __HAL_UART_CLEAR_IDLEFLAG
#define __HAL_UART_CLEAR_IDLEFLAG(__HANDLE__) \
  do{                                \
    __IO uint32_t tmpreg = 0x00U;    \
    tmpreg = (__HANDLE__)->Instance->SR;  \
    tmpreg = (__HANDLE__)->Instance->DR;  \
    UNUSED(tmpreg);                  \
  } while(0U)
#endif
```

## 更多信息

更多使用方法请参考`示例.md`文件和`example`目录下的示例代码。

## 最近更新

- **调试信息精简优化**：删除了代码中的调试printf信息，减少了系统资源占用，提高了运行效率。特别是在资源受限的嵌入式系统中，可以减少内存使用和串口通信负担。