#include "main.h"
#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
//--------------���������� �� ������ ������---------------//
extern  char send_buf [20];
extern struct DS1307 DS;
extern uint16_t tim_1, tim_3,tim_5,tim_6,sens_cod_led,adc;
extern struct parametrs param;
extern struct metrologis metrology;
extern struct bool_date boolean;
//--------------���������� �� ����� �����-----------------//
uint8_t mein_display,eror_sens_up;
int32_t massa, last_massa,massa_after_filtr;
struct filtros filtr;
 





int main(void)
{
     init_atmega ();                 // �������������� ��� ���������
     init_max7219 ();                // �������������� �������, ���� ���� �������� ��������� � ������ ������� �������
     for (int t = 0; t < 7; t++)     // ����� ������� ��� ���� ���� ����� ��� ��������� �� ��������� ������ ����!
     {send_buf [t] = 0x00;}
     indication ();
     #ifndef DEBUG_MOD                // ��������� � ������
     start ();                        // ������ ���������
     zastavka ();                     // � ��������� ��������
     #endif
     I2C_init();                      // �������������� �������� I2C
     sei ();                          // ��������� ����������. ������ ������, ��� ��� ����� ���������� � SPI ���������� �� ���������� �������
     DS_Write_init_date();            // ���� ���� ���� ��������, �� ��������� �� �� 90 ��� � ���������
     enter_password_init();           // ���� ������ �� ������������� ��� ����� ���36
     read_ALL_EEprom();               // ������ �� eeprom ��� ����������
     init_after_EEprom();             // �������������� �� ��������� ��� ������� �� ������
     for (;;)
     {
          
       if (!boolean.main_menu_bool) {                    // ���� ��� ������ � ���36 �� �������� � �����  � ��� �� ����������� ������ ��������� � ��������� �� �������� �� ���36
           if(boolean.funck1){boolean.funck1 = false;  if (!boolean.errors && !param.err36){TABLO_print(massa_after_filtr); }}  // � ����� �������������� ������ ��� ���
           if(boolean.funck2){boolean.funck2 = false; DS_Read(); off_key(); parser_visl(); }
           if(boolean.funck3){boolean.funck3 = false; adc_read_of();  no_pay_wizard();  reply_visl(); }     
           get_zemik_sensors(param.sensor_number);      //                                                          // � ������ �������������� ���������� �������
           if (button_press (KEY_DOWN)) {  (mein_display < 2) ? mein_display ++ : mein_display = 0; }      // ������ ����� � ���� �������� �������� ������
           if (button_press (KEY_UP))   {  (mein_display > 0) ? mein_display -- : mein_display = 2;}       // ������� ��������� ��������. ���� ������ ���� ��������, ���� ���� �� ������ 2 
           if (button_press (KEY_ENTER)) {boolean.main_menu_bool = 1; init_menu();   }                     // ���� ������ ����� ��������� � ����  
           if (!boolean.errors) {                                                                          // ���� ���� �������� �� �� ����������� ���� � ����      
             if ((button_press (KEY_RIGHT)) || boolean.uart_tara){  if (!boolean.tara_ok) { metrology.tara = massa_after_filtr;} boolean.tara_ok = !boolean.tara_ok; boolean.uart_tara = false;} 
             // ���� ������ ����, ��������, � ��������� ���� ��� ����������
             if ((button_press (KEY_LEFT)) || boolean.uart_zero) {  if (!boolean.first_zero) { if ((param.NPV / 4) > labs(massa_after_filtr + metrology.tara)) {zero_funk(0);boolean.first_zero = true;boolean.state_zero = 2;metrology.tara = 0;}\
             else {error_func (NO);boolean.state_zero = 1;}} else {if((param.NPV / 25 )> labs(massa_after_filtr + metrology.tara)) {zero_funk(0);boolean.state_zero = 2;metrology.tara = 0;} else {error_func (NO);boolean.state_zero = 1;}}  boolean.uart_zero = false;}
             // ������� ���������, ���� ������ ���� ������� ��������� �� ��������� ������ �� ������� ����� ��� 25%, ���� �� �� �������� � ��������� ���� ������� ���������. � ���� ���� ������ �� ��������� ��� �� 4%
             if(!boolean.avto_zero) {if (param.zero_start && boolean.data_ok) { if (!boolean.first_zero) { if ((param.NPV / 4) > labs(massa_after_filtr)) {zero_funk(0);} boolean.first_zero = true; boolean.avto_zero = true;}}}
             // ���� ����� ������ ��� ���� ����.                  
           }       
           if (!param.err36 && !boolean.err_adc){         // ���� ��� ������ �������� �������             

                switch (mein_display)                     // �������� �������
               {
                    case 0: filtr_massa(param.filtr);   break;
                    case 1: sprintf(send_buf, "%02d.%02d.%02d." ,  DS.hour,DS.min,DS.sec);    error_buzzer(false);       break;
                    case 2: sprintf(send_buf, "%02d.%02d.%02d." ,  DS.date,DS.month,DS.year); error_buzzer(false);       break;
                    case 3: if(!tim_6) { mein_display = 0; boolean.start_delay = 1; }strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[12]))); adc_read_of(); filtr_massa(param.filtr); break;     // ��� ������ �������� ��� ��� ���� ��� ������ ���������
               }              
           } 
           else {                                        // ���� ���� �� ���������� ��
                   while (1)
                   {
                       if (boolean.err_adc) { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[BAT])));  break; }     // ����
                       if (param.err36) {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR36])));  break;  }     // ���36
                       break;       
                   }                          
                }
      }            
        if (boolean.main_menu_bool) { main_menu (); error_buzzer(false);}       // ���� �� � ����, �� ���������� ������ ���. 
        err_and_led (); // ������ �� ����������� ��������  
                                                                      
    }
}

