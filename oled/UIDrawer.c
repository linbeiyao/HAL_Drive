#include "UIDrawer.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "main.h"

// 外部变量声明
extern unsigned int soil_moisture_value;
extern unsigned int voltage_value;
extern unsigned int current_value;
extern unsigned int power_value;
extern unsigned char is_pump_on;
extern unsigned char is_mode_auto;
extern float rain_probability;
extern uint8_t is_test;
extern char update_time[32];

extern uint8_t is_init_ok;

// 添加一个标志位表示是否正在获取数据
static uint8_t is_fetching_weather = 0;

// 子界面计数
static uint8_t sub_screen_count = 0;

// UI绘制函数指针数组
static UIDrawFunc_t s_drawFuncs[SCREEN_MAX] = {
    UI_DrawInit,             // SCREEN_INIT
    UI_DrawLoadingNetData,  // SCREEN_LOADING_NETDATA
    UI_DrawMain,            // SCREEN_MAIN
    UI_DrawStatus,          // SCREEN_STATUS  
    UI_DrawData,            // SCREEN_DATA
    UI_DrawEnv,             // SCREEN_ENV
    UI_DrawWeather,         // SCREEN_WEATHER
    UI_DrawTest            // SCREEN_TEST
};

// 获取UI绘制函数指针
UIDrawFunc_t UI_GetDrawFunction(UIScreen_t screen)
{
    if (screen < SCREEN_MAX)
    {
        return s_drawFuncs[screen];
    }
    return NULL;
}

// 设置获取天气状态的函数
void UI_SetWeatherFetching(uint8_t fetching)
{
    is_fetching_weather = fetching;
    UIManager_Update();  // 立即更新界面
}

// 子界面数量设置和获取
void UI_SetSubScreenCount(uint8_t count)
{
    sub_screen_count = count;
    UIScreenHierarchy_t hierarchy = UIManager_GetScreenHierarchy();
    hierarchy.subScreenCount = count;
}

uint8_t UI_GetSubScreenCount(void)
{
    return sub_screen_count;
}

// 从帧缓冲区获取像素值的辅助函数
uint8_t OLED_GetPixel(uint8_t x, uint8_t y, uint8_t *buffer)
{
    if (!buffer) return 0;
    
    // 计算像素在缓冲区中的位置
    uint16_t byte_pos = (y / 8) * 128 + x;
    uint8_t bit_pos = y % 8;
    
    // 检查指定位是否为1
    return (buffer[byte_pos] & (1 << bit_pos)) ? 1 : 0;
}

// 获取帧缓冲区的辅助函数
uint8_t* OLED_GetFrameBuffer(void)
{
    extern uint8_t OLED_GRAM[8][128]; // 从oled.c引入的外部变量
    return (uint8_t*)OLED_GRAM;
}

// 绘制像素的辅助函数
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    OLED_SetPixel(x, y, color ? OLED_COLOR_INVERT : OLED_COLOR_NORMAL);
}

// 绘制水平线的辅助函数
void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t length, uint8_t color)
{
    for (uint8_t i = 0; i < length; i++) {
        OLED_DrawPixel(x + i, y, color);
    }
}

// 绘制矩形的辅助函数
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    OLED_DrawHLine(x, y, width, color);  // 上边
    OLED_DrawHLine(x, y + height - 1, width, color);  // 下边
    
    for (uint8_t i = 0; i < height; i++) {
        OLED_DrawPixel(x, y + i, color);  // 左边
        OLED_DrawPixel(x + width - 1, y + i, color);  // 右边
    }
}

// 填充矩形的辅助函数
void OLED_FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    for (uint8_t j = 0; j < height; j++) {
        for (uint8_t i = 0; i < width; i++) {
            OLED_DrawPixel(x + i, y + j, color);
        }
    }
}

