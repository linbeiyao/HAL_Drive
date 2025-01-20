// ****************************************************************************
/// \file      ws2812b.c
///
/// \brief     WS2812B C Source File
///
/// \details   Driver Module for WS2812B leds.
///
/// \author    Nico Korn
///
/// \version   1.0.0.0
///
/// \date      24032021
///
/// \copyright Copyright (c) 2021 Nico Korn
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/// \pre
///
/// \bug
///
/// \warning
///
/// \todo
///
// ****************************************************************************

// Include ********************************************************************
#include "ws2812b.h"


// Private define *************************************************************

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

// Private types     **********************************************************
/* WS2812 GPIO output buffer size */
#define GPIO_BUFFERSIZE ROW *COL * 24u // see ROW as LED stripe number and COL LED pixel number on the stripe

// Private variables **********************************************************
static const uint16_t WS2812_High = 0xFFFF;
static const uint16_t WS2812_Low = 0x0000;
static uint16_t WS2812_Buffer[GPIO_BUFFERSIZE]; // ROW * COL * 24 bits (R(8bit), G(8bit), B(8bit)) = y --- output array transferred to GPIO output --- 1 array entry contents 16 bits parallel to GPIO outp
static WS2812B_StatusTypeDef WS2812_State = WS2812B_RESET;
static TIM_HandleTypeDef TIM2_Handle;
static DMA_HandleTypeDef DMA_HandleStruct_UEV;
static DMA_HandleTypeDef DMA_HandleStruct_CC1;
static DMA_HandleTypeDef DMA_HandleStruct_CC2;

// Private function prototypes ************************************************
static WS2812B_StatusTypeDef init_timer(void);
static WS2812B_StatusTypeDef init_dma(void);
static WS2812B_StatusTypeDef init_gpio(void);
static void DMA_SetConfiguration(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);
static void WS2812_TIM2_callback(void);

// Global variables ***********************************************************

// Functions ******************************************************************
// ----------------------------------------------------------------------------
/// \brief     Initialisation of the periphherals for using the ws2812b leds.
///
/// \param     none
///
/// \return    none
WS2812B_StatusTypeDef WS2812B_init(void)
{
    // init peripherals
    if (init_gpio() != WS2812B_OK)
    {
        WS2812_State = WS2812B_ERROR;
        return WS2812_State;
    }

    if (init_dma() != WS2812B_OK)
    {
        WS2812_State = WS2812B_ERROR;
        return WS2812_State;
    }

    if (init_timer() != WS2812B_OK)
    {
        WS2812_State = WS2812B_ERROR;
        return WS2812_State;
    }

    // set the ws2812b state flag to ready for operation
    WS2812_State = WS2812B_READY;

    return WS2812_State;
}