void adc_read_of (void)
{
    static uint8_t caunt_filtr;            // ������� ����������� �� ������
    static uint16_t adc_filtr;             // �������� ����������
	static uint16_t filtr_adc [10];        // ������� ���� ���������� ��� �������� �������������� ����������
    if (adc > 800 ) {send_point (4);} else {send_non_point(4);}   // ������ ��� �������� ������ ���� ����� ������ ��� ����� ���� 
    if (param.of_adc){                                            // ���� ��� �������� ���������� ��� �������
    filtr_adc [caunt_filtr] = adc;                                // ������� �������
    for (int i = 0; i < 10; i ++) { adc_filtr += filtr_adc[i];} 
	adc_filtr =  adc_filtr / 10;             
    caunt_filtr ++;
    if (caunt_filtr == 10 )  caunt_filtr = 0; 
    if (adc_filtr > 640 && boolean.start_delay) { boolean.err_adc = false;}  // ���� ������ 640 �� ��� ��
    if (adc_filtr <= 630 && boolean.start_delay) { boolean.err_adc = !boolean.err_adc; error_buzzer(boolean.err_adc);  } // ������ 630 ��� ������
    #ifndef DEBUG_MOD                                                                                                    // � ������ �� ��������� ������
    if (adc_filtr < 580 && filtr_adc[9] !=0 ) { while(1) {sprintf(send_buf, " -OFF-" ); _delay_ms(500); LMOFF; TENZO_OFF;}}  // ������ 580 ��������� �����
    #endif     
    }             
}

void err_and_led (void)      // ����������� ������ � ���� ��� ���� ����� ������� � �����
{
     if (boolean.errors || boolean.npv || boolean.main_menu_bool || param.err36 || boolean.err_adc) {if(!boolean.led_in_menu ) {led_led();}} 
     if (!boolean.npv && !boolean.err_adc ) {error_buzzer(false);}    
}

