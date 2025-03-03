#include "sg90.h"
#include "tim.h"


/* SG90(��ʱ������)��ʼ�� */
void Sg90_Init()
{
	//HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); /* ���� PD13 TIM4_Channel2 */
	
	/* TIM1_Channel3N ��N�� ��������� ����tim.c���ֶ�����pwmģʽ��cubemxû���Զ�����*/
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);	
}
/*
	*	PWM �ź���0-180����Ĺ�ϵ��
	*	0.5ms ---------------- 0��   

	*	1ms   ---------------- 45��  
	*	1.5ms ---------------- 90��  
	*	2ms   ---------------- 135�� 
	*	2.5ms ---------------- 180�� 
 
	*	SG90���Ƶ����ռ�ձȵļ��㣺
	*	SG90�����Ƶ��Ϊ50HZ��PWM����Ϊ20ms��0�ȶ�Ӧ��ռ�ձ�Ϊ0.025����0.05ms�ĸߵ�ƽ�����
*/
void Sg90_Angle(int angle)
{
	// kx + b = 2000 / 180 * x + 500
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, angle * 2000 / 180  + 500);	 /* ���� PD13 TIM4_Channel2 */
	
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, angle * 2000 / 180  + 500);    /* ���� PB15 TIM1_Channel3N */
}
