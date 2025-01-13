/* * * * * * * *
 * DWT_us.c 文件是实现了 stm32f103c8t6 的 us 延时
 * 比 HAL 库提供的 ms 延时更加短 适合一些 单总线通信的一些较短的延时场景，
 *
 * 作 者：王永成
 * 日 期：2024.12.12
 * 邮 箱：2042789231@qq.com
 */




// DWT 初始化函数
void DWT_Delay_Init(void)
{
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 启用 DWT
    DWT->CYCCNT = 0;                                // 清零计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 启用计数器
  }
}

// 微秒延时函数
void DWT_Delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;                                    // 获取当前计数器值
  uint32_t target_cycles = us * (HAL_RCC_GetHCLKFreq() / 1000000); // 目标周期数
  while ((DWT->CYCCNT - start) < target_cycles)
    ; // 等待目标时间
}
