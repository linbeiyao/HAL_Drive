# AS608_HAL - STM32 AS608指纹模块驱动库

## 硬件要求
- MCU: STM32F103ZET6
- 模块: AS608光学指纹识别模块

## 功能说明
- USART1: 串口打印调试信息
- USART3: 串口与AS608指纹模块通讯
- 支持功能:
  - 添加指纹
  - 删除指纹
  - 指纹识别
  - 指纹库管理

## 模块组成
- `Core/Hardware/as608.c/h`: AS608驱动核心代码
- `Core/Hardware/uart_user.c/h`: 串口通信接口
- `Core/Hardware/key.c/h`: 按键驱动
- `Core/Hardware/led.c/h`: LED驱动

## 使用说明
1. 按照硬件连接图连接AS608模块
2. 调用初始化函数
3. 使用提供的API进行指纹采集、比对、删除等操作
4. 通过3个按键可分别触发添加指纹、删除指纹、刷指纹三个功能

## API示例
```c
// 初始化
AS608_Init();

// 添加指纹
Add_FR();

// 指纹识别
FR_Identify();

// 删除指纹
Del_FR();
```

## 参考资料
> [基于STM32F103——AS608指纹模块+串口打印](https://shequ.stmicroelectronics.cn/thread-636463-1-1.html)
>
> [STM32（HAL库 标准库通用） AS608光学指纹模块驱动](https://blog.csdn.net/weixin_56565733/article/details/122137492)
>
> [以stm32f103c8t6为例的AS608指纹模块，CubeMX操作hal库（可移植）](https://blog.csdn.net/qq_74433035/article/details/132401606)

## 注意事项
- 指纹模块电源稳定性对识别率有影响
- 使用前建议对模块进行传感器自检
- 保持指纹传感器表面清洁

