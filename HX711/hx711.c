#include "hx711.h"
#include "hx711_config.h"
#include <stdio.h>
#include "UIManager.h"

#if (_HX711_USE_FREERTOS == 1)
#include "cmsis_os.h"
#define hx711_delay(x)    osDelay(x)
#else
#define hx711_delay(x)    HAL_Delay(x)
#endif


CalibrationState g_hx711_calibration_state;

//#############################################################################################
/**
 * @brief 延迟微秒级函数
 *
 * 此函数用于实现微秒级别的延迟，通过循环计数实现。
 */
void hx711_delay_us(void)
{
  uint32_t delay = _HX711_DELAY_US_LOOP;
  while (delay > 0)
  {
    delay--;
    __nop(); __nop(); __nop(); __nop();    
  }
}

//#############################################################################################
/**
 * @brief 锁定HX711设备
 *
 * 此函数用于锁定HX711设备，以确保在数据采集期间不会被其他操作干扰。
 *
 * @param hx711 指向HX711设备的指针
 */
void hx711_lock(hx711_t *hx711)
{
  // 等待直到设备解锁
  while (hx711->lock)
    hx711_delay(1);
  // 锁定设备
  hx711->lock = 1;      
}

//#############################################################################################
/**
 * @brief 解锁HX711设备
 *
 * 此函数用于解锁HX711设备，允许其他操作进行数据采集。
 *
 * @param hx711 指向HX711设备的指针
 */
void hx711_unlock(hx711_t *hx711)
{
  // 解锁设备
  hx711->lock = 0;
}

//#############################################################################################
/**
 * @brief 初始化HX711设备
 *
 * 此函数用于初始化HX711设备，配置GPIO引脚并设置初始状态。
 *
 * @param hx711 指向HX711设备的指针
 * @param clk_gpio 时钟引脚的GPIO端口
 * @param clk_pin 时钟引脚编号
 * @param dat_gpio 数据引脚的GPIO端口
 * @param dat_pin 数据引脚编号
 */
void hx711_init(hx711_t *hx711, GPIO_TypeDef *clk_gpio, uint16_t clk_pin, GPIO_TypeDef *dat_gpio, uint16_t dat_pin)
{
  // 锁定HX711设备以确保初始化期间不会被其他操作干扰
  hx711_lock(hx711);

  // 配置HX711设备的GPIO引脚
  hx711->clk_gpio = clk_gpio;
  hx711->clk_pin = clk_pin;
  hx711->dat_gpio = dat_gpio;
  hx711->dat_pin = dat_pin;

  GPIO_InitTypeDef gpio = {0};
  
  // 配置时钟引脚为输出模式
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pin = clk_pin;
  HAL_GPIO_Init(clk_gpio, &gpio);

  // 配置数据引脚为输入模式
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pin = dat_pin;
  HAL_GPIO_Init(dat_gpio, &gpio);

  // 初始化时钟引脚为高电平，然后低电平
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);
  hx711_delay(10);
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
  hx711_delay(10);  

  // 获取两次测量值以确保初始化完成
  hx711_value(hx711);
  hx711_value(hx711);

  // 解锁HX711设备
  hx711_unlock(hx711); 
}

//#############################################################################################
/**
 * @brief 从HX711传感器获取测量值
 *
 * 此函数与HX711传感器通信，以读取24位的数据值。该传感器用于重量测量。
 *
 * @param hx711 指向HX711设备结构的指针，包含GPIO配置和引脚编号
 * @return int32_t 返回从HX711传感器读取的24位数据值；如果读取失败，返回0
 */
int32_t hx711_value(hx711_t *hx711)
{
  uint32_t data = 0;
  uint32_t startTime = HAL_GetTick();

  // 等待数据准备好，最长等待150ms
  while(HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) == GPIO_PIN_SET)
  {
    hx711_delay(1);
    if(HAL_GetTick() - startTime > 150)
      return 0;
  }

  // 读取24位数据
  for(int8_t i=0; i<24 ; i++)
  {
    // 拉高时钟线，准备读取下一位数据
    HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);   
    hx711_delay_us();
    // 拉低时钟线，完成读取位的脉冲
    HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
    hx711_delay_us();
    // 将读取的数据位添加到数据变量中
    data = data << 1;    
    if(HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) == GPIO_PIN_SET)
      data++;
  }

  // 对数据进行修正，以符合HX711的数据格式
  data = data ^ 0x800000; 

  // 发送最后一个时钟脉冲以完成数据读取序列
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);   
  hx711_delay_us();
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
  hx711_delay_us();

  return data;    
}