// ----------------------------------------------------------------------------
/// \brief     Initialisation of the Timer.
///
/// \param     none
///
/// \return    none
static WS2812B_StatusTypeDef init_timer(void)
{
    TIM_OC_InitTypeDef TIM_OC1Struct; // cc1
    TIM_OC_InitTypeDef TIM_OC2Struct; // cc2
    uint16_t PrescalerValue;

    // TIM2 Periph clock enable
    __HAL_RCC_TIM2_CLK_ENABLE();

    // set prescaler to get a 24 MHz clock signal
    PrescalerValue = (uint16_t)(SystemCoreClock / 24000000) - 1;

    // Time base configuration
    TIM2_Handle.Instance = TIM2;
    // TIM2_Handle.Init.Period = 29; // set the period to get 29 to get a 800kHz timer -> T=1250 ns, NOTE: the ARR will be set for data transmission and also set for the deadtime/reset timer, so the arr value changes 2 time per complete led write attempt
    TIM2_Handle.Init.Period = 29;
    // TIM2_Handle.Init.Prescaler = PrescalerValue;
    TIM2_Handle.Init.Prescaler = PrescalerValue;
    TIM2_Handle.Init.ClockDivision = 0;
    TIM2_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    if (HAL_TIM_Base_Init(&TIM2_Handle) != HAL_OK)
    {
        return WS2812B_ERROR;
    }

    // 定时模式配置：捕获比较1
    TIM_OC1Struct.OCMode = TIM_OCMODE_TIMING;
    TIM_OC1Struct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OC1Struct.Pulse = 9; // 9 pulses => 9/30 => (8/30)*1250ns = 375 ns, ws2812b datasheet: 350 ns, DSO measured 348 ns

    // Configure the channel
    if (HAL_TIM_OC_ConfigChannel(&TIM2_Handle, &TIM_OC1Struct, TIM_CHANNEL_1) != HAL_OK)
    {
        return WS2812B_ERROR;
    }

    // 定时模式配置：捕获比较2
    TIM_OC2Struct.OCMode = TIM_OCMODE_TIMING;
    TIM_OC2Struct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OC2Struct.Pulse = 21; // 21个脉冲 => 21/29 => (21/29)*1250ns = 905.17 ns，ws2812b数据手册：900 ns

    // Configure the channel
    if (HAL_TIM_OC_ConfigChannel(&TIM2_Handle, &TIM_OC2Struct, TIM_CHANNEL_2) != HAL_OK)
    {
        return WS2812B_ERROR;
    }

    // 配置TIM2中断
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    return WS2812B_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Initialisation of the direct memory access DMA.
///
/// \param     none
///
/// \return    none
static WS2812B_StatusTypeDef init_dma(void)
{
    /**
     * 配置流程：
     * 1. 启用DMA1时钟：通过调用__HAL_RCC_DMA1_CLK_ENABLE()函数来启用DMA1的时钟。
     * 2. 配置DMA1通道2：用于TIM2更新事件的DMA传输。
     *    - 设置DMA_HandleStruct_UEV.Instance为DMA1_Channel2。
     *    - 设置传输方向为从内存到外设：DMA_MEMORY_TO_PERIPH。
     *    - 禁用外设地址自增：DMA_PINC_DISABLE。
     *    - 禁用内存地址自增：DMA_MINC_DISABLE。
     *    - 设置DMA模式为正常模式：DMA_NORMAL。
     *    - 设置外设数据对齐方式为半字对齐：DMA_PDATAALIGN_HALFWORD。
     *    - 设置内存数据对齐方式为半字对齐：DMA_MDATAALIGN_HALFWORD。
     *    - 设置DMA优先级为高优先级：DMA_PRIORITY_HIGH。
     *    - 调用HAL_DMA_Init()函数初始化DMA通道。
     * 3. 配置DMA1通道5：用于TIM2 CC1事件的数据传输。
     *    - 设置DMA_HandleStruct_CC1.Instance为DMA1_Channel5。
     *    - 设置传输方向为从内存到外设：DMA_MEMORY_TO_PERIPH。
     *    - 禁用外设地址自增：DMA_PINC_DISABLE。
     *    - 禁用内存地址自增：DMA_MINC_DISABLE。
     *    - 设置DMA模式为正常模式：DMA_NORMAL。
     *    - 设置外设数据对齐方式为半字对齐：DMA_PDATAALIGN_HALFWORD。
     *    - 设置内存数据对齐方式为半字对齐：DMA_MDATAALIGN_HALFWORD。
     *    - 设置DMA优先级为高优先级：DMA_PRIORITY_HIGH。
     *    - 调用HAL_DMA_Init()函数初始化DMA通道。
     * 4. 配置DMA1通道7：用于TIM2 CC2事件的数据传输。
     *    - 设置DMA_HandleStruct_CC2.Instance为DMA1_Channel7。
     *    - 设置传输方向为从内存到外设：DMA_MEMORY_TO_PERIPH。
     *    - 禁用外设地址自增：DMA_PINC_DISABLE。
     *    - 禁用内存地址自增：DMA_MINC_DISABLE。
     *    - 设置DMA模式为正常模式：DMA_NORMAL。
     *    - 设置外设数据对齐方式为半字对齐：DMA_PDATAALIGN_HALFWORD。
     *    - 设置内存数据对齐方式为半字对齐：DMA_MDATAALIGN_HALFWORD。
     *    - 设置DMA优先级为高优先级：DMA_PRIORITY_HIGH。
     *    - 调用HAL_DMA_Init()函数初始化DMA通道。
     */



    // activate bus on which dma1 is connected
    __HAL_RCC_DMA1_CLK_ENABLE();

    // TIM2 更新事件，高输出
    // 更新事件：当定时器的计数器溢出或达到预设的自动重装载值时，产生更新事件。更新事件可以用于触发DMA传输、更新定时器的计数值等操作。
    // DMA1 通道 2 配置 ----------------------------------------------------------
    DMA_HandleStruct_UEV.Instance = DMA1_Channel2; // 选择DMA1通道2
    DMA_HandleStruct_UEV.Init.Direction = DMA_MEMORY_TO_PERIPH; // 数据传输方向，从内存到外设
    DMA_HandleStruct_UEV.Init.PeriphInc = DMA_PINC_DISABLE; // 外设地址不自增 外设地址保持不变 用于固定的地址
    DMA_HandleStruct_UEV.Init.MemInc = DMA_MINC_DISABLE; // 内存地址不自增 内存地址保持不变 用于固定的地址
    DMA_HandleStruct_UEV.Init.Mode = DMA_NORMAL;
    DMA_HandleStruct_UEV.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    DMA_HandleStruct_UEV.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    DMA_HandleStruct_UEV.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&DMA_HandleStruct_UEV) != HAL_OK)
    {
        return WS2812B_ERROR;
    }

    //TIM2 CC1 事件，数据帧输出，需要存储器上的位增量 Capture/Compare 定时器的捕获/比较功能用于测量输入信号的时间特性或生成输出信号。
    // 捕获事件：用于测量输入信号的时间特性，比如脉冲宽度、频率等 当输入信号到达某个边沿时，定时器的当前计数值会被捕获，并存储在捕获寄存器中。
    // 比较事件：用于生成输出信号，比如PWM信号。当定时器的计数值与比较寄存器中的值相等时，输出信号会被置位或复位。
    //DMA1 通道 5 配置---------------------------------------------------------
    DMA_HandleStruct_CC1.Instance = DMA1_Channel5;
    DMA_HandleStruct_CC1.Init.Direction = DMA_MEMORY_TO_PERIPH; // 数据传输方向，从内存到外设
    DMA_HandleStruct_CC1.Init.PeriphInc = DMA_PINC_DISABLE;     // 外设地址不自增 外设地址保持不变 用于固定的地址
    DMA_HandleStruct_CC1.Init.MemInc = DMA_MINC_DISABLE;        // 内存地址不自增 内存地址保持不变 用于固定的地址
    DMA_HandleStruct_CC1.Init.Mode = DMA_NORMAL;                // 正常模式
    DMA_HandleStruct_CC1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; // 外设数据对齐方式，半字对齐 一个半字为16位
    DMA_HandleStruct_CC1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD; // 内存数据对齐方式，半字对齐 一个半字为16位
    DMA_HandleStruct_CC1.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&DMA_HandleStruct_CC1) != HAL_OK)
    {
        return WS2812B_ERROR;
    }

   //TIM2 CC2 事件，数据帧输出
    //DMA1 通道 7 配置---------------------------------------------
    DMA_HandleStruct_CC2.Instance = DMA1_Channel7;
    DMA_HandleStruct_CC2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    DMA_HandleStruct_CC2.Init.PeriphInc = DMA_PINC_DISABLE;
    DMA_HandleStruct_CC2.Init.MemInc = DMA_MINC_DISABLE;
    DMA_HandleStruct_CC2.Init.Mode = DMA_NORMAL;
    DMA_HandleStruct_CC2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    DMA_HandleStruct_CC2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    DMA_HandleStruct_CC2.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&DMA_HandleStruct_CC2) != HAL_OK)
    {
        return WS2812B_ERROR;
    }

    // register callbacks
    HAL_DMA_RegisterCallback(&DMA_HandleStruct_CC2, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
    HAL_DMA_RegisterCallback(&DMA_HandleStruct_CC2, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

    // NVIC configuration for DMA transfer complete interrupt
    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 1);

    // Enable interrupt
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    return WS2812B_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Initialisation of the GPIOS.
///
/// \param     none
///
/// \return    none
static WS2812B_StatusTypeDef init_gpio(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable clock on the bus
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin =  GPIO_PIN_1; // if you want also to use pin 1, then write GPIO_PIN_0 | GPIO_PIN_1 => GPIO_PIN_1 would be the second led row
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                              // configure pins for pp output
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                            // 50 MHz rate
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                      // disable pull
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                  // setting GPIO registers

    return WS2812B_OK;
}

// ----------------------------------------------------------------------------
/// \brief     Send buffer to the ws2812b leds.
///
/// \param     none
///
/// \return    none
void WS2812B_sendBuffer(void)
{
    // wait until last buffer transmission has been completed
    while (WS2812_State != WS2812B_READY)
        ;

    // transmission complete flag, indicate that transmission is taking place
    WS2812_State = WS2812B_BUSY;

    // set period to 1.25 us with the auto reload register
    TIM2->ARR = 29u;

    // set configuration
    DMA_SetConfiguration(&DMA_HandleStruct_UEV, (uint32_t)&WS2812_High, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
    DMA_SetConfiguration(&DMA_HandleStruct_CC1, (uint32_t)&WS2812_Buffer[0], (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
    DMA_SetConfiguration(&DMA_HandleStruct_CC2, (uint32_t)&WS2812_Low, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);

    // clear all relevant DMA flags from the channels 2,5 and 7
    __HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_UEV, DMA_FLAG_TC2 | DMA_FLAG_HT2 | DMA_FLAG_TE2 | DMA_FLAG_GL2);
    __HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_CC1, DMA_FLAG_TC5 | DMA_FLAG_HT5 | DMA_FLAG_TE5 | DMA_FLAG_GL5);
    __HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_CC2, DMA_FLAG_TC7 | DMA_FLAG_HT7 | DMA_FLAG_TE7 | DMA_FLAG_GL7);

    // Enable the selected DMA transfer interrupts
    __HAL_DMA_ENABLE_IT(&DMA_HandleStruct_CC2, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));

    // enable dma channels
    __HAL_DMA_ENABLE(&DMA_HandleStruct_UEV);
    __HAL_DMA_ENABLE(&DMA_HandleStruct_CC1);
    __HAL_DMA_ENABLE(&DMA_HandleStruct_CC2);

    // clear all TIM2 flags
    TIM2->SR = 0;

    // IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
    __HAL_TIM_ENABLE_DMA(&TIM2_Handle, TIM_DMA_UPDATE);
    __HAL_TIM_ENABLE_DMA(&TIM2_Handle, TIM_DMA_CC1);
    __HAL_TIM_ENABLE_DMA(&TIM2_Handle, TIM_DMA_CC2);

    // Enable the Output compare channel
    TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_1, TIM_CCx_ENABLE);
    TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_2, TIM_CCx_ENABLE);

    // preload counter with 29 so TIM2 generates UEV directly to start DMA transfer
    __HAL_TIM_SET_COUNTER(&TIM2_Handle, 29);

    // start TIM2
    __HAL_TIM_ENABLE(&TIM2_Handle);
}