// 新增 - Toast提示绘制函数
void UI_DrawToast(const char* message, ToastType_t type)
{
    uint8_t bg_color = OLED_COLOR_NORMAL;
    uint8_t fg_color = OLED_COLOR_INVERT;
    
    // 根据提示类型设置背景色
    switch(type) {
        case TOAST_SUCCESS:
            bg_color = OLED_COLOR_NORMAL;  // 成功为正常颜色
            break;
        case TOAST_WARNING:
        case TOAST_ERROR:
            bg_color = OLED_COLOR_INVERT;  // 警告和错误为反色
            break;
        case TOAST_INFO:
        default:
            bg_color = OLED_COLOR_NORMAL;  // 普通提示为正常颜色
            break;
    }
    
    // 计算提示框尺寸
    uint8_t msg_len = strlen(message);
    uint8_t toast_width = msg_len * 8 + 16;  // 每个字符8像素宽 + 两侧各8像素边距
    if (toast_width > 120) toast_width = 120;  // 限制最大宽度
    
    uint8_t toast_height = 24;  // 固定高度24像素
    
    // 计算居中位置
    uint8_t x = (128 - toast_width) / 2;
    uint8_t y = 20;  // 在屏幕上方偏下位置显示
    
    // 绘制提示框背景
    OLED_FillRect(x, y, toast_width, toast_height, bg_color);
    
    // 绘制边框
    OLED_DrawRect(x, y, toast_width, toast_height, fg_color);
    
    // 绘制提示文本
    // 居中显示
    uint8_t text_x = x + (toast_width - msg_len * 8) / 2;
    uint8_t text_y = y + (toast_height - 16) / 2;
    OLED_PrintString(text_x, text_y, (char*)message, &font8x16, bg_color == OLED_COLOR_NORMAL ? OLED_COLOR_INVERT : OLED_COLOR_NORMAL);
}

// 新增 - 对话框绘制函数
void UI_DrawDialog(const char* title, const char* message, DialogType_t type, uint8_t selected_button)
{
    // 对话框大小
    uint8_t dialog_width = 112;  // 较大的宽度
    uint8_t dialog_height = 48;  // 较大的高度
    
    // 计算居中位置
    uint8_t x = (128 - dialog_width) / 2;
    uint8_t y = (64 - dialog_height) / 2;
    
    // 绘制半透明背景蒙版 (使用交错的点阵实现)
    for (uint8_t by = 0; by < 64; by++) {
        for (uint8_t bx = 0; bx < 128; bx++) {
            if ((bx + by) % 2 == 0) {
                OLED_DrawPixel(bx, by, 1);
            }
        }
    }
    
    // 绘制对话框背景
    OLED_FillRect(x, y, dialog_width, dialog_height, OLED_COLOR_NORMAL);
    
    // 绘制对话框边框
    OLED_DrawRect(x, y, dialog_width, dialog_height, OLED_COLOR_INVERT);
    
    // 绘制标题
    uint8_t title_x = x + (dialog_width - strlen(title) * 8) / 2;
    OLED_PrintString(title_x, y + 2, (char*)title, &font8x16, OLED_COLOR_INVERT);
    
    // 绘制分隔线
    OLED_DrawHLine(x + 2, y + 18, dialog_width - 4, OLED_COLOR_INVERT);
    
    // 计算消息文本行数和每行长度
    uint8_t msg_len = strlen(message);
    uint8_t chars_per_line = dialog_width / 8 - 2;  // 每行字符数 (减去左右各1个字符的边距)
    uint8_t lines = (msg_len + chars_per_line - 1) / chars_per_line;  // 向上取整
    if (lines > 2) lines = 2;  // 最多显示2行
    
    // 绘制消息文本
    for (uint8_t i = 0; i < lines; i++) {
        char line_buf[20] = {0};  // 临时缓冲区
        uint8_t start = i * chars_per_line;
        uint8_t len = (start + chars_per_line > msg_len) ? (msg_len - start) : chars_per_line;
        strncpy(line_buf, message + start, len);
        line_buf[len] = '\0';
        
        uint8_t msg_x = x + (dialog_width - len * 8) / 2;
        OLED_PrintString(msg_x, y + 20 + i * 10, line_buf, &font8x16, OLED_COLOR_INVERT);
    }
    
    // 绘制按钮
    if (type == DIALOG_CONFIRM) {
        // 确认按钮
        uint8_t btn_width = 40;
        uint8_t btn_x1 = x + dialog_width / 3 - btn_width / 2;
        uint8_t btn_y = y + dialog_height - 14;
        
        // 绘制确认按钮 (选中时反色)
        OLED_FillRect(btn_x1, btn_y, btn_width, 12, selected_button == 0 ? OLED_COLOR_INVERT : OLED_COLOR_NORMAL);
        OLED_DrawRect(btn_x1, btn_y, btn_width, 12, OLED_COLOR_INVERT);
        OLED_PrintString(btn_x1 + 4, btn_y + 2, "确认", &font8x16, selected_button == 0 ? OLED_COLOR_NORMAL : OLED_COLOR_INVERT);
        
        // 取消按钮
        uint8_t btn_x2 = x + dialog_width * 2 / 3 - btn_width / 2;
        OLED_FillRect(btn_x2, btn_y, btn_width, 12, selected_button == 1 ? OLED_COLOR_INVERT : OLED_COLOR_NORMAL);
        OLED_DrawRect(btn_x2, btn_y, btn_width, 12, OLED_COLOR_INVERT);
        OLED_PrintString(btn_x2 + 4, btn_y + 2, "取消", &font8x16, selected_button == 1 ? OLED_COLOR_NORMAL : OLED_COLOR_INVERT);
    } else {
        // 仅确认按钮
        uint8_t btn_width = 40;
        uint8_t btn_x = x + (dialog_width - btn_width) / 2;
        uint8_t btn_y = y + dialog_height - 14;
        
        OLED_FillRect(btn_x, btn_y, btn_width, 12, OLED_COLOR_INVERT);
        OLED_DrawRect(btn_x, btn_y, btn_width, 12, OLED_COLOR_INVERT);
        OLED_PrintString(btn_x + 4, btn_y + 2, "确定", &font8x16, OLED_COLOR_NORMAL);
    }
}

