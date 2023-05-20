
#ifndef UART_H_
#define UART_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/delay.h>


void init_UART_tenzo(void);
void init_UART_tablo(void);
void init_UART_0_1 (uint8_t namber1, uint8_t namber2);
void init_Uart_0_1_start (void);
void uart0_send_string ( char *send0);
void uart1_send_string ( char *send1);
void uart2_send_string ( char *send2);
void uart3_send_string ( char *send3);
void get_zemik_sensors (uint8_t amount);
uint8_t get_number_sensor (void);
void send_new_adr (uint8_t last, uint8_t news);
void reply_visl (void);
void parser_visl (void);

uint32_t parser_buf_zemic (char *send);

struct bool_date {
  uint32_t errors:1 ;
  uint32_t led_display:3;
  uint32_t tara_ok:1;
  uint32_t first_zero:1;
  uint32_t avto_zero:1;
  uint32_t non_on:1;
  uint32_t main_menu_bool:1;
  uint32_t caunt_err:3;
  uint32_t non_rep:1;
  uint32_t change_item_mirr:1;
  uint32_t err_adc:1;
  uint32_t start_delay:1;
  uint32_t npv:1;
  uint32_t led_in_menu:1;
  uint32_t stop_spi:1;
  uint32_t stab:1;
  uint32_t funck1:1;
  uint32_t funck2:1;
  uint32_t funck3:1;
  uint32_t state_zero:2;
  uint32_t uart_tara:1;
  uint32_t uart_zero:1;
  uint32_t data_ok:1;
  uint32_t busy:1;
  
  };


enum                  // перечисление скоростей юарта
{
     B19200 = 51,
     B9600 = 103,
     B2400 = 416,
     B600 =  1666,
};

enum                  // это посылки которые запишем во флеш
{
     S98_MSV,
     S01,
     S02,
     S03,
     S04,
     S05,
     S06,
     S07,
     S08,
     S09,
     S10,
     S11,
     S12,
     TDD,
     AOK,
     ANOK,
     TOK,
     TNOK,
     XOK,
     XNOK,
     
     
};

// собственно их объ€вление
const char uart_string1[]  PROGMEM =  {0x53,0x39,0x38,0x3B,0x4D,0x53,0x56,0x3F,0x31,0x3B,0x24,0x00};     //;S98;MSV?1; это в вп. а по факту S98;MSV?1; +$
const char uart_string2[]  PROGMEM =  {0x53,0x30,0x31,0x3B,0x24,0x00};      // S01;
const char uart_string3[]  PROGMEM =  {0x53,0x30,0x32,0x3B,0x24,0x00};      // S02;
const char uart_string4[]  PROGMEM =  {0x53,0x30,0x33,0x3B,0x24,0x00};      // S03;
const char uart_string5[]  PROGMEM =  {0x53,0x30,0x34,0x3B,0x24,0x00};      // S04; 
const char uart_string6[]  PROGMEM =  {0x53,0x30,0x35,0x3B,0x24,0x00};      // S05;     
const char uart_string7[]  PROGMEM =  {0x53,0x30,0x36,0x3B,0x24,0x00};      // S06;     
const char uart_string8[]  PROGMEM =  {0x53,0x30,0x37,0x3B,0x24,0x00};      // S07; 
const char uart_string9[]  PROGMEM =  {0x53,0x30,0x38,0x3B,0x24,0x00};      // S08;                     
const char uart_string10[] PROGMEM =  {0x53,0x30,0x39,0x3B,0x24,0x00};      // S09;   
const char uart_string11[] PROGMEM =  {0x53,0x31,0x30,0x3B,0x24,0x00};      // S10;  
const char uart_string12[] PROGMEM =  {0x53,0x31,0x31,0x3B,0x24,0x00};      // S11; 
const char uart_string13[] PROGMEM =  {0x53,0x31,0x32,0x3B,0x24,0x00};      // S12; 
const char uart_string14[] PROGMEM =  {";S98;TDD1;$"};      // TDD;      
const char uart_string15[] PROGMEM =  {0x02,0x41,0x4F,0x4B,0x34,0x35,0x03,0x24,0x00};           // AOK   обнуление удачно
const char uart_string16[] PROGMEM =  {0x02,0x41,0x4E,0x4F,0x4B,0x30,0x42,0x03,0x24,0x00};      // ANOK  обнуление не удачно             ????????? 0x30, 0x42 --- 0B???
const char uart_string17[] PROGMEM =  {0x02,0x54,0x4F,0x4B,0x35,0x30,0x03,0x24,0x00};           // TOK   тара удачно            tik-tok axaxa
const char uart_string18[] PROGMEM =  {0x02,0x54,0x4E,0x4F,0x4B,0x31,0x45,0x03,0x24,0x00};      // TNOK  тара не удачно               
const char uart_string19[] PROGMEM =  {0x02,0x58,0x4F,0x4B,0x35,0x43,0x03,0x24,0x00};           // XOK   отмена тары удачно            
const char uart_string20[] PROGMEM =  {0x02,0x58,0x4E,0x4F,0x4B,0x31,0x32,0x03,0x24,0x00};      // XNOK  отмена тары не удачно   
                    
       
     
const char *const uart_string[] PROGMEM = {
     [S98_MSV] = uart_string1,
     [S01]  = uart_string2,
     [S02]  = uart_string3,
     [S03]  = uart_string4,
     [S04]  = uart_string5,
     [S05]  = uart_string6,
     [S06]  = uart_string7,
     [S07]  = uart_string8,
     [S08]  = uart_string9,  
     [S09]  = uart_string10,
     [S10]  = uart_string11,  
     [S11]  = uart_string12,
     [S12]  = uart_string13,  
     [TDD]  = uart_string14,   
     [AOK]  = uart_string15, 
     [ANOK] = uart_string16,
     [TOK]  = uart_string17,
     [TNOK] = uart_string18,
     [XOK]  = uart_string19,      
     [XNOK] = uart_string20,                     
};
// в конце каждой строки на отправку есть специальный символ $, который означает что мы передали всю строку

#endif /* UART_H_ */