
/*
 * si7021.c
 *
 *  Created on: Jan 14, 2022
 *      Author: alarm
 */
#include "si7021.h"

#ifdef SET_SI7021

//-----------------------------------------------------------------------------

result_t sens = {0.0};
uint8_t si_ack[8] = {0};
bool si_dump = false;
const char *_siON = "sion";
const char *_siOFF = "sioff";

//-----------------------------------------------------------------------------
//  Функция подсчитывает контрольную сумму байтов (полином P(x)=x^8+x^5+x^4+1 = 0x131)
//
uint8_t SF04_CheckCrc(uint8_t *data, uint8_t len)
{
uint8_t i, crc = 0;

	for (i = 0; i < len; ++i) {
		crc ^= data[i];
		for (uint8_t bit = 8; bit > 0; --bit) {
			if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
					   else crc = (crc << 1);
		}
	}

	return crc;
}
//-----------------------------------------------------------------------------
//  Функция выполняет программный сброс датчика
//
void si7021_reset()
{
uint8_t reg = SI_RESET;
//uint32_t sch = min_wait_ms;

	//while (!i2c_txReady && --sch) { HAL_Delay(1); }
	//if (!sch) devError |= HAL_TIMEOUT;
	//else {
		i2c_txReady = 0;
		if (HAL_I2C_Master_Transmit(portI2C, SI7021_ADDRESS << 1, &reg, 1, min_wait_ms) != HAL_OK) devError |= devI2C;
	//}
}
//-----------------------------------------------------------------------------
//   Функция читает данные параметра влажность (2 байта + crc)
//    и проверяет корректность контрольной суммы прочитанного параметра
//   При корректной сумме возвращает 'сырое' значение влажности,
//   иначе устанавливает ошибку в байте состояния устройства devError
//
void si7021_read_raw_rh(uint8_t *data_h, uint8_t *data_l)
{
uint8_t reg = SI_HUMI_READ;

	HAL_StatusTypeDef rt = HAL_I2C_Master_Transmit(portI2C, SI7021_ADDRESS << 1, &reg, 1, min_wait_ms);
	i2c_rxReady = 0;
    rt |= HAL_I2C_Master_Receive_DMA(portI2C, SI7021_ADDRESS << 1, si_ack, 3);
    uint32_t sch = min_wait_ms;
    while (!i2c_rxReady && --sch) { HAL_Delay(1); }
    if (!sch) rt = HAL_TIMEOUT;
	//rt |= HAL_I2C_Master_Receive(portI2C, SI7021_ADDRESS << 1, ack, sizeof(ack), max_wait_ms);
	//i2c_rxReady = 1;
    if (!rt) {
    	si_ack[3] = SF04_CheckCrc(si_ack, 2);
    	if (si_ack[3] == si_ack[2]) {
    		*data_h = si_ack[0];
    		*data_l = si_ack[1];
    	} else devError |= devCRC;
    }
    devError |= rt;
}
//----------------------------------------------------------------------------------
//   Функция читает данные параметра температура (2 байта + crc)
//    и проверяет корректность контрольной суммы прочитанного параметра
//   При корректной сумме возвращает 'сырое' значение температуры,
//   иначе устанавливает ошибку в байте состояния устройства devError
//
void si7021_read_raw_temp(uint8_t *data_h, uint8_t *data_l)
{
uint8_t reg = SI_TEMP_READ;
uint8_t *uk = &si_ack[4];

	HAL_StatusTypeDef rt = HAL_I2C_Master_Transmit(portI2C, SI7021_ADDRESS << 1, &reg, 1, min_wait_ms);
	i2c_rxReady = 0;
	rt |= HAL_I2C_Master_Receive_DMA(portI2C, SI7021_ADDRESS << 1, uk, 3);
	uint32_t sch = max_wait_ms;
	while (!i2c_rxReady && --sch) { HAL_Delay(1); }
	if (!sch) rt = HAL_TIMEOUT;
	//rt |= HAL_I2C_Master_Receive(portI2C, SI7021_ADDRESS << 1, ack, sizeof(ack), max_wait_ms);
	//i2c_rxReady = 1;
	if (!rt) {
		si_ack[7] = SF04_CheckCrc(uk, 2);
		if (si_ack[7] == *(uk + 2)) {
			*data_h = *uk++;
			*data_l = *uk;
		} else devError |= devCRC;
	}
	devError |= rt;
}
//----------------------------------------------------------------------------------
//   Функция пересчитывает 'сырое' значение температуры в градусы Цельсия
//
void si7021_read_temp()
{
uint8_t dataH, dataL;
float val;

	si7021_read_raw_temp(&dataH, &dataL);
	if (devError & devI2C) {
		sens.temp = 0.0;
	} else {
		val = dataH * 256 + dataL;
		sens.temp = ((val * 175.72) / 65536.0) - 46.85;
	}
}
//----------------------------------------------------------------------------------
//   Функция пересчитывает 'сырое' значение влажности в проценты
//
void si7021_read_humi()
{
uint8_t dataH, dataL;
float val;

	si7021_read_raw_rh(&dataH, &dataL);
	if (devError & devI2C) {
		sens.humi = 0.0;
	} else {
		val = dataH * 256 + dataL;
		sens.humi = ((val * 125.0)/65536.0) - 6.0;
	}
}
//-----------------------------------------------------------------------------

#endif
