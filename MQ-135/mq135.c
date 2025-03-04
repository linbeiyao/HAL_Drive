#include "mq135.h"

// 计算电压比例系数
double Vs = (double) Vin / (Res - 1);

/**
 * @brief 根据ADC值计算电压
 * @param ADC_val: ADC采样值
 * @return 计算得到的电压值
 */
double MQ135_getVoltage(uint32_t ADC_val) {
	return ADC_val * Vs;
}

/**
 * @brief 计算温湿度校正因子
 * @param tem: 温度值
 * @param hum: 湿度值
 * @return 校正因子
 */
double MQ135_getCorrectionFactor(float tem, float hum) {
	return (CorrA * pow(tem, 3) + CorrB * pow(tem, 2) - CorrC * tem + CorrD
			- (CorrE * hum - CorrE * 33));
}

/**
 * @brief 根据ADC值计算传感器电阻
 * @param ADC_val: ADC采样值
 * @return 传感器电阻值
 */
double MQ135_getResistance(uint32_t ADC_val) {
	double vol = MQ135_getVoltage(ADC_val);
	double rs = ((Vin * RL) / vol) - RL;
	if (rs > 0) {
		return rs;
	} else
		return 0;
}

/**
 * @brief 计算经过温湿度校正后的传感器电阻
 * @param tem: 温度值
 * @param hum: 湿度值
 * @param ADC_val: ADC采样值
 * @return 校正后的传感器电阻值
 */
double MQ135_getCorrectResistance(double tem, double hum, uint32_t ADC_val) {
	return MQ135_getResistance(ADC_val) / MQ135_getCorrectionFactor(tem, hum);
}

/**
 * @brief 根据ADC值计算气体ppm值
 * @param a: 气体特性参数a
 * @param b: 气体特性参数b
 * @param ADC_val: ADC采样值
 * @return 气体浓度ppm值
 */
double MQ135_getPPM(float a, float b, uint32_t ADC_val) {
	double ratio = MQ135_getResistance(ADC_val) / R0;
	double ppm = a * pow(ratio, b);
	if (ppm > 0)
		return ppm;
	else
		return 0;
}

/**
 * @brief 计算经过温湿度校正后的气体ppm值
 * @param a: 气体特性参数a
 * @param b: 气体特性参数b
 * @param tem: 温度值
 * @param hum: 湿度值
 * @param ADC_val: ADC采样值
 * @return 校正后的气体浓度ppm值
 */
double MQ135_getCorrectPPM(float a, float b, float tem, float hum,
		uint32_t ADC_val) {
	double ratio = MQ135_getCorrectResistance(tem, hum, ADC_val) / R0;
	double ppm = a * pow(ratio, b);
	return ppm;
}

/**
 * @brief 使用线性方法计算气体ppm值
 * @param a: 气体特性参数a
 * @param b: 气体特性参数b
 * @param ADC_val: ADC采样值
 * @return 气体浓度ppm值
 */
double MQ135_getPPMLinear(float a, float b, uint32_t ADC_val) {
	double ratio = MQ135_getResistance(ADC_val) / R0;
	double ppm_log = (log10(ratio) - b) / a;
	double ppm = pow(10, ppm_log);
	if (ppm > 0)
		return ppm;
	else
		return 0;
}

// 以下为各种气体的浓度计算函数
double MQ135_getAcetone(uint32_t ADC_val) {
	return MQ135_getPPM(34.688, -3.369, ADC_val);
}

double MQ135_getCorrectAcetone(float tem, float hum, uint32_t ADC_val) {
	return MQ135_getCorrectPPM(34.688, -3.369, tem, hum, ADC_val);
}

double MQ135_getAlcohol(uint32_t ADC_val) {
	return MQ135_getPPM(77.255, -3.18, ADC_val);
}

double MQ135_getCorrectAlcohol(float tem, float hum, uint32_t ADC_val) {
	return MQ135_getCorrectPPM(77.255, -3.18, tem, hum, ADC_val);
}

