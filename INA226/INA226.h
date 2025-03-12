/*
 * ina226.h
 *
 * 创建日期: 2024年10月10日
 * 作者: Pantelos
 * 使用教程：
 * 1. 初始化INA226：
 *    ina226_handle ina226;
 *    ina226_init(&ina226, &hi2c1, CONTINUOUS_MODE_ALL | V_SHUNT_140us | V_BUS_140us);
 * 
 * 2. 设置校准寄存器：
 *    ina226_set_cal_reg(&ina226);
 * 
 * 3. 读取电压/电流/功率：
 *    float bus_voltage = ina226_read_bus_voltage(&ina226);
 *    float current = ina226_current_via_reg(&ina226);
 *    float power = ina226_power_via_reg(&ina226);
 * 
 * 4. 检查转换状态：
 *    ina226_status status = check_if_conversion_ready(&ina226);
 *    if(status == INA_CONVERSION_READY) {
 *        // 数据已准备好
 *    }
 */

#ifndef INC_INA226_H_
#define INC_INA226_H_

// 在此定义系统HAL以便使用i2c HAL
#include "stm32f1xx_hal.h"

// INA 226电源监控IC的基本固件驱动。所有IO操作都在阻塞模式下进行。
// 定义了超时时间，以防测量永远不成功，这样应用程序就不会阻塞。

// 根据您的定时应用需求定义驱动延迟
#define TIME_OUT_MS 		100

// 根据IC硬件配置定义CAL_REG使用的值（单位：欧姆）（第15页）
// 在我的情况下，我设置最大电流为1A，Rs = 0.1。做了一些舍入简化。

// 最终校准值
#define CURRENT_LSB			0.000024
#define POWER_LSB			0.0006
#define CAL_FINAL 			2133

// 地址 A0 = GND A1 = GND （第18页）
#define INA226_I2C_ADDRESS (0x40 << 1)

// 配置寄存器映射（第22页）
#define CONFIG_REG 			0x00
#define SHUNT_VOLTAGE 		0x01
#define BUS_VOLTAGE 		0x02
#define POWER_REG 			0x03
#define CURRENT_REG			0x04
#define CAL_REG		 		0x05
#define MASK_EN_REG 		0x06
#define ALERT_LIM_REG 		0x07
#define MANUF_ID	 		0xFE
#define DIE_ID 				0xFF

// 配置寄存器位

// 模式位（第11页）
#define CONTINUOUS_SHUNT_V	0x0005
#define CONTINUOUS_BUS_V    0x0006
#define CONTINUOUS_MODE_ALL 0x0007
#define TRIGGER_SHUNT_V		0x0001
#define TRIGGER_BUS_V		0x0010
#define TRIGGER_BOTH		0x0011

// 转换时间
// 分流电压测量CT（时间见第5页，位见第24页）
#define V_SHUNT_140us		(0x0000 << 3)
#define V_SHUNT_204us		(0x0001 << 3)
#define V_SHUNT_332us		(0x0002 << 3)
#define V_SHUNT_588us		(0x0003 << 3)
#define V_SHUNT_1_1ms		(0x0004 << 3)
#define V_SHUNT_2_116ms		(0x0005 << 3)
#define V_SHUNT_4_156ms		(0x0006 << 3)
#define V_SHUNT_8_244ms		(0x0007 << 3)

// 总线电压测量CT（时间见第5页，位见第24页）
#define V_BUS_140us			(0x0000 << 6)
#define V_BUS_204us			(0x0001 << 6)
#define V_BUS_332us			(0x0002 << 6)
#define V_BUS_588us			(0x0003 << 6)
#define V_BUS_1_1ms			(0x0004 << 6)
#define V_BUS_2_116ms		(0x0005 << 6)
#define V_BUS_4_156ms		(0x0006 << 6)
#define V_BUS_8_244ms		(0x0007 << 6)

// 测量平均选择（时间见第5页，位见第24页）
#define TIMES_AVG_OFF		(0x0000 << 9)
#define TIMES_AVG_1			(0x0001 << 9)
#define TIMES_AVG_2			(0x0002 << 9)
#define TIMES_AVG_3			(0x0003 << 9)
#define TIMES_AVG_4			(0x0004 << 9)
#define TIMES_AVG_5			(0x0005 << 9)
#define TIMES_AVG_6			(0x0006 << 9)
#define TIMES_AVG_7			(0x0007 << 9)

// 将此写入配置寄存器以进行软件复位（软件复位位见第24页）
#define RESET_ENABLE				(0x0001 <<15)

// 对MASK_EN_REG读取的值进行掩码，以检查所有测量/计算是否准备就绪
#define	CONVERSION_READY_FLG_MASK 	(0x0008)

// 总线测量中ADC读数的1 LSB步长值，用于获取电压（第5页）
#define	BUS_VOL_STEP_VALUE			0.00125

/**
 * @brief INA226状态枚举
 * 定义INA226芯片操作可能返回的状态
 */
typedef enum
{
	INA_STATUS_OK,               // 操作成功
	INA_STATUS_I2C_FAIL,         // I2C通信失败
	INA_STATUS_TIMEOUT,          // 操作超时
	INA_CONVERSION_NOT_READY,    // 转换未完成
	INA_CONVERSION_READY,        // 转换已完成
}ina226_status;

/**
 * @brief INA226句柄结构体
 * 包含INA226芯片操作所需的基本信息
 */
typedef struct{
	I2C_HandleTypeDef *hi2c1;    // I2C通信句柄指针
}ina226_handle;

/* 函数声明 */

/**
 * @brief 初始化INA226芯片
 * @param ina226 INA226句柄指针
 * @param hi2c1 I2C句柄指针
 * @param configuration 配置寄存器值
 * @return 初始化状态
 */
ina226_status ina226_init(ina226_handle *ina226, I2C_HandleTypeDef *hi2c1, uint16_t configuration);

/**
 * @brief 设置校准寄存器
 * @param ina226 INA226句柄指针
 * @return 设置状态
 */
ina226_status ina226_set_cal_reg(ina226_handle *ina226);

/**
 * @brief 检查转换是否完成
 * @param ina226 INA226句柄指针
 * @return 转换状态
 */
ina226_status check_if_conversion_ready(ina226_handle *ina226);

/**
 * @brief 读取原始分流电压值
 * @param ina226 INA226句柄指针
 * @return 原始分流电压值
 */
uint16_t ina226_read_raw_shunt_voltage(ina226_handle *ina226);

/**
 * @brief 读取原始总线电压值
 * @param ina226 INA226句柄指针
 * @return 原始总线电压值
 */
uint16_t ina226_read_raw_bus_voltage(ina226_handle *ina226);

/**
 * @brief 读取总线电压
 * @param ina226 INA226句柄指针
 * @return 总线电压值（单位：V）
 */
float ina226_read_bus_voltage(ina226_handle *ina226);

/**
 * @brief 通过电流寄存器读取电流值
 * @param ina226 INA226句柄指针
 * @return 电流值（单位：A）
 */
float ina226_current_via_reg(ina226_handle *ina226);

/**
 * @brief 通过功率寄存器读取功率值
 * @param ina226 INA226句柄指针
 * @return 功率值（单位：W）
 */
float ina226_power_via_reg(ina226_handle *ina226);

#endif /* INC_INA226_H_ */