void filtr_massa (uint8_t power)     // ��������� ������ � ������� ��� ��� �������� ������ ����� ����� ������ �������
{
     static uint8_t namber_diskret;
     static uint8_t caunter_filtr;
     static int32_t filtr_32b [50]; 
     
          
                 // ����������� ��� � �����  
     if (boolean.data_ok)  {                            //����� ������ ������� 
          boolean.data_ok = false;                      // ������� ����
          converting_cod_to_massa ();
          massa = ((massa * param.calib_koef)+0.5);     // �������� �������� ����� 
          filtr_32b [caunter_filtr] = massa;      // ������� ��������� � ������
          
          caunter_filtr ++;                       // ������������ �� ������� �������
          if (caunter_filtr  >= (power )) { caunter_filtr = 0;  }        // ������ ��� ����� ������ ��� ��� ����� ������� ������� ��� ��������
          massa_after_filtr = 0;                  // �������� ���� �� ���� ����� 
          for (int i = 0; i < (power ); i ++)  // ��������� ��� �������� ������� � ����� �� ���������� ���������
          {
               massa_after_filtr += filtr_32b[i];
          }
          massa_after_filtr = massa_after_filtr / (power );
          if (boolean.tara_ok) { massa_after_filtr = massa_after_filtr - metrology.tara; send_point (1); }    else { send_non_point(1); metrology.tara = 0;}  // ���� �������� ���� �� �������� �� ��� �� �������� ��������, ���� ��� �� ���� + ������
          if (param.sill1 == 0 && param.sill2 == 0 ){ massa_after_filtr = diskret(massa_after_filtr,param.discret1); namber_diskret = 1;}        // ���� ������ � ������ ����� = 0 �� ���������� ������ �������
          if (massa_after_filtr < param.sill1 && param.sill1 != 0){ massa_after_filtr = diskret(massa_after_filtr,param.discret1); namber_diskret = 1; }     // ���� ����� ������ ��� ������ ����� � �� �� ����� 0 �� ����� ������ �������
          if (massa_after_filtr > param.sill1 && param.sill2 == 0){ massa_after_filtr = diskret(massa_after_filtr,param.discret2); namber_diskret = 2; }     // ���� ����� ������ ��� ������ ����� � ������ ����� 0 �� ���������� ������ �������
          if (massa_after_filtr > param.sill1 && massa_after_filtr < param.sill2 && param.sill2 != 0) {massa_after_filtr = diskret(massa_after_filtr,param.discret2); namber_diskret = 2; }  // ���� ����� ������ ������� ������ � ������ ������� � ������ ����� �� 0 ���������� 2 �������
          if (massa_after_filtr > param.sill2 && param.sill2 != 0){ massa_after_filtr = diskret(massa_after_filtr,param.discret3); namber_diskret = 3;  }    // ���� ����� ������ ������� ������ � �� �� 0 �� ���������� ������ �������

          switch (namber_diskret)
          {
               case 1: if(((param.discret1 * 9) + param.NPV )< massa_after_filtr + metrology.tara) { boolean.npv = true;} else {boolean.npv = false;} // �������� ���������� ��� labs??
               if (tim_5 == 0 && !boolean.errors) { if (massa_after_filtr < last_massa + param.discret1 *2  && massa_after_filtr > last_massa - param.discret1 *2 ) \
               {send_point(3);boolean.stab = true;}  else {send_non_point(3);boolean.stab = false;}  tim_5 = (param.tim_stab * 1000); last_massa = massa_after_filtr;}  break;   
                       
               case 2: if(((param.discret2 * 9) + param.NPV )< massa_after_filtr + metrology.tara) {boolean.npv = true;} else {boolean.npv = false;} 
               if (tim_5 == 0 && !boolean.errors) { if (massa_after_filtr < last_massa + param.discret2 *2 && massa_after_filtr > last_massa - param.discret2 *2 ) \
               {send_point(3);boolean.stab = true;}  else {send_non_point(3);boolean.stab = false;}  tim_5 = (param.tim_stab * 1000); last_massa = massa_after_filtr;}  break;
                       
               case 3: if(((param.discret3 * 9) + param.NPV )< massa_after_filtr + metrology.tara) { boolean.npv = true;} else {boolean.npv = false;} 
               if (tim_5 == 0 && !boolean.errors) { if (massa_after_filtr < last_massa + param.discret3 *2  && massa_after_filtr > last_massa - param.discret3 *2 ) \
               {send_point(3);boolean.stab = true;}  else {send_non_point(3);boolean.stab = false;}  tim_5 = (param.tim_stab * 1000); last_massa = massa_after_filtr;}  break;
               // ��� ������� ������������. ���� ���������� ��� � �������� ������ �������� � ��� ������� �� ��� ��������.      
          }
     }
 
     if (boolean.npv && !boolean.errors) {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR1]))); error_buzzer(true); }  // ���� � ��� ���������� ��� �������� ����, ������, �������� �����
     (massa_after_filtr == 0 && !boolean.errors ) ?   send_point (2) : send_non_point(2);   // ���� ����� ����� 0 �� ������ ������
     if (!boolean.errors && !boolean.npv && boolean.start_delay) {sprintf(send_buf, "%6ld" , massa_after_filtr ); eror_sens_up = 0; } // ���� ��� ������ �������� �����, ���������� ������� � 0
     if(boolean.errors && boolean.start_delay) {
          switch (eror_sens_up){
               case 0:  break;
               case 1:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x01)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[0])));  }    break;
               case 2:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x02)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[1])));  }    break;
               case 3:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x04)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[2])));  }    break;
               case 4:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x08)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[3])));  }    break;     
               case 5:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x10)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[4])));  }    break;
               case 6:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x20)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[5])));  }    break;  
               case 7:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x40)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[6])));  }    break;
               case 8:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x80)   {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[7])));  }    break;    
               case 9:  if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x100)  {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[8])));  }    break;
               case 10: if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x200)  {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[9])));  }    break; 
               case 11: if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x400)  {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[10])));  }   break;
               case 12: if (eror_sens_up <= param.sensor_number  && sens_cod_led & 0x800)  {tim_6 = 0;} else { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[11])));  }   break; 
               // ��� ����� ����� ����� ������ ��������, ���� ��� ������� ��������� � ������� ����� �� ���������� �������, ���� �� ��������� ������� � ������ ����.(����� ���6)                                                                               
          }
          if (!tim_6) {tim_6 = 5000; if (eror_sens_up < param.sensor_number) {eror_sens_up ++; } else {eror_sens_up = 1; } buzers (1000);}
     }
}

