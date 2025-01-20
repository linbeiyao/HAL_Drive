#include "BH1750.h"
#include <stdint.h>

// 定义新的光照强度和亮度之间的映射表，光照值区间调整到 0 -- 60 lux
#define NUM_LIGHT_LEVELS 16

// 光照映射表定义
static const LightTableEntry_t default_light_table[] = {
    {0,  100, 5},   // 光照值 0  -> 亮度 100, 调整因子 5
    {5,  95,  4},   // 光照值 5  -> 亮度 95,  调整因子 4
    {10, 90,  3},   // 光照值 10 -> 亮度 90,  调整因子 3
    {15, 85,  2},   // 光照值 15 -> 亮度 85,  调整因子 2
    {20, 80,  1},   // 光照值 20 -> 亮度 80,  调整因子 1
    {25, 75,  0},   // 光照值 25 -> 亮度 75,  调整因子 0
    {30, 70,  0},   // 光照值 30 -> 亮度 70,  调整因子 0
    {35, 65, -1},   // 光照值 35 -> 亮度 65,  调整因子 -1
    {40, 60, -2},   // 光照值 40 -> 亮度 60,  调整因子 -2
    {45, 55, -3},   // 光照值 45 -> 亮度 55,  调整因子 -3
    {50, 50, -4},   // 光照值 50 -> 亮度 50,  调整因子 -4
    {55, 45, -5},   // 光照值 55 -> 亮度 45,  调整因子 -5
    {60, 40, -6},   // 光照值 60 -> 亮度 40,  调整因子 -6
    {65, 30, -7},   // 光照值 65 -> 亮度 30,  调整因子 -7
    {70, 20, -8},   // 光照值 70 -> 亮度 20,  调整因子 -8
    {75, 10, -9}    // 光照值 75 -> 亮度 10,  调整因子 -9
};

// 全局设备实例
BH1750_Device_t g_BH1750_Device = {
    .hi2c = NULL,
    .currentMode = BH1750_CONT_HIGH_RES_MODE,
    .mtreg = 69,  // 默认值
    .lastLightValue = 0.0f,
    .isInitialized = 0,
    .lightTable = default_light_table,
    .tableSize = sizeof(default_light_table) / sizeof(default_light_table[0])
};

/**
 * 根据当前光照值获取目标亮度
 *
 * @param light_value 当前的光照值
 * @return 目标亮度值
 */
uint16_t BH1750_GetTargetBrightness(BH1750_Device_t* dev, uint16_t light_value) {
    // 光照值低于最低值，返回最大亮度
    if (light_value <= dev->lightTable[0].lightValue) {
        return dev->lightTable[0].brightness;
    }

    // 环境光过于亮
    if (light_value > 500) {
        return 0;
    }

    // 遍历查找光照值所在的区间
    for (int i = 0; i < dev->tableSize - 1; i++) {
        uint16_t light_low = dev->lightTable[i].lightValue;
        uint16_t light_high = dev->lightTable[i + 1].lightValue;

        if (light_value >= light_low && light_value < light_high) {
            uint16_t brightness_low = dev->lightTable[i].brightness;
            uint16_t brightness_high = dev->lightTable[i + 1].brightness;
            
            // 使用线性插值计算目标亮度
            uint16_t target_brightness = brightness_low + 
                ((light_value - light_low) * (brightness_high - brightness_low)) / 
                (light_high - light_low);

            return target_brightness;
        }
    }

    // 如果光照值超过最大值，返回最小亮度
    return dev->lightTable[dev->tableSize - 1].brightness;
}

// 光照测量模块的初始化函数
void BH1750_Init(BH1750_Device_t* dev, I2C_HandleTypeDef* hi2c)
{
    uint8_t cmd;
    dev->hi2c = hi2c;

    // 打开电源
    cmd = BH1750_POWER_ON;
    HAL_I2C_Master_Transmit(dev->hi2c, BH1750_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);

    // 复位
    cmd = BH1750_RESET;
    HAL_I2C_Master_Transmit(dev->hi2c, BH1750_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);

    // 设置为默认的连续高分辨率模式
    cmd = BH1750_CONT_HIGH_RES_MODE;
    HAL_I2C_Master_Transmit(dev->hi2c, BH1750_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);

    dev->isInitialized = 1;
}

// 切换测量模式
void BH1750_SetMode(BH1750_Device_t* dev, uint8_t mode)
{
    HAL_I2C_Master_Transmit(dev->hi2c, BH1750_I2C_ADDR, &mode, 1, HAL_MAX_DELAY);
    dev->currentMode = mode;
}

// 设置测量时间
void BH1750_SetMTReg(BH1750_Device_t* dev, uint8_t mtreg)
{
    if (mtreg < 31 || mtreg > 254) {
        return;
    }

    uint8_t cmd[2];
    cmd[0] = 0x40 | (mtreg >> 5);    // 高3位
    cmd[1] = 0x60 | (mtreg & 0x1F);  // 低5位

    HAL_I2C_Master_Transmit(dev->hi2c, BH1750_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    dev->mtreg = mtreg;
}

// 读取光强数据
float BH1750_ReadLight(BH1750_Device_t* dev)
{
    uint8_t buffer[2];
    uint16_t raw_data;

    HAL_I2C_Master_Receive(dev->hi2c, BH1750_I2C_ADDR, buffer, 2, HAL_MAX_DELAY);
    HAL_Delay(120);

    raw_data = (buffer[0] << 8) | buffer[1];
    dev->lastLightValue = raw_data / 1.2f;  // 转换为 lux
    
    return dev->lastLightValue;
}