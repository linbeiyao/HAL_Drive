#include "keyborad.h"
#include "stm32f1xx_hal_gpio.h"


KeyPin COL1 = {.port = GPIOA, .pin = GPIO_PIN_4};
KeyPin COL2 = {.port = GPIOA, .pin = GPIO_PIN_5};
KeyPin COL3 = {.port = GPIOA, .pin = GPIO_PIN_6};
KeyPin COL4 = {.port = GPIOA, .pin = GPIO_PIN_7};

KeyPin ROW1 = {.port = GPIOB, .pin = GPIO_PIN_12};
KeyPin ROW2 = {.port = GPIOB, .pin = GPIO_PIN_13};
KeyPin ROW3 = {.port = GPIOB, .pin = GPIO_PIN_14};
KeyPin ROW4 = {.port = GPIOB, .pin = GPIO_PIN_15};

KeyMatrix keyMatrix = {.col = {COL1, COL2, COL3, COL4}, .row = {ROW1, ROW2, ROW3, ROW4}};

void keyboard_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 初始化列引脚
    for (int i = 0; i < COL_NUM; i++)
    {
        GPIO_InitStruct.Pin = keyMatrix.col[i].GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(keyMatrix.col[i].GPIO_Port, &GPIO_InitStruct);
    }

    // 初始化行引脚
    for (int i = 0; i < ROW_NUM; i++)
    {
        GPIO_InitStruct.Pin = keyMatrix.row[i].GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(keyMatrix.row[i].GPIO_Port, &GPIO_InitStruct);
    }
}

uint8_t keyboard_scan(void)
{
    // 检查 ROW_NUM 和 COL_NUM 是否有效
    if (ROW_NUM <= 0 || COL_NUM <= 0) {
        return 0; // 或者返回其他错误码
    }

    for (int i = 0; i < ROW_NUM; i++)
    {
        // 置低当前行引脚，并检查是否成功
        if (HAL_GPIO_WritePin(keyMatrix.row[i].GPIO_Port, keyMatrix.row[i].GPIO_Pin, GPIO_PIN_RESET) != HAL_OK) {
            // 处理写入失败的情况
            return 0; // 或者返回其他错误码
        }

        for (int j = 0; j < COL_NUM; j++)       // 遍历列引脚
        {

            // 列引脚为高电平
            if (HAL_GPIO_ReadPin(keyMatrix.col[j].GPIO_Port, keyMatrix.col[j].GPIO_Pin)) {
                // 处理按键未按下的情况（可选）
            }
            else {
                return j * COL_NUM + i + 1;
            }
        }

        // 置高当前行引脚，避免影响后续行扫描
        if (HAL_GPIO_WritePin(keyMatrix.row[i].GPIO_Port, keyMatrix.row[i].GPIO_Pin, GPIO_PIN_SET) != HAL_OK) {
            // 处理写入失败的情况
            return 0; // 或者返回其他错误码
        }
    }
    return 1; // 表示扫描成功
}