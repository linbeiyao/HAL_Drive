#include "main.h"
#include "matrixkey.h"

/* 4*4 ��Ӧ���󰴼�ö��ֵ*/
static uint8_t MATRIXKEY_INT_Buttons[4][4] = {
	{0x01, 0x02, 0x03, 0x0A},
	{0x04, 0x05, 0x06, 0x0B},
	{0x07, 0x08, 0x09, 0x0C},
	{0x0E, 0x00, 0x0F, 0x0D},
};

/**
  * @brief  ����ĳһ��
  * @param	void
  * 
  * @retval ���ذ��µ�����
  */
static void MatrixKey_Row_PullDown(uint8_t row)
{
	switch(row)
	{
		case 1:
			MATRIXKEY_ROW_HIGH_BUT_1;
			break;
		case 2:
			MATRIXKEY_ROW_HIGH_BUT_2;
			break;	
		case 3:
			MATRIXKEY_ROW_HIGH_BUT_3;
			break;
		case 4:
			MATRIXKEY_ROW_HIGH_BUT_4;	
			break;		
		default:
			break;
	}
}

/**
  * @brief  ����ɨ�裨ֻ�ж��Ƿ��£�û�ж��ɿ����ⲽ�ں�������
  * @param	void
  * 
  * @retval 
  */
static uint8_t MatrixKey_Col_Scan(uint8_t row)
{
	/* ��һ�� */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][0];	
	}
	
	/* �ڶ��� */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][1];
	}
	
	/* ������ */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][2];	
	}
	
	/* ������ */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][3];	
	}
	
	return KEY_NO_PREESED;
}
/**
  * @brief  ��ȡ����ֵ��ʹ��ʱ����ȡ����ֵ֮����Ҫ�ٴλ�ȡ�ж��Ƿ��ɿ���
  * @param	void
  * 
  * @retval ����ֵ	
  */
uint8_t MatrixKey_GetNum(void)
{
	uint8_t i, row, keyNum;
	
	for(i = 0; i < 4; i++)
	{
		row = i + 1;
		MatrixKey_Row_PullDown(row);
		keyNum = MatrixKey_Col_Scan(row);
		
		if(keyNum != KEY_NO_PREESED)
			return keyNum;
	}
	
	return KEY_NO_PREESED;
}


