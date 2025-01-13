#include <ws2812b.h>
#include <stdint.h>
#include <string.h>
#include "usart.h"

struct ws2812b_color ws2812b_color_table[] = {
    {255, 0, 0},     // 红色            RED
    {255, 165, 0},   // 橙色            ORANGE
    {255, 255, 0},   // 黄色            YELLOW
    {0, 255, 0},     // 绿色            GREEN
    {0, 255, 127},   // 青色            CYAN
    {0, 0, 255},     // 蓝色            BLUE
    {138, 43, 226},  // 紫色            PURPLE
    {255, 255, 255}, // 白色            WHITE
    {0, 0, 0},       // 黑色            BLACK
    {255, 255, 240}, // 冷白色          COLD_WHITE
    {255, 182, 193}, // 桃色            PINK
    {255, 69, 0},    // 暖色            WARM
    {128, 0, 0},     // 深红色          DEEP_RED
    {255, 140, 0},   // 珊瑚色          CORAL
    {255, 215, 0},   // 金色            GOLD
    {173, 255, 47},  // 鲜黄色          LIME
    {0, 128, 0},     // 深绿色          DEEP_GREEN
    {75, 0, 130},    // 靛蓝色          INDIGO
    {255, 105, 180}, // 热粉色          HOT_PINK
    {255, 20, 147},  // 深粉色          DEEP_PINK
    {255, 160, 122}, // 浅珊瑚色        LIGHT_CORAL
    {30, 144, 255},  // 董青色          DODGER_BLUE
    {255, 228, 196}, // 蜜桃色          PEACH
    {255, 218, 185}, // 杏色            APRICOT
    {240, 230, 140}, // 卡其色          KHAKI
    {255, 239, 213}, // 柠檬绸色        LEMON_CHIFFON
    {255, 245, 238}, // 海贝壳色        SEASHELL
    {210, 180, 140}, // 沙棕色          SANDY_BROWN
    {255, 228, 196}, // 古董白          ANTIQUE_WHITE
    {173, 216, 230}, // 浅钢蓝色        LIGHT_STEEL_BLUE
    {240, 128, 128}, // 玫瑰棕色        BROWN_ROSE
    {255, 192, 203}, // 米色            BEIGE
    {255, 182, 193}, // 浅粉色          LIGHT_PINK
    {255, 160, 122}, // 浅珊瑚色        LIGHT_CORAL_2
    {210, 105, 30},  // 金菊色          GOLDENROD
    {255, 165, 0},   // 巧克力色        CHOCOLATE
    {255, 127, 39},  // 铜色            COPPER
    {255, 69, 0},    // 番茄色          TOMATO
    {255, 99, 71},   // 珊瑚粉          CORAL_PINK
    {255, 111, 207}, // 兰花紫          ORCHID
    {255, 20, 147},  // 深洋红色        DEEP_FUCHSIA
    {0, 100, 0},     // 森林绿          FORREST_GREEN
    {72, 61, 139},   // 暗紫色          DARK_PURPLE
    {106, 90, 205},  // 暗蓝色          DARK_BLUE
    {255, 140, 104}, // 桃粉色          PEACH_PINK
    {255, 160, 122}, // 浅珊瑚色        LIGHT_CORAL_3
    {255, 218, 185}  // 杏色            APRICOT_2
};

// 场景模式标识符
uint8_t homeflag = 0;
uint8_t bookbarflag = 0;
uint8_t meetingflag = 0;
uint8_t restflag = 0;
uint8_t enterflag = 0;

// 这里的数组用于平滑渐变的 RGB 通道存储
// 从当前颜色到目标颜色之间平滑过渡
// 当前颜色存储
uint8_t current_R[LED_NUMS] = {0}, current_G[LED_NUMS] = {0}, current_B[LED_NUMS] = {0};
// 目标颜色存储
uint8_t target_R[LED_NUMS] = {0}, target_G[LED_NUMS] = {0}, target_B[LED_NUMS] = {0};

