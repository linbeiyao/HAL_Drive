/**
 * @file 28BYJ48_ULN2003_example.c
 * @brief 28BYJ48步进电机和ULN2003驱动板使用示例
 */

#include "main.h"
#include "28BYJ48_ULN2003.h"

// 全局电机结构体
StepMotor_t stepper;

/**
 * @brief 步进电机使用示例
 * 此函数展示了如何初始化和控制28BYJ48步进电机
 * 
 * 需要在CubeMX中配置四个GPIO引脚为输出模式，并在主函数中调用本函数
 */
void StepMotor_Example(void) {
    // 1. 初始化步进电机 - 根据实际硬件连接修改GPIO端口和引脚
    // 参数依次为：电机结构体，四个引脚端口和引脚，步进模式，延时（毫秒）
    StepMotor_Init(&stepper, 
                   GPIOX, GPIO_PIN_X,  // 修改为实际的IN1引脚
                   GPIOX, GPIO_PIN_X,  // 修改为实际的IN2引脚
                   GPIOX, GPIO_PIN_X,  // 修改为实际的IN3引脚
                   GPIOX, GPIO_PIN_X,  // 修改为实际的IN4引脚
                   STEP_MODE_HALF,     // 使用半步进模式，提高精度
                   2);                 // 2毫秒的步进延时
    
    // 2. 旋转指定步数示例 - 顺时针旋转2048步（全步进模式下一圈）
    StepMotor_RotateSteps(&stepper, 2048, STEP_DIR_CW);
    
    // 延时等待完成
    HAL_Delay(1000);
    
    // 3. 旋转指定角度示例 - 逆时针旋转90度
    StepMotor_RotateAngle(&stepper, 90.0f, STEP_DIR_CCW);
    
    // 延时等待完成
    HAL_Delay(1000);
    
    // 4. 改变步进模式示例
    StepMotor_SetMode(&stepper, STEP_MODE_FULL);
    
    // 5. 改变速度示例 - 减小延时，加快速度
    StepMotor_SetSpeed(&stepper, 1);  // 1毫秒的步进延时
    
    // 旋转一圈
    StepMotor_RotateAngle(&stepper, 360.0f, STEP_DIR_CW);
    
    // 延时等待完成
    HAL_Delay(1000);
    
    // 6. 波驱动模式示例（单线圈激活，节省功耗但扭矩较小）
    StepMotor_SetMode(&stepper, STEP_MODE_WAVE);
    StepMotor_RotateSteps(&stepper, 512, STEP_DIR_CW);
    
    // 延时等待完成
    HAL_Delay(1000);
    
    // 7. 释放电机（不保持位置，节省功耗）
    StepMotor_Release(&stepper);
}

/**
 * 在main.c的主循环中调用示例：
 * 
 * int main(void) {
 *     // STM32 HAL库初始化代码...
 *     
 *     // 步进电机示例
 *     StepMotor_Example();
 *     
 *     while (1) {
 *         // 主循环代码...
 *     }
 * }
 */ 