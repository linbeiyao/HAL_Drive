/**
 * @file 28BYJ48_ULN2003.c
 * @brief 28BYJ48步进电机和ULN2003驱动板的驱动程序实现
 */

#include "28BYJ48_ULN2003.h"
#include "main.h"

// 28BYJ48 电机常量
// 28BYJ48 是一个减速比为1:64的步进电机
// 步进角度为5.625/64度（在全步进模式下）
// 一圈需要64 * (360/5.625) = 4096步（全步进模式下）
#define STEPS_PER_REVOLUTION_FULL 2048  // 全步进模式下一圈的步数（厂家参数）
#define STEPS_PER_REVOLUTION_HALF 4096  // 半步进模式下一圈的步数

// 全步进模式序列
static const uint8_t FULL_STEP_SEQUENCE[4][4] = {
    {1, 0, 0, 1},  // 步骤1
    {1, 1, 0, 0},  // 步骤2
    {0, 1, 1, 0},  // 步骤3
    {0, 0, 1, 1}   // 步骤4
};

// 半步进模式序列
static const uint8_t HALF_STEP_SEQUENCE[8][4] = {
    {1, 0, 0, 0},  // 步骤1
    {1, 1, 0, 0},  // 步骤2
    {0, 1, 0, 0},  // 步骤3
    {0, 1, 1, 0},  // 步骤4
    {0, 0, 1, 0},  // 步骤5
    {0, 0, 1, 1},  // 步骤6
    {0, 0, 0, 1},  // 步骤7
    {1, 0, 0, 1}   // 步骤8
};

// 波驱动模式序列（单线圈激活）
static const uint8_t WAVE_STEP_SEQUENCE[4][4] = {
    {1, 0, 0, 0},  // 步骤1
    {0, 1, 0, 0},  // 步骤2
    {0, 0, 1, 0},  // 步骤3
    {0, 0, 0, 1}   // 步骤4
};

/**
 * @brief 设置单个电机线圈状态
 * @param motor 步进电机结构体指针
 * @param in1 IN1线圈状态
 * @param in2 IN2线圈状态
 * @param in3 IN3线圈状态
 * @param in4 IN4线圈状态
 */
static void SetCoils(StepMotor_t* motor, uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4) {
    HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, in1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, in2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->IN3_Port, motor->IN3_Pin, in3 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->IN4_Port, motor->IN4_Pin, in4 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void StepMotor_Init(StepMotor_t* motor, 
                    GPIO_TypeDef* IN1_Port, uint16_t IN1_Pin,
                    GPIO_TypeDef* IN2_Port, uint16_t IN2_Pin,
                    GPIO_TypeDef* IN3_Port, uint16_t IN3_Pin,
                    GPIO_TypeDef* IN4_Port, uint16_t IN4_Pin,
                    StepMode_t mode, uint32_t step_delay) {
    // 初始化电机结构体
    motor->IN1_Port = IN1_Port;
    motor->IN1_Pin = IN1_Pin;
    motor->IN2_Port = IN2_Port;
    motor->IN2_Pin = IN2_Pin;
    motor->IN3_Port = IN3_Port;
    motor->IN3_Pin = IN3_Pin;
    motor->IN4_Port = IN4_Port;
    motor->IN4_Pin = IN4_Pin;
    motor->mode = mode;
    motor->step_delay = step_delay;
    motor->position = 0;
    
    // GPIO 已在 CubeMX 中配置为输出，此处无需再次配置
    
    // 初始时释放所有线圈（无保持力矩）
    StepMotor_Release(motor);
}

void StepMotor_SetMode(StepMotor_t* motor, StepMode_t mode) {
    motor->mode = mode;
}

void StepMotor_SetSpeed(StepMotor_t* motor, uint32_t step_delay) {
    motor->step_delay = step_delay;
}

void StepMotor_Step(StepMotor_t* motor, StepDirection_t dir) {
    static uint8_t step_index_full = 0;    // 全步进模式当前步骤索引
    static uint8_t step_index_half = 0;    // 半步进模式当前步骤索引
    static uint8_t step_index_wave = 0;    // 波驱动模式当前步骤索引
    
    switch (motor->mode) {
        case STEP_MODE_FULL:
            // 全步进模式 - 4步序列
            if (dir == STEP_DIR_CW) {
                step_index_full = (step_index_full + 1) % 4;
            } else {
                step_index_full = (step_index_full + 3) % 4; // -1 + 4 = 3
            }
            
            SetCoils(motor, 
                     FULL_STEP_SEQUENCE[step_index_full][0], 
                     FULL_STEP_SEQUENCE[step_index_full][1], 
                     FULL_STEP_SEQUENCE[step_index_full][2], 
                     FULL_STEP_SEQUENCE[step_index_full][3]);
            break;
            
        case STEP_MODE_HALF:
            // 半步进模式 - 8步序列
            if (dir == STEP_DIR_CW) {
                step_index_half = (step_index_half + 1) % 8;
            } else {
                step_index_half = (step_index_half + 7) % 8; // -1 + 8 = 7
            }
            
            SetCoils(motor, 
                     HALF_STEP_SEQUENCE[step_index_half][0], 
                     HALF_STEP_SEQUENCE[step_index_half][1], 
                     HALF_STEP_SEQUENCE[step_index_half][2], 
                     HALF_STEP_SEQUENCE[step_index_half][3]);
            break;
            
        case STEP_MODE_WAVE:
            // 波驱动模式 - 4步序列
            if (dir == STEP_DIR_CW) {
                step_index_wave = (step_index_wave + 1) % 4;
            } else {
                step_index_wave = (step_index_wave + 3) % 4; // -1 + 4 = 3
            }
            
            SetCoils(motor, 
                     WAVE_STEP_SEQUENCE[step_index_wave][0], 
                     WAVE_STEP_SEQUENCE[step_index_wave][1], 
                     WAVE_STEP_SEQUENCE[step_index_wave][2], 
                     WAVE_STEP_SEQUENCE[step_index_wave][3]);
            break;
    }
    
    // 更新位置
    if (dir == STEP_DIR_CW) {
        motor->position++;
    } else {
        motor->position--;
    }
    
    // 延时控制速度
    HAL_Delay(motor->step_delay);
}

void StepMotor_RotateSteps(StepMotor_t* motor, uint32_t steps, StepDirection_t dir) {
    for (uint32_t i = 0; i < steps; i++) {
        StepMotor_Step(motor, dir);
    }
}

void StepMotor_RotateAngle(StepMotor_t* motor, float angle, StepDirection_t dir) {
    uint32_t steps;
    
    // 根据角度和步进模式计算步数
    if (motor->mode == STEP_MODE_HALF) {
        steps = (uint32_t)((angle / 360.0f) * STEPS_PER_REVOLUTION_HALF);
    } else {
        steps = (uint32_t)((angle / 360.0f) * STEPS_PER_REVOLUTION_FULL);
    }
    
    StepMotor_RotateSteps(motor, steps, dir);
}

void StepMotor_Release(StepMotor_t* motor) {
    // 关闭所有线圈以释放电机
    SetCoils(motor, 0, 0, 0, 0);
} 