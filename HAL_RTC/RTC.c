/*
 * @Author: linbeiyao linbei0522@gmail.com
 * @Date: 2024-12-14 16:01:29
 * @LastEditors: linbeiyao linbei0522@gmail.com
 * @LastEditTime: 2024-12-14 20:25:14
 * @FilePath: \Smart_water_cup_design_based_on_STM32_microcontroller\Core\Src\RTC\RTC.c
 * @Description:
 *
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved.
 */
#include "RTC.h"
#include <stdio.h>
#include "ASRPRO.h"
#include "SmartCup.h"

RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

void MX_RTC_Init(void)
{
    // 1. 启动电源时钟一访问备份寄存器
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // 2. 启用LSE (32.768KHz) 外部晶振
    __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
        ;

    // 3. 启用RTC时钟源为LSE，并使能RTC
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
    __HAL_RCC_RTC_ENABLE();

    // 4. 配置RTC
    // 初始化RTC句柄
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = 32767;           // 异步预分频器   32767   1s (LSE 时钟频率 / (32767 + 1) = 1Hz)
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE; // 不输出 RTC 信号到外部
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler(); // 如果初始化失败，进入错误处理
    }
}

// 设置时间函数
void RTC_SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    RTC_TimeTypeDef sTime = {0};

    sTime.Hours = hour;
    sTime.Minutes = minute;
    sTime.Seconds = second;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)

    {
        Error_Handler();
    }
}

// 设置日期函数
void RTC_SetDate(uint8_t year, uint8_t month, uint8_t date, uint8_t weekday)
{
    RTC_DateTypeDef sDate = {0};

    sDate.Year = year;       // 设置年份，范围 0-99
    sDate.Month = month;     // 设置月份，范围 1-12
    sDate.Date = date;       // 设置日期，范围 1-31
    sDate.WeekDay = weekday; // 设置星期几，范围 1-7

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
		

//		HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn); // 使能 RTC 中断

}

// 读取时间和日期
void RTC_GetTimeAndDate(void)
{

    // 读取时间
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    // 读取日期
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    printf("Time: %02d:%02d:%02d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
    printf("Date: 20%02d-%02d-%02d (WeekDay: %d)\n",
           sDate.Year, sDate.Month, sDate.Date, sDate.WeekDay);
}




// 设置RTC闹钟，用于任意分钟提醒
void RTC_SetAlarmForReminder(uint16_t minutes)
{
    RTC_AlarmTypeDef sAlarm = {0};

    // 获取当前时间
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    // 计算新时间
    uint8_t newMinutes = sTime.Minutes + minutes;
    if (newMinutes >= 60) {
        newMinutes -= 60;
        sTime.Hours++;
        if (sTime.Hours >= 24) {
            sTime.Hours = 0;
        }
    }

    // 配置闹钟
    sAlarm.AlarmTime.Hours = sTime.Hours;
    sAlarm.AlarmTime.Minutes = newMinutes;
    sAlarm.AlarmTime.Seconds = 0;
    sAlarm.Alarm = RTC_ALARM_A;

    // 使能闹钟中断
    if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
        Error_Handler();
    }
}


uint8_t alarm_over = 0;

// RTC闹钟中断处理函数
void RTC_Alarm_IRQHandler(void)
{
	 printf("alarm irq\r\n");
    // 检查是否是闹钟A中断
    if (__HAL_RTC_ALARM_GET_IT(&hrtc, RTC_IT_ALRA)) {
        // 清除闹钟中断标志
        __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);

        smart_cup.alarm_time_set = 1;       // 设置闹钟时间已设置标志
				RTC_GetTimeAndDate();
				
				printf("alarm over!!");
				alarm_over = 1;

//				while(1){
//					HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
//					HAL_Delay(1000);
//				}
			
        // 重新设置闹钟，以实现周期性提醒
        RTC_SetAlarmForReminder(1);			
				printf("set 30 minute");
    }
}
