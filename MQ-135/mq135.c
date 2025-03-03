/*
 * mq135.c
 *
 *  Created on: Dec 8, 2021
 *      Author: alarm
 */
#include "hdr.h"


#include "main.h"
#include "mq135.h"

#ifdef SET_MQ135

uint8_t pinMQ;
float rzero = RZERO;

//-----------------------------------------------------------------------------
//Get the correction factor to correct for temperature and humidity
//param[in] t  The ambient air temperature
//param[in] h  The relative humidity
//return The calculated correction factor
//
float MQ135_getCorrectionFactor(float t, float h)
{
  return CORA * t * t - CORB * t + CORC - (h - 33.0) * CORD;
}
//-----------------------------------------------------------------------------
//Get the resistance of the sensor, ie. the measurement value
//return The sensor resistance in kOhm
//
float MQ135_getResistance()
{
  int val = valMQ;//analogRead(pinMQ);

  //return ((1023.0 / (float)val) * 5.0 - 1.0) * RLOAD;
  return ((4095.0 / (float)val) * 5.0 - 1.0) * RLOAD;
}
//-----------------------------------------------------------------------------
//Get the resistance of the sensor, ie. the measurement value corrected for temp/hum
//param[in] t  The ambient air temperature
//param[in] h  The relative humidity
//return The corrected sensor resistance kOhm
//
float MQ135_getCorrectedResistance(float t, float h)
{
  return MQ135_getResistance() / MQ135_getCorrectionFactor(t, h);
}
//-----------------------------------------------------------------------------
//Get the ppm of CO2 sensed (assuming only CO2 in the air)
//return The ppm of CO2 in the air
//
float MQ135_getPPM()
{
  return PARA * pow((MQ135_getResistance() / rzero/*RZERO*/), -PARB);
}
//-----------------------------------------------------------------------------
//Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected for temp/hum
//param[in] t  The ambient air temperature
//param[in] h  The relative humidity
//return The ppm of CO2 in the air
//
float MQ135_getCorrectedPPM(float t, float h)
{
  return PARA * pow((MQ135_getCorrectedResistance(t, h) / rzero/*RZERO*/), -PARB);
}
//-----------------------------------------------------------------------------
//Get the resistance RZero of the sensor for calibration purposes
//return The sensor resistance RZero in kOhm
//
float MQ135_getRZero()
{
  return MQ135_getResistance() * pow((ATMOCO2 / PARA), (1.0 / PARB));
}
//-----------------------------------------------------------------------------
//Get the corrected resistance RZero of the sensor for calibration purposes
//param[in] t  The ambient air temperature
//param[in] h  The relative humidity
//return The corrected sensor resistance RZero in kOhm
//
float MQ135_getCorrectedRZero(float t, float h)
{
  return MQ135_getCorrectedResistance(t, h) * pow((ATMOCO2 / PARA), (1.0 / PARB));
}
//-----------------------------------------------------------------------------
uint16_t ppmColor(float p)
{
// ppm = 0..400 - OK (green)
// ppm = 400..600 +- (yellow)
// ppm = 600..1000 - (magenta)
// ppm = 1000.. -- (red)
uint16_t ret = GREEN;

	if ((p > 400.0) && (p < 600)) ret = YELLOW;
	else
	if ((p > 600.0) && (p < 1000)) ret = MAGENTA;
	else
	if (p > 1000.0) ret = RED;

	return ret;
}
//-----------------------------------------------------------------------------



#endif
