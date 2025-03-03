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

    /* 手动模式界面 */
    SCREEN_STATUS,                // 状态界面
    SCREEN_DATA,                  // 数据界面



    SCREEN_MAX                    // 界面总数
} UIScreen_t;

void UIManager_Init(void);
void UIManager_Update(void);
void UIManager_SwitchScreen(UIScreen_t screen);

#ifdef __cplusplus
}
#endif

#endif // __UI_MANAGER_H
