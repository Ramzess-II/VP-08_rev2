#include "UART.h"

#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "RTC1307.h"
#include "EEprom.h"
//----------���������� ������----------//
// UART0 - ���
// UART1 - 232/485
// UART2 - �������
// UART3 - ����� // ��� � ��� �������

//--------------���������� �� ������ ������---------------//
extern struct metrologis metrology;
extern struct parametrs param;
extern uint16_t tim_2,tim_4;
extern int32_t massa_after_filtr;
//--------------���������� �� ����� �����-----------------//
#define TIM_TO_UART 5000               //����� �� �����, ���� ��� ������ ������ �� �������� � ����
#define TIM_SPEED 2000
volatile bool rx0_done,rx1_done,rx2_done;
volatile uint8_t tim_uart0 = 100;
volatile uint8_t tim_uart1 = 100;      // ������, �� ������ ������� ������� ��� ������� ���������
volatile uint8_t tim_uart2 = 150;     // ���� ����� �� ���������� ��� �� ���
uint16_t sens_cod,sens_cod_led;
struct bool_date boolean;

//--------------������� UART-----------------//
char str_cpy_buf [15];                        // ��� ������������� ������� ���� ����� ���� �� ������� ����������� ������ � �������� ����
char mirror_buf [20];

char uart0_tx_buf [20];                       // ������ �� ��������
char uart1_tx_buf [20];
char uart2_tx_buf [15];
char uart3_tx_buf [15];

volatile uint8_t uart0_tx_caunter;             // �������� ���� �� ��������
volatile uint8_t uart1_tx_caunter;
volatile uint8_t uart2_tx_caunter;
volatile uint8_t uart3_tx_caunter;

char uart0_rx_buf [15];                        // ������ �������� ����
char uart1_rx_buf [15];
char uart2_rx_buf [10];

volatile uint8_t uart0_rx_caunter;             // �������� �������� ����
volatile uint8_t uart1_rx_caunter;
volatile uint8_t uart2_rx_caunter;

volatile uint8_t data_out ;


void init_UART_tenzo(void)//������������� ������ USART
{
     UBRR2 = B9600;
     UCSR2B=(1<<RXEN2)|( 1<<TXEN2);                  //�������� ����� � �������� �� USART
     UCSR2B |= (1<<RXCIE2) ;                         //��������� ���������� ��� ������
     UCSR2C = (1<<UPM21)|(1<<UCSZ21)|(1<<UCSZ20);    //������� 1, 8 ���
}

void init_UART_tablo(void)//������������� ������ USART
{
     UBRR3 = B600;
     UCSR3B=( 1<<TXEN2);                             //�������� �������� �� USART
     UCSR3C = (1<<UPM31)|(1<<UCSZ31)|(1<<UCSZ30);    //������� 1, 8 ���
}

void init_Uart_0_1_start (void)
{
     UCSR0B=(1<<RXEN0)|( 1<<TXEN0);                  //�������� ����� � �������� �� USART
     UCSR0B |= (1<<RXCIE0);                          //��������� ���������� ��� ������
     UCSR0C = (1<<UPM01)|(1<<UCSZ01)|(1<<UCSZ00);    //������� 1, 8 ���  ???
     UCSR1B=(1<<RXEN1)|( 1<<TXEN1);                  //�������� ����� � �������� �� USART
     UCSR1B |= (1<<RXCIE1);                          //��������� ���������� ��� ������
     UCSR1C = (1<<UPM11)|(1<<UCSZ11)|(1<<UCSZ10);    //������� 1, 8 ���  ???
}

void init_UART_0_1 (uint8_t namber1, uint8_t namber2)         // ����������� ������ �� ����
{
     switch (namber1){
          case 0:UBRR0 = B600;    break;
          case 1:UBRR0 = B2400;   break;
          case 2:UBRR0 = B9600;   break;
          case 3:UBRR0 = B19200;  break;
     }
     switch (namber2){
          case 0:UBRR1 = B600;    break;
          case 1:UBRR1 = B2400;   break;
          case 2:UBRR1 = B9600;   break;
          case 3:UBRR1 = B19200;  break;
     }
}

