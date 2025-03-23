# MQTT_HAL_OneNET - STM32 OneNET MQTT通信模板库

## 硬件要求
- MCU: STM32F103ZET6
- 模块: ESP8266 WiFi模块

## 功能说明
- USART1: 串口打印调试信息
- USART2: 串口与ESP8266通讯
- 使用MQTT协议连接中国移动OneNET物联网平台
- 支持设备数据上报、指令接收等功能

## 模块组成
- `Core/Network/esp8266.c/h`: ESP8266 WiFi模块驱动
- `Core/Network/mqtt.c/h`: MQTT协议实现

## 使用说明
1. 配置设备信息（ProductID、DeviceID、APIKey等）
2. 连接WiFi网络
3. 建立与OneNET平台的MQTT连接
4. 发布/订阅主题

## 参考资料
> [OneNET平台MQTT接入文档](https://open.iot.10086.cn/doc/mqtt/book/introduction/README.html)
> [STM32+ESP8266连接OneNET MQTT平台](https://blog.csdn.net/qq_42600433/article/details/124935485)

## 注意事项
- 使用前请在OneNET平台注册并创建产品和设备
- 确保ESP8266模块固件支持MQTT功能
- 配置正确的OneNET连接参数和认证信息



