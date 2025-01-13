#include "SG90.h"

/**
 * @brief 设置SG90舵机的角度。
 * 
 * 该函数根据角度参数计算相应的脉冲宽度，
 * 然后启动TIM2定时器的指定通道上的PWM信号以驱动舵机旋转到指定角度。
 * 
 * @param angle 要设置的舵机角度，单位为度（0-180°）。
 */
void SG90_SetAngle(uint8_t target_angle) {
    // 限制舵机角度在 0 到 180 度之间
    if (target_angle > 180) {
        target_angle = 180;
    } else if (target_angle < 0) {
        target_angle = 0;
    }

    // 静态变量保存当前角度
    static uint8_t current_angle = 0;

    // 计算步长和方向
    int step = (target_angle > current_angle) ? 1 : -1;

    // 使用 for 循环逐步改变角度
    for (uint8_t angle = current_angle; angle != target_angle; angle += step) {
        // 计算脉冲宽度：将角度转换为脉冲宽度，使用线性方程
        uint16_t pulse = (angle * 20) + 500;

        // 设置 TIM2 定时器通道 2 的比较值
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);

        // 启动 TIM2 定时器通道 2 的 PWM 信号以驱动舵机
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

        // 添加一个小延迟，以便观察到角度变化
        HAL_Delay(1); // 延迟 10 毫秒
    }

    // // 设置最终目标角度
    // uint16_t pulse = (target_angle * 20) + 500;
    // __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
    // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    // 更新当前角度
    current_angle = target_angle;
}
