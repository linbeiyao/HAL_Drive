#include "Animation.h"
#include "UIDrawer.h"
#include "oled.h"
#include <string.h>

// 动画实例
static Animation_t s_animation = {
    .fromScreen = SCREEN_INIT,
    .toScreen = SCREEN_INIT,
    .type = ANIMATION_SLIDE,
    .progress = 0,
    .state = ANIMATION_IDLE
};

// 初始化动画
void Animation_Init(void)
{
    s_animation.state = ANIMATION_IDLE;
    s_animation.progress = 0;
}

// 启动动画
void Animation_Start(UIScreen_t fromScreen, UIScreen_t toScreen, AnimationType_t type)
{
    s_animation.fromScreen = fromScreen;
    s_animation.toScreen = toScreen;
    s_animation.type = type;
    s_animation.progress = 0;
    s_animation.state = ANIMATION_RUNNING;
}

// 更新动画
void Animation_Update(void)
{
    if (s_animation.state != ANIMATION_RUNNING) return;

    // 更新进度
    s_animation.progress += 5;  // 每次增加5%
    if (s_animation.progress >= 100) {
        s_animation.progress = 100;
        s_animation.state = ANIMATION_IDLE;
        UIManager_SetScreen(s_animation.toScreen);
        return;
    }

    // 备份当前帧缓冲区
    uint8_t frame_buf_copy[128*64/8];
    memcpy(frame_buf_copy, OLED_GetFrameBuffer(), 128*64/8);

    // 获取源界面和目标界面的绘制函数
    UIDrawFunc_t from_func = UI_GetDrawFunction(s_animation.fromScreen);
    UIDrawFunc_t to_func = UI_GetDrawFunction(s_animation.toScreen);

    // 如果是滑动动画
    if (s_animation.type == ANIMATION_SLIDE) {
        // 先绘制源界面到缓冲区
        OLED_NewFrame();
        if (from_func) from_func();
        
        // 保存源界面帧缓冲区
        uint8_t from_frame[128*64/8];
        memcpy(from_frame, OLED_GetFrameBuffer(), 128*64/8);
        
        // 绘制目标界面到缓冲区
        OLED_NewFrame();
        if (to_func) to_func();
        
        // 保存目标界面帧缓冲区
        uint8_t to_frame[128*64/8];
        memcpy(to_frame, OLED_GetFrameBuffer(), 128*64/8);
        
        // 创建新帧，绘制混合效果
        OLED_NewFrame();
        
        // 计算滑动偏移量
        int16_t offset = (int16_t)(128 * (100 - s_animation.progress) / 100);
        
        // 绘制源界面（向左移出）
        for (uint8_t y = 0; y < 64; y++) {
            for (uint8_t x = 0; x < 128; x++) {
                if (x + offset >= 0 && x + offset < 128) {
                    // 从源帧缓冲区获取像素
                    uint8_t pixel = OLED_GetPixel(x, y, from_frame);
                    // 设置到当前帧缓冲区
                    OLED_DrawPixel(x + offset, y, pixel);
                }
            }
        }
        
        // 绘制目标界面（从右移入）
        for (uint8_t y = 0; y < 64; y++) {
            for (uint8_t x = 0; x < 128; x++) {
                if (x - 128 + offset >= 0 && x - 128 + offset < 128) {
                    // 从目标帧缓冲区获取像素
                    uint8_t pixel = OLED_GetPixel(x, y, to_frame);
                    // 设置到当前帧缓冲区
                    OLED_DrawPixel(x - 128 + offset, y, pixel);
                }
            }
        }
    } 
    // 如果是淡入淡出动画
    else if (s_animation.type == ANIMATION_FADE) {
        // 先绘制源界面到缓冲区
        OLED_NewFrame();
        if (from_func) from_func();
        
        // 保存源界面帧缓冲区
        uint8_t from_frame[128*64/8];
        memcpy(from_frame, OLED_GetFrameBuffer(), 128*64/8);
        
        // 绘制目标界面到缓冲区
        OLED_NewFrame();
        if (to_func) to_func();
        
        // 保存目标界面帧缓冲区
        uint8_t to_frame[128*64/8];
        memcpy(to_frame, OLED_GetFrameBuffer(), 128*64/8);
        
        // 创建新帧，绘制混合效果
        OLED_NewFrame();
        
        // 根据progress决定点阵混合比例
        for (uint8_t y = 0; y < 64; y++) {
            for (uint8_t x = 0; x < 128; x++) {
                // 使用伪随机数来决定显示哪一帧的像素
                uint8_t rand_val = (x * 13 + y * 7) % 100;
                
                if (rand_val < s_animation.progress) {
                    // 显示目标帧的像素
                    uint8_t pixel = OLED_GetPixel(x, y, to_frame);
                    OLED_DrawPixel(x, y, pixel);
                } else {
                    // 显示源帧的像素
                    uint8_t pixel = OLED_GetPixel(x, y, from_frame);
                    OLED_DrawPixel(x, y, pixel);
                }
            }
        }
    }
    
    OLED_ShowFrame();
}

// 获取动画状态
AnimationState_t Animation_GetState(void)
{
    return s_animation.state;
} 