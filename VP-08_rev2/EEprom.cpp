#include "Eeprom.h"
#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"


extern struct parametrs param;
extern struct metrologis metrology;

void EEPROM_write(uint16_t uiAddress, uint8_t ucData)      // функция записи в еепром 8 битного числа  по заданному адресу
{
	cli();
	while(EECR & (1<<EEPE)){}       //ждем освобождения флага окончания последней операцией с памятью
	EEAR = uiAddress;               //Устанавливаем адрес
	EEDR = ucData;                  //Пищем данные в регистр
	EECR |= (1<<EEMPE);             //Разрешаем запись
	EECR |= (1<<EEPE);              //Пишем байт в память по русски 
	sei();
}

uint8_t EEPROM_read(uint16_t uiAddress)                     // функция чтения 8 битного числа из еепром
{
	cli();
	while(EECR & (1<<EEPE)){}                              //ждем освобождения флага окончания последней операцией с памятью
	EEAR = uiAddress;                                      //Устанавливаем адрес
	EECR |= (1<<EERE);                                     //Запускаем операцию считывания из памяти в регистр данных
	sei();
	return EEDR; //Возвращаем результат
}

uint32_t EEPROM_read_32t (uint16_t uiAddress)                // функция чтения 32битного числа из еепром по заданному дресу
{
	static uint32_t receive_data;                            // создали переменную
	receive_data = EEPROM_read (uiAddress+0x03);             // считали в нее старшый байт
	receive_data = receive_data << 8;                        // сдвинули освободив место под младший
	receive_data |= EEPROM_read (uiAddress+0x02);            // считали второй байт и присвоили без изменения других битов
	receive_data = receive_data << 8;                        // сдвинули
	receive_data |= EEPROM_read (uiAddress+0x01);
	receive_data = receive_data << 8;
	receive_data |= EEPROM_read (uiAddress);
	return receive_data;                                     // вернули итог
}

void EEPROM_write_32t (uint16_t uiAddress, uint32_t ucData)   // запись 32 битного числа
{
	EEPROM_write (uiAddress,ucData);                          // в обратном порядке складываем в память
	EEPROM_write (uiAddress + 0x01 ,ucData >> 8);
	EEPROM_write (uiAddress + 0x02 ,ucData >> 16);
	EEPROM_write (uiAddress + 0x03 ,ucData >> 24);
}

void EEPROM_write_float (uint16_t uiAddress, double ucData)        // запись float. 
{
     static union                                                  // создаем объеденение
     { 
         double in_float;                                          // два числа которые начинаются с одного адресса но имеют разный тип
         uint32_t out_float;                                       // мы флоат записываем как флоат, а потом при записи в еепром уже пользуемся 32 бит переменной
     }floatchik;
     floatchik.in_float = ucData;                                 
     EEPROM_write (uiAddress,floatchik.out_float);                          // в обратном порядке складываем в память
     EEPROM_write (uiAddress + 0x01 ,floatchik.out_float >> 8);
     EEPROM_write (uiAddress + 0x02 ,floatchik.out_float >> 16);
     EEPROM_write (uiAddress + 0x03 ,floatchik.out_float >> 24);
}

double EEPROM_read_float (uint16_t uiAddress)                       // вычитываем float
{
     static union                                                   // так же само, создаем объеденение, и обращаемся к одним и тем же данным по разному
     {
          double in_float;
          uint32_t out_float;
     }floatchik;
	floatchik.out_float = EEPROM_read (uiAddress+0x03);                // считали в нее старшый байт
	floatchik.out_float = floatchik.out_float << 8;                    // сдвинули освободив место под младший
	floatchik.out_float |= EEPROM_read (uiAddress+0x02);               // считали второй байт и присвоили без изменения других битов
	floatchik.out_float = floatchik.out_float << 8;                    // сдвинули
	floatchik.out_float |= EEPROM_read (uiAddress+0x01);
	floatchik.out_float = floatchik.out_float << 8;
	floatchik.out_float |= EEPROM_read (uiAddress);
	return floatchik.in_float;                                         // вернули итог типа флоат
}

uint16_t EEPROM_read_16t (uint16_t uiAddress)                 // тоже самое только для 16 бит
{
	static uint32_t receive_data;
	receive_data |= EEPROM_read (uiAddress+0x01);
	receive_data = receive_data << 8;
	receive_data |= EEPROM_read (uiAddress);
	return receive_data;
}

void EEPROM_write_16t (uint16_t uiAddress, uint16_t ucData)
{
	EEPROM_write (uiAddress,ucData);
	EEPROM_write (uiAddress + 0x01 ,ucData >> 8);
}


