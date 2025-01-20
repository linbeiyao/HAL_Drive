#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"
#include <stdint.h>

// BH1750 I2C地址
#define BH1750_I2C_ADDR 0x46

// 电源控制命令
#define BH1750_POWER_DOWN           0x00  // 关机模式：关闭 BH1750，进入低功耗状态
#define BH1750_POWER_ON             0x01  // 打开电源：BH1750 进入待机状态，需进一步配置
#define BH1750_RESET                0x07  // 模块复位：需先打开电源后使用，重置内部寄存器

// 连续测量模式
#define BH1750_CONT_HIGH_RES_MODE   0x10  // 高分辨率模式，1 lx 分辨率
#define BH1750_CONT_HIGH_RES_MODE_2 0x11  // 高分辨率模式 2，0.5 lx 分辨率
#define BH1750_CONT_LOW_RES_MODE    0x13  // 低分辨率模式，4 lx 分辨率

// 单次测量模式
#define BH1750_SINGLE_HIGH_RES_MODE   0x20  // 高分辨率模式，1 lx 分辨率
#define BH1750_SINGLE_HIGH_RES_MODE_2 0x21  // 高分辨率模式 2，0.5 lx 分辨率
#define BH1750_SINGLE_LOW_RES_MODE    0x23  // 低分辨率模式，4 lx 分辨率

// 光照映射表项结构体
typedef struct {
    uint16_t lightValue;    // 光照值
    uint16_t brightness;    // 对应的亮度值
    int8_t   adjustFactor;  // 调整因子
} LightTableEntry_t;

// BH1750设备结构体
typedef struct {
    I2C_HandleTypeDef* hi2c;         // I2C句柄
    uint8_t currentMode;             // 当前测量模式
    uint8_t mtreg;                   // 测量时间寄存器值
    float lastLightValue;            // 最后一次测量的光照值
    uint8_t isInitialized;           // 初始化标志
    const LightTableEntry_t* lightTable;  // 光照映射表
    uint8_t tableSize;               // 映射表大小
} BH1750_Device_t;

/** 
 * 使用流程：
 * 1. 初始化设备：
 *    BH1750_Init(&g_BH1750_Device, &hi2c1);
 * 
 * 2. 设置测量模式：
 *    BH1750_SetMode(&g_BH1750_Device, BH1750_CONT_HIGH_RES_MODE);
 * 
 * 3. 设置测量时间：
 *    BH1750_SetMTReg(&g_BH1750_Device, 69);
 * 
 * 4. 读取光照值：
 *    float light = BH1750_ReadLight(&g_BH1750_Device);
 * 
 * 5. 获取目标亮度：
 *    uint16_t brightness = BH1750_GetTargetBrightness(&g_BH1750_Device, light);
 * 
 * 6. 根据需要重复步骤 4 和 5 以实时获取光照值和目标亮度。
 */


// 函数声明
void BH1750_Init(BH1750_Device_t* dev, I2C_HandleTypeDef* hi2c);
void BH1750_SetMode(BH1750_Device_t* dev, uint8_t mode);
void BH1750_SetMTReg(BH1750_Device_t* dev, uint8_t mtreg);
float BH1750_ReadLight(BH1750_Device_t* dev);
uint16_t BH1750_GetTargetBrightness(BH1750_Device_t* dev, uint16_t light_value);

// 外部变量声明
extern BH1750_Device_t g_BH1750_Device;

#endif