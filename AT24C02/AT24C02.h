// at24c02.h

#ifndef __AT24C02_H
#define __AT24C02_H

#include "stm32f1xx_hal.h"  // 包含 STM32 HAL 库头文件

#define AT24C02_ADDR_WRITE  0xA0  // AT24C02 I2C 写地址 (7位地址左移一位)
#define AT24C02_ADDR_READ   0xA1  // AT24C02 I2C 读地址 (7位地址左移一位)

void AT24C02_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef AT24C02_Write(uint16_t memAddress, uint8_t data);
HAL_StatusTypeDef AT24C02_Read(uint16_t memAddress, uint8_t *data);

HAL_StatusTypeDef AT24C02_Write_Array(uint16_t memAddress, uint8_t *data, uint16_t length);
HAL_StatusTypeDef AT24C02_Read_Array(uint16_t memAddress, uint8_t *data, uint16_t length);


#endif /* __AT24C02_H */
