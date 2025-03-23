# MQTT_HAL_aliyun - STM32 阿里云MQTT通信模板库

## 硬件要求
- MCU: STM32F103ZET6
- 模块: ESP8266 WiFi模块

## 功能说明
- USART1: 串口打印调试信息
- USART2: 串口与ESP8266通讯
- 使用MQTT协议连接阿里云物联网平台
- 支持设备数据上报、指令接收等功能

## 模块组成
- `Core/Network/esp8266.c/h`: ESP8266 WiFi模块驱动
- `Core/Network/mqtt.c/h`: MQTT协议实现

## 使用说明
1. 配置设备信息（ProductKey、DeviceName、DeviceSecret）
2. 连接WiFi网络
3. 建立MQTT连接
4. 发布/订阅主题

## 参考资料
> [【物联网】手把手完整实现STM32+ESP8266+MQTT+阿里云+APP应用](https://blog.csdn.net/m0_61712829/article/details/135248254)

## 注意事项
- 请确保ESP8266模块固件更新至最新版本
- 使用前请在阿里云物联网平台注册设备并获取设备密钥



