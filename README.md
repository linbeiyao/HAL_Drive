# HAL_Drive 驱动库项目

HAL_Drive是一个为嵌入式系统设计的驱动库集合，提供了多种常用设备的驱动实现和工具库。项目采用模块化设计，支持多种单片机平台，便于快速开发嵌入式应用。

## 项目结构

- **oled/** - OLED显示驱动和UI管理
- **fsm/** - 有限状态机实现
- ... (其他驱动模块)

## 主要特性

### OLED显示驱动

OLED显示驱动库提供了完整的屏幕控制功能，包括：
- 基本绘图函数（点、线、矩形、圆等）
- 文字显示（支持多种字体）
- 界面管理系统，支持多个界面切换
- 弹窗和提示消息功能
- 动画过渡效果

详细信息请查看 [OLED库文档](./oled/README.md)

### 有限状态机(FSM)

FSM模块提供了轻量级但功能强大的状态机实现，用于简化复杂逻辑处理：
- 状态管理和转换控制
- 事件驱动的状态切换
- 支持条件判断和动作执行
- 状态超时自动转换功能
- 完善的错误处理机制

详细信息请查看 [FSM库文档](./fsm/README.md)

## 使用环境

- 支持标准C99
- 可用于各种单片机平台（STM32、ESP32等）
- 低内存占用，适合资源受限的嵌入式系统

## 开发指南

### 添加新驱动模块

1. 在根目录创建新的模块文件夹
2. 实现驱动的核心功能和API
3. 编写详细的文档和使用示例
4. 确保代码风格一致性和兼容性

### 编码规范

- 使用标准C语言
- 函数和变量采用模块前缀命名
- 详细注释（使用中文注释）
- 错误处理完善
- 避免对特定硬件的直接依赖

## 贡献指南

欢迎贡献代码或提出建议，请确保：
1. 提交前完成充分测试
2. 遵循现有的代码风格
3. 提供完整的文档和使用示例
4. 不破坏现有功能的兼容性

## 许可证

本项目采用MIT许可证，详情请参阅LICENSE文件。

## 联系方式

如有问题或建议，请联系项目维护者：
- 邮箱：example@example.com
- 项目主页：https://github.com/linbeiyao/HAL_Drive
