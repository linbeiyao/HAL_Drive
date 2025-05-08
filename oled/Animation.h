#ifndef __ANIMATION_H
#define __ANIMATION_H

#include "UIManager.h"

// 动画类型定义
typedef enum {
    ANIMATION_SLIDE,  // 滑动动画
    ANIMATION_FADE    // 淡入淡出动画
} AnimationType_t;

// 动画状态定义
typedef enum {
    ANIMATION_IDLE,   // 空闲状态
    ANIMATION_RUNNING // 运行状态
} AnimationState_t;

// 动画参数结构
typedef struct {
    UIScreen_t fromScreen;
    UIScreen_t toScreen;
    AnimationType_t type;
    uint8_t progress;  // 0-100
    AnimationState_t state;
} Animation_t;

// 动画管理函数
void Animation_Init(void);
void Animation_Start(UIScreen_t fromScreen, UIScreen_t toScreen, AnimationType_t type);
void Animation_Update(void);
AnimationState_t Animation_GetState(void);

#endif /* __ANIMATION_H */ 