//#############################################################################################
/**
 * @brief 获取多次采样的平均测量值
 *
 * 此函数通过多次调用`hx711_value`函数来获取多个样本，并计算这些样本的平均值。
 *
 * @param hx711 指向HX711设备的指针
 * @param sample 用于计算的样本数量
 * @return int32_t 返回多次采样的平均值
 */
int32_t hx711_value_ave(hx711_t *hx711, uint16_t sample)
{
  // 锁定HX711设备以确保数据采集期间不会被其他操作干扰
  hx711_lock(hx711);

  // 初始化累加器以计算多次测量的平均值
  int64_t ave = 0;
  
  // 循环多次采样，以提高测量的准确性和稳定性
  for(uint16_t i=0 ; i<sample ; i++)
  {
    ave += hx711_value(hx711);
    hx711_delay(5);
  }

  // 计算多次测量的平均值，并将结果转换为32位整数
  int32_t answer = (int32_t)(ave / sample);

  // 解锁HX711设备，允许其他操作进行数据采集
  hx711_unlock(hx711);

  return answer;
}

//#############################################################################################
/**
 * @brief 归零（去皮）
 *
 * 此函数将当前测量值设为零点，以便后续测量从零开始。
 *
 * @param hx711 指向HX711设备的指针
 * @param sample 用于计算的样本数量
 */
void hx711_tare(hx711_t *hx711, uint16_t sample)
{
  // 锁定HX711设备以确保数据采集期间不会被其他操作干扰
  hx711_lock(hx711);

  // 初始化累加器以计算多次测量的平均值
  int64_t ave = 0;

  // 循环多次采样，以提高测量的准确性和稳定性
  for(uint16_t i=0 ; i<sample ; i++)
  {
    ave += hx711_value(hx711);
    hx711_delay(5);
  }

  // 设置偏移量为多次测量的平均值
  hx711->offset = (int32_t)(ave / sample);

  // 解锁HX711设备，允许其他操作进行数据采集
  hx711_unlock(hx711);
}

//#############################################################################################
/**
 * @brief 校准传感器
 *
 * 此函数对传感器进行校准，以确保测量结果的准确性。
 *
 * @param hx711 指向HX711设备的指针
 * @param noload_raw 无负载时的原始测量值
 * @param load_raw 加载时的原始测量值
 * @param scale 已知加载的重量值
 */
void hx711_calibration(hx711_t *hx711, int32_t noload_raw, int32_t load_raw, float scale)
{
  // 锁定HX711设备以确保数据采集期间不会被其他操作干扰
  hx711_lock(hx711);

  // 设置偏移量为无负载时的原始测量值
  hx711->offset = noload_raw;

  // 计算并设置转换系数
  hx711->coef = (load_raw - noload_raw) / scale;  

  // 解锁HX711设备，允许其他操作进行数据采集
  hx711_unlock(hx711);
}

//#############################################################################################
/**
 * @brief 计算重量
 *
 * 此函数通过多次采样计算重量，以提高测量的准确性和稳定性。它首先锁定HX711设备以确保数据的一致性，
 * 然后通过多次调用`hx711_value`函数来收集样本，计算这些样本的平均值，从而得到更准确的测量结果。
 * 最后，通过减去偏移量并除以系数，将原始测量值转换为重量值。
 *
 * @param hx711 指向HX711设备的指针，该设备用于重量测量
 * @param sample 用于计算的样本数量，更多的样本可以提高准确性，但会增加测量时间
 * @return 返回计算出的重量值，单位取决于应用设置
 */
float hx711_weight(hx711_t *hx711, uint16_t sample)
{
  // 锁定HX711设备以确保数据采集期间不会被其他操作干扰
  hx711_lock(hx711);

  // 初始化累加器以计算多次测量的平均值
  int64_t ave = 0;

  // 循环多次采样，以提高测量的准确性和稳定性
  for(uint16_t i=0 ; i<sample ; i++)
  {
    // 将每次测量的结果累加到ave变量中
    ave += hx711_value(hx711);
    // 延迟5毫秒，以确保每次测量之间有足够的间隔，避免干扰
    hx711_delay(5);
  }

  // 计算多次测量的平均值，并将结果转换为32位整数
  int32_t data = (int32_t)(ave / sample);

  // 将原始测量值转换为重量值，通过减去偏移量并除以系数完成
  float answer = (data - hx711->offset) / hx711->coef;

  // 解锁HX711设备，允许其他操作进行数据采集
  hx711_unlock(hx711);

  // 返回计算出的重量值
  return answer;
}

