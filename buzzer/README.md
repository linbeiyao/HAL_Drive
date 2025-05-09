# 介绍

一个微小而简单的蜂鸣器库，实现了简单的打开，关闭，以及播放循环和铃声的能力。
所有功能都以非阻滞模式起作用，因此，您的应用程序在处理设备时永远不会停止执行，并且包含在实施设备时`callback`当操作结束时。

支持任何微控制器，被动或主动蜂鸣器。
### 要求

-GPIO（活动）或PWM（被动）输出；
 -任务或计时器中断；
＃＃＃ 特征
-便于使用;
 -手动开始和停止；
 -开始定义的时间；
 -固定时期的“眨眼”；
 -玩铃声；
 -非阻滞功能；
 -回调告诉您操作完成。
＃ 如何使用
首先，buzzer_t至少使用 或声明你的pwmOut函数gpioOut（只需选择一个！另一个函数必须是NULL）。关于函数：

void pwmOut(uint32_t freq)：用于被动蜂鸣器，此函数void仅返回和接收freq参数。当实现频率为 0 时，设备必须关闭 PWM。
void gpioOut(uint32_t val)：用于激活蜂鸣器，val当 为 时需要开启蜂鸣器1，当 为 时0关闭蜂鸣器。此类蜂鸣器无法设置频率值。
该库将通过检查定义的 fxn 自动识别 上的蜂鸣器类型。如果用户想要使用和，buzzer_init()则需要实现的另一个参数是，它仅指示在计时器/任务例程中调用 的频率（以毫秒为单位）。buzzer_startbuzzer_start_arrayinterruptMsbuzzer_interrupt

此参数用于计时蜂鸣器，打开或关闭蜂鸣器，用于循环、铃声等。
##为被动蜂鸣器配置

在此示例中，Bellow在通用芯片组中配置了一个蜂鸣器，其中有50ms的计时器中断和PWM函数。
```C
void __pwm_buzzer_chipset(uint32_t freq);
// 蜂鸣器处理程序
buzzer_t Buzzer = {  
  .pwmOut = __pwm_chipset,  
  .interruptMs = 50
}

// 功能
void __pwm_buzzer_chipset(uint32_t freq){
  if (freq == 0){    
    chipset_pwm_turnoff();  
  }  
  else{    
    chipset_pwm_turnon();    
    chipset_pwm_set_freq(freq);  
  }
}
  
// 中断
void __tim_interrupt_50ms(){  
  buzzer_interrupt(&Buzzer);
}

// Main
void main(){
  buzzer_init(&Buzzer);
}
```

## 为活动蜂鸣器配置

在此示例中，将配置和实现一个活动蜂鸣器，并使用100ms的计时器中断和GPIO来控制蜂鸣器。
```C
void __gpio_pwm_chipset(uint32_t val);
// Buzzer Handler
buzzer_t Buzzer = {  
  .pwmOut = __pwm_chipset,  
  .interruptMs = 100
}
  
// 功能
void __gpio_pwm_chipset(uint32_t val){  
  chipset_gpio(BUZZER_GPIO_Port, BUZZER_Pin, val);
}

// 中断
void __tim_interrupt_100ms(){  
  buzzer_interrupt(&Buzzer);
}

// Main
void main(){  
  buzzer_init(&Buzzer);
}
```

# Examples

对于示例，请考虑一个被动蜂鸣器，配置了一个计时器和所有内容：)对于活动蜂鸣器，所有FREQ参数都可以为`0`或`null''。
## 打开蜂鸣器并手动关闭

```C
void main(){  
  ...  
  // turn on the buzzer with a 1500Hz frequency  
  buzzer_turn_on(&Buzzer, 1500);    
  
  // delay for 500ms  
  chipset_delay_ms(500);    
  
  // turnoff buzzer  
  buzzer_stop(&Buzzer);
}
```

## Turnon蜂鸣器500ms

```C
void main(){  
  ...  
  // turnon buzzer with a 1500Hz frequency for 500ms  
  buzzer_start(&Buzzer, 1500, 500, BUZZER_LOOP_OFF);
}
```

## "Blink" 蜂鸣器为500ms

```C
void main(){  
  ...  
  // 用2500Hz频率打开蜂鸣器 
  buzzer_start(&Buzzer, 2500, 500, BUZZER_LOOP_ON);    
  
  // buzzer_loop_on参数表明蜂鸣器将以500ms为单位
  // 同一时期的转换，重复该过程
}
```

## 播放超级马里奥铃声，结束后打开LED

```C
// 当Buzzer_start或Buzzer_start_array完成时，调用此回调
// 征求行动。在这种情况下，当马里奥铃声是
// 完成，回调将被调用
void buzzer_end_callback(buzzer_t *buzzer){  
  led_red(TRUE);
}

void main(){  
  ...  
  // 播放马里奥主题铃声，其中包括在图书馆中：D  
  buzzer_start_array(&Buzzer, mario_theme_time, mario_theme_melody, mario_theme_len);
}
```

# Doubts

任何疑问或问题，都会发布问题。我们也有一个在STM32F411（黑色药丸）上实现的示例。
最好的问候。