// ----------------------------------------------------------------------------
/// \brief      DMA1 Channe7 Interrupt Handler gets executed once the complete
///             frame buffer has been transmitted to the LEDs.
///
/// \param      none
///
/// \return     none
void DMA1_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&DMA_HandleStruct_CC2);
}

// ----------------------------------------------------------------------------
/// \brief      Timer 2 interrupt handler.
///
/// \param      none
///
/// \return     none
void TIM2_IRQHandler(void)
{
    WS2812_TIM2_callback();
}

// ----------------------------------------------------------------------------
/// \brief      Used to wait, until deadtime/reset period is finished, thus
///             the leds have accepted their values.
///
/// \param      none
///
/// \return     none
static void WS2812_TIM2_callback(void)
{
    // Clear TIM2 Interrupt Flag
    HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);

    // stop TIM2 now because dead period has been reached
    __HAL_TIM_DISABLE(&TIM2_Handle);

    // disable the TIM2 Update interrupt again so it doesn't occur while transmitting data
    __HAL_TIM_DISABLE_IT(&TIM2_Handle, TIM_IT_UPDATE);

    // finally indicate that the data frame has been transmitted
    WS2812_State = WS2812B_READY;
}

// ----------------------------------------------------------------------------
/// \brief      Sets the DMA Transfer parameter.
///
/// \param      [in]    pointer to a DMA_HandleTypeDef structure that contains
///                     the configuration information for the specified DMA Stream.
/// \param      [in]    The source memory Buffer address
/// \param      [in]    The destination memory Buffer address
/// \param      [in]    The length of data to be transferred from source to destination
///
/// \return     none
static void DMA_SetConfiguration(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
    // Clear all flags
    hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << hdma->ChannelIndex);

    // Configure DMA Channel data length
    hdma->Instance->CNDTR = DataLength;

    // Memory to Peripheral
    if ((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH)
    {
        // Configure DMA Channel destination address
        hdma->Instance->CPAR = DstAddress;

        // Configure DMA Channel source address
        hdma->Instance->CMAR = SrcAddress;
    }
    // Peripheral to Memory
    else
    {
        // Configure DMA Channel source address
        hdma->Instance->CPAR = SrcAddress;

        // Configure DMA Channel destination address
        hdma->Instance->CMAR = DstAddress;
    }
}

