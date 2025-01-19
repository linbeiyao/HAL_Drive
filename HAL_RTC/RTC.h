#ifndef __RTC_H
#define __RTC_H

#include "main.h"


extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

void MX_RTC_Init(void);
void RTC_SetTime(uint8_t hour, uint8_t minute, uint8_t second);
void RTC_SetDate(uint8_t year, uint8_t month, uint8_t date, uint8_t weekday);

void RTC_GetTimeAndDate(void);
void RTC_SetAlarmForReminder(uint16_t minutes);          // 通过传入的分钟数设置闹钟

extern uint8_t alarm_over;
	
#endif