uint32_t parser_buf_zemic (char *send)                             // ������� �������������� ������ ��������� �� ������� � ����������
{
     uint32_t assembly;                                             // ��������� ����������
     assembly = 0;                                                  // ���������� � ����
     rx2_done = false;                                              // ������� ���� �������� ������ �� �����
     if (send[0] == 0x00 && send[4] == 0x0D && send[5] == 0x0A ) {  // ���� ������ ���� ���� � 4 � 5 0� 0�
          assembly = send[3];                                       // ���������� � ������� ����, ��� ��� �� ���� ���������
          assembly = assembly << 8;                                 // ��������
          assembly |= send[2];                                      // ������� ��� ���� ����
          assembly = assembly << 8;
          assembly |= send[1];
          assembly ^= 0x800000;                                     // ���������� ��� �����, ��� ���� ���� ������ ���� � ����� ������
          uart2_rx_buf[4] = 0x00; uart2_rx_buf[5] = 0x00;
          return assembly;                                          // ������ ��������
     }
     else {return 8388608;}                                         // ���� ������ ����� ������� ��� ����
}

//strcpy_P �������� ���� �� �������� ������ 0!!
void get_zemik_sensors (uint8_t amount)                             // ������� ��� ������ �������� ����� ( �������� ���������� �������� ������� ����� ��������)
{
     static uint8_t caunter_send;                                   // ������� ����������� �� ������
     switch (caunter_send){
          case 0: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S98_MSV])));                 caunter_send ++; uart2_send_string (str_cpy_buf);   tim_2 = TIM_TO_UART ; break ;
           // ���������� ������� ��� ���� ��������, ������� ������� ���� ������� � ��������� ����     
          case 1: if (tim_2 < 4000){strcpy_P( str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S01])));    caunter_send ++; uart2_send_string (str_cpy_buf);   tim_2 = TIM_TO_UART;} break;
          // ����� ���������� ��� ������� �������, ����� ������� ��������� � ��������� �����
          case 2: if(rx2_done){ metrology.cod_sensor1 = parser_buf_zemic (uart2_rx_buf);                caunter_send ++; sens_cod |= 1; }  break;
          // ������� � ������� �������� �� ���� �������� ������, ���� �� �� ����������� ��� ������� � �������� �������
          case 3: strcpy_P( str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S02])));               caunter_send ++; uart2_send_string (str_cpy_buf);   tim_2 = TIM_TO_UART; break;
          case 4: if(rx2_done){ metrology.cod_sensor2 = parser_buf_zemic (uart2_rx_buf);                caunter_send ++; sens_cod |= 2;\
          if (amount == 2 && sens_cod == 3){boolean.data_ok = true; }      }   break;     
          // ���� ������� ��� ������� � ����� �� ���������� �� ��������� ���� ��� ������ �������, ���������� ������� � �������� ��� � ����, ���� ������ �� �������� �� � �����������
          case 5: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S03])));                     caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;
          case 6: if (rx2_done){ metrology.cod_sensor3 = parser_buf_zemic (uart2_rx_buf);               caunter_send ++; sens_cod |= 4;}  break;
          case 7: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S04])));                     caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;     
          case 8: if (rx2_done){ metrology.cod_sensor4 = parser_buf_zemic (uart2_rx_buf);               caunter_send ++; sens_cod |= 8;\
          if (amount == 4 && sens_cod == 15){ boolean.data_ok = true; }     }  break;
          case 9: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S05])));                     caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;
          case 10: if (rx2_done){ metrology.cod_sensor5 = parser_buf_zemic (uart2_rx_buf);              caunter_send ++; sens_cod |= 16; }  break;     
          case 11:strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S06])));                     caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;
          case 12: if (rx2_done){ metrology.cod_sensor6 = parser_buf_zemic (uart2_rx_buf);              caunter_send ++; sens_cod |= 32;\
          if (amount == 6 && sens_cod == 63){boolean.data_ok = true; }     }  break;
          case 13: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S07])));                    caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;
          case 14: if (rx2_done){ metrology.cod_sensor7 = parser_buf_zemic (uart2_rx_buf);              caunter_send ++; sens_cod |= 64; }  break;
          case 15: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S08])));                    caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;
          case 16: if (rx2_done){ metrology.cod_sensor8 = parser_buf_zemic (uart2_rx_buf);              caunter_send ++; sens_cod |= 128;\
          if (amount == 8 && sens_cod == 255){ boolean.data_ok = true; }   }  break;    
          case 17: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S09])));                    caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;    
          case 18: if (rx2_done){ metrology.cod_sensor9 = parser_buf_zemic (uart2_rx_buf);              caunter_send ++; sens_cod |= 256;}  break; 
          case 19: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S10])));                    caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;  
          case 20: if (rx2_done){ metrology.cod_sensor10 = parser_buf_zemic (uart2_rx_buf);             caunter_send ++; sens_cod |= 512;\
          if (amount == 10 && sens_cod == 1023){boolean.data_ok = true; }  }  break;       
          case 21: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S11])));                    caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;    
          case 22: if (rx2_done){ metrology.cod_sensor11 = parser_buf_zemic (uart2_rx_buf);             caunter_send ++; sens_cod |= 1024;}  break; 
          case 23: strcpy_P(str_cpy_buf, (PGM_P)pgm_read_word(&(uart_string[S12])));                    caunter_send ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_TO_UART; break;     
          case 24: if (rx2_done){ metrology.cod_sensor12 = parser_buf_zemic (uart2_rx_buf);             caunter_send ++; sens_cod |= 2048;\
          if (amount == 12 && sens_cod == 4095){boolean.data_ok = true; }  }  break;
     }
     
        if (tim_2 == 0 ) {  if(caunter_send < 24){caunter_send ++;} else {caunter_send = 0; sens_cod_led = sens_cod; sens_cod = 0;if (boolean.caunt_err < 5) boolean.caunt_err ++; if (boolean.caunt_err > 1){ boolean.errors = 1;}}}
        // ���� ������ ��������� ������ ����� �� �����, �������� ����������� ������� ���� ��� �������� �� �����, � ��� �� ������� ��� � ��� � ��������� ���� �� ��� �� 24 �����, ���������� � ����, �������� ����� ������� � ������ � ����������     
        if (boolean.data_ok) { boolean.errors = 0; boolean.caunt_err = 0; sens_cod = 0; caunter_send = 0; }
        // ��� ������� ��������� �� �������� ������������ �����, � �������� ����, ��������� ���� ���� � ������ ���������� ������ ������. � �������� ���� ����� ����� �������� �� ������ �� �����, ��� ��� �� ���� �������������  
}               