uint8_t ws2812_set_color_smooth_RGB_enum(uint8_t num); // 实现单个灯的渐变函数

/**
 * 限制输入值范围在 0 到 255 之间
 *
 * @param value 输入值
 * @return 限制后的值
 */
uint8_t clamp(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return (uint8_t)value;
}

/**
 * 设置单个 LED 的 RGB 颜色值到 WS2812 的数据缓冲区
 *
 * @param R 红色分量（0-255）
 * @param G 绿色分量（0-255）
 * @param B 蓝色分量（0-255）
 * @param num 指定的 LED 编号（从 0 开始）
 */
void ws2812_set_RGB(uint8_t R, uint8_t G, uint8_t B, uint16_t num)
{
    // 计算指定 LED 在缓冲区中的位置
    uint16_t *p = (RGB_buffur + RESET_PULSE) + (num * LED_DATA_LEN);

    // 将 RGB 数据按照 WS2812 协议填充到缓冲区
    for (uint16_t i = 0; i < 8; i++)
    {
        p[i] = (G << i) & 0x80 ? ONE_PULSE : ZERO_PULSE;
        p[i + 8] = (R << i) & 0x80 ? ONE_PULSE : ZERO_PULSE;
        p[i + 16] = (B << i) & 0x80 ? ONE_PULSE : ZERO_PULSE;
    }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
}

// 使用 结构体 进行颜色设置
void ws2812_set_color_struct(void *color_ptr, uint8_t num)
{
    struct ws2812b_color *color = (struct ws2812b_color *)color_ptr;
    ws2812_set_RGB(color->R, color->G, color->B, num);
}

// 使用 枚举 进行颜色设置
void ws2812_set_color_enum(enum color_enum color_num, uint8_t num)
{
    ws2812_set_RGB(ws2812b_color_table[color_num].R, ws2812b_color_table[color_num].G, ws2812b_color_table[color_num].B, num);
}

// 设置目标颜色 在功能函数中调用  并且调用平滑刷新函数
void ws2812_set_target_color(enum color_enum color_num, uint8_t num)
{
    target_R[num] = ws2812b_color_table[color_num].R;
    target_G[num] = ws2812b_color_table[color_num].G;
    target_B[num] = ws2812b_color_table[color_num].B;
}

// 求两数的差值绝对值
#define abs_diff(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

// 宏定义实现 round 函数
#define ROUND(x) ((x) >= 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))

// 获取两个数的最大值
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**
 * 对单个 LED 进行平滑颜色调整
 *
 * @param num 要调整的 LED 编号
 * @return 1 如果颜色调整成功，0 如果不需要调整
 */
uint8_t ws2812_set_color_smooth_RGB_enum(uint8_t num)
{
    if (num >= LED_NUMS)
        return 0; // 防止数组越界

    // 检查是否需要调整颜色
    if (current_R[num] != target_R[num] ||
        current_G[num] != target_G[num] ||
        current_B[num] != target_B[num])
    {

        // 获取调整次数，基于最大分量的差值
        uint8_t adjus_count = MAX(abs_diff(current_R[num], target_R[num]),
                                  MAX(abs_diff(current_G[num], target_G[num]),
                                      abs_diff(current_B[num], target_B[num])));

        if (adjus_count == 0)
            return 0; // 避免除零

        // 计算每个颜色分量的调整步长
        float R_step = (float)(target_R[num] - current_R[num]) / adjus_count;
        float G_step = (float)(target_G[num] - current_G[num]) / adjus_count;
        float B_step = (float)(target_B[num] - current_B[num]) / adjus_count;

        // 使用浮点变量避免误差累积
        float current_R_float = current_R[num];
        float current_G_float = current_G[num];
        float current_B_float = current_B[num];

        // 按照调整步数逐步调整颜色
        for (uint8_t i = 0; i < adjus_count; i++)
        {
            current_R_float += R_step;
            current_G_float += G_step;
            current_B_float += B_step;

            // 四舍五入后更新整数值
            current_R[num] = clamp(ROUND(current_R_float));
            current_G[num] = clamp(ROUND(current_G_float));
            current_B[num] = clamp(ROUND(current_B_float));

            // 更新当前 LED 的颜色
            ws2812_set_RGB(current_R[num], current_G[num], current_B[num], num);
            HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)RGB_buffur, RESET_PULSE + WS2812_DATA_LEN);

            // 延迟用于平滑效果
            HAL_Delay(5);
        }
        return 1; // 调整成功
    }
    return 0; // 无需调整
}

