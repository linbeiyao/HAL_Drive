#include "buzzer.h"

/**
 * @brief  蜂鸣器初始化
 * @param  无
 * @retval 无
 */
void Buzzer_Init(void)
{
    BUZZER_OFF();  // 初始化时关闭蜂鸣器
}

/**
 * @brief  蜂鸣器响指定时间
 * @param  time_ms: 蜂鸣时间(毫秒)
 * @retval 无
 */
void Buzzer_Beep(uint16_t time_ms)
{
    BUZZER_ON();
    HAL_Delay(time_ms);
    BUZZER_OFF();
}

/**
 * @brief  蜂鸣器响指定次数
 * @param  times: 蜂鸣次数
 * @retval 无
 */
void Buzzer_BeepTimes(uint8_t times)
{
    while(times--)
    {
        BUZZER_ON();
        HAL_Delay(100);  // 响100ms
        BUZZER_OFF();
        HAL_Delay(100);  // 停100ms
    }
}

/**
 * @brief  报警模式 - 快速响三声
 * @param  无
 * @retval 无
 */
void Buzzer_Alarm(void)
{
    for(uint8_t i = 0; i < 3; i++)
    {
        BUZZER_ON();
        HAL_Delay(50);   // 响50ms
        BUZZER_OFF();
        HAL_Delay(50);   // 停50ms
    }
} 