void converting_cod_to_massa (void)
{
     while (1)  //������ ���������, �������� � ����, � ����� �� ���� ���� �� �������� ����, � �� ������� �� ���� ������� ������� ��������
     {
          filtr.sens1 = (((metrology.cod_sensor1 - metrology.cod_zero_sensor1) * metrology.koef_sensor1) + 0.5);
          filtr.sens2 = (((metrology.cod_sensor2 - metrology.cod_zero_sensor2) * metrology.koef_sensor2) + 0.5);
          if(param.sensor_number == 2){ massa = filtr.sens1 + filtr.sens2;  break;}

          filtr.sens3 = (((metrology.cod_sensor3 - metrology.cod_zero_sensor3) * metrology.koef_sensor3) + 0.5);
          filtr.sens4 = (((metrology.cod_sensor4 - metrology.cod_zero_sensor4) * metrology.koef_sensor4) + 0.5);
          if(param.sensor_number == 4){massa = filtr.sens1 + filtr.sens2 + filtr.sens3 + filtr.sens4; break;}
          
          filtr.sens5 = (((metrology.cod_sensor5 - metrology.cod_zero_sensor5) * metrology.koef_sensor5) + 0.5);
          filtr.sens6 = (((metrology.cod_sensor6 - metrology.cod_zero_sensor6) * metrology.koef_sensor6) + 0.5);
          if(param.sensor_number == 6){massa = filtr.sens1 + filtr.sens2 + filtr.sens3 + filtr.sens4 + filtr.sens5 + filtr.sens6 ; break;}
          
          filtr.sens7 = (((metrology.cod_sensor7 - metrology.cod_zero_sensor7) * metrology.koef_sensor7) + 0.5);
          filtr.sens8 = (((metrology.cod_sensor8 - metrology.cod_zero_sensor8) * metrology.koef_sensor8) + 0.5);
          if(param.sensor_number == 8){massa = filtr.sens1 + filtr.sens2 + filtr.sens3 + filtr.sens4 + filtr.sens5 + filtr.sens6 + filtr.sens7 + filtr.sens8 ; break;}
          
          filtr.sens9 = (((metrology.cod_sensor9 - metrology.cod_zero_sensor9) * metrology.koef_sensor9) + 0.5);
          filtr.sens10 = (((metrology.cod_sensor10 - metrology.cod_zero_sensor3) * metrology.koef_sensor3) + 0.5);
          if(param.sensor_number == 10){massa = filtr.sens1 + filtr.sens2 + filtr.sens3 + filtr.sens4 + filtr.sens5 + filtr.sens6 + filtr.sens7 + filtr.sens8 + filtr.sens9 + filtr.sens10 ; break;}
          
          filtr.sens11 = (((metrology.cod_sensor11 - metrology.cod_zero_sensor4) * metrology.koef_sensor4) + 0.5);
          filtr.sens12 = (((metrology.cod_sensor12 - metrology.cod_zero_sensor5) * metrology.koef_sensor5) + 0.5);
          if(param.sensor_number == 12){
          massa = filtr.sens1 + filtr.sens2 + filtr.sens3 + filtr.sens4 + filtr.sens5 + filtr.sens6 + filtr.sens7 + filtr.sens8 + filtr.sens9 + filtr.sens10 + filtr.sens11 + filtr.sens12  ; break;}
     }
}

