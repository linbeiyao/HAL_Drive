
/*
 * si7021.h
 *
 *  Created on: Jan 14, 2022
 *      Author: alarm
 */

#ifndef INC_SI7021_H_
#define INC_SI7021_H_

#include "main.h"

#ifdef SET_SI7021

//**********************************************************************************

#define SI7021_ADDRESS 0x40
#define SI_RESET       0xFE
#define SI_TEMP_READ   0xE3
#define SI_HUMI_READ   0xE5

#define CHECKSUM_ERROR 0X04
#define POLYNOMIAL 0x131 //P(x)=x^8+x^5+x^4+1 = 100110001 = 0x131

//----------------------------------------------------------------------------------

#pragma pack(push,1)
typedef struct {
	float humi;
	float temp;
} result_t;
#pragma pack(pop)

//----------------------------------------------------------------------------------

result_t sens;
uint8_t si_ack[8];
bool si_dump;
const char *_siON;
const char *_siOFF;

//----------------------------------------------------------------------------------

void si7021_reset();
void si7021_read_temp();
void si7021_read_humi();

//**********************************************************************************

#endif

#endif /* INC_SI7021_H_ */
