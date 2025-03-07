#include "stm32f10x.h"
#include "led.h"
#include "key.h"
#include "usart1.h"
#include "delay.h"
#include "DS1302.h"

#define AHB_INPUT 72

unsigned char uart1_buf[11]={0};
volatile u32 time; // ms 计时变量

u16    t=0,mode=0,flag=0;
u8 label2[16]; //时间存放数组显示格式
u8 Settime[6]={0x20,0x09,0x16,0x20,0x59,0x50};  // 初始时间设置: 年 月 日  时 分 秒

int main(void)
{
    int tt = 0;

    SystemInit();    // 配置系统时钟为72M
    delay_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);            //配置完中断向量表以后才能初始化其他东西
    uart_init(9600);

    TIM2_NVIC_Configuration();                                                     // TIM2 定时配置
    TIM2_Configuration();
    START_TIME;                                                                                     // TIM2 开始计时

    DS1302_init(Settime);                                                                //时间初始化
  DS1302_SetTime(Settime);                                                        //设置初始时间


    while(1)
  {
        tt++;
        if(tt>=3200000)
        {
                tt = 0;
            DS1302_Readtime();
        printf("%lld,",num);
                printf("20");
                printf("%d",label2[0]);
                printf("%d",label2[1]);
                printf("%c",label2[2]);
                printf("%d",label2[3]);
                printf("%d",label2[4]);
                printf("%c",label2[5]);
                printf("%d",label2[6]);
                printf("%d, ",label2[7]);

                printf("%d",label2[8]);
                printf("%d",label2[9]);
                printf("%c",label2[10]);
                printf("%d",label2[11]);
                printf("%d",label2[12]);
                printf("%c",label2[13]);
                printf("%d",label2[14]);
                printf("%d, ",label2[15]);
                printf("%.2f, %.2f ",PM25,PM10);
                printf("%c",0x0d);
                printf("%c",0x0a);
        }

  }
}
