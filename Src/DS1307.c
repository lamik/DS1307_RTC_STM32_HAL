/*
 * DS1307.c
 *
 *  Created on: 4.09.2019
 *      Author: Mateusz Salamon
 *		Contact: mateusz@msalamon.pl
 *
 *      Website:
 *      GitHub:
 */
#include "main.h"
#include "DS1307.h"

I2C_HandleTypeDef *hi2c_ds1307;
uint8_t Ds1307Buffer[7];

void DS1307_SetControlRegister(uint8_t Value)
{
	Value &= 0b10010011; // Put zeros where zero is needed
	HAL_I2C_Mem_Write(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_CONTROL, 1, &Value, 1, DS1307_I2C_TIMEOUT);
}

void DS1307_GetControlRegister(uint8_t *Value)
{
	HAL_I2C_Mem_Read(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_CONTROL, 1, Value, 1, DS1307_I2C_TIMEOUT);
}

void WriteBitToControlRegister(uint8_t BitNumber, uint8_t Value)
{
	uint8_t tmp;

	if(Value>1) Value = 1;

	DS1307_GetControlRegister(&tmp);
	tmp &= ~(1<<BitNumber);
	tmp |= (Value<<BitNumber);
	DS1307_SetControlRegister(tmp);
}
void DS1307_SQWEnable(uint8_t Enable)
{
	WriteBitToControlRegister(DS1307_CONTROL_SQUARE_WAVE_ENABLE, Enable);
}

void DS1307_SQWRateSelect(uint8_t Rate)
{
	uint8_t tmp;

	if(Rate>3) Rate = 3;

	DS1307_GetControlRegister(&tmp);
	tmp &= ~(3<<DS1307_CONTROL_RATE_SELECT_0);
	tmp |= (Rate<<DS1307_CONTROL_RATE_SELECT_0);
	DS1307_SetControlRegister(tmp);
}

void DS1307_OutputControl(uint8_t Enable)
{
	WriteBitToControlRegister(DS1307_CONTROL_OUTPUT_CONTROL, Enable);
}

void DS1307_ClockHalt(uint8_t Enable)
{
	uint8_t tmp;

	if(Enable>1) Enable = 1;

	HAL_I2C_Mem_Read(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_SECONDS, 1, &tmp, 1, DS1307_I2C_TIMEOUT);
	tmp &= ~(1<<7);
	tmp |= (Enable<<7);
	HAL_I2C_Mem_Write(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_SECONDS, 1, &tmp, 1, DS1307_I2C_TIMEOUT);
}

uint8_t bcd2dec(uint8_t BCD)
{
	return (((BCD & 0xF0)>>4) *10) + (BCD & 0xF);
}

uint8_t dec2bcd(uint8_t DEC)
{
	return ((DEC / 10)<<4) + (DEC % 10);
}

int dayofweek(int Day, int Month, int Year)
{
    int Y, C, M, N, D;
    M = 1 + (9 + Month) % 12;
    Y = Year - (M > 10);
    C = Y / 100;
    D = Y % 100;
    N = ((13 * M - 1) / 5 + D + D / 4 + 6 * C + Day + 5) % 7;
    return (7 + N) % 7;
}

void DS1307_SetDateTime(RTCDateTime *DateTime)
{
	uint8_t tmp[7];

	if(DateTime->Second > 59) DateTime->Second = 59;
	if(DateTime->Minute > 59) DateTime->Minute = 59;
	if(DateTime->Hour > 23) DateTime->Hour = 23;
	if(DateTime->Day > 31) DateTime->Day = 31;
	if(DateTime->Month > 12) DateTime->Month = 12;
	if(DateTime->Year> 2099) DateTime->Year = 2099;

	tmp[0] = dec2bcd(DateTime->Second);
	tmp[1] = dec2bcd(DateTime->Minute);
	tmp[2] = dec2bcd(DateTime->Hour);
	tmp[3] = dayofweek(DateTime->Day, DateTime->Month, DateTime->Year) + 1;
	tmp[4] = dec2bcd(DateTime->Day);
	tmp[5] = dec2bcd(DateTime->Month);
	tmp[6] = dec2bcd(DateTime->Year - 2000);

	HAL_I2C_Mem_Write(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_TIME, 1, tmp, 7, DS1307_I2C_TIMEOUT);
}

void DS1307_CalculateDateTime(RTCDateTime *DateTime)
{
	DateTime->Second = bcd2dec(Ds1307Buffer[0]);
	DateTime->Minute = bcd2dec(Ds1307Buffer[1]);
	DateTime->Hour = bcd2dec(Ds1307Buffer[2] & 0x3F);
	DateTime->DayOfWeek = Ds1307Buffer[3];
	DateTime->Day = bcd2dec(Ds1307Buffer[4]);
	DateTime->Month = bcd2dec(Ds1307Buffer[5] & 0x1F);
	DateTime->Year = 2000 + bcd2dec(Ds1307Buffer[6]);
}

#ifdef DS1307_USE_DMA
void DS1307_ReceiveDateTimeDMA(void)
{
	HAL_I2C_Mem_Read_DMA(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_TIME, 1, Ds1307Buffer, 7);
}
#else
void DS1307_GetDateTime(RTCDateTime *DateTime)
{
	HAL_I2C_Mem_Read(hi2c_ds1307, DS1307_ADDRESS, DS1307_REG_TIME, 1, Ds1307Buffer, 7, DS1307_I2C_TIMEOUT);

	void DS1307_CalculateDateTime(DateTime);
}
#endif

void DS1307_Init(I2C_HandleTypeDef *hi2c)
{
	hi2c_ds1307 = hi2c;

	DS1307_SQWRateSelect(SQW_RATE_1HZ);
	DS1307_SQWEnable(1);
	DS1307_ClockHalt(0);
}

