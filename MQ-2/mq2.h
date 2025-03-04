/**
 * @brief MQ-2 气体传感器使用指南
 * 
 * 1. 初始化传感器：
 *    - 调用 Mq2_init() 函数，传入 MQ2 结构体指针、ADC 句柄以及检测气体的 a、b 参数
 *    - 示例：Mq2_init(&mq2, &hadc1, 987.99, -2.162); // 检测氢气
 * 
 * 2. 在 main 函数中初始化后，在 while 循环中持续调用 Mq2_handle() 函数处理数据
 *    - 示例：Mq2_handle(&mq2);
 * 
 * 3. 获取检测结果：
 *    - 通过 mq2.percent_result 获取气体浓度百分比
 *    - 通过 mq2.ppm 获取气体浓度 ppm 值
 * 
 * 4. 自定义回调函数：
 *    - 实现 Mq2_callback() 函数，当有新数据时会自动调用
 *    - 示例：
 *      void Mq2_callback(float value) {
 *          printf("当前气体浓度：%.2f%%\n", value);
 *      }
 * 
 * 注意事项：
 * - 传感器需要预热时间，建议上电后等待 20-30 秒再读取数据
 * - 不同气体需要不同的 a、b 参数，请根据检测气体选择合适的参数
 * - 传感器应避免接触水蒸气和高浓度酒精
 */



#ifndef MQ2_H
#define MQ2_H
#include "stdint.h"
#include "stm32f1xx_hal.h"
#define    Vref           3.3   // STM32 || ESP32 ADC最大电压
#define    Number_sample  4095  // 12位ADC => 2^12 - 1 = 4095 
#define    RL             10    // 与RS串联的电阻 
typedef struct {
	float a;              // 截距
	float b;              // 斜率
	float R0 ;            // 传感器在清洁空气中的电阻
	float RS_air ;			  // 清洁空气中的RS值
	float RS_gas;		      // 含有其他气体时的RS值
	float ppm;            // 气体浓度(ppm)
	float ratino;         // 比率 
	float Sensor_volt;    // 从ADC引脚读取的电压
	float percent_result;  // 我们需要的浓度结果
}Tyde_Def_MQ2;
/**
 * @brief MQ2传感器初始化函数
 * @note a和b参数用于检测不同气体，以下是常见气体的指数回归参数：
 * 
 * | 气体类型       | a值     | b值     |
 * |----------------|---------|---------|
 * | 氢气(H2)       | 987.99  | -2.162  |
 * | 液化石油气(LPG)| 574.25  | -2.222  |
 * | 一氧化碳(CO)   | 36974   | -3.109  |
 */
void Mq2_init(Tyde_Def_MQ2 *_mq2,ADC_HandleTypeDef *_hadc, float a , float b);
// 处理函数，在while循环中持续调用
void Mq2_handle(Tyde_Def_MQ2 *_mq2);
// 将数字值转换为电压
void set_Sensor_volt( Tyde_Def_MQ2 *_mq2 , uint16_t _adcValue );
void set_RS_air(Tyde_Def_MQ2 *_mq2);
void set_RO(Tyde_Def_MQ2 *_mq2);
void set_RS_gas(Tyde_Def_MQ2 *_mq2);
void set_ratino(Tyde_Def_MQ2 *_mq2);
void set_ppm(Tyde_Def_MQ2 *_mq2);
void set_percent_result(Tyde_Def_MQ2 *_mq2);
#endif
