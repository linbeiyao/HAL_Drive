/*
@file: motor.h
@author: Asaka(Yuzupi)
@time: 2023/11/15 21:08
@brief: 本文件使用STM32 HAL API驱动有刷直流电机

HOW TO USE:
1. 初始化电机:
   - 开环模式: 使用 motor_init_openloop() 初始化
   - 闭环模式: 使用 motor_init_closedloop() 初始化

2. 设置电机参数:
   - 齿轮比、每转脉冲数、旋转半径等

3. 控制电机:
   - 开环模式: 使用 motor_rotate_openloop() 直接设置PWM占空比
   - 闭环模式:
	 * 速度控制: 使用 motor_arrive_velocity() 或 motor_set_velocity()
	 * 位置控制: 使用 motor_arrive_position() 或 motor_set_position()

4. 停止电机:
   - 使用 motor_stop() 停止电机

5. 获取状态:
   - 使用 motor_get_velocity() 获取当前速度
   - 使用 motor_get_position() 获取当前位置

示例代码：
// 开环模式示例
motor_t myMotor;
motor_init_openloop(&myMotor, &htim3, TIM_CHANNEL_1);
motor_rotate_openloop(&myMotor, 500, CLOCKWISE);  // 50%占空比顺时针旋转

// 闭环模式示例
motor_init_closedloop(&myMotor, &htim3, TIM_CHANNEL_1, &henc);
motor_set_velocity(&myMotor, 100);  // 设置目标速度100rpm
motor_arrive_position(&myMotor, 360, angle);  // 旋转到360度位置

注意事项:
- 使用前请确保已正确配置TIM定时器和GPIO引脚
- 闭环模式需要连接编码器
- 根据实际硬件调整电机参数
*/

#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"
#include "pid.h"
#include "stdlib.h"

/*用户自定义配置*/
#define TB6612_DRIVER 1			// 使用TB6612电机驱动
#define MOTOR_CLOSEDLOOP 0		// 使用闭环模式
#define LOWPASS_FILTER 1		// 低通滤波器

#if (LOWPASS_FILTER == 1)
#define FILTER_NUM 5
#endif

/*常量参数*/
#define _PI (3.14159)			// 圆周率

/*枚举类型*/
typedef enum
{
	CLOCKWISE = 1,	   // 顺时针
	ANTICLOCKWISE = -1 // 逆时针
} rotateDir_t;

typedef enum
{
	MOTOR_FALSE = 0,	// 假
	MOTOR_TRUE = 1		// 真
} motor_bool;

typedef enum
{
	MOTOR_INITING = 0, // 初始化中
	MOTOR_INIT_OK,	   // 初始化完成
} motor_initStatus_t;

typedef enum
{
	rad = 0, // 弧度
	angle,	 // 角度
	line_mm, // 毫米
	line_m	 // 米
} motor_dataType;

#if (MOTOR_CLOSEDLOOP == 1)
/*电机闭环控制结构体*/
typedef enum
{
	MOTOR_CTRL_NON = 0, // 无控制
	MOTOR_CTRL_VEL,		// 速度控制
	MOTOR_CTRL_POS,		// 位置控制
} motor_ctrl_t;

typedef struct motor_param_t
{
	uint16_t gearRatio;		// 齿轮比
	uint16_t pulsePerRound; // 每转脉冲数
	float rotateRadius_mm;	// 旋转半径，以毫米为单位
} motor_param_t;

typedef struct motor_tick_t
{
	uint32_t usTickPeriod; // 微秒周期
	uint32_t nowTick;	   // 当前tick
	uint32_t lastTick;	   // 上次tick
} motor_tick_t;

typedef struct motor_data_t
{
	rotateDir_t rotateDir;	 // 旋转方向
	motor_dataType dataType; // 数据类型
	float velocity;			 // 速度
	float position;			 // 位置
	float targetVelo;		 // 目标速度
	float targetPos;		 // 目标位置
} motor_data_t;

typedef struct motor_encoder_t
{
	TIM_HandleTypeDef *pEncHtim; // 编码器定时器句柄
	int lastEncCnt;				 // 上次编码器计数
	int nowEncCnt;				 // 当前编码器计数
	int totalEncCnt;			 // 总编码器计数
	int encOverflowNum;			 // 编码器溢出次数
} motor_encoder_t;

#if (LOWPASS_FILTER == 1)
/*低通滤波器结构体*/
typedef struct filter_t
{
	float filter[FILTER_NUM]; // 滤波器数组
} filter_t;
#endif

