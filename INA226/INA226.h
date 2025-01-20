/*
*@autor DanielBlancoR
*@email danielcblancor@gmail.com
*@ide Keil uVision
*/

#ifndef INA226_H_
#define INA226_H_


#ifdef __cplusplus
extern "C"
{	
#endif

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "stm32f1xx_hal.h"
//#include "main.h"
// INA226 I2C 设备地址，用于与设备进行通信
#define INA226_ADDRESS           0x40

// 分流电阻值 (单位：欧姆)，用于计算电流
#define R_SHUNT                  0.01

// INA226 寄存器地址定义，用于访问不同的功能和数据
#define INA226_CONFIG_REG        0x00  // 配置寄存器：设置设备的工作模式和参数
#define INA226_SHUNT_VOLTAGE_REG 0x01  // 分流电压寄存器：存储测量的分流电压值
#define INA226_BUS_VOLTAGE_REG 	 0x02  // 总线电压寄存器：存储测量的总线电压值
#define INA226_POWER_REG         0x03  // 功率寄存器：存储计算的功率值
#define INA226_CURRENT_REG       0x04  // 电流寄存器：存储计算的电流值
#define INA226_CALIBRATION_REG   0x05  // 校准寄存器：设置校准参数以确保测量精度
#define INA226_MASK_ENABLE_REG   0x06  // 掩码/使能寄存器：配置报警和限值功能
#define INA226_ALERT_LIMIT_REG   0x07  // 报警限值寄存器：设置报警触发的限值
#define INA226_MANUFACTURER_REG  0xFE  // 制造商寄存器：存储制造商信息
#define INA226_DIE_ID_REG        0xFF  // 芯片ID寄存器：存储芯片的唯一标识符

// 使用方法：
// 1. 初始化 I2C 接口以与 INA226 进行通信。
// 2. 使用 INA226_ADDRESS 作为设备地址进行 I2C 读写操作。
// 3. 通过访问不同的寄存器地址（如 INA226_CONFIG_REG）来配置和读取设备数据。
// 4. 根据需要设置校准参数（通过 INA226_CALIBRATION_REG）以确保测量精度。
// 5. 读取电压、电流和功率数据以进行监控和分析。

// 平均模式枚举类型，用于设置测量的平均次数，以提高测量的稳定性和精度
typedef enum
{	
	Num_AVG_1    = 0x0,  // 1次平均，适用于快速响应但对精度要求不高的场景
	Num_AVG_4    = 0x1,  // 4次平均，适用于需要一定精度且响应速度较快的场景
	Num_AVG_16   = 0x2,  // 16次平均，适用于对精度有较高要求的场景
	Num_AVG_64   = 0x3,  // 64次平均，适用于需要高精度测量的场景
	Num_AVG_128  = 0x4,  // 128次平均，适用于非常高精度要求的场景
	Num_AVG_256  = 0x5,  // 256次平均，适用于极高精度要求且响应速度可以较慢的场景
	Num_AVG_512  = 0x6,  // 512次平均，适用于超高精度要求的场景
	Num_AVG_1024 = 0x7   // 1024次平均，适用于最高精度要求且响应速度不敏感的场景
}Bit_AVG_t;

// 转换时间枚举类型，用于设置测量的转换时间，以平衡测量速度和功耗
typedef enum
{
	ConvTime_140us  = 0x0, // 140微秒转换时间，适用于快速测量但功耗较高的场景
	ConvTime_204us  = 0x1, // 204微秒转换时间，适用于快速响应的场景
	ConvTime_332us  = 0x2, // 332微秒转换时间，适用于中等速度和功耗的场景
	ConvTime_588us  = 0x3, // 588微秒转换时间，适用于平衡速度和功耗的场景
	ConvTime_1ms1   = 0x4, // 1.1毫秒转换时间，适用于较低功耗的场景
	ConvTime_2ms116 = 0x5, // 2.116毫秒转换时间，适用于低功耗和高精度的场景
	ConvTime_4ms156 = 0x6, // 4.156毫秒转换时间，适用于非常低功耗的场景
	ConvTime_8ms244 = 0x7  // 8.244毫秒转换时间，适用于最低功耗和最高精度的场景
}Bit_ConvTime_t;

typedef enum
{
	PowerDown        = 0x0, // 关闭电源模式：设备处于低功耗状态，不进行测量，适用于长时间不需要监测的场景
	ShuntVoltage     = 0x1, // 分流电压测量模式：仅测量分流电压，适用于需要监测电流变化的场景
	BusVoltage       = 0x2, // 总线电压测量模式：仅测量总线电压，适用于需要监测电压稳定性的场景
	ShuntAndBus      = 0x3, // 分流电压和总线电压测量模式：同时测量分流电压和总线电压，适用于全面监测电流和电压的场景
	PowerDown_       = 0x4, // 备用电源关闭模式：与PowerDown相似，可能用于不同的电源管理策略，适用于备用电源管理的场景
	ShuntVoltageCont = 0x5, // 连续分流电压测量模式：持续测量分流电压，适用于需要实时监测电流的场景
	BusVoltageCont   = 0x6, // 连续总线电压测量模式：持续测量总线电压，适用于需要实时监测电压的场景
	ShuntAndBusCont  = 0x7  // 连续分流电压和总线电压测量模式：持续同时测量分流电压和总线电压，适用于需要实时全面监测的场景
}Mode_t;
}Mode_t;

