#include "max7219.h"

#include "Interupt.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
#include "main.h"
//--------------переменные из других файлов---------------//
extern uint16_t tim_0;
extern struct parametrs param;
extern struct bool_date boolean;
//--------------переменные из этого файла-----------------//
static volatile bool  spi_two_send;
static volatile uint8_t spi_send;
static uint8_t blink_led;
volatile bool spi_stop;
volatile char send_buf [20] = {};
volatile static char buf_max7219 [10] = {};
//---------------функции----------------------------------//

void send_data (uint8_t adress, uint8_t byte){      // функция единичной посылки из двух байт
   if (!spi_stop){              // был глюк с наложением байтов, сделал защиту
     boolean.stop_spi = true;   // и еще одну
     SPCR &=  ~0x80;            // тут выключим прерывания SPI
     SSOFF;                     // чип селект в ноль
     SPDR = adress;	            // загрузили данные
     while(!(SPSR & (1<<SPIF)));//подождем пока данные передадутся
     SPDR = byte;               // загрузили следующие данные
     while(!(SPSR & (1<<SPIF)));//подождем пока данные передадутся
     send_buf[10] = SPDR;       // таким образом происходит сброс регистра флага прерывания SPI, без сброса можем попасть в прерывание и все
     SSON;                      // чип селект в единицу
     spi_two_send = false;      //?? 
     boolean.stop_spi = false;         
     SPCR |= 0x80;              // тут включим прерывания SPI
   }    
}

void init_max7219 (void){         // инициализация индикаторов
     asm("nop");
     send_data ( 0x0F,  0x00);        // регистр тестирования индикатора
     send_data ( 0x0C,  0x01);        // регистр спящего режима
     send_data ( 0x0A,  0x0F);        // настройка яркости свечения
     send_data ( 0x09,  0x00);        // регистр включения декодирования данных
     send_data ( 0x0B,  0x06);        // настройка количества активных элементов
}

void init_light (void)
{
     switch (param.lighte){           // настройка яркости свечения
          case 0:  if (!spi_stop) send_data ( 0x0A,  0x04);break;
          case 1:  if (!spi_stop) send_data ( 0x0A,  0x07);break;
          case 2:  if (!spi_stop) send_data ( 0x0A,  0x0F);break;
          default: if (!spi_stop) send_data ( 0x0A,  0x0F);break;
     }
}

void indication (void) {                      // индикация, сюда передаем буффер с символами которые нужно отобразить на экране
     send_data ( 0x01,  send_buf [4]);         // в разнобой потому что так разведена плата
     send_data ( 0x02,  send_buf [0]);
     send_data ( 0x03,  send_buf [3]);
     send_data ( 0x04,  send_buf [2]);
     send_data ( 0x05,  send_buf [5]);
     send_data ( 0x06,  send_buf [1]);
     send_data ( 0x07,  send_buf [6]);          // это светики внизу
}

void to_send_led (void){                       // заменяем символы в буфере на понятные мах7219 символы и переносим их в другой буффер (это позволит вставлять точку куда попало)
     static uint8_t bufer;
     static uint8_t caunter;
     caunter = 0;
     for (int i = 0; i < 6; i ++){            // каждый раз очищаем буфер, чтоб не делать это в других местах
          buf_max7219 [i] = 0x00;
     }
     for (int i = 0; i < 10; i ++){            // в цикле записываем новые данные в буффер
          bufer = send_buf[i];
          if (bufer == 0) break;                                                             // если это ноль выйти из цикла
          if (bufer == 0x2E && i > 0) {buf_max7219[caunter-1] |= 0x80;  continue;}           // для спринтф если символ точка и он стоит не в нулевом сиволе то ставим точку предыдущему символу
          if (bufer == 0x2E && i == 0) {buf_max7219[caunter] |= 0x80;  continue;}	           // а если это нулевой символ то добавляем ему
          buf_max7219[caunter] = pgm_read_byte_near(&(tab_seg [bufer - 0x20]));              // минус код пробела
          if (caunter < 5)  caunter ++;                                                      // пока меньше 5 итерируем
          else break;                                                                        // в противном случае ливаем
     }
     if (tim_0 <=5000 && tim_0 > 0){                                                        // это моргание пока мы в диапазоне 0-5000 символ погашен
          for (int i = 0; i < 6; i ++){                                                      // в цикле сдвигаем биты в право и проверяем на 1. если это так то гасим этот символ
               if ((blink_led >> i) & 0x01 ) {buf_max7219 [i] =  0x00;}                      // сдвигаем в другую сторону так как символы с лева на право
          }
     }
}

