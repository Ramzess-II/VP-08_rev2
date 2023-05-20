#include "RTC1307.h"

bool false_I2C;            // на будующее флаг ошибки I2C
struct DS1307 DS;          // объявляем структуру с данными для часов


void I2C_init (void)       // инициализируем I2C
{
     TWBR=0x20;  // 12Khz
}

void I2C_StartCondition(void)       // отправка стартового бита
{
     TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
     while(!(TWCR&(1<<TWINT)));//подождем пока установится TWIN // тоесть ответ от микры 
}

void I2C_StopCondition(void)        // отправка стоп бита
{
     TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);     // записывем один в флаговый бит, включаем стоп, и запускаем I2C
}

void I2C_SendByte(uint8_t c)              // передача байта
{
     TWDR = c;//запишем байт в регистр данных
     TWCR = (1<<TWINT)|(1<<TWEN);//включим передачу байта
     while (!(TWCR & (1<<TWINT)));//подождем пока установится TWIN
}

void I2C_write_ds1338 (void)            // это запись в часы (важен последний бит)
{
     I2C_SendByte (0b11010000);
     if ((TWSR & 0xF8) != TW_MT_DATA_ASK) {false_I2C = true;}     // тут еще отслеживаем ошибку
}

void I2C_read_ds1338 (void)             // это чтение из часов (важен последний бит) тут он единица
{
     I2C_SendByte (0b11010001);
     if ((TWSR & 0xF8) != TW_MT_DATA_ASK) {false_I2C = true;}
}

uint8_t I2C_ReadByte(void)              // это чтение по I2C
{
     TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);  // тут разрешаем отправлять подтверждение после принятого байта
     while (!(TWCR & (1<<TWINT)));//ожидание установки бита TWIN
     return TWDR;//читаем регистр данных
}

uint8_t I2C_ReadLastByte(void)           // это чтение последнего байта, 
{
     TWCR = (1<<TWINT)|(1<<TWEN);        // тут не отправляем бит подтверждения
     while (!(TWCR & (1<<TWINT)));//ожидание установки бита TWIN
     return TWDR;//читаем регистр данных
}

void DS_Write_time (void)                // функция записи времени в микру
{
     I2C_StartCondition();               // отправим старт
     I2C_write_ds1338();                 // отправим адрес микры и бит записи
     I2C_SendByte (0x00);                // отправим регистр с которого начнем запись
     I2C_SendByte (C_BinDec(DS.sec));    //секунды + преобразование в понятное для часов
     I2C_SendByte (C_BinDec(DS.min));    //минуты
     I2C_SendByte (C_BinDec(DS.hour));   //часы
     I2C_StopCondition();                // отправим стоп
}

void DS_Write_date (void)                // функция записи даты
{
     I2C_StartCondition();               // отправим старт
     I2C_write_ds1338();                 // отправим адрес микры и бит записи
     I2C_SendByte (0x04);                // отправим регистр с которого записываем
     I2C_SendByte (C_BinDec(DS.date));   //дата
     I2C_SendByte (C_BinDec(DS.month));  //месяц
     I2C_SendByte (C_BinDec(DS.year));   //год
     I2C_StopCondition();                // стоп
}

void DS_Write_init_date (void)           // при включении считать часы
{
     DS_Read ();
     if (DS.sec == 80) {                 // если была вытянута батарейка то бит работы сброшен, значит проведем инициализацию
          I2C_StartCondition();          // старт
          I2C_write_ds1338();            // адрес микры и бит записи
          I2C_SendByte (0x00);           // адрес с которого начинаем писать
          I2C_SendByte (C_BinDec(10));   //секунды
          I2C_SendByte (C_BinDec(10));   //минуты
          I2C_SendByte (C_BinDec(10));   //часы
          I2C_SendByte (C_BinDec(1));    //дни
          I2C_SendByte (C_BinDec(01));   //дата
          I2C_SendByte (C_BinDec(01));   //месяц
          I2C_SendByte (C_BinDec(90));   //год 90 для удобного отслеживания
          I2C_StopCondition();           // отпавим стоп
     }
}

void DS_Read(void)                       // чтение из микры часов
{
     I2C_StartCondition();               // отправим старт
     I2C_write_ds1338();                 // отправим адрес микры и бит записи
     I2C_SendByte (0x00);                // отправим адрес с которого читать
     I2C_StopCondition();                // отправим стоп
     I2C_StartCondition();               // отправим старт    // тут вроде можно отправлять повторный старт хз
     I2C_read_ds1338();                  // отправим адрес микры и бит чтения
     DS.sec = I2C_ReadByte(); DS.min = I2C_ReadByte(); DS.hour = I2C_ReadByte();    // считываем всю фигню
     DS.day = I2C_ReadByte(); DS.date = I2C_ReadByte(); DS.month = I2C_ReadByte();
     DS.year = I2C_ReadLastByte();       // по особому читаем последний байт
     I2C_StopCondition();                // стоп
     //преобразуем в десятичный формат
     DS.sec = C_Dec(DS.sec); DS.min = C_Dec(DS.min);
     DS.hour &= ~0xC0;
     DS.hour = C_Dec(DS.hour);
     DS.year = C_Dec(DS.year); DS.month = C_Dec(DS.month);
     DS.date = C_Dec(DS.date);
}

uint8_t C_Dec(uint8_t data)      // функция для преобразования данных из микросхемы в нормальные символы
{
     return((data>>4)*10+(0x0F&data));
}

uint8_t C_BinDec(uint8_t data)   // функция для преобразования из нормальных символов в микросхемопонятную кодировку
{
     return((data/10)<<4)|(data%10);
}