double MQ135_getCO2(uint32_t ADC_val) {
	return MQ135_getPPM(110.47, -2.862, ADC_val) + ATMOCO2;
}

double MQ135_getCorrectCO2(float tem, float hum, uint32_t ADC_val) {
	return MQ135_getCorrectPPM(110.47, -2.862, tem, hum, ADC_val) + ATMOCO2;
}

double MQ135_getCO(uint32_t ADC_val) {
	return MQ135_getPPM(605.18, -3.937, ADC_val);
}

double MQ135_getCorrectCO(float tem, float hum, uint32_t ADC_val) {
	return MQ135_getCorrectPPM(605.18, -3.937, tem, hum, ADC_val);
}

double MQ135_getNH4(uint32_t ADC_val) {
	return MQ135_getPPM(102.2, -2.473, ADC_val);
}

double MQ135_getCorrectNH4(float tem, float hum, uint32_t ADC_val) {
	return MQ135_getCorrectPPM(102.2, -2.473, tem, hum, ADC_val);
}

double MQ135_getToluene(uint32_t ADC_val) {
	return MQ135_getPPM(44.947, -3.445, ADC_val);
}

double MQ135_getCorrectToluene(float tem, float hum, uint32_t ADC_val) {
	return MQ135_getCorrectPPM(44.947, -3.445, tem, hum, ADC_val);
}

/*
 float MQ135::getR0() {
 double r0 = getResistance() / 3.6;
 return r0;
 }

 float MQ135::getCorrectedR0(float tem, float hum) {
 double r0 = getCorrectedResistance(tem, hum) / 3.6;
 return r0;
 }

 double MQ135::getR0ByCO2Level(float ppm) {
 if (ppm > ATMOCO2) {
 ppm -= ATMOCO2;
 }
 else {
 return NAN;
 }
 double tmp = -(log10(ppm / 110.47) / -2.862) + log10(getResistance());
 return pow(10, tmp);
 }

 double MQ135::getCorrectedR0ByCO2Level(float ppm, float tem, float hum) {
 if (ppm > ATMOCO2) {
 ppm -= ATMOCO2;
 }
 else {
 return NAN;
 }
 double tmp = -(log10(ppm / 110.47) / -2.862) + log10(getCorrectedResistance(tem, hum));
 return pow(10, tmp);
 }

 void MQ135::setR0(float r0) {
 R0 = r0;
 }

/**
 * @brief 获取未校正的空气质量参数
 * @param aqi: 空气质量参数结构体指针
 * @param ADC_val: ADC采样值
 */
void getAQI_val(Air_param_t *aqi, uint32_t ADC_val) {
	aqi->Acetone = MQ135_getAcetone(ADC_val);
	aqi->Alcohol = MQ135_getAlcohol(ADC_val);
	aqi->CO = MQ135_getCO(ADC_val);
	aqi->CO2 = MQ135_getCO2(ADC_val);
	aqi->NH4 = MQ135_getNH4(ADC_val);
	aqi->Toluene = MQ135_getToluene(ADC_val);
}

/**
 * @brief 获取经过温湿度校正的空气质量参数
 * @param aqi: 空气质量参数结构体指针
 * @param tem: 温度值
 * @param hum: 湿度值
 * @param ADC_val: ADC采样值
 */
void getAQI_Correctval(Air_param_t *aqi, int tem, int hum, uint32_t ADC_val) {
	aqi->Acetone = MQ135_getCorrectAcetone(tem, hum, ADC_val);
	aqi->Alcohol = MQ135_getCorrectAlcohol(tem, hum, ADC_val);
	aqi->CO = MQ135_getCorrectCO(tem, hum, ADC_val);
	aqi->CO2 = MQ135_getCorrectCO2(tem, hum, ADC_val);
	aqi->NH4 = MQ135_getCorrectNH4(tem, hum, ADC_val);
	aqi->Toluene = MQ135_getCorrectToluene(tem, hum, ADC_val);
}