// 整条灯平滑设置

uint8_t current_R_global = 0;
uint8_t current_G_global = 0;
uint8_t current_B_global = 0;

/**
 * 平滑过渡环境灯效设置函数
 *
 * 本函数旨在为所有LED灯设置目标颜色，以实现平滑过渡的灯效每个LED灯将逐步改变其颜色，直到达到指定的目标颜色
 * 这是一个平滑的颜色变化过程，旨在创建一个渐变的视觉效果
 *
 * @param target_color 目标颜色，属于预定义的颜色枚举类型这指定了所有LED灯将要改变到的颜色
 */
void ws2812b_SetTarget_ALL(enum color_enum target_color)
{
    // 遍历所有LED灯，为每个LED灯设置相同的目标颜色
    for (int i = 0; i < LED_NUMS; i++)
    {
        target_R[i] = ws2812b_color_table[target_color].R;
        target_G[i] = ws2812b_color_table[target_color].G;
        target_B[i] = ws2812b_color_table[target_color].B;
    }
}

// 获取整条灯同时渐变所需要的次数
// 通过计算每个灯珠的分量的 当前颜色和目标颜色的差值 取最大值 作为调整次数
/**
 * 获取 WS2812 LED 灯条中需要调整次数最多的 LED 的调整次数
 *
 * 此函数遍历所有 LED，计算每个 LED 的颜色（红、绿、蓝）与目标颜色之间的差值，
 * 并确定需要调整的最大次数。这对于同步 LED 灯光效果或动画非常有用。
 *
 * @return uint8_t 返回需要调整次数最多的 LED 的调整次数
 */
uint8_t ws2812_getq_adjust_count(void)
{
    // 初始化最大调整次数为 0
    uint8_t max_adjus_count = 0;

    // 遍历所有 LED
    for (int i = 0; i < LED_NUMS; i++)
    {
        // 获取当前 LED 的颜色与目标颜色的差值
        uint8_t adjus_count = MAX(abs_diff(current_R[i], target_R[i]),
                                  MAX(abs_diff(current_G[i], target_G[i]),
                                      abs_diff(current_B[i], target_B[i])));
        // 如果当前 LED 的调整次数大于已记录的最大调整次数，则更新最大调整次数
        if (adjus_count > max_adjus_count)
        {
            max_adjus_count = adjus_count;
        }
    }

    // 返回需要调整次数最多的 LED 的调整次数
    return max_adjus_count;
}

/**
 * 计算每个LED的RGB值调整步长
 *
 * 该函数用于计算每个LED的红、绿、蓝颜色值从当前状态调整到目标状态所需的每一步的变化量
 * 这对于平滑地改变LED颜色非常有用通过将总变化量除以调整次数，得到每次调整的步长
 *
 * @param R_step 指向用于存储每个LED红色值调整步长的数组
 * @param G_step 指向用于存储每个LED绿色值调整步长的数组
 * @param B_step 指向用于存储每个LED蓝色值调整步长的数组
 * @param adjus_count 调整次数，用于计算每一步的变化量
 */