uint8_t get_number_sensor (void)
{
     static uint8_t caunter;
     static uint8_t caunter_sens;
     static uint8_t namber_sensor;
     caunter = 0; caunter_sens = 0; namber_sensor = 0;
     rx2_done = false;
     while (caunter != 25){
          switch (caunter)
          {
               case 0: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S98_MSV]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED + 500;   break;
               case 1: if (tim_2 == 0){strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S01]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED; }  break;
               case 2: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++;  namber_sensor = 1; }  caunter ++; }  break;
               case 3: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S02]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 4: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++;  namber_sensor = 2;}  caunter ++; }  break;
               case 5: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S03]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 6: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++;  namber_sensor = 3;}  caunter ++; }  break;
               case 7: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S04]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 8: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++;  namber_sensor = 4;}  caunter ++; }  break;
               case 9: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S05]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 10: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 5;} caunter ++; }   break;
               case 11: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S06]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 12: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 6;} caunter ++; }   break;
               case 13: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S07]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 14: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 7;} caunter ++; }   break;
               case 15: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S08]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 16: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 8;} caunter ++; }   break;
               case 17: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S09]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 18: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 9;} caunter ++; }   break;
               case 19: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S10]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 20: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 10;} caunter ++; }   break;
               case 21: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S11]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 22: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 11;} caunter ++; }   break;
               case 23: strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[S12]))); caunter ++; uart2_send_string (str_cpy_buf); tim_2 = TIM_SPEED;   break;
               case 24: if (rx2_done || tim_2 == 0){ if(rx2_done){ rx2_done = false; caunter_sens ++; namber_sensor = 12;} caunter ++; }   break;
          }
     }
     if (caunter_sens == 1)  {return namber_sensor;}
     else { return 100; }  
}

void send_new_adr (uint8_t last, uint8_t news)
{
  sprintf(str_cpy_buf, ";S%02d;ADR%02d;$", last, news);   
  uart2_send_string (str_cpy_buf);
  _delay_ms(50);
  strcpy_P(str_cpy_buf ,(PGM_P)pgm_read_word_near(&(uart_string[TDD])));   
  uart2_send_string (str_cpy_buf);
  _delay_ms(50); 
}