void zero_funk (uint8_t eprom_ok)                      // ������� ���������. ���� �� �������� 0 �� ����� ������ ���������, � ���� 1 �� ��� � ������ � ������
{
     metrology.cod_zero_sensor1  = metrology.cod_sensor1;
     metrology.cod_zero_sensor2  = metrology.cod_sensor2;
     metrology.cod_zero_sensor3  = metrology.cod_sensor3;
     metrology.cod_zero_sensor4  = metrology.cod_sensor4;
     metrology.cod_zero_sensor5  = metrology.cod_sensor5;
     metrology.cod_zero_sensor6  = metrology.cod_sensor6;
     metrology.cod_zero_sensor7  = metrology.cod_sensor7;
     metrology.cod_zero_sensor8  = metrology.cod_sensor8;
     metrology.cod_zero_sensor9  = metrology.cod_sensor9;
     metrology.cod_zero_sensor10 = metrology.cod_sensor10;
     metrology.cod_zero_sensor11 = metrology.cod_sensor11;
     metrology.cod_zero_sensor12 = metrology.cod_sensor12;
     if (eprom_ok) {
          EEPROM_write_32t(EE_ZEROSENS1,metrology.cod_zero_sensor1);
          EEPROM_write_32t(EE_ZEROSENS2,metrology.cod_zero_sensor2);
          EEPROM_write_32t(EE_ZEROSENS3,metrology.cod_zero_sensor3);
          EEPROM_write_32t(EE_ZEROSENS4,metrology.cod_zero_sensor4);
          EEPROM_write_32t(EE_ZEROSENS5,metrology.cod_zero_sensor5);
          EEPROM_write_32t(EE_ZEROSENS6,metrology.cod_zero_sensor6);
          EEPROM_write_32t(EE_ZEROSENS7,metrology.cod_zero_sensor7);
          EEPROM_write_32t(EE_ZEROSENS8,metrology.cod_zero_sensor8);
          EEPROM_write_32t(EE_ZEROSENS9,metrology.cod_zero_sensor9);
          EEPROM_write_32t(EE_ZEROSENS10,metrology.cod_zero_sensor10);
          EEPROM_write_32t(EE_ZEROSENS11,metrology.cod_zero_sensor11);
          EEPROM_write_32t(EE_ZEROSENS12,metrology.cod_zero_sensor12);
     }
}

void no_pay_wizard (void)        // �������� �� ������������ ���36 
{
     if (param.year_err <= DS.year){ if (param.mount_err <= DS.month){if (param.data_err <= DS.date){   // ���� ���� ��������� ��� ��� ������ �� �������� ���36
          if(param.err36 == 0) { param.err36 = 1; EEPROM_write(EE_ERRPR36, param.err36);}              // �������� ���� ��� � ������ �� ��������, ���� �� ������� ������ 
           }}}
}