// 新增 - 界面切换动画绘制函数
void UI_DrawAnimation(UIScreen_t from_screen, UIScreen_t to_screen, uint8_t progress, uint8_t type)
{
    UIDrawFunc_t from_func = UI_GetDrawFunction(from_screen);
    UIDrawFunc_t to_func = UI_GetDrawFunction(to_screen);
    
    // 备份当前帧缓冲区
    uint8_t frame_buf_copy[128*64/8];
    memcpy(frame_buf_copy, OLED_GetFrameBuffer(), 128*64/8);
    
    // 如果是滑动动画
    if (type == 0) {
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
        int16_t offset = (int16_t)(128 * (100 - progress) / 100);
        
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
    else if (type == 1) {
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
                
                if (rand_val < progress) {
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

// ========== 初始化界面 ==========
void UI_DrawInit(void)
{
    static uint8_t dots = 0;
    char loading[32] = {0};
    
    OLED_NewFrame();
    
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统初始化", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示加载动画
    sprintf(loading, "Loading%.*s", dots % 4, "...");
    OLED_PrintString((128 - (8 * strlen(loading))) / 2, 24, loading, &font8x16, OLED_COLOR_NORMAL);
    dots++;
    
    // 显示提示
    OLED_PrintString((128 - (16 * 4)) / 2, 48, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 主界面 ==========
void UI_DrawMain(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    
    if (!is_init_ok) {
        // 显示初始化界面
        OLED_PrintString((128 - (16 * 4)) / 2, 16, "系统初始化", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    } else {
        // 显示标题
        OLED_PrintString((128 - (16 * 5)) / 2, 0, "土壤湿度系统", &font16x16, OLED_COLOR_NORMAL);
        
        // 显示土壤湿度（反向计算：越湿润值越大）
        sprintf(buffer, "湿度: %d%%", 100 - (int)(soil_moisture_value * 100 / 4095));
        OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示水泵状态
        sprintf(buffer, "水泵: %s", is_pump_on ? "开启" : "关闭");
        OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示模式
        sprintf(buffer, "模式: %s", is_mode_auto ? "自动" : "手动");
        OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    }
    
    OLED_ShowFrame();
}

// ========== 状态界面 ==========
void UI_DrawStatus(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统状态", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示电压 (voltage_value单位为厘伏，显示为V)
    sprintf(buffer, "电压: %.2fV", voltage_value / 100.0f);
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示电流 (current_value单位为毫安，显示为mA)
    sprintf(buffer, "电流: %dmA", current_value);
    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示功率 (power_value单位为毫瓦，显示为mW)
    sprintf(buffer, "功率: %dmW", power_value);
    OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 数据界面 ==========
void UI_DrawData(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统信息", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示阈值设置
    sprintf(buffer, "湿度阈值: %d%%", 30);
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示版本信息
    sprintf(buffer, "版本: V1.0");
    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 环境信息界面 ==========
void UI_DrawEnv(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    
    if(is_fetching_weather) {
        // 显示加载界面
        OLED_PrintString((128 - (16 * 4)) / 2, 0, "正在获取", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 16, "天气数据", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
        
        // 显示动态加载动画
        static uint8_t dots = 0;
        char loading[32] = "Loading";
        for(uint8_t i = 0; i < (dots % 3 + 1); i++) {
            strcat(loading, ".");
        }
        dots++;
        OLED_PrintString((128 - (8 * strlen(loading))) / 2, 48, loading, &font8x16, OLED_COLOR_NORMAL);
    } else {
        // 显示环境信息界面
        OLED_PrintString((128 - (16 * 4)) / 2, 0, "环境信息", &font16x16, OLED_COLOR_NORMAL);
        
        // 显示土壤湿度
        sprintf(buffer, "土壤湿度: %d%%", 100 - (int)(soil_moisture_value * 100 / 4095));
        OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示降雨概率
        sprintf(buffer, "降雨概率: %.1f%%", rain_probability);
        OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示更新时间
        OLED_PrintString(0, 48, update_time, &font8x16, OLED_COLOR_NORMAL);
    }
    
    OLED_ShowFrame();
}

// ========== 天气详情界面 ==========
void UI_DrawWeather(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    
    if(is_fetching_weather) {
        // 显示加载界面
        OLED_PrintString((128 - (16 * 4)) / 2, 0, "正在获取", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 16, "天气数据", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    } else {
        // 显示天气详情界面
        OLED_PrintString((128 - (16 * 4)) / 2, 0, "天气详情", &font16x16, OLED_COLOR_NORMAL);
        
        // 示例天气数据显示
        sprintf(buffer, "今日: 晴转多云");
        OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        sprintf(buffer, "温度: 22-28℃");
        OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        sprintf(buffer, "湿度: 45%% 东风3级");
        OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    }
    
    OLED_ShowFrame();
} 

void UI_DrawLoadingNetData(void)
{
    OLED_NewFrame();
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "正在获取", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString((128 - (16 * 4)) / 2, 16, "天气数据", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示动态加载动画
    static uint8_t dots = 0;
    char loading[32] = "Loading";
    for(uint8_t i = 0; i < (dots % 3 + 1); i++) {
        strcat(loading, ".");
    }
    dots++;
    OLED_PrintString((128 - (8 * strlen(loading))) / 2, 48, loading, &font8x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// 测试界面绘制函数
void UI_DrawTest(void)
{
    char buffer[32];
    static uint8_t current_item = 0;
    
    OLED_NewFrame();
    
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "测试界面", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示测试项目
    const char* test_items[] = {
        "Toast提示",
        "对话框",
        "切换动画",
        "子界面",
    };
    
    // 显示菜单项
    for(uint8_t i = 0; i < 4; i++) {
        if(i == current_item) {
            // 当前选中项使用反色显示
            OLED_FillRect(0, 16 + i * 12, 128, 12, OLED_COLOR_INVERT);
            OLED_PrintString(4, 16 + i * 12, (char*)test_items[i], &font8x16, OLED_COLOR_NORMAL);
        } else {
            OLED_PrintString(4, 16 + i * 12, (char*)test_items[i], &font8x16, OLED_COLOR_INVERT);
        }
    }
    
    OLED_ShowFrame();
}



