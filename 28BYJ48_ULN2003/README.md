# 28BYJ48步进电机 + ULN2003驱动板

这是一个基于STM32 HAL库的28BYJ48步进电机驱动程序，适用于与ULN2003驱动板配合使用。

## 硬件简介

28BYJ48是一款常见的小型步进电机，具有以下特性：
- 工作电压：5V DC
- 相数：4相（5线）
- 减速比：1:64
- 步进角度：5.625° / 64
- 频率：100Hz
- 每圈步数：64 * (360° / 5.625°) = 4096步（全步进模式约为2048步）

ULN2003是一个达林顿晶体管阵列驱动芯片，能够提供足够的电流来驱动步进电机。

## 接线方式

28BYJ48步进电机通常有5根线：红色、橙色、黄色、粉色和蓝色。其中红色为公共线（VCC），其余四根连接到ULN2003驱动板的四个输入端。

ULN2003驱动板连接到STM32开发板的引脚：
- IN1: 连接到STM32的GPIO_PIN_X (自定义)
- IN2: 连接到STM32的GPIO_PIN_X (自定义)
- IN3: 连接到STM32的GPIO_PIN_X (自定义)
- IN4: 连接到STM32的GPIO_PIN_X (自定义)

## 库文件结构

- `28BYJ48_ULN2003.h`: 头文件，包含API函数声明和数据结构定义
- `28BYJ48_ULN2003.c`: 源文件，包含API函数实现
- `28BYJ48_ULN2003_example.c`: 使用示例

## 依赖

- STM32 HAL库

## 功能特性

- 支持全步进、半步进和波驱动模式
- 精确控制角度、步数
- 可调节速度
- 支持顺时针和逆时针旋转
- 低功耗模式（电机释放功能）

## 使用方法

### 1. 初始化

在使用前需要初始化步进电机：

```c
// 定义电机结构体
StepMotor_t stepper;

// 初始化电机
StepMotor_Init(&stepper, 
               GPIOX, GPIO_PIN_X,  // IN1引脚
               GPIOX, GPIO_PIN_X,  // IN2引脚
               GPIOX, GPIO_PIN_X,  // IN3引脚
               GPIOX, GPIO_PIN_X,  // IN4引脚
               STEP_MODE_HALF,     // 步进模式
               2);                 // 延时（毫秒）
```

### 2. 控制电机

```c
// 旋转指定步数
StepMotor_RotateSteps(&stepper, 2048, STEP_DIR_CW);  // 顺时针旋转2048步

// 旋转指定角度
StepMotor_RotateAngle(&stepper, 90.0f, STEP_DIR_CCW);  // 逆时针旋转90度

// 改变步进模式
StepMotor_SetMode(&stepper, STEP_MODE_FULL);  // 切换到全步进模式

// 改变电机速度
StepMotor_SetSpeed(&stepper, 1);  // 设置步进延时为1毫秒

// 释放电机
StepMotor_Release(&stepper);  // 释放所有线圈，不保持位置
```

## 注意事项

1. 必须在CubeMX中将连接到ULN2003的四个GPIO引脚配置为输出模式。
2. 在高速运行时可能需要外部电源，因为电机电流可能超过开发板可提供的最大电流。

## 进阶应用

- 可以结合定时器中断实现更精确的步进控制。
- 可以实现加速/减速控制，使电机运行更平滑。
- 可以通过PWM控制信号实现微步进控制，进一步提高精度。 