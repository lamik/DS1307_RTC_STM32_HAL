/*
 * DS1307.h
 *
 * 	The MIT License.
 *  Created on: 4.09.2019
 *      Author: Mateusz Salamon
 *		Contact: mateusz@msalamon.pl
 *
  *      Website: https://msalamon.pl/dalsze-zmagania-z-rtc-ds1307-i-pcf8563-na-stm32/
  *      GitHub: https://github.com/lamik/DS1307_RTC_STM32_HAL
 */

//
//	Uncomment when you are using DMA reading
//
#define DS1307_USE_DMA

#define DS1307_ADDRESS              (0x68<<1)
#define DS1307_I2C_TIMEOUT			100

#define DS1307_REG_TIME             0x00
#define DS1307_REG_SECONDS          0x00
#define DS1307_REG_MINUTES          0x01
#define DS1307_REG_HOURS          	0x02
#define DS1307_REG_DAY              0x03
#define DS1307_REG_DATE             0x04
#define DS1307_REG_MONTH            0x05
#define DS1307_REG_YEAR             0x06
#define DS1307_REG_CONTROL          0x07
#define DS1307_REG_RAM_START        0x08
#define DS1307_REG_RAM_END     		0x3F

//
//	Controll register 0x07
//
#define DS1307_CONTROL_OUTPUT_CONTROL			7
#define DS1307_CONTROL_SQUARE_WAVE_ENABLE		4
#define DS1307_CONTROL_RATE_SELECT_1			1
#define DS1307_CONTROL_RATE_SELECT_0			0

typedef enum
{
	SQW_RATE_1HZ 		= 0,
	SQW_RATE_4096HZ 	= 1,
	SQW_RATE_8192HZ 	= 2,
	SQW_RATE_32768HZ 	= 3
}SQW_Rate;

typedef struct
{
	uint16_t 	Year;
	uint8_t  	Month;
	uint8_t		Day;
	uint8_t		Hour;
	uint8_t		Minute;
	uint8_t		Second;
	uint8_t		DayOfWeek;
}RTCDateTime;

void DS1307_SQWEnable(uint8_t Enable);
void DS1307_SQWRateSelect(uint8_t Rate);
void DS1307_OutputControl(uint8_t Enable);
void DS1307_ClockHalt(uint8_t Enable);

void DS1307_ReadRAM(uint8_t Address, uint8_t *Value, uint8_t Length);
void DS1307_WriteRAM(uint8_t Address, uint8_t *Value, uint8_t Length);

void DS1307_SetDateTime(RTCDateTime *DateTime);
#ifdef DS1307_USE_DMA
void DS1307_ReceiveDateTimeDMA(void);	// Use in DS1307 Interrupt handler
void DS1307_CalculateDateTime(RTCDateTime *DateTime);	// Use in DMA Complete Receive interrupt
#else
void DS1307_GetDateTime(RTCDateTime *DateTime);	// Use in blocking/interrupt mode in DS1307_INT EXTI handler
#endif
void DS1307_Init(I2C_HandleTypeDef *hi2c);