void init_EEprom (void)                                       // инициализация еепром, забиваем стандартные значения
{
    EEPROM_write(EE_LIGHT,2);
    EEPROM_write(EE_DISCKRET1,2);
    EEPROM_write(EE_DISCKRET2,5);
    EEPROM_write(EE_DISCKRET3,10);
    EEPROM_write_32t(EE_SILL1,10000);
    EEPROM_write_32t(EE_SILL2,50000);
    EEPROM_write(EE_SENNUMBER,8);
    EEPROM_write(EE_TIPCD,1);
    EEPROM_write(EE_BAUD1,2);
    EEPROM_write(EE_BAUD2,2);
    EEPROM_write(EE_PROTOCOL1,1);
    EEPROM_write(EE_PROTOCOL2,1);
    EEPROM_write(EE_FILTR,4);
    EEPROM_write(EE_ZEROSTART,1);
    EEPROM_write_32t(EE_NPV, 80000);
    EEPROM_write_32t(EE_CAUNTERVEIGHT,0);
    EEPROM_write_float(EE_KALIBKOEF,1.000);
    EEPROM_write(EE_TIM_STAB,10);
    EEPROM_write(EE_OF_ADC,0);
    
 //----------metrology----------//
    EEPROM_write_float(EE_KALIBSENSOR1,1.000);
    EEPROM_write_float(EE_KALIBSENSOR2,1.000);
    EEPROM_write_float(EE_KALIBSENSOR3,1.000);
    EEPROM_write_float(EE_KALIBSENSOR4,1.000);
    EEPROM_write_float(EE_KALIBSENSOR5,1.000);
    EEPROM_write_float(EE_KALIBSENSOR6,1.000);
    EEPROM_write_float(EE_KALIBSENSOR7,1.000);
    EEPROM_write_float(EE_KALIBSENSOR8,1.000);
    EEPROM_write_float(EE_KALIBSENSOR9,1.000);
    EEPROM_write_float(EE_KALIBSENSOR10,1.000);
    EEPROM_write_float(EE_KALIBSENSOR11,1.000);
    EEPROM_write_float(EE_KALIBSENSOR12,1.000);
    
    EEPROM_write_32t(EE_ZEROSENS1,8388608);
    EEPROM_write_32t(EE_ZEROSENS2,8388608);
    EEPROM_write_32t(EE_ZEROSENS3,8388608);
    EEPROM_write_32t(EE_ZEROSENS4,8388608);
    EEPROM_write_32t(EE_ZEROSENS5,8388608);
    EEPROM_write_32t(EE_ZEROSENS6,8388608);
    EEPROM_write_32t(EE_ZEROSENS7,8388608);
    EEPROM_write_32t(EE_ZEROSENS8,8388608);
    EEPROM_write_32t(EE_ZEROSENS9,8388608);
    EEPROM_write_32t(EE_ZEROSENS10,8388608);
    EEPROM_write_32t(EE_ZEROSENS11,8388608);
    EEPROM_write_32t(EE_ZEROSENS12,8388608);
     
}

void eeprom_calib_sens (void)
{
    EEPROM_write_float(EE_KALIBSENSOR1,metrology.koef_sensor1);
    EEPROM_write_float(EE_KALIBSENSOR2,metrology.koef_sensor2);
    EEPROM_write_float(EE_KALIBSENSOR3,metrology.koef_sensor3);
    EEPROM_write_float(EE_KALIBSENSOR4,metrology.koef_sensor4);
    EEPROM_write_float(EE_KALIBSENSOR5,metrology.koef_sensor5);
    EEPROM_write_float(EE_KALIBSENSOR6,metrology.koef_sensor6);
    EEPROM_write_float(EE_KALIBSENSOR7,metrology.koef_sensor7);
    EEPROM_write_float(EE_KALIBSENSOR8,metrology.koef_sensor8);
    EEPROM_write_float(EE_KALIBSENSOR9,metrology.koef_sensor9);
    EEPROM_write_float(EE_KALIBSENSOR10,metrology.koef_sensor10);
    EEPROM_write_float(EE_KALIBSENSOR11,metrology.koef_sensor11);
    EEPROM_write_float(EE_KALIBSENSOR12,metrology.koef_sensor12);     
}   

