#include "max7219.h"

#include "Interupt.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
#include "main.h"
//--------------���������� �� ������ ������---------------//
extern uint16_t tim_0;
extern struct parametrs param;
extern struct bool_date boolean;
//--------------���������� �� ����� �����-----------------//
static volatile bool  spi_two_send;
static volatile uint8_t spi_send;
static uint8_t blink_led;
volatile bool spi_stop;
volatile char send_buf [20] = {};
volatile static char buf_max7219 [10] = {};
//---------------�������----------------------------------//

void send_data (uint8_t adress, uint8_t byte){      // ������� ��������� ������� �� ���� ����
   if (!spi_stop){              // ��� ���� � ���������� ������, ������ ������
     boolean.stop_spi = true;   // � ��� ����
     SPCR &=  ~0x80;            // ��� �������� ���������� SPI
     SSOFF;                     // ��� ������ � ����
     SPDR = adress;	            // ��������� ������
     while(!(SPSR & (1<<SPIF)));//�������� ���� ������ �����������
     SPDR = byte;               // ��������� ��������� ������
     while(!(SPSR & (1<<SPIF)));//�������� ���� ������ �����������
     send_buf[10] = SPDR;       // ����� ������� ���������� ����� �������� ����� ���������� SPI, ��� ������ ����� ������� � ���������� � ���
     SSON;                      // ��� ������ � �������
     spi_two_send = false;      //?? 
     boolean.stop_spi = false;         
     SPCR |= 0x80;              // ��� ������� ���������� SPI
   }    
}

void init_max7219 (void){         // ������������� �����������
     asm("nop");
     send_data ( 0x0F,  0x00);        // ������� ������������ ����������
     send_data ( 0x0C,  0x01);        // ������� ������� ������
     send_data ( 0x0A,  0x0F);        // ��������� ������� ��������
     send_data ( 0x09,  0x00);        // ������� ��������� ������������� ������
     send_data ( 0x0B,  0x06);        // ��������� ���������� �������� ���������
}

void init_light (void)
{
     switch (param.lighte){           // ��������� ������� ��������
          case 0:  if (!spi_stop) send_data ( 0x0A,  0x04);break;
          case 1:  if (!spi_stop) send_data ( 0x0A,  0x07);break;
          case 2:  if (!spi_stop) send_data ( 0x0A,  0x0F);break;
          default: if (!spi_stop) send_data ( 0x0A,  0x0F);break;
     }
}

void indication (void) {                      // ���������, ���� �������� ������ � ��������� ������� ����� ���������� �� ������
     send_data ( 0x01,  send_buf [4]);         // � �������� ������ ��� ��� ��������� �����
     send_data ( 0x02,  send_buf [0]);
     send_data ( 0x03,  send_buf [3]);
     send_data ( 0x04,  send_buf [2]);
     send_data ( 0x05,  send_buf [5]);
     send_data ( 0x06,  send_buf [1]);
     send_data ( 0x07,  send_buf [6]);          // ��� ������� �����
}

void to_send_led (void){                       // �������� ������� � ������ �� �������� ���7219 ������� � ��������� �� � ������ ������ (��� �������� ��������� ����� ���� ������)
     static uint8_t bufer;
     static uint8_t caunter;
     caunter = 0;
     for (int i = 0; i < 6; i ++){            // ������ ��� ������� �����, ���� �� ������ ��� � ������ ������
          buf_max7219 [i] = 0x00;
     }
     for (int i = 0; i < 10; i ++){            // � ����� ���������� ����� ������ � ������
          bufer = send_buf[i];
          if (bufer == 0) break;                                                             // ���� ��� ���� ����� �� �����
          if (bufer == 0x2E && i > 0) {buf_max7219[caunter-1] |= 0x80;  continue;}           // ��� ������� ���� ������ ����� � �� ����� �� � ������� ������ �� ������ ����� ����������� �������
          if (bufer == 0x2E && i == 0) {buf_max7219[caunter] |= 0x80;  continue;}	           // � ���� ��� ������� ������ �� ��������� ���
          buf_max7219[caunter] = pgm_read_byte_near(&(tab_seg [bufer - 0x20]));              // ����� ��� �������
          if (caunter < 5)  caunter ++;                                                      // ���� ������ 5 ���������
          else break;                                                                        // � ��������� ������ ������
     }
     if (tim_0 <=5000 && tim_0 > 0){                                                        // ��� �������� ���� �� � ��������� 0-5000 ������ �������
          for (int i = 0; i < 6; i ++){                                                      // � ����� �������� ���� � ����� � ��������� �� 1. ���� ��� ��� �� ����� ���� ������
               if ((blink_led >> i) & 0x01 ) {buf_max7219 [i] =  0x00;}                      // �������� � ������ ������� ��� ��� ������� � ���� �� �����
          }
     }
}

void blink_simbol (uint8_t namber)                                                        // �������� ����� ��������
{
     static uint8_t bias;                                                                  // ������� ����������
     bias = 0x01;                                                                          // ����� ������� ��� ������� � �������
     blink_led = bias << namber;                                                           // �������� ��� �� ����� ���������� ��� �������� � �������
     if (tim_0 == 0) tim_0 = 10000;                                                        // �������� ������
}

void blink_many_simbol (uint8_t namber)                                                   // �������� ����������� ���������
{
     if (tim_0 == 0) tim_0 = 10000;                                                        // �������� ������
     blink_led = namber;                                                                   // �������� ������ ���� � ���������� ��������
}

void send_point (uint8_t point)
{
     buf_max7219 [6] |= pgm_read_byte_near(&(tab_seg [point]));           // ��������� ����� ��� ������� (��������� �� ������ ������ ������ � ��������� ����)
}

void send_non_point (uint8_t point)
{
     buf_max7219 [6] &= ~ pgm_read_byte_near(&(tab_seg [point]));	     // ������ ����� ��� ������� 
}

void led_led (void)                                                      // �������� ��� �����
{
     buf_max7219 [6] = 0x00;  
}




void interupt_send (void)            // ������� ��� �������� ������� ���7219 ����� ���������� SPI
{
     static uint8_t spi_caunter ;     // ������� ��� ����������� �� ������
     if (!spi_stop && !boolean.stop_spi) {                 // ���� � ��� ��� �� ��������� ������ ���� �� ��������� ���� ��������
          spi_stop = true;             // ���� ����� �� ��������� ���� ���� �� �������� ���� �� �� ��������� � ����������
          switch (spi_caunter){        // ����� ��� �������� ��������, ������ ��� ���������� �����
               case (0):
               spi_two_send = false;    // �������� ���� ���� � ���������� �������� ������ ����
               SSOFF;                   // ��� ������ � ����
               SPDR = 0x01;             // ��������� ������ ��� ������� �������
               spi_send = buf_max7219 [4]; // � � ������ ������� ������ ��� �������� ������� ��������
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
          spi_caunter ++;           // ����������� �������
          if (spi_caunter > 6) spi_caunter = 0;
     }
}

ISR (SPI_STC_vect) {
     if (!spi_two_send){          // ���� ������ ���� ������ �������, ������ ����� �� ���������
          SPDR = spi_send;
          spi_two_send = true;     // � ������� ���� ��� ��� �������
     }
     else { SSON; spi_stop = false;}   // ���� �� �������� � ���������� ������ ��� �� ��������� ��� ������ � ��������� ���� ��� ����� ���������� ��������� ������
}
