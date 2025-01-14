#ifndef HX711_H_
#define HX711_H_

/*
  Author:     Nima Askari
  WebSite:    http://www.github.com/NimaLTD
  Instagram:  http://instagram.com/github.NimaLTD
  Youtube:    https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw

  Version:    1.1.1


  Reversion History:

  (1.1.1):
    Add power down/up.
  (1.1.0):
    Add structure, Add calibration, Add weight, change names, ...
  (1.0.0):
    First Release.
*/

/**
 * HX711 库使用方法
 *
 * 1. 引入头文件
 *    在需要使用HX711传感器的C文件中，引入头文件：
 *    ```c
 *    #include "hx711.h"
 *    ```
 *
 * 2. 初始化HX711设备
 *    初始化HX711传感器，配置GPIO引脚：
 *    ```c
 *    hx711_t hx711_instance;
 *    hx711_init(&hx711_instance, GPIOA, GPIO_PIN_0, GPIOB, GPIO_PIN_1);
 *    ```
 *
 * 3. 获取传感器测量值
 *    获取单次测量值或多次采样的平均值：
 *    ```c
 *    int32_t single_value = hx711_value(&hx711_instance);
 *    int32_t average_value = hx711_value_ave(&hx711_instance, 10); // 采样10次
 *    ```
 *
 * 4. 设置和获取转换系数
 *    设置和获取转换系数：
 *    ```c
 *    hx711_coef_set(&hx711_instance, 500.0f); // 设置转换系数
 *    float current_coef = hx711_coef_get(&hx711_instance); // 获取转换系数
 *    ```
 *
 * 5. 校准传感器
 *    对传感器进行校准：
 *    ```c
 *    hx711_calibration(&hx711_instance, noload_raw, load_raw, scale);
 *    ```
 *
 * 6. 归零（去皮）
 *    将当前测量值设为零点：
 *    ```c
 *    hx711_tare(&hx711_instance, 10); // 采样10次归零
 *    ```
 *
 * 7. 计算重量
 *    根据偏移量和转换系数计算重量：
 *    ```c
 *    float weight = hx711_weight(&hx711_instance, 10); // 采样10次计算重量
 *    ```
 *
 * 8. 电源管理
 *    控制HX711传感器的电源状态：
 *    ```c
 *    hx711_power_down(&hx711_instance); // 关闭电源
 *    hx711_power_up(&hx711_instance);   // 打开电源
 *    ```
 */

// #include "hx711.h"

// // 定义 HX711 实例
// hx711_t hx711;

// int main(void)
// {
//     HAL_Init();
//     SystemClock_Config();
//     MX_GPIO_Init();

//     // 初始化 HX711
//     hx711_init(&hx711, GPIOB, GPIO_PIN_0, GPIOB, GPIO_PIN_1);

//     // 去皮操作
//     HX711_Tare(&hx711);

//     // 校准操作
//     float known_weight = 100.0;  // 设定已知重量为 100g
//     HX711_Calibrate(&hx711, known_weight);

//     // 主循环
//     while (1)
//     {
//         // 测量重量
//         float weight = hx711_weight(&hx711, 10);
//         printf("Measured weight: %.2f g\n", weight);

//         HAL_Delay(1000);  // 每秒读取一次
//     }
// }

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

  // ####################################################################################################################

  /**
   * HX711传感器结构体
   *
   * 用于定义HX711传感器的相关参数和状态
   */
  typedef struct
  {
    GPIO_TypeDef *clk_gpio; // 时钟引脚的GPIO
    GPIO_TypeDef *dat_gpio; // 数据引脚的GPIO
    uint16_t clk_pin;       // 时钟引脚编号
    uint16_t dat_pin;       // 数据引脚编号
    int32_t offset;         // 偏移量
    float coef;             // 系数
    uint8_t lock;           // 锁定状态

  } hx711_t;

  // ####################################################################################################################
  void hx711_init(hx711_t *hx711, GPIO_TypeDef *clk_gpio, uint16_t clk_pin, GPIO_TypeDef *dat_gpio, uint16_t dat_pin); // 初始化HX711传感器
  int32_t hx711_value(hx711_t *hx711);                                                                                 // 获取HX711传感器的测量值
  int32_t hx711_value_ave(hx711_t *hx711, uint16_t sample);                                                            // 获取HX711传感器的平均测量值

  void hx711_coef_set(hx711_t *hx711, float coef);                                               // 设置HX711传感器的系数
  float hx711_coef_get(hx711_t *hx711);                                                          // 获取HX711传感器的系数
  void hx711_calibration(hx711_t *hx711, int32_t value_noload, int32_t value_load, float scale); // 校准HX711传感器
  void hx711_tare(hx711_t *hx711, uint16_t sample);                                              // 去皮操作，设置偏移量
  float hx711_weight(hx711_t *hx711, uint16_t sample);                                           // 计算并返回重量值
  void hx711_power_down(hx711_t *hx711);                                                         // 关闭HX711传感器电源
  void hx711_power_up(hx711_t *hx711);
  HAL_StatusTypeDef hx711_calibrate(hx711_t *hx711, float known_weight, uint16_t sample);       // 校准HX711传感器
  // ####################################################################################################################

#ifdef __cplusplus
}
#endif

#endif