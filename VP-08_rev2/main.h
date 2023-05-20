
#ifndef MAIN_H_
#define MAIN_H_

//------------------------------------------------------------------------------
// подключаем файлы
//------------------------------------------------------------------------------

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>



//------------------------------------------------------------------------------
// дефайним порты
//------------------------------------------------------------------------------
//#define DEBUG_MOD

#define LEDON            PORTB |= 0x80
#define LEDOFF           PORTB &= ~0x80
#define LEDIVERT         PORTB ^= 0x80

#define SSON             PORTB |= 0x01
#define SSOFF            PORTB &= ~0x01

#define BZRON            PORTD |= 0x10
#define BZROFF           PORTD &= ~0x10

#define LMON             PORTJ |= 0x20   // питание самого прибора
#define LMOFF            PORTJ &= ~0x20

#define PIN_OFF        ~ PINJ &  0x40    // проверка на 0
#define PIN_OFF_OF       PINJ &  0x40    // проверка на 1

#define TENZO_ON         PORTG |= 0x20   // питание датчиков
#define TENZO_OFF        PORTG &= ~0x20

#define BLK            ~ PINB &  0x80
#define INIT_KEY       ~ PINL &  0x08

#define TIM_ON           1000       // 100-2000       время удержания кнопки включения для включения
#define TIM_OFF          6        // 10-250         время удержания кнопки включения для выключения
#define BOUNCE           220        // время антидребезга кнопок
#define LONG_BOUNCE      200       // скорость с которой инкрементируются числа при удержании кнопки
#define SPEED_BOUNCE     10        // скорость с которой инкрементируются числа после длительного удержания кнопки
#define TIM_TO_SPEED     8          // сколько чисел должно пройти пока не начнется ускорение счетчика кнопок

//-------------------кнопки---------------------//
#define KEY_UP           0x3E
#define KEY_DOWN         0x1F//0x3D
#define KEY_RIGHT        0x2F//0x3B
#define KEY_LEFT         0x37
#define KEY_ENTER        0x3B // 0x2F
#define KEY_ESC          0x3D//0x1F
//-------------------пароли---------------------//
#define PASWORD_ADMIN    1125
#define PASWORD_SUPER    6235
#define PASWORD_INIT     1125
#define PASWORD_ERR36    8791

struct filtros
{
     int32_t sens1;
     int32_t sens2;
     int32_t sens3;
     int32_t sens4;
     int32_t sens5;
     int32_t sens6;
     int32_t sens7;
     int32_t sens8;
     int32_t sens9;
     int32_t sens10;
     int32_t sens11;
     int32_t sens12;
    };
    
const char er_sens0[] PROGMEM = "ERR0.01";
const char er_sens1[] PROGMEM = "ERR0.02";
const char er_sens2[] PROGMEM = "ERR0.03";
const char er_sens3[] PROGMEM = "ERR0.04";
const char er_sens4[] PROGMEM = "ERR0.05";
const char er_sens5[] PROGMEM = "ERR0.06";
const char er_sens6[] PROGMEM = "ERR0.07";
const char er_sens7[] PROGMEM = "ERR0.08";
const char er_sens8[] PROGMEM = "ERR0.09";
const char er_sens9[] PROGMEM = "ERR0.10";
const char er_sens10[] PROGMEM = "ERR0.11";
const char er_sens11[] PROGMEM = "ERR0.12";
const char er_sens12[] PROGMEM = " VP.08 ";

// а это массив указателей на строки
const char *const er_sens[] PROGMEM = {
 er_sens0, er_sens1,er_sens2,er_sens3,er_sens4,er_sens5,er_sens6,er_sens7,er_sens8,er_sens9,er_sens10,er_sens11,er_sens12,
};


//------------------------------------------------------------------------------
// инициализируем функции
//------------------------------------------------------------------------------
void adc_read_of (void);
void init_atmega (void);
void start (void);
void zastavka (void);
void off_key (void);
void enter_password_init (void);
void init_after_EEprom (void);
uint32_t diskret (uint32_t data, uint8_t dskrt);
void converting_cod_to_massa (void);
void TABLO_print(int32_t ves);
void zero_funk (uint8_t eprom_ok);
void filtr_massa (uint8_t power);
void no_pay_wizard (void);
void err_and_led (void);


#endif 