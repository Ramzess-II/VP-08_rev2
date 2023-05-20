

#ifndef EEPROM_H_
#define EEPROM_H_

#include <avr/io.h>



void EEPROM_write(uint16_t uiAddress, uint8_t ucData);
uint8_t EEPROM_read(uint16_t uiAddress);
uint32_t EEPROM_read_32t (uint16_t uiAddress);
void EEPROM_write_32t (uint16_t uiAddress, uint32_t ucData) ;
void EEPROM_write_float (uint16_t uiAddress, double ucData) ;
double EEPROM_read_float (uint16_t uiAddress) ;
uint16_t EEPROM_read_16t (uint16_t uiAddress);
void EEPROM_write_16t (uint16_t uiAddress, uint16_t ucData);
void init_EEprom (void);
void read_ALL_EEprom (void);
void init_err36 (void);
void eeprom_calib_sens (void);
void write_no_pay (void);



// тут храним адреса еепром памяти для всех нужных переменных
enum {
  EE_LIGHT           = 0x00,  // 8
  EE_DISCKRET1       = 0x01,  // 8
  EE_DISCKRET2       = 0x02,  // 8 
  EE_DISCKRET3       = 0x03,  // 8
  EE_SILL1           = 0x04,  // 32
  EE_SILL2           = 0x08,  // 32
  EE_SENNUMBER       = 0x0C,  // 8
  EE_TIPCD           = 0x0D,  // 8
  EE_BAUD1           = 0x0E,  // 8
  EE_BAUD2           = 0x0F,  // 8
  EE_PROTOCOL1       = 0x10,  // 8
  EE_PROTOCOL2       = 0x11,  // 8
  EE_FILTR           = 0x12,  // 8
  EE_ZEROSTART       = 0x13,  // 8
  EE_NPV             = 0x14,  // 32
  EE_KALIBKOEF       = 0x18,  // 32
  EE_ERRPR36         = 0x1C,  // 8
  EE_CAUNTERVEIGHT   = 0x20,  // 32
   //----------metrology----------// 
  EE_KALIBSENSOR1    = 0x28,  // 32
  EE_KALIBSENSOR2    = 0x2C,  // 32
  EE_KALIBSENSOR3    = 0x30,  // 32  
  EE_KALIBSENSOR4    = 0x34,  // 32
  EE_KALIBSENSOR5    = 0x38,  // 32
  EE_KALIBSENSOR6    = 0x3C,  // 32
  EE_KALIBSENSOR7    = 0x40,  // 32
  EE_KALIBSENSOR8    = 0x44,  // 32
  EE_KALIBSENSOR9    = 0x48,  // 32
  EE_KALIBSENSOR10   = 0x4C,  // 32
  EE_KALIBSENSOR11   = 0x50,  // 32
  EE_KALIBSENSOR12   = 0x54,  // 32
  
  EE_ZEROSENS1       = 0x58,  // 32
  EE_ZEROSENS2       = 0x5C,  // 32
  EE_ZEROSENS3       = 0x60,  // 32
  EE_ZEROSENS4       = 0x64,  // 32
  EE_ZEROSENS5       = 0x68,  // 32
  EE_ZEROSENS6       = 0x6C,  // 32
  EE_ZEROSENS7       = 0x70,  // 32
  EE_ZEROSENS8       = 0x74,  // 32
  EE_ZEROSENS9       = 0x78,  // 32
  EE_ZEROSENS10      = 0x7C,  // 32
  EE_ZEROSENS11      = 0x80,  // 32
  EE_ZEROSENS12      = 0x84,  // 32
  
 //----------что добавил после----------//  
  EE_TIM_STAB        = 0x88,  // 8
  EE_OF_ADC          = 0x89,  // 8
  
  
  EE_DATE1            = 0x100,  // 32
  EE_MOUNT1           = 0x104,  // 32
  EE_YEAR1            = 0x108,  // 32
  EE_PSWRD1           = 0x10C,  // 32
     };



#endif /* EEPROM_H_ */