/* 配置寄存器 (00h) (读/写)
		[15] RST : 复位位。
		[9-11] AVG : 平均模式。
		[6-8] VBUSCT : 总线电压转换时间。
		[3-5] VSHCT : 分流电压转换时间。
		[0-2] MODE : 工作模式。 */

typedef struct
{
	uint16_t       Config_mask;
	uint8_t        RST; 
	Bit_AVG_t      AVG;
	Bit_ConvTime_t VBUSCT;
	Bit_ConvTime_t VSHCT;
	Mode_t         MODE;

}INA226_Init;

typedef struct
{
	uint16_t ShuntVoltage;
	uint16_t BusVoltage;
	uint16_t Power;
	uint16_t Current;
	uint16_t Calibration;
	
	float CurrentLSB;
	
	
}INA226_Values;

/**
 * 使用流程：
 * 1. 调用 INA226_INIT() 函数初始化 INA226 传感器。
 * 2. 使用 INA226_Config() 函数配置传感器的工作模式和转换时间。
 * 3. 调用 INA226_SetCalibration() 函数设置校准参数以提高测量精度。
 * 4. 使用 INA226_Voltage_Current_Power() 函数计算并获取电压、电流和功率值。
 * 5. 调用 INA226_ShuntVoltage() 函数读取分流电压值。
 * 6. 调用 INA226_BusVoltage() 函数读取总线电压值。
 * 7. 使用 INA226_Power() 函数计算并获取功率值。
 * 8. 使用 INA226_Current() 函数计算并获取电流值。
 * 9. 调用 INA226_Reset() 函数重置传感器到默认状态。
 * 10. 使用 INA226_ID() 函数读取并返回设备 ID。
 * 11. 调用 INA226_GetCalibration() 函数获取当前的校准参数。
 */

void INA226_INIT (void); // 初始化 INA226 传感器，准备进行测量
void INA226_Config (Mode_t mode, Bit_ConvTime_t shuntVoltTime, Bit_ConvTime_t BusVoltTime , Bit_AVG_t AVGMode); // 配置 INA226 的工作模式和转换时间
void INA226_Reset (void); // 重置 INA226 传感器到默认状态
uint16_t INA226_ID(void); // 读取并返回 INA226 的设备 ID

void INA226_Voltage_Current_Power(float *volt, float *current, float *power); // 计算并返回电压、电流和功率
float INA226_ShuntVoltage (void); // 读取并返回分流电压值
float INA226_BusVoltage (void); // 读取并返回总线电压值
float INA226_Power (void); // 计算并返回功率值
float INA226_Current(void); // 计算并返回电流值
void INA226_SetCalibration(float R_Shunt,float MaxExpCurrent); // 设置校准参数以提高测量精度
uint16_t INA226_GetCalibration(void); // 获取当前的校准参数

//void INA226_I2C_Write (uint8_t pByte, uint16_t Data);
//void INA226_I2C_Read (uint8_t pByte, uint16_t* pData);


#ifdef __cplusplus
}
#endif
#endif