#endif
/*电机驱动结构体*/
#if (TB6612_DRIVER == 1)
typedef struct motor_drv_t
{
	/*驱动引脚*/
	GPIO_TypeDef *pDrvGPIO;		// 驱动GPIO端口
	uint16_t drvGPIO_Pin;		// 驱动GPIO引脚
	/*方向控制引脚*/
	GPIO_TypeDef *pGPIO_IN1;	// IN1 GPIO端口
	uint16_t pGPIO_IN1_Pin;		// IN1 GPIO引脚
	GPIO_TypeDef *pGPIO_IN2;	// IN2 GPIO端口
	uint16_t pGPIO_IN2_Pin;		// IN2 GPIO引脚
} motor_drv_t;
#endif
/*电机结构体*/
typedef struct motor_t
{
	/*PWM外设*/
	TIM_HandleTypeDef *phtim;	// PWM定时器句柄
	uint32_t motorChannel;		// 电机通道
	/*电机驱动*/
#if (TB6612_DRIVER == 1)
	motor_drv_t driver;			// 电机驱动结构体
#endif

#if (MOTOR_CLOSEDLOOP == 1)
	motor_encoder_t encoder;	// 编码器结构体
	/*电机参数*/
	motor_param_t param;		// 电机参数结构体
	/*绑定PID结构体*/
	pid_typedef *pVelPID;		// 速度PID指针
	pid_typedef *pPosPID;		// 位置PID指针
	/*闭环模式数据*/
	motor_data_t data;			// 电机数据结构体
	/*tick参数*/
	motor_tick_t tick;			// tick结构体
	/*电机状态*/
	motor_initStatus_t initStatus; // 初始化状态
	/*电机控制类型*/
	motor_ctrl_t ctrlType;		// 控制类型
#endif

#if (LOWPASS_FILTER == 1 && MOTOR_CLOSEDLOOP == 1)
	filter_t filter;			// 滤波器结构体
#endif
	/*测试电机旋转方向是否正确*/
	motor_bool ifDrvInv;		// 是否反转驱动方向
	motor_bool ifCalInv;		// 是否反转校准方向
} motor_t;

/*声明外部变量*/
// extern motor_t rMotor;
// extern motor_t lMotor;
/*外设操作函数*/
void motor_channelSetCompare(motor_t *motor, uint32_t compareVal); // 设置PWM比较值
uint32_t motor_channelGetCompare(motor_t *motor); // 获取PWM比较值
uint32_t motor_channelGetPeriod(motor_t *motor); // 获取PWM周期值

/*驱动函数*/
#if (TB6612_DRIVER == 1)
void motor_driverInit_TB6612(motor_t *motor,
							 GPIO_TypeDef *drvGPIO, uint16_t drvGPIO_Pin,	/*STBY引脚*/
							 GPIO_TypeDef *GPIO_IN1, uint16_t GPIO_IN1_PIN, /*A/BIN1引脚*/
							 GPIO_TypeDef *GPIO_IN2, uint16_t GPIO_IN2_PIN, /*A/BIN2引脚*/
							 motor_bool ifDrvInv);							/*是否反转驱动方向*/
void motor_setRotateDir_TB6612(motor_t *motor, rotateDir_t rotateDir); // 设置旋转方向
#endif

/*电机开环运行模式*/
#if (MOTOR_CLOSEDLOOP == 0)
void motor_init_MOTOR_CLOSEDLOOPopenloop(motor_t *motor, TIM_HandleTypeDef *phtim, uint32_t channel); // 开环模式初始化
void motor_rotate_openloop(motor_t *motor, uint32_t compareVal, rotateDir_t rotateDir); // 开环模式旋转
#endif

/*电机闭环运行模式*/
#if (MOTOR_CLOSEDLOOP == 1)
#if (LOWPASS_FILTER == 1)
/*速度滤波函数*/
float motor_filter_calc(filter_t *f, float in); // 滤波器计算
#endif

/*电机tick函数*/
uint32_t motor_getTick(motor_t *motor);
float motor_getTime_s(motor_t *m);
void motor_timeTick(motor_t *motor);

float motor_get_velocity(motor_t *motor);
float motor_get_position(motor_t *motor);

void motor_init_closedloop(motor_t *motor, motor_dataType dataType,			/*电机初始化*/
						   pid_typedef *velPID, pid_typedef *posPID,		/*PID初始化*/
						   TIM_HandleTypeDef *htim, uint32_t channel,		/*PWM初始化*/
						   TIM_HandleTypeDef *enchtim, motor_bool ifCalInv, /*定时器编码器初始化*/
						   uint32_t usTickPeriod,							/*设置电机tick周期*/
						   int gearRatio, int ppr, float radius);			/*设置电机参数*/

void motor_update(motor_t *motor);
void motor_checkReload(motor_t *motor);

void motor_arrive_velocity(motor_t *m, float targetVel);
void motor_arrive_position(motor_t *m, float targetPos);

void motor_set_velocity(motor_t *m, float tV);
void motor_set_position(motor_t *m, float tP);
void motor_stop(motor_t *m);

void motor_run(motor_t *m);

#endif

#endif
