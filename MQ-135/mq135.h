/*
 * mq135.h
 *
 *  Created on: Dec 8, 2021
 *      Author: alarm
 */

#ifndef __MQ135_H__
#define __MQ135_H__

#include "hdr.h"
#include "main.h"

#ifdef SET_MQ135

//-----------------------------------------------------------------------------

/// The load resistance on the board
#define RLOAD 10.0
/// Calibration resistance at atmospheric CO2 level
#define RZERO 323.0//526.10
/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 116.6020682
#define PARB 2.769034857

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 407.05

//-----------------------------------------------------------------------------

uint8_t _pin;

//-----------------------------------------------------------------------------

float MQ135_getCorrectionFactor(float t, float h);
float MQ135_getResistance();
float MQ135_getCorrectedResistance(float t, float h);
float MQ135_getPPM();
float MQ135_getCorrectedPPM(float t, float h);
float MQ135_getRZero();
float MQ135_getCorrectedRZero(float t, float h);
uint16_t ppmColor(float p);

//-----------------------------------------------------------------------------

#endif

#endif /* __MQ135_H__ */
