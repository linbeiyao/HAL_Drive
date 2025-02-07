#include "SG90.h"

uint8_t sg90_init(SG90_t *sg90, TIM_HandleTypeDef *htim, uint8_t channel){
    sg90->htim = htim;
    sg90->channel = channel;

    

    sg90->setangle = SG90_SetAngle;

}



/**
 * @brief 设置SG90舵机的角度
 * 
 * 该函数根据角度参数计算相应的脉冲宽度，
 * 然后通过指定定时器的指定通道上的PWM信号控制舵机转到指定角度。
 * 
 * @param angle 要设置的目标角度，单位为度（0-180度）。
 */
void SG90_SetAngle(SG90_t *sg90, uint8_t target_angle) {
    // 限制目标角度在 0 到 180 度之间
    if (target_angle > 180) {
        target_angle = 180;
    } else if (target_angle < 0) {
        target_angle = 0;
    }

    // 静态变量保存当前角度
    static uint8_t current_angle = 0;

    // 计算步进方向
    int step = (target_angle > current_angle) ? 1 : -1;

    // 使用 for 循环逐步改变角度
    for (uint8_t angle = current_angle; angle != target_angle; angle += step) {
        // 计算脉冲宽度，将角度转换为脉冲宽度，使用线性方程
        uint16_t pulse = (angle * 20) + 500;

        // 设置定时器通道的比较值
        __HAL_TIM_SET_COMPARE(sg90->htim, sg90->channel, pulse);

        // 启动定时器通道的 PWM 信号输出
        HAL_TIM_PWM_Start(sg90->htim, sg90->channel);

        // 添加一个小延迟，以便观察到角度变化
        HAL_Delay(1); // 延迟 1 毫秒
    }

    // 更新当前角度
    current_angle = target_angle;
}