void reply_visl (void)      // ���������� ������ ��������� ���� ����� �������� �� ����� ������
{
    volatile static uint8_t xor_state;
    if(!boolean.errors  && !param.err36 && !boolean.err_adc){         // ���� ���� ������ �� ���������
     switch (data_out)
     {
        case 0: boolean.busy = false; break;                                               // ������� ������ ����������
        case 1: sprintf(mirror_buf, "%+08ld", massa_after_filtr);    // ������ ������ �����
          xor_state = 0x03;
          for (int i = 0; i < 8; i ++){ xor_state ^=  mirror_buf[i];}
          sprintf(mirror_buf, "2AB%+08ld%02x3$", massa_after_filtr, xor_state); 
          mirror_buf[0] = 0x02; mirror_buf[13] = 0x03;  uart0_send_string(mirror_buf);  
        data_out = 0;  break;    
        case 2: sprintf(mirror_buf, "%+08ld", massa_after_filtr);    // ������ ����� + ������������
          xor_state = 0x03;
          for (int i = 0; i < 8; i ++){ xor_state ^=  mirror_buf[i];}
          sprintf(mirror_buf, "2AS%+08ld  %02x3$", massa_after_filtr, xor_state);
          mirror_buf[0] = 0x02; mirror_buf[11] = 0x2C; if (boolean.stab) {mirror_buf[12] = 0x31;}else{mirror_buf[12] = 0x30;} mirror_buf[15] = 0x03; uart0_send_string(mirror_buf);  
        data_out = 0;  break; 
        case 3:  data_out ++; boolean.uart_zero = true;   break;   // ������ ������� ���� ��� ����� ���������� � ������� �� ���� ����
        case 4: if(boolean.state_zero == 2){strcpy_P(mirror_buf, (PGM_P)pgm_read_word(&(uart_string[AOK])));  uart0_send_string(mirror_buf);    }   // ��������� ������� ���������� ��� ���  � ��������  
                if(boolean.state_zero == 1){strcpy_P(mirror_buf, (PGM_P)pgm_read_word(&(uart_string[ANOK]))); uart0_send_string(mirror_buf);    } 
        data_out = 0; boolean.state_zero = 0;   break;
        case 5: if(!boolean.tara_ok){strcpy_P(mirror_buf, (PGM_P)pgm_read_word(&(uart_string[TOK])));  uart0_send_string(mirror_buf); boolean.uart_tara = true;  }  // ����� ���� ���� ��������� �� �������� � ��������
                if(boolean.tara_ok){strcpy_P(mirror_buf, (PGM_P)pgm_read_word(&(uart_string[TNOK]))); uart0_send_string(mirror_buf);    }                           // � ���� �������� �� ������ ������� ��� �� ��
        data_out = 0;     break;   
        case 6: if(boolean.tara_ok){strcpy_P(mirror_buf, (PGM_P)pgm_read_word(&(uart_string[XOK])));  uart0_send_string(mirror_buf); boolean.uart_tara = true;  }  // ������ ���� ���� �������� �� ��������� � ��������
                if(!boolean.tara_ok){strcpy_P(mirror_buf, (PGM_P)pgm_read_word(&(uart_string[XNOK]))); uart0_send_string(mirror_buf);    }                         // � ���� ��������� �� ������ ������� ��� �� ��
        data_out = 0;     break;                     
     }
   }   else {data_out = 0; }       
}
// ������ ���������: �� �������� � ��� ������� � ����� �� �����������, � � ����������� �� ����� ��������� ��������, � �������� ���� ���� ������ ������ �� ���������

