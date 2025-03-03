#ifndef _MQTT_H_
#define _MQTT_H_

#include "stm32f1xx_hal.h"

void mqtt_connect(void);
void mqtt_pub(void);
void mqtt_sub(unsigned char* dataPtr);

#endif