// ----------------------------------------------------------------------------
/// \brief      DMA 转换 完成 回调。
///
/// \param      [in]    pointer to a DMA_HandleTypeDef structure that contains
///                     the configuration information for the specified DMA Stream.
///
/// \return     none
static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
{
    // clear DMA7 transfer complete interrupt flag
    HAL_NVIC_ClearPendingIRQ(DMA1_Channel7_IRQn);

    // disable the DMA channels
    __HAL_DMA_DISABLE(&DMA_HandleStruct_UEV);
    __HAL_DMA_DISABLE(&DMA_HandleStruct_CC1);
    __HAL_DMA_DISABLE(&DMA_HandleStruct_CC2);

    // IMPORTANT: disable the DMA requests, too!
    __HAL_TIM_DISABLE_DMA(&TIM2_Handle, TIM_DMA_UPDATE);
    __HAL_TIM_DISABLE_DMA(&TIM2_Handle, TIM_DMA_CC1);
    __HAL_TIM_DISABLE_DMA(&TIM2_Handle, TIM_DMA_CC2);

    // disable the capture compare events
    TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_1, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_2, TIM_CCx_DISABLE);

    // enable TIM2 Update interrupt to append min. 50us dead/reset period
    TIM2->ARR = 1500u; // 1 tick = 41.67ns => 1500 ticks = ~60us
    TIM2->CNT = 0u;
    __HAL_TIM_ENABLE_IT(&TIM2_Handle, TIM_IT_UPDATE);
}

