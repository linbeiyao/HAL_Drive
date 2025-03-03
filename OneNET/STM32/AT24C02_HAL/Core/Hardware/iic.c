#include "main.h"
#include "iic.h"


/*微秒级延时函数*/
static void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt = 0;
	uint32_t reload = SysTick->LOAD;			//LOAD的值
	ticks = nus * 72; 						//需要的节拍数
	told = SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told) tcnt += told - tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt += reload - tnow + told;
			told = tnow;
			if(tcnt >= ticks) break;			//时间超过/等于要延迟的时间,则退出.
		}
	}
}




/**
 * @brief I2C初始化
 * @param  无
 * @return 无
 */
void I2C_Init(void)
{
	 GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, I2C1_SCL_Pin|I2C1_SDA_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = I2C1_SCL_Pin|I2C1_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief 设置SDA为输入模式
 * @param  无
 * @return 无
 */
static void Set_I2C_SDA_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = I2C1_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief 设置SDA为输出模式
 * @param  无
 * @return 无
 */
static void Set_I2C_SDA_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = I2C1_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief 设置SCL高低电平
 * @param  BitValue 1：高电平，0：低电平
 * @return 无
 */
static void I2C_SCL(uint8_t BitValue)
{
	HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, (GPIO_PinState)BitValue);
}

/**
 * @brief 设置SDA高低电平
 * @param  BitValue 1：高电平，0：低电平
 * @return 无
 */
static void I2C_SDA(uint8_t BitValue)
{
	HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, (GPIO_PinState)BitValue);
}

/**
 * @brief I2C起始信号
 * @param  无
 * @return 无
 */
void I2C_Start(void)
{
	Set_I2C_SDA_Output();	// 设置输出模式
	/* SCL为高电平时，SDA由高电平1--->低电平0 */
	I2C_SDA(1);
	I2C_SCL(1);
	delay_us(4);
	I2C_SDA(0);		
	delay_us(4);
	I2C_SCL(0);		// 主机释放I2C总线
}

/**
 * @brief I2C停止信号
 * @param  无
 * @return 无
 */
void I2C_Stop(void)
{
	Set_I2C_SDA_Output();	// 设置输出模式
	/* SCL为高电平时，SDA由低电平0--->高电平1 */
	I2C_SCL(0);
	I2C_SDA(0);
	delay_us(4);
	I2C_SCL(1);
	I2C_SDA(1);		
	delay_us(4);
}

/**
 * @brief I2C等待应答信号
 * @param  无
 * @return 0：应答，1：非应答
 */
uint8_t I2C_Wait_Ack(void)
{
	uint8_t waitTime;
	//Set_I2C_SDA_Output();	// 设置输出模式
	Set_I2C_SDA_Input();	// 设置输入模式
	I2C_SDA(1);
	delay_us(1);	
	I2C_SCL(1);
	delay_us(1);
	while(HAL_GPIO_ReadPin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin))
	{
		waitTime++;
		if(waitTime > 200)
		{
			I2C_Stop();
			return 1;
		}
	}
	I2C_SCL(0);
	return 0;
}

/**
 * @brief I2C产生ACK应答
 * @param  无
 * @return 无
 */
void I2C_Ack(void)
{
	I2C_SCL(0);
	Set_I2C_SDA_Output();	// 设置输出模式
	I2C_SDA(0);
	delay_us(2);
	I2C_SCL(1);
	delay_us(2);
	I2C_SCL(0);	
}

/**
 * @brief I2C不产生ACK应答
 * @param  无
 * @return 无
 */
void I2C_NAck(void)
{
	I2C_SCL(0);
	Set_I2C_SDA_Output();	// 设置输出模式
	I2C_SDA(1);
	delay_us(2);
	I2C_SCL(1);
	delay_us(2);
	I2C_SCL(0);	
}

/**
 * @brief I2C发送一个字节
 * @param  无
 * @return 无
 */
void I2C_Send_Byte(uint8_t sendByte)
{
	uint8_t i;
	Set_I2C_SDA_Output();	// 设置输出模式
	I2C_SCL(0);		// 拉低时钟开始数据传输
	for(i = 0; i < 8; i++)
	{
//		// 从高位一次发
//		if(sendByte & 0x80)
//			I2C_SDA(1);
//		else
//			I2C_SDA(0);
		I2C_SDA((sendByte & 0x80) >> 7);
		delay_us(2);
		I2C_SCL(1);		
		delay_us(2);
		I2C_SCL(0);		
		sendByte <<= 1; // 依次取最高位
	}
}

/**
 * @brief I2C接收一个字节
 * @param  ack=1：发送ACK，ack=0：发送NACK 
 * @return 接收的数据
 */
uint8_t I2C_Read_Byte(uint8_t ack)
{
	uint8_t i;
	uint8_t recvByte = 0;
	Set_I2C_SDA_Input();	// 设置输入模式
	for(i = 0; i < 8; i++)
	{
		I2C_SCL(0);		// 时钟线拉低，告诉从机，主机需要数据
		delay_us(2);	// 电平保持时间，等待从机发送数据
		I2C_SCL(1);		// 时钟线拉高，告诉从机，主机正在读取数据
		recvByte <<= 1;
		if(HAL_GPIO_ReadPin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin))
			recvByte |= 0x01;
		delay_us(1);
	}
	if(!ack)
		I2C_NAck();
	else
		I2C_Ack();
	//printf("recvByte:%d\n", recvByte);
	return recvByte;
}