void blink_simbol (uint8_t namber)                                                        // моргание одним символом
{
     static uint8_t bias;                                                                  // создали переменную
     bias = 0x01;                                                                          // самый младший бит возвели в еденицу
     blink_led = bias << namber;                                                           // сдвинули его на такое количество как передали в функцию
     if (tim_0 == 0) tim_0 = 10000;                                                        // сбросили таймер
}

void blink_many_simbol (uint8_t namber)                                                   // моргание несколькими символами
{
     if (tim_0 == 0) tim_0 = 10000;                                                        // сбросили таймер
     blink_led = namber;                                                                   // записали нужные биты в переменную моргания
}

void send_point (uint8_t point)
{
     buf_max7219 [6] |= pgm_read_byte_near(&(tab_seg [point]));           // поставить точку под экраном (считываем из памяти нужный символ и загружаем сюда)
}

void send_non_point (uint8_t point)
{
     buf_max7219 [6] &= ~ pgm_read_byte_near(&(tab_seg [point]));	     // убрать точку под экраном 
}

void led_led (void)                                                      // погасить все точки
{
     buf_max7219 [6] = 0x00;  
}




void interupt_send (void)            // функция для передачи посылки мах7219 через прерывания SPI
{
     static uint8_t spi_caunter ;     // счетчик для перемещения по кейсам
     if (!spi_stop && !boolean.stop_spi) {                 // если у нас еще не отправлен второй байт то запрещаем сюда заходить
          spi_stop = true;             // если зашли то поднимаем флаг чтоб не заходить пока он не сбросится в прерывании
          switch (spi_caunter){        // свитч для передачи символов, каждый раз передается новый
               case (0):
               spi_two_send = false;    // опустили флаг чтоб в прерывании передать фторой байт
               SSOFF;                   // чип селект в ноль
               SPDR = 0x01;             // загружаем данные для первого символа
               spi_send = buf_max7219 [4]; // и в память заносим данные для передачи первому регистру
               break;
               case (1):
               spi_two_send = false;
               SSOFF;
               SPDR = 0x02;
               spi_send = buf_max7219 [0];
               break;
               case (2):
               spi_two_send = false;
               SSOFF;
               SPDR = 0x03;
               spi_send = buf_max7219 [3];
               break;
               case (3):
               spi_two_send = false;
               SSOFF;
               SPDR = 0x04;
               spi_send = buf_max7219 [2];
               break;
               case (4):
               spi_two_send = false;
               SSOFF;
               SPDR = 0x05;
               spi_send = buf_max7219 [5];
               break;
               case (5):
               spi_two_send = false;
               SSOFF;
               SPDR = 0x06;
               spi_send = buf_max7219 [1];
               break;
               case (6):
               spi_two_send = false;
               SSOFF;
               SPDR = 0x07;
               spi_send = buf_max7219 [6];
               break;
          }
          spi_caunter ++;           // увеличиваем счетчик
          if (spi_caunter > 6) spi_caunter = 0;
     }
}

ISR (SPI_STC_vect) {
     if (!spi_two_send){          // если опущен флаг второй посылки, значит нужно ее отправить
          SPDR = spi_send;
          spi_two_send = true;     // и поднять флаг что это сделали
     }
     else { SSON; spi_stop = false;}   // если мы попадаем в прерывание второй раз то поднимаем чип селект и поднимаем флаг что можно отправлять следующий символ
}
