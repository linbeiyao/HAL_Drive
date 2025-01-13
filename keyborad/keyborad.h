#ifndef __KEYBORAD_H__
#define __KEYBORAD_H__

/**
 * 这里是 矩阵 键盘的驱动代码
 * @file keyborad.h
 *
 *
 */

#define ROW1_Pin GPIO_PIN_12
#define ROW1_GPIO_Port GPIOB
#define ROW2_Pin GPIO_PIN_13
#define ROW2_GPIO_Port GPIOB
#define ROW3_Pin GPIO_PIN_14
#define ROW3_GPIO_Port GPIOB
#define ROW4_Pin GPIO_PIN_15
#define ROW4_GPIO_Port GPIOB
#define COL1_Pin GPIO_PIN_4
#define COL1_GPIO_Port GPIOA
#define COL2_Pin GPIO_PIN_5
#define COL2_GPIO_Port GPIOA
#define COL3_Pin GPIO_PIN_6
#define COL3_GPIO_Port GPIOA
#define COL4_Pin GPIO_PIN_7
#define COL4_GPIO_Port GPIOA

#define KEY_NUM 16
#define ROW_NUM 4
#define COL_NUM 4

typedef struct
{
    (GPIO_TypeDef *)GPIOx;
    uint16_t GPIO_Pin;
} KeyPin;

typedef struct
{
    KeyPin row[ROW_NUM];
    KeyPin col[COL_NUM];

} KeyMatrix;

/* 外部变量 */
extern KeyMatrix keyMatrix;

void KeyMatrix_Init(void);
uint8_t KeyMatrix_Scan(void);
void KeyMatrix_Scan_Test(void);

#endif