void parser_visl (void)                                          // ������� ������� �������� ���� 
{
   if (rx0_done || rx1_done)                                // ���� ������ ������ � ���� ��������� ������                    
   { 
     if(!boolean.busy){   
        if(rx0_done) {
          rx0_done = false; if(rx1_done) {rx1_done = false;}
          if(uart0_rx_buf[0] == 0x02 && uart0_rx_buf[1] == 0x41){
             if( uart0_rx_buf[2] == 0x42 && uart0_rx_buf[3] == 0x30 && uart0_rx_buf[4] == 0x33) { data_out = 1; boolean.busy = true;}   // AB03
             if( uart0_rx_buf[2] == 0x53 && uart0_rx_buf[3] == 0x31 && uart0_rx_buf[4] == 0x32) { data_out = 2; boolean.busy = true;}   // AS  
             if( uart0_rx_buf[2] == 0x48 && uart0_rx_buf[3] == 0x30 && uart0_rx_buf[4] == 0x39) { data_out = 3; boolean.busy = true;}   // AH         
             if( uart0_rx_buf[2] == 0x54 && uart0_rx_buf[3] == 0x31 && uart0_rx_buf[4] == 0x35) { data_out = 5; boolean.busy = true;}   // AT  
             if( uart0_rx_buf[2] == 0x58 && uart0_rx_buf[3] == 0x31 && uart0_rx_buf[4] == 0x39) { data_out = 6; boolean.busy = true;}   // AX                
          }
             for(int i = 0; i < 10; i ++) {uart0_rx_buf[i] = 0;}  
       }
       if(rx1_done) {
             rx1_done = false; 
             if(uart1_rx_buf[0] == 0x02 && uart1_rx_buf[1] == 0x41){
                  if( uart1_rx_buf[2] == 0x42 && uart1_rx_buf[3] == 0x30 && uart1_rx_buf[4] == 0x33) { data_out = 1; boolean.busy = true;}   // AB03
                  if( uart1_rx_buf[2] == 0x53 && uart1_rx_buf[3] == 0x31 && uart1_rx_buf[4] == 0x32) { data_out = 2; boolean.busy = true;}   // AS
                  if( uart1_rx_buf[2] == 0x48 && uart1_rx_buf[3] == 0x30 && uart1_rx_buf[4] == 0x39) { data_out = 3; boolean.busy = true;}   // AH
                  if( uart1_rx_buf[2] == 0x54 && uart1_rx_buf[3] == 0x31 && uart1_rx_buf[4] == 0x35) { data_out = 5; boolean.busy = true;}   // AT
                  if( uart1_rx_buf[2] == 0x58 && uart1_rx_buf[3] == 0x31 && uart1_rx_buf[4] == 0x39) { data_out = 6; boolean.busy = true;}   // AX
             }
             for(int i = 0; i < 10; i ++) {uart1_rx_buf[i] = 0;}
       }                              
     }                      
   }           
}

ISR (USART0_RX_vect)
{
     if (UCSR0A & (1<<UPE0)){uart0_rx_buf[19] = UDR0; uart0_rx_caunter = 0;}     // �� ��������� ����� ������ ���� parity error
     else
     {
          tim_uart0 = 0;                                                        // ������� ������, �� 150 (��� ����� �������� ��� ���� ������ ��� ������ ����� ������ �����)
          uart0_rx_buf[uart0_rx_caunter] = UDR0;                                // ��������� �������� ����
          uart0_rx_caunter ++; if(uart0_rx_caunter > 13) uart0_rx_caunter = 0;  // ���� ������� ������� 0 �� ����������, ���� ������� ��������� �� 8 �� ������� � ����                                                              
     }       
}

ISR (USART1_RX_vect)
{
     if (UCSR1A & (1<<UPE1)){uart1_rx_buf[19] = UDR1; uart1_rx_caunter = 0;}     // �� ��������� ����� ������ ���� parity error
     else
     {
          tim_uart1 = 0;                                                        // ������� ������, �� 150 (��� ����� �������� ��� ���� ������ ��� ������ ����� ������ �����)
          uart1_rx_buf[uart1_rx_caunter] = UDR1;                                // ��������� �������� ����
          uart1_rx_caunter ++; if(uart1_rx_caunter > 13) uart1_rx_caunter = 0;  // ���� ������� ������� 0 �� ����������, ���� ������� ��������� �� 8 �� ������� � ����
     }
}


ISR (USART2_RX_vect)
{
     if (UCSR2A & (1<<UPE2)){uart2_rx_buf[9] = UDR2; uart2_rx_caunter = 0;}     // �� ��������� ����� ������ ���� parity error
     else
     {
          tim_uart2 = 0;                                                        // ������� ������, �� 150 (��� ����� �������� ��� ���� ������ ��� ������ ����� ������ �����)
          uart2_rx_buf[uart2_rx_caunter] = UDR2;                                // ��������� �������� ����
          if (uart2_rx_buf[0] == 0x00) { uart2_rx_caunter ++; if(uart2_rx_caunter > 8) uart2_rx_caunter = 0;}  // ���� ������� ������� 0 �� ����������, ���� ������� ��������� �� 8 �� ������� � ����
          else {uart2_rx_caunter = 0;}                                           // ���� ������� ���� �� ���� �� �������� � ����� ������� ������
     }
}