// ----------------------------------------------------------------------------
/// \brief     DMA 传输错误回调。
///
/// \param      [in]    pointer to a DMA_HandleTypeDef structure that contains
///                     the configuration information for the specified DMA Stream.
///
/// \return     none
static void TransferError(DMA_HandleTypeDef *DmaHandle)
{
    while (1)
    {
        // do nothing stay here
        printf("DMA Error\r\n");
    }
}

// ----------------------------------------------------------------------------
/// \brief      清除 ws2812b 缓冲区。发送后所有像素均为黑色。
///
/// \param      none
///
/// \f
void WS2812B_clearBuffer(void)
{
    // wait until last buffer transmission has been completed
    while (WS2812_State != WS2812B_READY)
        ;

    // clear frame buffer
    for (uint8_t y = 0; y < ROW; y++)
    {
        for (uint16_t x = 0; x < COL; x++)
        {
            WS2812B_setPixel(y, x, 0x00, 0x00, 0x00);
        }
    }
}

// ----------------------------------------------------------------------------
/// \brief      此函数指定清除某一行的缓冲区
///
/// \param      [in]    uint8_t row
///
void WS2812B_setclearBuffer(uint8_t row)
{
    // wait until last buffer transmission has been completed
    while (WS2812_State != WS2812B_READY)
        ;

    // 清除某一行的缓冲区
    for (uint16_t x = 0; x < COL; x++)
    {
        WS2812B_setPixel(row, COL, 0x00, 0x00, 0x00);
    }
}