void ws2812b_GetStep(float *R_step, float *G_step, float *B_step, uint8_t adjus_count)
{
    // 遍历每个LED，计算其RGB值的调整步长
    for (int i = 0; i < LED_NUMS; i++)
    {
        // 计算红色值的调整步长
        R_step[i] = (float)(target_R[i] - current_R[i]) / adjus_count;
        // 计算绿色值的调整步长
        G_step[i] = (float)(target_G[i] - current_G[i]) / adjus_count;
        // 计算蓝色值的调整步长
        B_step[i] = (float)(target_B[i] - current_B[i]) / adjus_count;
    }
}

/**
 * @brief 平滑设置所有LED灯珠的颜色
 *
 * 该函数通过逐步调整RGB值来实现LED灯珠颜色的平滑过渡。它会根据预设的目标颜色和调整步数，
 * 计算每次调整的颜色步长，并逐步更新LED灯珠的颜色，直到达到目标颜色。
 *
 * @param target_R 目标红色值数组
 * @param target_G 目标绿色值数组
 * @param target_B 目标蓝色值数组
 * @return uint8_t 返回调整状态：0-还在进行中，1-调整完成，2-提前中断
 */
uint8_t ws2812_set_color_smooth_all(uint8_t target_R[], uint8_t target_G[], uint8_t target_B[])
{
    // 用于保存当前的调整步数
    static uint8_t adjus_count = 0;
    // 用于保存每个LED灯珠的RGB步长
    static float R_step[LED_NUMS], G_step[LED_NUMS], B_step[LED_NUMS];
    // 用于保存当前的RGB浮点值
    static float current_R_float[LED_NUMS], current_G_float[LED_NUMS], current_B_float[LED_NUMS];

    // 检查是否需要开始新的平滑调整
    if (adjus_count == 0)
    {
        // 获取调整步数
        adjus_count = ws2812_getq_adjust_count();
        // 如果调整步数为0，则无需调整，直接返回
        if (adjus_count == 0)
            return 1;

        // 初始化当前颜色数组
        for (uint8_t i = 0; i < LED_NUMS; i++)
        {
            current_R_float[i] = current_R[i];
            current_G_float[i] = current_G[i];
            current_B_float[i] = current_B[i];
        }

        // 计算步长
        ws2812b_GetStep(R_step, G_step, B_step, adjus_count);
    }

    // 执行一次平滑更新
    for (uint8_t i = 0; i < LED_NUMS; i++)
    {
        current_R_float[i] += R_step[i];
        current_G_float[i] += G_step[i];
        current_B_float[i] += B_step[i];

        // 将浮点数取整并限制在有效范围内，然后更新当前颜色值
        current_R[i] = clamp(ROUND(current_R_float[i]));
        current_G[i] = clamp(ROUND(current_G_float[i]));
        current_B[i] = clamp(ROUND(current_B_float[i]));
    }

    // 更新灯带
    for (uint8_t i = 0; i < LED_NUMS; i++)
    {
        // 设置LED灯珠的颜色
        ws2812_set_RGB(current_R[i], current_G[i], current_B[i], i);
    }

    // 使用DMA传输方式更新灯带颜色
    if (HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)RGB_buffur, RESET_PULSE + WS2812_DATA_LEN) != HAL_OK)
    {
        return 0; // DMA 失败
    }

    // 检查是否提前中断
    if (uartDataReady == 1)
    {
        uartDataReady = 0;
        return 2; // 提前中断
    }

    // 更新步数
    adjus_count--;

    // 检查是否完成调整
    if (adjus_count == 0)
    {
        // 当过渡完成时，恢复原始颜色
        memcpy(current_R, target_R, LED_NUMS);
        return 1; // 调整完成
    }

    return 0; // 还在进行中
}



///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////  功能函数  ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

// 通用环境设置函数（可实现单色或交替色）
void ws2812b_SetEnv(enum color_enum color1, enum color_enum color2, uint8_t single_color)
{
    for (int i = 0; i < LED_NUMS; i++)
    {
        if (single_color || (i % 2 == 0))
            ws2812_set_target_color(color1, i); // 使用平滑的方式设置颜色
        else
            ws2812_set_target_color(color2, i); // 使用平滑的方式设置颜色
    }
}