//----------������� ������������� �������� ������----------//
void uart0_send_string ( char *send0)                // ���� �������� ������ � ����� ������� ������ $
{
     if (uart0_tx_caunter == 0)                     // ���� ������� �� ���� ����� �������� �����, ���� ��� �� ����������
     {
          for (int i = 0; i < 20; i ++)             // �������� ������ �������� ��������� �� ���������� ������
          {
               uart0_tx_buf[i] = send0[i];
               if (uart0_tx_buf[i] == '$') break;   // ���� �� �������� ������ $. �� ��� �� ���� �������
          }
          UDR0 = uart0_tx_buf[uart0_tx_caunter];    // ���������� ������ ����
          uart0_tx_caunter ++;                      // ����������� �������
          UCSR0B |= ( 1<<UDRIE0);                   // �������� ���������� �� ������������ ������� ��������
     }
}

void uart1_send_string ( char *send1)
{
     if (uart1_tx_caunter == 0)
     {
          for (int i = 0; i < 20; i ++)
          {
               uart1_tx_buf[i] = send1[i];
               if (uart1_tx_buf[i] == '$') break;
          }
          UDR1 = uart1_tx_buf[uart1_tx_caunter];
          uart1_tx_caunter ++;
          UCSR1B |= ( 1<<UDRIE1);
     }
}

void uart2_send_string ( char *send2)
{
     if (uart2_tx_caunter == 0)
     {
          for (int i = 0; i < 20; i ++)
          {
               uart2_tx_buf[i] = send2[i];
               if (uart2_tx_buf[i] == '$') break;
          }
          UDR2 = uart2_tx_buf[0];
          uart2_tx_caunter ++;
          UCSR2B |= ( 1<<UDRIE2);
     }
}

void uart3_send_string ( char *send3)
{
     if (uart3_tx_caunter == 0)
     {
          for (int i = 0; i < 20; i ++)
          {
               uart3_tx_buf[i] = send3[i];
               if (uart3_tx_buf[i] == '$') break;
          }
          UDR3 = uart3_tx_buf[uart3_tx_caunter];
          uart3_tx_caunter ++;
          UCSR3B |= ( 1<<UDRIE3);
     }
}

//----------����������� ���������� �� �������� ����������----------//
ISR (USART0_UDRE_vect)                                      // ���������� �� ����������� ������� ��������
{
     if(uart0_tx_buf[uart0_tx_caunter] != '$')              // ���� ������ � ������� �� ����� $
     {
          UDR0 = uart0_tx_buf[uart0_tx_caunter];            // ���������� ���������� �� ������� ����� � ����
          uart0_tx_caunter ++;                              // �� � ����������� ������������� �� �������
     }
     else                                                   // ���� ����� �� ������� $
     {
          uart0_tx_caunter = 0;                             // ���������� �������
          UCSR0B  &= ~ ( 1<<UDRIE0);                        // �������� ���������� �� ����������� ������
     }
}

ISR (USART1_UDRE_vect)
{
     if(uart1_tx_buf[uart1_tx_caunter] != '$')
     {
          UDR1 = uart1_tx_buf[uart1_tx_caunter];
          uart1_tx_caunter ++;
     }
     else
     {
          uart1_tx_caunter = 0;
          UCSR1B  &= ~ ( 1<<UDRIE1);
     }
}

ISR (USART2_UDRE_vect)
{
     if(uart2_tx_buf[uart2_tx_caunter] != '$')
     {
          UDR2 = uart2_tx_buf[uart2_tx_caunter];
          uart2_tx_caunter ++;
     }
     else
     {
          uart2_tx_caunter = 0;
          UCSR2B  &= ~ ( 1<<UDRIE2);
     }
}

ISR (USART3_UDRE_vect)
{
     if(uart3_tx_buf[uart3_tx_caunter] != '$')
     {
          UDR3 = uart3_tx_buf[uart3_tx_caunter];
          uart3_tx_caunter ++;
     }
     else
     {
          uart3_tx_caunter = 0;
          UCSR3B  &= ~ ( 1<<UDRIE3);
     }
}