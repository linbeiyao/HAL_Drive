#include "sg90.h"
#include "tim.h"


/* SG90(定时器启动)初始化 */
void Sg90_Init()
{
	//HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); /* 启动 PD13 TIM4_Channel2 */
	
	/* TIM1_Channel3N 带N的 用这个函数 并在tim.c中手动开启pwm模式，cubemx没有自动配置*/
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);	
}
/*
	*	PWM 信号与0-180舵机的关系：
	*	0.5ms ---------------- 0度   

	*	1ms   ---------------- 45度  
	*	1.5ms ---------------- 90度  
	*	2ms   ---------------- 135度 
	*	2.5ms ---------------- 180度 
 
	*	SG90舵机频率与占空比的计算：
	*	SG90舵机的频率为50HZ，PWM周期为20ms，0度对应的占空比为0.025，即0.05ms的高电平输出。
*/
void Sg90_Angle(int angle)
{
	// kx + b = 2000 / 180 * x + 500
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, angle * 2000 / 180  + 500);	 /* 设置 PD13 TIM4_Channel2 */
	
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, angle * 2000 / 180  + 500);    /* 设置 PB15 TIM1_Channel3N */
}
