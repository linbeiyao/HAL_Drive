#include "main.h"
#include "matrixkey.h"

/* 4*4 对应矩阵按键枚举值*/
static uint8_t MATRIXKEY_INT_Buttons[4][4] = {
	{0x01, 0x02, 0x03, 0x0A},
	{0x04, 0x05, 0x06, 0x0B},
	{0x07, 0x08, 0x09, 0x0C},
	{0x0E, 0x00, 0x0F, 0x0D},
};

/**
  * @brief  拉低某一行
  * @param	void
  * 
  * @retval 返回按下的那行
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
  * @brief  逐列扫描（只判断是否按下，没判断松开后，这步在后面做）
  * @param	void
  * 
  * @retval 
  */
static uint8_t MatrixKey_Col_Scan(uint8_t row)
{
	/* 第一列 */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][0];	
	}
	
	/* 第二列 */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][1];
	}
	
	/* 第三列 */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][2];	
	}
	
	/* 第四列 */
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == RESET)
			return MATRIXKEY_INT_Buttons[row - 1][3];	
	}
	
	return KEY_NO_PREESED;
}
/**
  * @brief  获取按键值（使用时，获取按键值之后需要再次获取判断是否松开）
  * @param	void
  * 
  * @retval 按键值	
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


