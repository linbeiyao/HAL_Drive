/***********************************************
使用说明

示例代码：

// 1. 声明空气参数结构体
Air_param_t airParams;

// 2. 获取ADC值（假设使用HAL库）
uint32_t adcValue = HAL_ADC_GetValue(&hadc1);

// 3. 获取空气参数
// 3.1 无温湿度校正
airParams.CO2 = MQ135_getCO2(adcValue);
airParams.CO = MQ135_getCO(adcValue);

// 3.2 有温湿度校正（假设温度25℃，湿度50%）
float temperature = 25.0;
float humidity = 50.0;
airParams.CO2 = MQ135_getCorrectCO2(temperature, humidity, adcValue);
airParams.CO = MQ135_getCorrectCO(temperature, humidity, adcValue);

// 4. 使用获取到的参数
printf("CO2浓度: %d ppm\n", airParams.CO2);
printf("CO浓度: %d ppm\n", airParams.CO);

*********************************************/

#include <stdio.h>
#include "main.h"
#include "stdint.h"
#include <math.h>

#define RL 10 // 千欧
#define Vin 5
#define Res 4096 // 分辨率
#define ATMOCO2 397.13

// 校正值
#define CorrA -0.000002469136
#define CorrB 0.00048148148
#define CorrC 0.0274074074
#define CorrD 1.37530864197
#define CorrE 0.0019230769
#define R0 76.63 // R0 的平均值 (千欧)
/// 辅助函数：从输入值计算电压
/// 电压 = 输入值 * Vin / (分辨率 - 1)

typedef struct
{
    uint16_t Acetone; // 丙酮
    uint16_t Alcohol; // 酒精
    uint16_t CO;      // 一氧化碳
    uint16_t CO2;     // 二氧化碳
    uint16_t NH4;     // 氨气
    uint16_t Toluene; // 甲苯
} Air_param_t;

// struct MQ135 {
//	uint8_t pin;
// };

double MQ135_getVoltage(uint32_t ADC_val);                                          // 根据ADC值计算电压
double MQ135_getCorrectionFactor(float temparature, float humidity);                // 计算温度和湿度的校正因子
double MQ135_getResistance(uint32_t ADC_val);                                       // 根据ADC值计算传感器电阻
double MQ135_getCorrectResistance(double tem, double hum, uint32_t ADC_val);        // 计算经过温湿度校正后的传感器电阻
// a 和 b 取决于气体类型
double MQ135_getPPM(float a, float b, uint32_t ADC_val);                              // 根据ADC值计算气体ppm值
double MQ135_getCorrectPPM(float a, float b, float tem, float hum, uint32_t ADC_val); // 计算经过温湿度校正后的气体ppm值
double MQ135_getPPMLinear(float a, float b, uint32_t ADC_val);                        // 使用线性方法计算气体ppm值
double MQ135_getAcetone(uint32_t ADC_val);                                            // 计算丙酮浓度
double MQ135_getCorrectAcetone(float tem, float hum, uint32_t ADC_val);               // 计算经过温湿度校正后的丙酮浓度
double MQ135_getAlcohol(uint32_t ADC_val);                                            // 计算酒精浓度
double MQ135_getCorrectAlcohol(float tem, float hum, uint32_t ADC_val);               // 计算经过温湿度校正后的酒精浓度
double MQ135_getCO2(uint32_t ADC_val);                                                // 计算二氧化碳浓度
double MQ135_getCorrectCO2(float tem, float hum, uint32_t ADC_val);                   // 计算经过温湿度校正后的二氧化碳浓度
double MQ135_getCO(uint32_t ADC_val);                                                 // 计算一氧化碳浓度
double MQ135_getCorrectCO(float tem, float hum, uint32_t ADC_val);                    // 计算经过温湿度校正后的一氧化碳浓度
double MQ135_getNH4(uint32_t ADC_val);                                                // 计算氨气浓度
double MQ135_getCorrectNH4(float tem, float hum, uint32_t ADC_val);                   // 计算经过温湿度校正后的氨气浓度
double MQ135_getToluene(uint32_t ADC_val);                                            // 计算甲苯浓度
double MQ135_getCorrectToluene(float tem, float hum, uint32_t ADC_val);               // 计算经过温湿度校正后的甲苯浓度
void getAQI_val(Air_param_t *aqi, uint32_t ADC_val);                                  // 获取未校正的空气质量参数
void getAQI_Correctval(Air_param_t *aqi, int tem, int hum, uint32_t ADC_val);         // 获取经过温湿度校正的空气质量参数