

#ifndef RTC1307_H_
#define RTC1307_H_

#include <avr/io.h>
#define TW_MT_DATA_ASK 0x28 // Ведущий передал данные и ведомый подтвердил прием
#define TW_MR_DATA_ASK 0x50 // Ведущий принял данные и передал подтверждение
#define TW_MR_DATA_NASK 0x58 // Ведущий передал данные и ведомый подтвердил прием


void I2C_init (void);
void I2C_StartCondition(void);
void I2C_StopCondition(void);
void I2C_SendByte(uint8_t c);


void I2C_write_ds1338 (void);
void I2C_read_ds1338 (void);

uint8_t I2C_ReadByte(void);
uint8_t I2C_ReadLastByte(void);

void DS_Write_time (void);
void DS_Write_date (void);
void DS_Write_init_date (void);
void DS_Read(void) ;
uint8_t C_BinDec(uint8_t data);
uint8_t C_Dec(uint8_t data);

struct DS1307            // структура для хранения данных из микры часов
{
     unsigned int sec,min,hour,day,
     date,month,year;
};

#endif /* RTC1307_H_ */