uint32_t diskret (uint32_t data, uint8_t dskrt)        // ������� ��������, ����� � �����
{
     static uint32_t temp;
     if (dskrt == 1) return (data);
     temp = (data / dskrt) * dskrt;
     if ((data % dskrt) >= dskrt/2) temp += dskrt;
     return (temp);
}

void TABLO_print(int32_t ves)                          // �������� � ������� �����. ���� ��������
{
     char buf_tablo[4];
     uint32_t abs_ves = labs(ves);                     // ��� ����� ����� �� ������:)
     buf_tablo[0] = (ves >= 0) ? 0 : 0x08;
     buf_tablo[0] |= ((abs_ves & 0x00010000) == 0x00010000) ? 0x40 : 0;
     buf_tablo[0] |= ((abs_ves & 0x00020000) == 0x00020000) ? 0x80 : 0;
     buf_tablo[1] = (abs_ves >> 8) & 0xff;
     buf_tablo[2] = 0x0100 + (abs_ves & 0xff);
     buf_tablo[3] = 0x24;
     uart3_send_string(buf_tablo);
}

void init_atmega (void)      // ������������� ������, ���������
{
     DDRB = 0x8F;            // ����� SPI �� ����� + ������ �� �����
     PORTB = 0x05;           // ������ �������
     
     DDRD = 0x10;            // ��� ������
     PORTD = 0x00;
     
     DDRJ = 0x20;            // ��� ��������� ������� � ��� ������������ ������� ������ �������
     PORTJ = 0x00;
     
     DDRG = 0x20;            // ��� ��������� 12 ����� �� �������
     PORTG = 0x00;
     
     DDRL = 0x00;            // ���� ������ �� ����
     PORTL = 0x3F;           // �������� � �����, ���� ���������
     
     SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0) );  //������� ���� SPI, ������� �������, �������� �� 16
     SPCR |= 0x80;           // ������� ����������
     SSON;                   // �������� ��� ������
     
     ADMUX = 0x40;           // ������� �������� �������� � ������� �����  0x40
     ADCSRA = 0xEF;          // �������� ���, �����, ����� �� �������� , �������� �������� �������������� �� 32
     DIDR0 = 0x01;           // ��������� ����� 0 �� �������� ���
     
     TCCR0B = 0x01;          // �������� 0 �� ������ 0       62 K����
     TIMSK0 = 0x01;          // �������� ���������� �� ������������
     
     TCCR1B = 0x01;          // �������� 0 �� ������ 1  + TCNT1 = 0xE800 ��� ���� ������� 2,6 ���
     TIMSK1 = 0x01;          // �������� ���������� �� ������������
     
     // TCCR2B = 0x02;          // �������� �� 8 � ���� ������� 8 �����
     //	TIMSK1 = 0x01;          // �������� ���������� �� ������������
     
     
     init_UART_tenzo();
     init_UART_tablo();
     init_Uart_0_1_start();
     LEDON;                  // ���� ���� ���� ����������
     #ifdef DEBUG_MOD        // ���� � ������� �� ������� ��������� ������ ��������
     LMON;
     TENZO_ON;
     #endif
}

void start (void)                            // ������� ��������� �������
{
     static uint16_t on_on = 0;                    // ������� ���������� �������
     while (on_on != TIM_ON){                      // ���� ���� ���������� �� ��������� �������� ��������� � �������
          if (PIN_OFF)  {                          // ���� ������ ������
               if (on_on < TIM_ON) on_on ++;       // ����������� ����������
               _delay_ms(1);
          }
          else on_on = 0;                          // ���������, �������� � ����
     }
     LMON;                                         // �������� ������� � ������
     TENZO_ON;
}

