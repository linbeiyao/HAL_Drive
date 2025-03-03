#include "main.h"
#include "iic.h"


/*΢�뼶��ʱ����*/
static void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt = 0;
	uint32_t reload = SysTick->LOAD;			//LOAD��ֵ
	ticks = nus * 72; 						//��Ҫ�Ľ�����
	told = SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told) tcnt += told - tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt += reload - tnow + told;
			told = tnow;
			if(tcnt >= ticks) break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}
	}
}




/**
 * @brief I2C��ʼ��
 * @param  ��
 * @return ��
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
 * @brief ����SDAΪ����ģʽ
 * @param  ��
 * @return ��
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
 * @brief ����SDAΪ���ģʽ
 * @param  ��
 * @return ��
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
 * @brief ����SCL�ߵ͵�ƽ
 * @param  BitValue 1���ߵ�ƽ��0���͵�ƽ
 * @return ��
 */
static void I2C_SCL(uint8_t BitValue)
{
	HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, (GPIO_PinState)BitValue);
}

/**
 * @brief ����SDA�ߵ͵�ƽ
 * @param  BitValue 1���ߵ�ƽ��0���͵�ƽ
 * @return ��
 */
static void I2C_SDA(uint8_t BitValue)
{
	HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, (GPIO_PinState)BitValue);
}

/**
 * @brief I2C��ʼ�ź�
 * @param  ��
 * @return ��
 */
void I2C_Start(void)
{
	Set_I2C_SDA_Output();	// �������ģʽ
	/* SCLΪ�ߵ�ƽʱ��SDA�ɸߵ�ƽ1--->�͵�ƽ0 */
	I2C_SDA(1);
	I2C_SCL(1);
	delay_us(4);
	I2C_SDA(0);		
	delay_us(4);
	I2C_SCL(0);		// �����ͷ�I2C����
}

/**
 * @brief I2Cֹͣ�ź�
 * @param  ��
 * @return ��
 */
void I2C_Stop(void)
{
	Set_I2C_SDA_Output();	// �������ģʽ
	/* SCLΪ�ߵ�ƽʱ��SDA�ɵ͵�ƽ0--->�ߵ�ƽ1 */
	I2C_SCL(0);
	I2C_SDA(0);
	delay_us(4);
	I2C_SCL(1);
	I2C_SDA(1);		
	delay_us(4);
}

/**
 * @brief I2C�ȴ�Ӧ���ź�
 * @param  ��
 * @return 0��Ӧ��1����Ӧ��
 */
uint8_t I2C_Wait_Ack(void)
{
	uint8_t waitTime;
	//Set_I2C_SDA_Output();	// �������ģʽ
	Set_I2C_SDA_Input();	// ��������ģʽ
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
 * @brief I2C����ACKӦ��
 * @param  ��
 * @return ��
 */
void I2C_Ack(void)
{
	I2C_SCL(0);
	Set_I2C_SDA_Output();	// �������ģʽ
	I2C_SDA(0);
	delay_us(2);
	I2C_SCL(1);
	delay_us(2);
	I2C_SCL(0);	
}

/**
 * @brief I2C������ACKӦ��
 * @param  ��
 * @return ��
 */
void I2C_NAck(void)
{
	I2C_SCL(0);
	Set_I2C_SDA_Output();	// �������ģʽ
	I2C_SDA(1);
	delay_us(2);
	I2C_SCL(1);
	delay_us(2);
	I2C_SCL(0);	
}

/**
 * @brief I2C����һ���ֽ�
 * @param  ��
 * @return ��
 */
void I2C_Send_Byte(uint8_t sendByte)
{
	uint8_t i;
	Set_I2C_SDA_Output();	// �������ģʽ
	I2C_SCL(0);		// ����ʱ�ӿ�ʼ���ݴ���
	for(i = 0; i < 8; i++)
	{
//		// �Ӹ�λһ�η�
//		if(sendByte & 0x80)
//			I2C_SDA(1);
//		else
//			I2C_SDA(0);
		I2C_SDA((sendByte & 0x80) >> 7);
		delay_us(2);
		I2C_SCL(1);		
		delay_us(2);
		I2C_SCL(0);		
		sendByte <<= 1; // ����ȡ���λ
	}
}

/**
 * @brief I2C����һ���ֽ�
 * @param  ack=1������ACK��ack=0������NACK 
 * @return ���յ�����
 */
uint8_t I2C_Read_Byte(uint8_t ack)
{
	uint8_t i;
	uint8_t recvByte = 0;
	Set_I2C_SDA_Input();	// ��������ģʽ
	for(i = 0; i < 8; i++)
	{
		I2C_SCL(0);		// ʱ�������ͣ����ߴӻ���������Ҫ����
		delay_us(2);	// ��ƽ����ʱ�䣬�ȴ��ӻ���������
		I2C_SCL(1);		// ʱ�������ߣ����ߴӻ����������ڶ�ȡ����
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