// ----------------------------------------------------------------------------
/// \brief      此函数设置单个像素的颜色。
///
/// \param      [in]    uint8_t row
/// \param      [in]    uint16_t col
/// \param      [in]    uint8_t red
/// \param      [in]    uint8_t green
/// \param      [in]    uint8_t blue
///
/// \return     none
void WS2812B_setPixel(uint8_t row, uint16_t col, uint8_t red, uint8_t green, uint8_t blue)
{
    // check if the col and row are valid
    if (row >= ROW || col >= COL)
    {
        return;
    }

    // wait until last buffer transmission has been completed
    while (WS2812_State != WS2812B_READY)
        ;

    // write pixel into the buffer
    for (uint8_t i = 0; i < 8; i++)
    {
        /* clear the data for pixel */
        WS2812_Buffer[((col * 24) + i)] &= ~(0x01 << row);
        WS2812_Buffer[((col * 24) + 8 + i)] &= ~(0x01 << row);
        WS2812_Buffer[((col * 24) + 16 + i)] &= ~(0x01 << row);

        /* write new data for pixel */
        WS2812_Buffer[((col * 24) + i)] |= ((((green << i) & 0x80) >> 7) << row);
        WS2812_Buffer[((col * 24) + 8 + i)] |= ((((red << i) & 0x80) >> 7) << row);
        WS2812_Buffer[((col * 24) + 16 + i)] |= ((((blue << i) & 0x80) >> 7) << row);
    }
}

// ----------------------------------------------------------------------------
/// \brief      此函数用来显示设置好的动画
///
/// \param      [in]    uint8_t animation
///
/// \return     none
void WS2812B_setAnimation(uint8_t animation)
{
    switch (animation)
    {
    case WS2812B_Animation_RunningWaterLamps:

        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < COL; j++)
            {
                WS2812B_clearBuffer();
                WS2812B_setPixel(1, j++, rand() & 0xFF, rand() % 0xFF, rand() % 0xFF);
                WS2812B_sendBuffer();
                HAL_Delay(20);
            }
        }
        break;
    case WS2812B_Animation_Init:
        uint8_t a = 0, BigLight_Index = 1;
        uint8_t DrivingLight_Index = 1;
        // 这里是启动时的灯光效果
        /* 两个顺时针彩灯转圈  行车灯流水灯效果  */
        /* 转圈 5 圈 */
        for (a; a < 5; a++)
        {
            for (BigLight_Index;
                 BigLight_Index < COL || DrivingLight_Index < COL;)
            {
                BigLight_Index++;
                DrivingLight_Index++;

                // WS2812B_setclearBuffer(0);
                // WS2812B_setclearBuffer(1);
                WS2812B_clearBuffer();
                // 左侧设置
                WS2812B_setPixel(0, BigLight_Index, rand() % 0xFF, rand() % 0xFF, rand() % 0xFF);
                // 右侧设置
                WS2812B_setPixel(1, BigLight_Index, rand() % 0xFF, rand() % 0xFF, rand() % 0xFF);

                //                // 行车灯的流水灯
                //                WS2812B_setclearBuffer(2);
                WS2812B_setPixel(2, DrivingLight_Index, 0xff, 0x00, 0x00);

                // 雾灯
                //WS2812B_setPixel(FogLight,0,0xff,0xff,0xff);
                //WS2812B_setPixel(FogLight,1,0x00,0x00,0x00);
                WS2812B_setPixel(FogLight, 9, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 2, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 3, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 4, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 5, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 6, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 7, 0xff, 165, 0);
                WS2812B_setPixel(FogLight, 8, 0xff, 165, 0);

                WS2812B_sendBuffer();

                HAL_Delay(200);
            }
            BigLight_Index = 1;
            DrivingLight_Index = 1;
        }
        WS2812B_clearBuffer();
        WS2812B_sendBuffer(); // 发送

    default:
        break;
    }
}