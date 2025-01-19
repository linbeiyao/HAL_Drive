#ifndef __UI_MANAGER_H
#define __UI_MANAGER_H

#include <stdint.h>
#include "oled.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 界面ID的枚举，统一管理所有UI页面
 */
typedef enum
{
    /* 主要界面 */
    SCREEN_MAIN = 0,              // 主界面
    SCREEN_STATUS,                // 状态界面
    SCREEN_DATA,                  // 数据界面
    SCREEN_ENV,                   // 环境界面

    /* 秤重相关界面 */
    SCREEN_SCALES_1,              // 秤1界面
    SCREEN_SCALES_2,              // 秤2界面
    SCREEN_SCALES_3,              // 秤3界面
    SCREEN_SELECT_FRUIT,          // 选择水果界面
    SCREEN_HX711_CALIBRATION,     // HX711校准界面

    /* 支付相关界面 */
    SCREEN_PAYMENT,               // 支付界面（显示金额）
    SCREEN_PAY_SUCCESS,           // 支付成功界面（显示成功及花费金额）
    SCREEN_PAY_FAILED,            // 支付失败界面（显示失败及原因）
    SCREEN_PAY_CANCEL,            // 支付取消界面

    /* 用户卡相关界面 */
    SCREEN_CARD_INPUT,            // 卡片录入界面
    SCREEN_CARD_INPUT_SUCCESS,    // 卡片录入成功界面
    SCREEN_CARD_INPUT_FAILED,     // 卡片录入失败界面
    SCREEN_CARD_RECHARGE,         // 卡片充值界面
    SCREEN_CARD_BALANCE,          // 卡片余额显示界面

    /* 网络相关界面 */
    SCREEN_NET_CONNECT,           // 网络连接成功界面
    SCREEN_NET_DISCONNECT,        // 网络连接失败界面
    SCREEN_NET_SYNC,              // 数据同步界面

    /* 系统相关界面 */
    SCREEN_SYSTEM_ERROR,          // 系统错误界面
    SCREEN_SYSTEM_INFO,           // 系统信息界面
    
    SCREEN_MAX                    // 界面总数
} UIScreen_t;

void UIManager_Init(void);
void UIManager_Update(void);
void UIManager_SwitchScreen(UIScreen_t screen);

#ifdef __cplusplus
}
#endif

#endif // __UI_MANAGER_H
