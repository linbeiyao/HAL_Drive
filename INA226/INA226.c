/*
 * ina226.c
 *
 *  Created on: Oct 10, 2024
 *  Author: Pantelos
 *  
 *  INA226 功率监测芯片驱动实现文件
 *  提供对INA226芯片的初始化、校准、电压/电流/功率读取等功能
 */

#include "ina226.h"
#include "stdint.h"

/**
 * @brief 初始化INA226芯片
 * @param ina226 INA226句柄指针
 * @param hi2c1 I2C句柄指针
 * @param configuration 配置寄存器值
 * @return 初始化状态
 */
ina226_status ina226_init(ina226_handle *ina226, I2C_HandleTypeDef *hi2c1, uint16_t configuration)
{
    uint8_t configuration_data[2];

    // 保存I2C句柄
    ina226->hi2c1 = hi2c1;

    // 检查设备是否就绪
    if (HAL_I2C_IsDeviceReady(ina226->hi2c1, INA226_I2C_ADDRESS, 10, 30) != HAL_OK)
    {
        return INA_STATUS_I2C_FAIL;
    }

    // 软件复位
    configuration_data[0] = (RESET_ENABLE & 0xff00) >> 8;
    configuration_data[1] = 0 & 0x00ff;
    if (HAL_I2C_Mem_Write(ina226->hi2c1, INA226_I2C_ADDRESS, (uint16_t) CONFIG_REG, 1, configuration_data, 2, 30) != HAL_OK)
    {
        return INA_STATUS_I2C_FAIL;
    }

    HAL_Delay(100);  // 等待复位完成

    // 写入配置寄存器
    configuration_data[0] = (configuration & 0xff00) >> 8;
    configuration_data[1] = configuration & 0x00ff;

    if (HAL_I2C_Mem_Write(ina226->hi2c1, INA226_I2C_ADDRESS, (uint16_t) CONFIG_REG, 1, configuration_data, 2, 30) != HAL_OK)
    {
        return INA_STATUS_I2C_FAIL;
    }

    return INA_STATUS_OK;
}

/**
 * @brief 设置校准寄存器
 * @param ina226 INA226句柄指针
 * @return 设置状态
 */
ina226_status ina226_set_cal_reg(ina226_handle *ina226)
{
	uint8_t cal_reg_data[2];
	uint16_t calibration_val = CAL_FINAL;
	cal_reg_data[0] = (calibration_val & 0xff00) >> 8;
	cal_reg_data[1] = calibration_val & 0x00ff;
	
	if (HAL_I2C_Mem_Write(ina226->hi2c1, INA226_I2C_ADDRESS, (uint16_t) CAL_REG, 1, cal_reg_data, 2, 30) != HAL_OK)
	{
		return INA_STATUS_I2C_FAIL;
	}
	return INA_STATUS_OK;
}

/**
 * @brief 读取原始分流电压值
 * @param ina226 INA226句柄指针
 * @return 原始分流电压值
 */
uint16_t ina226_read_raw_shunt_voltage(ina226_handle *ina226)
{
	uint8_t raw_shunt_reading[2];

	if (HAL_I2C_Mem_Read(ina226->hi2c1, INA226_I2C_ADDRESS,(uint16_t) SHUNT_VOLTAGE, 1, raw_shunt_reading, 2, 30) != HAL_OK)
	{
		return INA_STATUS_I2C_FAIL;
	}

	return raw_shunt_reading[0] << 8 | raw_shunt_reading[1];
}

/**
 * @brief 读取原始总线电压值
 * @param ina226 INA226句柄指针
 * @return 原始总线电压值
 */
uint16_t ina226_read_raw_bus_voltage(ina226_handle *ina226)
{
	uint8_t raw_bus_reading[2];

	if (HAL_I2C_Mem_Read(ina226->hi2c1, INA226_I2C_ADDRESS,(uint16_t) BUS_VOLTAGE, 1, raw_bus_reading, 2, 30) != HAL_OK)
	{
		return INA_STATUS_I2C_FAIL;
	}

	return raw_bus_reading[0] << 8 | raw_bus_reading[1];
}

/**
 * @brief 读取总线电压
 * @param ina226 INA226句柄指针
 * @return 总线电压值（单位：V）
 */
float ina226_read_bus_voltage(ina226_handle *ina226)
{
	uint16_t raw_voltage = ina226_read_raw_bus_voltage(ina226);

	return raw_voltage * BUS_VOL_STEP_VALUE;
}

/**
 * @brief 通过电流寄存器读取电流值
 * @param ina226 INA226句柄指针
 * @return 电流值（单位：A）
 */
float ina226_current_via_reg(ina226_handle *ina226)
{
	uint8_t current_reg_data[2];
	int16_t raw_current;

	if (HAL_I2C_Mem_Read(ina226->hi2c1, INA226_I2C_ADDRESS, (uint16_t)CURRENT_REG, 1, current_reg_data, 2, 30) != HAL_OK)
	{
	    return INA_STATUS_I2C_FAIL;
	}

	raw_current = (int16_t)((current_reg_data[0] << 8) | current_reg_data[1]);

	return raw_current * CURRENT_LSB;
}

/**
 * @brief 通过功率寄存器读取功率值
 * @param ina226 INA226句柄指针
 * @return 功率值（单位：W）
 */
float ina226_power_via_reg(ina226_handle *ina226)
{
	uint8_t power_reg_data[2];
	int16_t raw_power;
	
	if (HAL_I2C_Mem_Read(ina226->hi2c1, INA226_I2C_ADDRESS, (uint16_t)POWER_REG, 1, power_reg_data, 2, 30) != HAL_OK)
	{
	    return INA_STATUS_I2C_FAIL;
	}
	
	raw_power = (int16_t)((power_reg_data[0] << 8) | power_reg_data[1]);
	
	return raw_power * POWER_LSB;
}

/**
 * @brief 检查转换是否完成
 * @param ina226 INA226句柄指针
 * @return 转换状态
 */
ina226_status check_if_conversion_ready(ina226_handle *ina226)
{
		uint8_t mask_reg_data[2];
		uint16_t conversion_ready = 0;

		if (HAL_I2C_Mem_Read(ina226->hi2c1, INA226_I2C_ADDRESS, (uint16_t)MASK_EN_REG, 1, mask_reg_data, 2, 30) != HAL_OK)
		{
		    return INA_STATUS_I2C_FAIL;
		}

		conversion_ready = (int16_t)(((mask_reg_data[0] << 8) | mask_reg_data[1]) & 0x0008);
		if (conversion_ready)
		{
			return INA_CONVERSION_READY;
		}
		return INA_CONVERSION_NOT_READY;
}