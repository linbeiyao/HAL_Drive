
#include "Mq2.h"
#include "math.h"
ADC_HandleTypeDef *hadc;

__WEAK void Mq2_callback(float value){
	
}
void Mq2_init(Tyde_Def_MQ2 *_mq2,ADC_HandleTypeDef *_hadc, float a , float b){
	hadc = _hadc;
	_mq2->a = a;
	_mq2->b = b;
}
void Mq2_handle(Tyde_Def_MQ2 *_mq2){
	  HAL_ADC_Start(hadc);
		HAL_ADC_PollForConversion(hadc,1000);
		uint16_t value_before = HAL_ADC_GetValue(hadc);
		set_Sensor_volt(_mq2,value_before);
		set_RS_air(_mq2);
    set_RO(_mq2);
	  uint16_t value_after = HAL_ADC_GetValue(hadc);
	  set_Sensor_volt(_mq2,value_after);
	  set_RS_gas(_mq2);
    set_ratino(_mq2);
	  set_ppm(_mq2);
    set_percent_result(_mq2);
		HAL_ADC_Stop(hadc);
}
void set_Sensor_volt(Tyde_Def_MQ2 *_mq2 , uint16_t _adcValue){
  float Volt = (Vref*_adcValue / (Number_sample)) ;
  _mq2->Sensor_volt = Volt;
}
void set_RS_air(Tyde_Def_MQ2 *_mq2){
	_mq2->RS_air = ((3.3*RL)/_mq2->Sensor_volt)-RL;
}
void set_RS_gas(Tyde_Def_MQ2 *_mq2){
	_mq2->RS_gas = ((3.3*RL)/_mq2->Sensor_volt)-RL;
}
void set_RO(Tyde_Def_MQ2 *_mq2){
	//RS / R0 = 4.4 ppm : ty so can o kk trong lanh
	_mq2->R0 = _mq2->RS_air/4.4;
}
void set_ratino(Tyde_Def_MQ2 *_mq2){
	_mq2->ratino = _mq2->RS_gas / _mq2->R0 ;
}
void set_ppm(Tyde_Def_MQ2 *_mq2){
	_mq2->ppm = pow(10, ((log10(_mq2->ratino)-_mq2->a)/(_mq2->b)));
}

void set_percent_result(Tyde_Def_MQ2 *_mq2){
 _mq2->percent_result = _mq2->ppm / 10000;
	Mq2_callback(_mq2->percent_result);
}
