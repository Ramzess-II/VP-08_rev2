#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
#include "main.h"

//------------------------------------------------------------------------------
// инициализируем глобальные переменные
//------------------------------------------------------------------------------
volatile bool err_buz;
volatile uint16_t adc ;
volatile static uint16_t buzer, pic_buzer;
volatile uint16_t tim_0, tim_1, tim_2, tim_3, tim_4, tim_5,tim_6;
// tim_0 - это блинк, tim_1 - отображение ероров и при выводе АЦП на экран, tim_2 - под юарт tim_3 - частота опроса часов, tim_4 -  , tim_5  - время стабилизации, tim_6 - показ ошибок датчиков
// можно исползовать таймеры в разных местах, где они 100% не пересекуться 
extern uint8_t tim_uart0, tim_uart2, tim_uart1, rx0_done,rx1_done, rx2_done;
extern uint8_t uart0_rx_caunter, uart1_rx_caunter, uart2_rx_caunter;
extern struct bool_date boolean;


ISR (ADC_vect){
	adc = ADC;
}

ISR (TIMER0_OVF_vect){                  // 62 KГерц скорость срабатывания
	if (buzer > 1) {buzer --; BZRON;}   
	if (buzer == 1) {buzer --; BZROFF;}
    
    if (err_buz && pic_buzer == 0) { pic_buzer = 14000; BZRON;}
    if (pic_buzer == 10000) {  BZROFF;} 
    if (pic_buzer == 7000) {  BZRON;}  
    if (pic_buzer == 2000) {  BZROFF;}              
    if (pic_buzer > 0) pic_buzer --;
    
	if (tim_3 > 0){ tim_3 --;}else {tim_3 = 15000;}  
    if (tim_3 == 15000 ) {boolean.funck1 = true;}      
    if (tim_3 == 10000 ) {boolean.funck2 = true;} 
    if (tim_3 == 5000 )  {boolean.funck3 = true;}      
         
	if (tim_0 > 0) tim_0 --;
	if (tim_1 > 0) tim_1 --;
	if (tim_2 > 0) tim_2 --;
    //if (tim_4 > 0) tim_4 --;
    if (tim_5 > 0) tim_5 --;
    
	if (tim_uart2 < 150) {tim_uart2 ++;}
    if (tim_uart2 == 90) { uart2_rx_caunter = 0; rx2_done = true; }  
     
           
}

/*ISR (TIMER2_OVF_vect)                  // 8 KГерц скорость срабатывания
{
     
     
}*/

ISR (TIMER1_OVF_vect)                 // 2,6 КГерц
{               
	TCNT1 = 0xE800;
	skan_key ();
	to_send_led();
	interupt_send ();
    if (tim_6 > 0) tim_6 --;
    
	if (tim_uart0 < 100) {tim_uart0 ++;}
	if (tim_uart0 == 99) { uart0_rx_caunter = 0; rx0_done = true; }    
         
	if (tim_uart1 < 100) {tim_uart1 ++;}
	if (tim_uart1 == 99) { uart1_rx_caunter = 0; rx1_done = true; }             
  
}

void buzers (uint16_t tim)
{
    #ifndef DEBUG_MOD  
     if (pic_buzer == 0) {buzer = tim; } // если уже пищит то не пищать, чтоб звук не сбивался
    #endif
}

void error_buzzer (bool sig)
{
     #ifndef DEBUG_MOD 
     err_buz = sig;
     #endif
}
