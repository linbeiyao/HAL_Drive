/**
 * @file 28BYJ48_ULN2003.h
 * @brief 28BYJ48步进电机和ULN2003驱动板的驱动程序
 */

#ifndef __28BYJ48_ULN2003_H
#define __28BYJ48_ULN2003_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stdint.h"

/**
 * @brief 步进电机运行模式枚举
 */
typedef enum {
    STEP_MODE_FULL = 0,    // 全步进模式（双线圈激活，扭矩最大，每步11.25度）
    STEP_MODE_HALF = 1,    // 半步进模式（交替单双线圈，精度最高，每步5.625度）
    STEP_MODE_WAVE = 2     // 波驱动模式（单线圈激活，功耗最低，扭矩最小）
} StepMode_t;

/**
 * @brief 步进电机旋转方向枚举
 */
typedef enum {
    STEP_DIR_CW = 0,       // 顺时针
    STEP_DIR_CCW = 1       // 逆时针
} StepDirection_t;

/**
 * @brief 步进电机结构体
 */
typedef struct {
    GPIO_TypeDef* IN_1_Port;    // IN1引脚端口
    uint16_t IN_1_Pin;          // IN1引脚
    GPIO_TypeDef* IN_2_Port;    // IN2引脚端口
    uint16_t IN_2_Pin;          // IN2引脚
    GPIO_TypeDef* IN_3_Port;    // IN3引脚端口
    uint16_t IN_3_Pin;          // IN3引脚
    GPIO_TypeDef* IN_4_Port;    // IN4引脚端口
    uint16_t IN_4_Pin;          // IN4引脚
    StepMode_t mode;           // 运行模式
    uint32_t step_delay;       // 步进延迟（毫秒）
    uint32_t position;         // 当前位置
} StepMotor_t;

/**
 * @brief 初始化步进电机
 * @param motor 步进电机结构体指针
 * @param IN1_Port IN1引脚端口
 * @param IN1_Pin IN1引脚
 * @param IN2_Port IN2引脚端口
 * @param IN2_Pin IN2引脚
 * @param IN3_Port IN3引脚端口
 * @param IN3_Pin IN3引脚
 * @param IN4_Port IN4引脚端口
 * @param IN4_Pin IN4引脚
 * @param mode 步进模式
 * @param step_delay 步进延迟（毫秒）
 */
void StepMotor_Init(StepMotor_t* motor, 
                    GPIO_TypeDef* IN_1_Port, uint16_t IN_1_Pin,
                    GPIO_TypeDef* IN_2_Port, uint16_t IN_2_Pin,
                    GPIO_TypeDef* IN_3_Port, uint16_t IN_3_Pin,
                    GPIO_TypeDef* IN_4_Port, uint16_t IN_4_Pin,
                    StepMode_t mode, uint32_t step_delay);

/**
 * @brief 设置步进电机运行模式
 * @param motor 步进电机结构体指针
 * @param mode 步进模式
 */
void StepMotor_SetMode(StepMotor_t* motor, StepMode_t mode);

/**
 * @brief 设置步进电机速度（通过调整延迟）
 * @param motor 步进电机结构体指针
 * @param step_delay 步进延迟（毫秒）
 */
void StepMotor_SetSpeed(StepMotor_t* motor, uint32_t step_delay);

/**
 * @brief 单步旋转
 * @param motor 步进电机结构体指针
 * @param dir 旋转方向
 */
void StepMotor_Step(StepMotor_t* motor, StepDirection_t dir);

/**
 * @brief 旋转指定步数
 * @param motor 步进电机结构体指针
 * @param steps 步数
 * @param dir 旋转方向
 */
void StepMotor_RotateSteps(StepMotor_t* motor, uint32_t steps, StepDirection_t dir);

/**
 * @brief 旋转指定角度
 * @param motor 步进电机结构体指针
 * @param angle 角度（度）
 * @param dir 旋转方向
 */
void StepMotor_RotateAngle(StepMotor_t* motor, float angle, StepDirection_t dir);

/**
 * @brief 停止电机并释放（断电所有线圈）
 * @param motor 步进电机结构体指针
 */
void StepMotor_Release(StepMotor_t* motor);

#ifdef __cplusplus
}
#endif

#endif /* __28BYJ48_ULN2003_H */ 