void read_ALL_EEprom (void)                                   // считываем значение из еепром при старте для всех переменных 
{
    	param.discret1              = EEPROM_read(EE_DISCKRET1);
    	param.discret2              = EEPROM_read(EE_DISCKRET2);;
    	param.discret3              = EEPROM_read(EE_DISCKRET3);;
    	param.sill1                 = EEPROM_read_32t(EE_SILL1);
    	param.sill2                 = EEPROM_read_32t(EE_SILL2);
    	param.NPV                   = EEPROM_read_32t(EE_NPV);
    	param.sensor_number         = EEPROM_read(EE_SENNUMBER);
    	param.tip_cd                = EEPROM_read(EE_TIPCD);
    	param.baud1                 = EEPROM_read(EE_BAUD1);
    	param.baud2                 = EEPROM_read(EE_BAUD2);
    	param.protokol1             = EEPROM_read(EE_PROTOCOL1);
    	param.protokol2             = EEPROM_read(EE_PROTOCOL2);
    	param.lighte                = EEPROM_read(EE_LIGHT);
    	param.filtr                 = EEPROM_read(EE_FILTR);
    	param.calib_koef            = EEPROM_read_float(EE_KALIBKOEF);
        param.err36                 = EEPROM_read(EE_ERRPR36);
        param.caunter_weight        = EEPROM_read_32t(EE_CAUNTERVEIGHT);
        param.tim_stab              = EEPROM_read(EE_TIM_STAB);
        param.of_adc                = EEPROM_read(EE_OF_ADC);
        param.data_err              = EEPROM_read_32t(EE_DATE1);
        param.mount_err             = EEPROM_read_32t(EE_MOUNT1);
        param.year_err              = EEPROM_read_32t(EE_YEAR1);
        param.pasw_err              = EEPROM_read_32t(EE_PSWRD1);
        
             //----------metrology----------//       
        metrology.koef_sensor1      = EEPROM_read_float(EE_KALIBSENSOR1);
        metrology.koef_sensor2      = EEPROM_read_float(EE_KALIBSENSOR2);
        metrology.koef_sensor3      = EEPROM_read_float(EE_KALIBSENSOR3);
        metrology.koef_sensor4      = EEPROM_read_float(EE_KALIBSENSOR4);
        metrology.koef_sensor5      = EEPROM_read_float(EE_KALIBSENSOR5);
        metrology.koef_sensor6      = EEPROM_read_float(EE_KALIBSENSOR6);
        metrology.koef_sensor7      = EEPROM_read_float(EE_KALIBSENSOR7);
        metrology.koef_sensor8      = EEPROM_read_float(EE_KALIBSENSOR8);
        metrology.koef_sensor9      = EEPROM_read_float(EE_KALIBSENSOR9);
        metrology.koef_sensor10     = EEPROM_read_float(EE_KALIBSENSOR10);
        metrology.koef_sensor11     = EEPROM_read_float(EE_KALIBSENSOR11);
        metrology.koef_sensor12     = EEPROM_read_float(EE_KALIBSENSOR12);
        
        metrology.cod_zero_sensor1  = EEPROM_read_32t(EE_ZEROSENS1);
        metrology.cod_zero_sensor2  = EEPROM_read_32t(EE_ZEROSENS2);
        metrology.cod_zero_sensor3  = EEPROM_read_32t(EE_ZEROSENS3);
        metrology.cod_zero_sensor4  = EEPROM_read_32t(EE_ZEROSENS4);
        metrology.cod_zero_sensor5  = EEPROM_read_32t(EE_ZEROSENS5);
        metrology.cod_zero_sensor6  = EEPROM_read_32t(EE_ZEROSENS6);
        metrology.cod_zero_sensor7  = EEPROM_read_32t(EE_ZEROSENS7);
        metrology.cod_zero_sensor8  = EEPROM_read_32t(EE_ZEROSENS8);
        metrology.cod_zero_sensor9  = EEPROM_read_32t(EE_ZEROSENS9);
        metrology.cod_zero_sensor10 = EEPROM_read_32t(EE_ZEROSENS10);
        metrology.cod_zero_sensor11 = EEPROM_read_32t(EE_ZEROSENS11);
        metrology.cod_zero_sensor12 = EEPROM_read_32t(EE_ZEROSENS12);
        
}

void init_err36 (void)                                      // с помощью спец пароля можно сбросить ерр36 (для разработчиков) 
{
     EEPROM_write(EE_ERRPR36,0);
     EEPROM_write_32t(EE_DATE1, 01);
     EEPROM_write_32t(EE_MOUNT1, 01);
     EEPROM_write_32t(EE_YEAR1, 99);
     EEPROM_write_32t(EE_PSWRD1,888888);
}

void write_no_pay (void)                                  // запись в еепром даты, месяца и года срабатывания блокировки                 
{
   EEPROM_write_32t(EE_DATE1,param.data_err);
   EEPROM_write_32t(EE_MOUNT1,param.mount_err);
   EEPROM_write_32t(EE_YEAR1,param.year_err);  
}
