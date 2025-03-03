#ifndef MQ2_H
#define MQ2_H
#include "stdint.h"
#include "stm32f1xx_hal.h"
#define    Vref           3.3   // dien ap toi da ADC STM32 || ESP32
#define    Number_sample  4095  // vi co 12 bit ma hoa => 2^12 - 1 = 4095 
#define    RL             10    // dien tro noi tiep voi RS 
typedef struct {
	float a;              // Diem chan
	float b;              // Do doc
	float R0 ;            // Dien tro cua cam bien trong khong khi trong lanh
	float RS_air ;			  // RS trong khong khi trong lanh
	float RS_gas;		      // RS trong khong khi co khi khac
	float ppm;            // 
	float ratino;         // Ty le 
	float Sensor_volt;    // Diep ap doc ve tu chan ADC
	float percent_result;  // ket qua nong do chung ta can
}Tyde_Def_MQ2;
//ham init mq2
//a va b bien doi de doc tung khi 
/*
    Exponential regression:
        Gas    | a      | b
    khi H2     | 987.99 | -2.162
    khi LPG    | 574.25 | -2.222
    khi CO     | 36974  | -3.109
  */
void Mq2_init(Tyde_Def_MQ2 *_mq2,ADC_HandleTypeDef *_hadc, float a , float b);
//ham handle la ham goi lien tuc trong ham while
void Mq2_handle(Tyde_Def_MQ2 *_mq2);
//chuyen doi gia tri tu so -> Volt
void set_Sensor_volt( Tyde_Def_MQ2 *_mq2 , uint16_t _adcValue );
void set_RS_air(Tyde_Def_MQ2 *_mq2);
void set_RO(Tyde_Def_MQ2 *_mq2);
void set_RS_gas(Tyde_Def_MQ2 *_mq2);
void set_ratino(Tyde_Def_MQ2 *_mq2);
void set_ppm(Tyde_Def_MQ2 *_mq2);
void set_percent_result(Tyde_Def_MQ2 *_mq2);
#endif