//#############################################################################################
/**
 * @brief 设置转换系数
 *
 * 此函数用于设置HX711设备的转换系数。该系数用于将原始测量值转换为实际重量值。
 * 通过调整此系数，可以校准传感器以获得更准确的测量结果。
 *
 * @param hx711 指向HX711设备的指针
 * @param coef 新的转换系数值
 */
void hx711_coef_set(hx711_t *hx711, float coef)
{
  // 更新HX711设备的转换系数
  hx711->coef = coef;  
}

//#############################################################################################
/**
 * @brief 获取转换系数
 *
 * 此函数用于获取当前的转换系数。
 *
 * @param hx711 指向HX711设备的指针
 * @return 当前的转换系数值
 */
float hx711_coef_get(hx711_t *hx711)
{
  return hx711->coef;  
}

//#############################################################################################
/**
 * @brief 关闭HX711传感器电源
 *
 * 此函数用于关闭HX711传感器的电源，以节省功耗。
 *
 * @param hx711 指向HX711设备的指针
 */
void hx711_power_down(hx711_t *hx711)
{
  // 关闭电源
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);
  hx711_delay(1);  
}

//#############################################################################################
/**
 * @brief 打开HX711传感器电源
 *
 * 此函数用于打开HX711传感器的电源。
 *
 * @param hx711 指向HX711设备的指针
 */
void hx711_power_up(hx711_t *hx711)
{
  // 打开电源
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
}




/**
 * @brief 对 HX711 进行完整校准
 *
 * 该函数集成了去皮和校准功能，用户只需提供一个已知重量值，函数会自动完成偏移量设置和校准系数的计算。
 *
 * @param hx711 指向 HX711 设备的指针
 * @param known_weight 校准用的已知重量（单位：g）
 * @param sample 采样次数，用于去皮和校准
 * @return HAL_StatusTypeDef 返回 HAL_OK 表示校准成功，返回 HAL_ERROR 表示校准失败
 */
HAL_StatusTypeDef hx711_calibrate(hx711_t *hx711, float known_weight, uint16_t sample)
{
    g_hx711_calibration_state = STATE_TARE;      // 当前状态
    int32_t no_load_raw = 0;      // 空载值
    int32_t load_raw = 0;         // 加载值
    


    while (1)
    {
        switch (g_hx711_calibration_state)
        {
            case STATE_TARE:
                // printf("Starting tare operation...\n");
                hx711_tare(hx711, sample);
                no_load_raw = hx711->offset;
                // printf("Tare completed. Offset: %ld\n", no_load_raw);
                g_hx711_calibration_state = STATE_WAIT_FOR_WEIGHT;
                UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                HAL_Delay(500);
                break;
                
            case STATE_WAIT_FOR_WEIGHT:
                // printf("Place a known weight of %.2f g on the scale.\n", known_weight);
                HAL_Delay(5000);  // 等待 5 秒钟，让用户放置砝码
                g_hx711_calibration_state = STATE_READ_LOAD;
                UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                HAL_Delay(500);
                break;
            case STATE_READ_LOAD:
                load_raw = hx711_value_ave(hx711, sample);
                if (load_raw == 0)
                {
                    // printf("Error: Unable to read load value. Calibration failed.\n");
                    g_hx711_calibration_state = STATE_ERROR;
                    UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                    HAL_Delay(500);
                }
                else
                {
                    // printf("Load value: %ld\n", load_raw);

                    g_hx711_calibration_state = STATE_CHECK_VALUE;
                    UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                    HAL_Delay(500);
                }

                break;
                

            case STATE_CHECK_VALUE:
                if (load_raw == no_load_raw)
                {
                    // printf("Error: No difference between load and no-load values. Calibration failed.\n");
                    g_hx711_calibration_state = STATE_ERROR;
                    UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                    HAL_Delay(500);
                }
                else
                {
                    g_hx711_calibration_state = STATE_CALCULATE;
                    UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                    HAL_Delay(500);
                }
                break;
                


            case STATE_CALCULATE:
                hx711_calibration(hx711, no_load_raw, load_raw, known_weight);
                // printf("Calibration completed. Coefficient: %.6f\n", hx711->coef);
                g_hx711_calibration_state = STATE_DONE;
                UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                HAL_Delay(500);
                break;
                


            case STATE_DONE:
                UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                HAL_Delay(500);
                return HAL_OK;
                


            case STATE_ERROR:
                UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                HAL_Delay(500);
                return HAL_ERROR;
                


            default:
                UIManager_SwitchScreen(SCREEN_HX711_CALIBRATION);
                HAL_Delay(500);
                return HAL_ERROR;

        }
    }

}
