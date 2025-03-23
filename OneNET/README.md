# HAL_Drive 驱动和通信库集合

本仓库包含基于STM32 HAL库开发的驱动程序和通信协议库，可直接用于项目开发，无需修改底层驱动代码。

## 仓库内容

### 1. MQTT_HAL_aliyun
STM32基于ESP8266的阿里云MQTT通信模板库，支持设备连接阿里云物联网平台，实现数据上传和指令接收。

[查看详情](./MQTT_HAL_aliyun/README.md)

### 2. MQTT_HAL_OneNET
STM32基于ESP8266的OneNET MQTT通信模板库，支持设备连接中国移动OneNET物联网平台，实现数据上传和指令接收。

[查看详情](./MQTT_HAL_OneNET/README.md)

### 3. AS608_HAL
STM32 AS608指纹模块驱动库，支持指纹的添加、删除、识别等功能，可直接用于指纹识别项目。

[查看详情](./AS608_HAL/README.md)

## 使用说明

每个库可独立使用，无需修改驱动层代码，只需按照各自README中的指引进行配置和使用。所有库均基于STM32 HAL库开发，提供良好的可移植性。

## 硬件支持

- MCU：STM32F103系列（可移植到其他STM32系列）
- 外设：ESP8266 WiFi模块、AS608指纹模块

## 开发环境

- STM32CubeMX
- Keil MDK-ARM

## 注意事项

- 请遵循各库自身的协议要求
- 建议在使用前先阅读各库的详细说明文档
- 若有任何问题或建议，欢迎提出
