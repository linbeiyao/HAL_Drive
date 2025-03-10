#ifndef SG90_h
#define SG90_h

/**
 * 1.0    2024.12.7 基于STM32单片机的智能水杯设计
 * 2.0    2025.1.13 智能水果售货机
 */


#include "tim.h"

// 这里使用的是TIM2的CH2通道   引脚为PA1
// 因为  TIM2  频率为 72M/72=1M  所以  周期为 20ms  所以  占空比为 0~20ms
// 与SG90的关系为  角度=占空比/20ms*180度
// SG90的工作频率为50Hz  占空比为 0.5ms~2.5ms  角度为0~180度
#define TIM2_Prescaler 72-1
#define TIM2_Period 20000-1


typedef struct SG90_t{
    TIM_HandleTypeDef *htim;
    uint8_t channel;

    uint16_t angle;
    void (*setangle)(struct SG90_t *sg90, uint8_t target_angle);
} SG90_t;
#include "main.h"


uint8_t sg90_init(SG90_t *sg90, TIM_HandleTypeDef *htim, uint8_t channel);



/* * * 
 *
 *  SG90
 *  这里使用的是TIM2的CH2通道   引脚为PA1
 *  PWM频率为50Hz  占空比为0~20ms
 *  角度为0~180度  角度与占空比的关系为  角度=占空比/20ms*180度
 *  当前项目中 time2 设置的 预分频器 设置的是 72-1  所以  频率为 72M/72=1M
 *  自动重载 20000  所以  周期为 20ms 
 * * */

// 设置舵机角度  0~180度 
void SG90_SetAngle(SG90_t *sg90, uint8_t target_angle);











#endif

