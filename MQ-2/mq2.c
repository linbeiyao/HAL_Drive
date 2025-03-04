
#include "Mq2.h"
#include "math.h"
#include "main.h"

ADC_HandleTypeDef *hadc;

__WEAK void Mq2_callback(float value)
{
}
/**
 * @brief 初始化MQ-2传感器
 * @param _mq2: MQ-2传感器结构体指针
 * @param _hadc: ADC句柄指针
 * @param a: 传感器特性曲线参数a
 * @param b: 传感器特性曲线参数b
 * @return 无
 * 
 * 功能描述：
 * 1. 设置ADC句柄，用于后续ADC采样
 * 2. 配置MQ-2传感器的特性曲线参数a和b
 * 3. 这些参数将用于后续的ppm值计算
 */
void Mq2_init(Tyde_Def_MQ2 *_mq2, ADC_HandleTypeDef *_hadc, float a, float b)
{
	hadc = _hadc;       // 设置全局ADC句柄
	_mq2->a = a;        // 设置特性曲线参数a
	_mq2->b = b;        // 设置特性曲线参数b
}
void Mq2_handle(Tyde_Def_MQ2 *_mq2)
{
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, 1000);
	uint16_t value_before = HAL_ADC_GetValue(hadc);
	set_Sensor_volt(_mq2, value_before);
	set_RS_air(_mq2);
	set_RO(_mq2);
	uint16_t value_after = HAL_ADC_GetValue(hadc);
	set_Sensor_volt(_mq2, value_after);
	set_RS_gas(_mq2);
	set_ratino(_mq2);
	set_ppm(_mq2);
	set_percent_result(_mq2);
	HAL_ADC_Stop(hadc);
}
void set_Sensor_volt(Tyde_Def_MQ2 *_mq2, uint16_t _adcValue)
{
	float Volt = (Vref * _adcValue / (Number_sample));
	_mq2->Sensor_volt = Volt;
}
void set_RS_air(Tyde_Def_MQ2 *_mq2)
{
	_mq2->RS_air = ((3.3 * RL) / _mq2->Sensor_volt) - RL;
}
void set_RS_gas(Tyde_Def_MQ2 *_mq2)
{
	_mq2->RS_gas = ((3.3 * RL) / _mq2->Sensor_volt) - RL;
}
void set_RO(Tyde_Def_MQ2 *_mq2)
{
	// RS / R0 = 4.4 ppm : 清洁空气中的比例
	_mq2->R0 = _mq2->RS_air / 4.4;
}
void set_ratino(Tyde_Def_MQ2 *_mq2)
{
	_mq2->ratino = _mq2->RS_gas / _mq2->R0;
}
void set_ppm(Tyde_Def_MQ2 *_mq2)
{
	_mq2->ppm = pow(10, ((log10(_mq2->ratino) - _mq2->a) / (_mq2->b)));
}

void set_percent_result(Tyde_Def_MQ2 *_mq2)
{
	_mq2->percent_result = _mq2->ppm / 10000;
	Mq2_callback(_mq2->percent_result);
}
