#include "INA226.h"

uint16_t Buff[10], tempReg;

extern I2C_HandleTypeDef hi2c1;
INA226_Values INA226_values;

void INA226_I2C_Write(uint8_t pByte, uint16_t Data);
void INA226_I2C_Read(uint8_t pByte, uint16_t* pData);
void INA226_I2C_ReadBuff(uint8_t pByte, uint8_t* pData, uint16_t size);

void INA226_INIT(void)
{
	INA226_I2C_Write(INA226_CONFIG_REG, 0x4127); // Default
	INA226_SetCalibration(R_SHUNT,8);
//	INA226_Config(ShuntAndBusCont, ConvTime_1ms1, ConvTime_1ms1, Num_AVG_1); // Default
}

void INA226_Config (Mode_t mode, Bit_ConvTime_t shuntVoltTime, Bit_ConvTime_t BusVoltTime , Bit_AVG_t AVGMode)
{
	INA226_Init INA226Handle;
	
	INA226Handle.Config_mask = (0x1 << 14) | (AVGMode << 9) | (BusVoltTime << 6) | (shuntVoltTime << 3) | mode;
	INA226Handle.MODE = mode;
	INA226Handle.VSHCT = shuntVoltTime;
	INA226Handle.VBUSCT = BusVoltTime;
	INA226Handle.AVG = AVGMode;
	
	INA226_I2C_Write(INA226_CONFIG_REG, INA226Handle.Config_mask);
	
	INA226_SetCalibration(R_SHUNT,8);
}

void INA226_Reset(void)
{
	uint16_t reg;
	
	INA226_I2C_Read(INA226_CONFIG_REG,&reg);
	reg |= (0x1 << 14U);
	
	INA226_I2C_Write(INA226_CONFIG_REG, reg);
}

uint16_t INA226_ID(void)
{
	INA226_I2C_Read(INA226_DIE_ID_REG,Buff);
	tempReg = Buff[0]; 
	return tempReg;
}

void INA226_Voltage_Current_Power(float *volt, float *current, float *power)
{
	uint8_t BuffData[10];
	INA226_I2C_ReadBuff(INA226_BUS_VOLTAGE_REG,BuffData,6);
	
	INA226_values.BusVoltage = (BuffData[0] << 8) | BuffData[1];
//	INA226_values.Power = (BuffData[2] << 8) | BuffData[3];
//	INA226_values.Current = (BuffData[4] << 8) | BuffData[5];
	
	*volt    = INA226_values.BusVoltage * 1.25e-03;
//	*current = INA226_values.Current * 1e-03;
//	*power   = INA226_values.Power * 25e-03;
	
}

float INA226_ShuntVoltage (void){

	INA226_I2C_Read(INA226_SHUNT_VOLTAGE_REG,&tempReg);
	return tempReg * 2.5e-06;

}

float INA226_BusVoltage (void)
{
	INA226_I2C_Read(INA226_BUS_VOLTAGE_REG,&tempReg);
	return tempReg * 1.25e-03f;	
}

float INA226_Power (void)
{
	INA226_I2C_Read(INA226_POWER_REG,&tempReg);
	
	return tempReg * (INA226_values.CurrentLSB*25);
}

float INA226_Current (void)
{
	INA226_I2C_Read(INA226_CURRENT_REG,&tempReg);
	
	return tempReg * INA226_values.CurrentLSB;
//	return ((INA226_ShuntVoltage()*INA226_values.Calibration)/2048) * INA226_values.CurrentLSB;
}

void INA226_SetCalibration (float R_Shunt,float MaxExpCurrent)
{
	float Current_LSB;
	float CAL;
	
	if(MaxExpCurrent > 8)
	{
		MaxExpCurrent = 8.192f;
	}
	
	Current_LSB = MaxExpCurrent / 32768.0f;
	
	CAL = 0.00512f/(Current_LSB*R_Shunt);
	INA226_values.Calibration = CAL;
	INA226_values.CurrentLSB = Current_LSB;
	INA226_I2C_Write(INA226_CALIBRATION_REG, (uint16_t)CAL);
}

uint16_t INA226_GetCalibration(void)
{
	uint16_t reg;
	INA226_I2C_Read(INA226_CALIBRATION_REG,&reg);
	return reg;
}

/*	*/

void INA226_I2C_Write(uint8_t pByte, uint16_t Data)
{
	uint8_t reg[2];
	
	reg[0] = (Data & 0xFF00) >> 8;
	reg[1] =  Data & 0xFF;
	
	HAL_I2C_Mem_Write(&hi2c1, INA226_ADDRESS << 1, pByte, 1, reg, 2, 100);
	
}

void INA226_I2C_Read(uint8_t pByte, uint16_t* pData)
{
	uint8_t reg[2];

	HAL_I2C_Mem_Read(&hi2c1, INA226_ADDRESS << 1, pByte, 1, reg, 2, 100);
	*pData = (reg[0] << 8) | reg[1];
	
}

void INA226_I2C_ReadBuff(uint8_t pByte, uint8_t* pData, uint16_t size)
{
	HAL_I2C_Mem_Read(&hi2c1, INA226_ADDRESS << 1, pByte, 1, pData, size, 10);
}