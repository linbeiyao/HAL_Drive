// at24c02.c

#include "at24c02.h"

// 初始化 I2C 设置
void AT24C02_Init(I2C_HandleTypeDef *hi2c)
{


}

// 写操作
HAL_StatusTypeDef AT24C02_Write(uint16_t memAddress, uint8_t data)
{
    uint8_t buffer[3];

    // 发送内存地址和数据
    buffer[0] = (memAddress >> 8) & 0xFF;  // 高地址字节
    buffer[1] = memAddress & 0xFF;         // 低地址字节
    buffer[2] = data;                      // 数据字节

    // 发送数据到 AT24C02
    return HAL_I2C_Master_Transmit(&hi2c1, AT24C02_ADDR_WRITE, buffer, 3, 1000);
}

// 读操作
HAL_StatusTypeDef AT24C02_Read(uint16_t memAddress, uint8_t *data)
{
    uint8_t address[2];

    // 发送内存地址
    address[0] = (memAddress >> 8) & 0xFF;  // 高地址字节
    address[1] = memAddress & 0xFF;         // 低地址字节

    // 向 AT24C02 发送内存地址
    if (HAL_I2C_Master_Transmit(&hi2c1, AT24C02_ADDR_WRITE, address, 2, 1000) != HAL_OK)
    {
        return HAL_ERROR;  // 发送失败
    }

    // 切换到读取模式并接收数据
    return HAL_I2C_Master_Receive(&hi2c1, AT24C02_ADDR_READ, data, 1, 1000);
}


// 写入字节数组到 AT24C02
HAL_StatusTypeDef AT24C02_Write_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2];  // 用于存储内存地址
    uint16_t i;

    for (i = 0; i < length; i++)
    {
        // 先构建内存地址
        buffer[0] = (memAddress >> 8) & 0xFF;  // 高地址字节
        buffer[1] = memAddress & 0xFF;         // 低地址字节

        // 发送内存地址
        status = HAL_I2C_Master_Transmit(&hi2c1, AT24C02_ADDR_WRITE, buffer, 2, 1000);
        if (status != HAL_OK)
        {
            return status;  // 如果地址发送失败，直接返回
        }

        // 发送当前数据字节
        status = HAL_I2C_Master_Transmit(&hi2c1, AT24C02_ADDR_WRITE, &data[i], 1, 1000);
        if (status != HAL_OK)
        {
            return status;  // 如果数据发送失败，直接返回
        }

        // 等待写入操作完成，典型延迟为 5-10ms
        HAL_Delay(10);

        // 更新内存地址
        memAddress++;  // 每次写入一个字节，内存地址递增
    }

    return status;  // 返回操作状态
}

// 从 AT24C02 读取字节数组
HAL_StatusTypeDef AT24C02_Read_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2];  // 用于存储内存地址
    uint16_t i;

    // 发送内存地址（高字节 + 低字节）
    buffer[0] = (memAddress >> 8) & 0xFF;  // 高地址字节
    buffer[1] = memAddress & 0xFF;         // 低地址字节
    status = HAL_I2C_Master_Transmit(&hi2c1, AT24C02_ADDR_WRITE, buffer, 2, 1000);
    if (status != HAL_OK)
    {
        return status;  // 发送内存地址失败
    }

    // 发送读取命令并接收数据
    status = HAL_I2C_Master_Receive(&hi2c1, AT24C02_ADDR_READ, data, length, 1000);
    return status;
}