void zastavka (void)                         // �������� ��� � ������� 888888 ��� ������
{

//      indication ();                           // ���������� ������ �����
     for (int t = 0; t < 7; t++){             // � ����� �������� ���������� 8 ������ � ������ �������������, ��� �������������� ����� �������
          send_buf [t] = 0x7F;
          BZRON;                               // ������
          _delay_ms(50);                      // ����
          BZROFF;
          indication ();                       // ����������
          _delay_ms(50);                      // ����
     }
     _delay_ms(500);
      for (int t = 0; t < 6; t++){             // ������� �������� ������ ?? ����� �� ��� ��
           send_buf [t] = 0x00;
      }     
}

void off_key (void)                          // ������������ ������ ����������
{
     static uint8_t on_of = 0;                       // ������� ������ ���������� ��������
     if (PIN_OFF_OF && !boolean.non_on) {boolean.non_on = true;}     // ��� ������ ���� ������ �� ����� ���������� ���� �� ��������� ������ ��������� ����� ���������
     if (PIN_OFF && boolean.non_on)  {                               // ������ ��� �� ������� ����������� �������, ������� ����������
          if (on_of < TIM_OFF) on_of ++;
          if (on_of == 1) buzers (10000);             // ������
     }
     else on_of = 0;                                 // ���� ��������� �������� � ����
     if (on_of >= TIM_OFF ){                         // ���� �������� �������� ������� �� ����������
          sprintf(send_buf, " -OFF-" );               // �������� ���
          buzers (10000);
          LMOFF;
          TENZO_OFF;
          while (1){
               sprintf(send_buf, " -OFF-" );           // �������� ��� � ����� ��� ���� �� �������� �����
          }
     }
}

void enter_password_init (void)              // ������� ������������� ��� ����� ������
{
     if (INIT_KEY  )
     {
          
          static uint32_t mirror_data;
          static uint8_t  caunter_input;               // ������� �������
          static char itedger [10] ;                   // ������� ������
          caunter_input = 2;
          sprintf(itedger, "%06lu"  ,(uint32_t) mirror_data );
          while (1){                                                                                                                // �� ���������� ������� �������� ��������
               if (button_press (KEY_RIGHT)) { if (caunter_input < 5  ) caunter_input ++; }                                         // ������ � �����, �������� ���������
               if (button_press (KEY_LEFT))  {if (caunter_input > 2)     caunter_input --;}                                         // ���������� �������� � ������ ������� ���� �����
               if (button_press (KEY_UP)) {
                    if (itedger[caunter_input] - '0' < 9) itedger[caunter_input]  ++;                                               // ��� � � ������� ������� ��������� ����� ������ � ��� � �������� ������ � �� ����������� �����������
                    else if(itedger[caunter_input] - '0' == 9) itedger[caunter_input] = '0';
               }
               if (button_press (KEY_DOWN)) {
                    if (itedger[caunter_input] - '0'  > 0) itedger[caunter_input]  --;
                    else if(itedger[caunter_input] - '0'  == 0) itedger[caunter_input]  = '9';
               }
               mirror_data = atol (itedger);                                                                                      // ��� ����� ������� ���� � ��� �������� ��� �� ����
               sprintf(send_buf, "  %04lu"  ,(uint32_t) mirror_data );
               blink_simbol (caunter_input);                                                                                      // ������� ���������� ��������
               if (button_press (KEY_ENTER))
               {
                    if (mirror_data == PASWORD_INIT)  {  init_EEprom (); error_func (NOERROR); break;}
                    if (mirror_data == PASWORD_ERR36) {  init_err36 (); error_func (NOERROR);  break;}
                    if (mirror_data != PASWORD_INIT && mirror_data != PASWORD_ERR36) {error_func (ERROR5); break;}                // ���� ������ �� ������������� �� �����
               }
               if (button_press (KEY_ESC)) {break;}
          }
     }
}

void init_after_EEprom (void)                // ������������� ����� ������ ������
{
     init_light ();                             // ������� �������
     init_UART_0_1 (param.baud1, param.baud2);  // �������� ������
     eror_sens_up = 0;                          // ������������ �� 0 ���� ������ ��������
     tim_6 = 8000;                              // ������� �� ����� ��������
     mein_display = 3;                          // � ��� ���� �� ���������� ��� ������ �����. ���� � ��
}

