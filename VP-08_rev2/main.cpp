#include "main.h"
#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
//--------------переменные из других файлов---------------//
extern  char send_buf [20];
extern struct DS1307 DS;
extern uint16_t tim_1, tim_3,tim_5,tim_6,sens_cod_led,adc;
extern struct parametrs param;
extern struct metrologis metrology;
extern struct bool_date boolean;
//--------------переменные из этого файла-----------------//
uint8_t mein_display,eror_sens_up;
int32_t massa, last_massa,massa_after_filtr;
struct filtros filtr;
 





int main(void)
{
     init_atmega ();                 // инициализируем всю переферию
     init_max7219 ();                // инициализируем матрицу, сюда надо добавить включение с разным уровнем яркости
     for (int t = 0; t < 7; t++)     // тупая функция для того чтоб экран при включении не показывал всякую дичь!
     {send_buf [t] = 0x00;}
     indication ();
     #ifndef DEBUG_MOD                // отключаем в дебаге
     start ();                        // кнопка включения
     zastavka ();                     // и рисовалка символов
     #endif
     I2C_init();                      // инициализируем скорость I2C
     sei ();                          // разрешаем прерывания. раньше нельзя, так как тогда провалимся в SPI прерывание от стартового рисунка
     DS_Write_init_date();            // если часы были сброшены, то перевести их на 90 год и запустить
     enter_password_init();           // ввод пароля на инициализацию или сброс ерр36
     read_ALL_EEprom();               // читаем из eeprom все переменные
     init_after_EEprom();             // инициализируем те параметры что зависят от еепром
     for (;;)
     {
          
       if (!boolean.main_menu_bool) {                    // если нет ошибок и ерр36 то передаем в табло  а так же отслеживаем кнопку включения и проверяем не сработал ли ерр36
           if(boolean.funck1){boolean.funck1 = false;  if (!boolean.errors && !param.err36){TABLO_print(massa_after_filtr); }}  // с одной переодичностью делать вот это
           if(boolean.funck2){boolean.funck2 = false; DS_Read(); off_key(); parser_visl(); }
           if(boolean.funck3){boolean.funck3 = false; adc_read_of();  no_pay_wizard();  reply_visl(); }     
           get_zemik_sensors(param.sensor_number);      //                                                          // с другой переодичностью опрашивать датчики
           if (button_press (KEY_DOWN)) {  (mein_display < 2) ? mein_display ++ : mein_display = 0; }      // кнопки вверх и вниз показать основные экраны
           if (button_press (KEY_UP))   {  (mein_display > 0) ? mein_display -- : mein_display = 2;}       // впервые тернарный оператор. если больше нуля отнимаем, если ноль то делаем 2 
           if (button_press (KEY_ENTER)) {boolean.main_menu_bool = 1; init_menu();   }                     // если нажали ентер переходим в меню  
           if (!boolean.errors) {                                                                          // если еерр датчиков то не отслеживать тару и ноль      
             if ((button_press (KEY_RIGHT)) || boolean.uart_tara){  if (!boolean.tara_ok) { metrology.tara = massa_after_filtr;} boolean.tara_ok = !boolean.tara_ok; boolean.uart_tara = false;} 
             // если нажали тару, тарируем, и поднимаем флаг что тарировано
             if ((button_press (KEY_LEFT)) || boolean.uart_zero) {  if (!boolean.first_zero) { if ((param.NPV / 4) > labs(massa_after_filtr + metrology.tara)) {zero_funk(0);boolean.first_zero = true;boolean.state_zero = 2;metrology.tara = 0;}\
             else {error_func (NO);boolean.state_zero = 1;}} else {if((param.NPV / 25 )> labs(massa_after_filtr + metrology.tara)) {zero_funk(0);boolean.state_zero = 2;metrology.tara = 0;} else {error_func (NO);boolean.state_zero = 1;}}  boolean.uart_zero = false;}
             // функция обнуления, если опущен флаг первого обнуления то проверяем меньше ли текущая масса чем 25%, если да то обнуляем и поднимаем флаг первого обнуления. а если флаг поднят то проверяем уже на 4%
             if(!boolean.avto_zero) {if (param.zero_start && boolean.data_ok) { if (!boolean.first_zero) { if ((param.NPV / 4) > labs(massa_after_filtr)) {zero_funk(0);} boolean.first_zero = true; boolean.avto_zero = true;}}}
             // тоже самое только для авто нуля.                  
           }       
           if (!param.err36 && !boolean.err_adc){         // если нет ошибок основные дисплеи             

                switch (mein_display)                     // основные дисплеи
               {
                    case 0: filtr_massa(param.filtr);   break;
                    case 1: sprintf(send_buf, "%02d.%02d.%02d." ,  DS.hour,DS.min,DS.sec);    error_buzzer(false);       break;
                    case 2: sprintf(send_buf, "%02d.%02d.%02d." ,  DS.date,DS.month,DS.year); error_buzzer(false);       break;
                    case 3: if(!tim_6) { mein_display = 0; boolean.start_delay = 1; }strcpy_P(send_buf, (PGM_P)pgm_read_word(&(er_sens[12]))); adc_read_of(); filtr_massa(param.filtr); break;     // при старте показать вот это чтоб все успело включится
               }              
           } 
           else {                                        // если есть то показывать их
                   while (1)
                   {
                       if (boolean.err_adc) { strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[BAT])));  break; }     // акум
                       if (param.err36) {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR36])));  break;  }     // ерр36
                       break;       
                   }                          
                }
      }            
        if (boolean.main_menu_bool) { main_menu (); error_buzzer(false);}       // если мы в меню, то показываем только его. 
        err_and_led (); // следим за выключением светиков  
                                                                      
    }
}

void adc_read_of (void)
{
    static uint8_t caunt_filtr;            // счетчик перемещения по кейсам
    static uint16_t adc_filtr;             // итоговая переменная
	static uint16_t filtr_adc [10];        // создали дабл переменную для хранения промежуточного вычисления
    if (adc > 800 ) {send_point (4);} else {send_non_point(4);}   // зажечь или погасить светик сети можно менять при каком коде 
    if (param.of_adc){                                            // если вкл параметр отключения при разряде
    filtr_adc [caunt_filtr] = adc;                                // бегущее среднее
    for (int i = 0; i < 10; i ++) { adc_filtr += filtr_adc[i];} 
	adc_filtr =  adc_filtr / 10;             
    caunt_filtr ++;
    if (caunt_filtr == 10 )  caunt_filtr = 0; 
    if (adc_filtr > 640 && boolean.start_delay) { boolean.err_adc = false;}  // если больше 640 то все ок
    if (adc_filtr <= 630 && boolean.start_delay) { boolean.err_adc = !boolean.err_adc; error_buzzer(boolean.err_adc);  } // меньше 630 уже разряд
    #ifndef DEBUG_MOD                                                                                                    // в дебаге не выключать прибор
    if (adc_filtr < 580 && filtr_adc[9] !=0 ) { while(1) {sprintf(send_buf, " -OFF-" ); _delay_ms(500); LMOFF; TENZO_OFF;}}  // меньше 580 выключить нафиг
    #endif     
    }             
}

void err_and_led (void)      // отслеживаем ошибки и если они есть гасим светики и бузер
{
     if (boolean.errors || boolean.npv || boolean.main_menu_bool || param.err36 || boolean.err_adc) {if(!boolean.led_in_menu ) {led_led();}} 
     if (!boolean.npv && !boolean.err_adc ) {error_buzzer(false);}    
}

void filtr_massa (uint8_t power)     // объеденил фильтр и дискрет так как вызываем только тогда когда данные приняты
{
     static uint8_t namber_diskret;
     static uint8_t caunter_filtr;
     static int32_t filtr_32b [50]; 
     
          
                 // преобразуем код в массу  
     if (boolean.data_ok)  {                            //когда данные приняли 
          boolean.data_ok = false;                      // сбросим флаг
          converting_cod_to_massa ();
          massa = ((massa * param.calib_koef)+0.5);     // получаем реальную массу 
          filtr_32b [caunter_filtr] = massa;      // заносим показания в фильтр
          
          caunter_filtr ++;                       // перемещаемся по массиву фильтра
          if (caunter_filtr  >= (power )) { caunter_filtr = 0;  }        // больше или равно потому что при смене размера фильтра все ложиться
          massa_after_filtr = 0;                  // обнуляем чтоб не было лагов 
          for (int i = 0; i < (power ); i ++)  // суммируем все элементы массива и делим на количество элементов
          {
               massa_after_filtr += filtr_32b[i];
          }
          massa_after_filtr = massa_after_filtr / (power );
          if (boolean.tara_ok) { massa_after_filtr = massa_after_filtr - metrology.tara; send_point (1); }    else { send_non_point(1); metrology.tara = 0;}  // если включена тара то отнимать ее вес от текущего значения, если нет то ноль + светик
          if (param.sill1 == 0 && param.sill2 == 0 ){ massa_after_filtr = diskret(massa_after_filtr,param.discret1); namber_diskret = 1;}        // если первый и второй порог = 0 то показываем первый дискрет
          if (massa_after_filtr < param.sill1 && param.sill1 != 0){ massa_after_filtr = diskret(massa_after_filtr,param.discret1); namber_diskret = 1; }     // если масса меньше чем первый порог и он не равен 0 то снова первый дискрет
          if (massa_after_filtr > param.sill1 && param.sill2 == 0){ massa_after_filtr = diskret(massa_after_filtr,param.discret2); namber_diskret = 2; }     // если масса больше чем первый порог а второй равен 0 то показываем второй дискрет
          if (massa_after_filtr > param.sill1 && massa_after_filtr < param.sill2 && param.sill2 != 0) {massa_after_filtr = diskret(massa_after_filtr,param.discret2); namber_diskret = 2; }  // если масса больше первого порога и меньше второго и второй порог не 0 показываем 2 дискрет
          if (massa_after_filtr > param.sill2 && param.sill2 != 0){ massa_after_filtr = diskret(massa_after_filtr,param.discret3); namber_diskret = 3;  }    // если масса больше второго порога и он не 0 то показываем третий дискрет

          switch (namber_diskret)
          {
               case 1: if(((param.discret1 * 9) + param.NPV )< massa_after_filtr + metrology.tara) { boolean.npv = true;} else {boolean.npv = false;} // показать превышение НПВ labs??
               if (tim_5 == 0 && !boolean.errors) { if (massa_after_filtr < last_massa + param.discret1 *2  && massa_after_filtr > last_massa - param.discret1 *2 ) \
               {send_point(3);boolean.stab = true;}  else {send_non_point(3);boolean.stab = false;}  tim_5 = (param.tim_stab * 1000); last_massa = massa_after_filtr;}  break;   
                       
               case 2: if(((param.discret2 * 9) + param.NPV )< massa_after_filtr + metrology.tara) {boolean.npv = true;} else {boolean.npv = false;} 
               if (tim_5 == 0 && !boolean.errors) { if (massa_after_filtr < last_massa + param.discret2 *2 && massa_after_filtr > last_massa - param.discret2 *2 ) \
               {send_point(3);boolean.stab = true;}  else {send_non_point(3);boolean.stab = false;}  tim_5 = (param.tim_stab * 1000); last_massa = massa_after_filtr;}  break;
                       
               case 3: if(((param.discret3 * 9) + param.NPV )< massa_after_filtr + metrology.tara) { boolean.npv = true;} else {boolean.npv = false;} 
               if (tim_5 == 0 && !boolean.errors) { if (massa_after_filtr < last_massa + param.discret3 *2  && massa_after_filtr > last_massa - param.discret3 *2 ) \
               {send_point(3);boolean.stab = true;}  else {send_non_point(3);boolean.stab = false;}  tim_5 = (param.tim_stab * 1000); last_massa = massa_after_filtr;}  break;
               // это строчки стабилизации. если предыдущий вес в пределах одного дискрета в обе стороны то вес стабилен.      
          }
     }
 
     if (boolean.npv && !boolean.errors) {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR1]))); error_buzzer(true); }  // если у нас превышение НПВ показать ерор, пикать, погасить лампы
     (massa_after_filtr == 0 && !boolean.errors ) ?   send_point (2) : send_non_point(2);   // если масса равна 0 то зажечь светик
     if (!boolean.errors && !boolean.npv && boolean.start_delay) {sprintf(send_buf, "%6ld" , massa_after_filtr ); eror_sens_up = 0; } // если нет ошибок показать массу, перекинуть счетчик в 0
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
               // вот такой тупой показ ероров датчиков, если код датчика совпадает с номером кейса то показываем оошибку, если не совпадает прыгаем в другой кейс.(через тим6)                                                                               
          }
          if (!tim_6) {tim_6 = 5000; if (eror_sens_up < param.sensor_number) {eror_sens_up ++; } else {eror_sens_up = 1; } buzers (1000);}
     }
}

void converting_cod_to_massa (void)
{
     while (1)  //логика следующая, попадаем в вайл, и летим по нему пока не встретим бряк, а он зависит от того сколько выбрано датчиков
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

void zero_funk (uint8_t eprom_ok)                      // функция обнуления. если ей передать 0 то будет просто обнуление, а если 1 то еще и запись в еепром
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

void no_pay_wizard (void)        // проверка на срабатывание ерр36 
{
     if (param.year_err <= DS.year){ if (param.mount_err <= DS.month){if (param.data_err <= DS.date){   // если дата совпадает или уже меньше то включить ерр36
          if(param.err36 == 0) { param.err36 = 1; EEPROM_write(EE_ERRPR36, param.err36);}              // записать один раз и больше не заходить, чтоб не убивать еепром 
           }}}
}

uint32_t diskret (uint32_t data, uint8_t dskrt)        // функция дискрета, взята с впшки
{
     static uint32_t temp;
     if (dskrt == 1) return (data);
     temp = (data / dskrt) * dskrt;
     if ((data % dskrt) >= dskrt/2) temp += dskrt;
     return (temp);
}

void TABLO_print(int32_t ves)                          // передача в токовую петлю. тоже украдено
{
     char buf_tablo[4];
     uint32_t abs_ves = labs(ves);                     // это взять число по модулю:)
     buf_tablo[0] = (ves >= 0) ? 0 : 0x08;
     buf_tablo[0] |= ((abs_ves & 0x00010000) == 0x00010000) ? 0x40 : 0;
     buf_tablo[0] |= ((abs_ves & 0x00020000) == 0x00020000) ? 0x80 : 0;
     buf_tablo[1] = (abs_ves >> 8) & 0xff;
     buf_tablo[2] = 0x0100 + (abs_ves & 0xff);
     buf_tablo[3] = 0x24;
     uart3_send_string(buf_tablo);
}

void init_atmega (void)      // инициализация портов, переферии
{
     DDRB = 0x8F;            // Ножки SPI на выход + светик на выход
     PORTB = 0x05;           // низкий уровень
     
     DDRD = 0x10;            // пин бузера
     PORTD = 0x00;
     
     DDRJ = 0x20;            // пин включения питания и пин отслеживания нажатой кнопки питания
     PORTJ = 0x00;
     
     DDRG = 0x20;            // пин включения 12 вольт на датчики
     PORTG = 0x00;
     
     DDRL = 0x00;            // пины кнопок на вход
     PORTL = 0x3F;           // подтяжка к плюсу, пока внутрення
     
     SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0) );  //Включим шину SPI, объявим ведущим, делитель на 16
     SPCR |= 0x80;           // включим прерывание
     SSON;                   // поднимем чип селект
     
     ADMUX = 0x40;           // внешний источник опорного и нулевой канал  0x40
     ADCSRA = 0xEF;          // включить ацп, старт, старт по триггеру , делитель скорости преобразования на 32
     DIDR0 = 0x01;           // отключить канал 0 от регистра ддр
     
     TCCR0B = 0x01;          // делитель 0 на таймер 0       62 KГерц
     TIMSK0 = 0x01;          // включить прерывания по переполнению
     
     TCCR1B = 0x01;          // делитель 0 на таймер 1  + TCNT1 = 0xE800 это дает частоту 2,6 КГц
     TIMSK1 = 0x01;          // включить прерывания по переполнению
     
     // TCCR2B = 0x02;          // делитель на 8 и того частота 8 КГерц
     //	TIMSK1 = 0x01;          // включить прерывания по переполнению
     
     
     init_UART_tenzo();
     init_UART_tablo();
     init_Uart_0_1_start();
     LEDON;                  // пока нету пина блокировки
     #ifdef DEBUG_MOD        // чтоб в отладке не вводить постоянно пароли конченые
     LMON;
     TENZO_ON;
     #endif
}

void start (void)                            // функция включения прибора
{
     static uint16_t on_on = 0;                    // создаем переменную счетчик
     while (on_on != TIM_ON){                      // ждем пока переменная не достигнет значения указаного в дефайне
          if (PIN_OFF)  {                          // если кнопка нажата
               if (on_on < TIM_ON) on_on ++;       // увеличиваем переменную
               _delay_ms(1);
          }
          else on_on = 0;                          // отпустили, сбросили в ноль
     }
     LMON;                                         // включили датчики и прибор
     TENZO_ON;
}

void zastavka (void)                         // печатаем как в диниках 888888 при старте
{

//      indication ();                           // показываем пустой экран
     for (int t = 0; t < 7; t++){             // в цикле начинаем записывать 8 только в другом представлении, без преобразования через таблицу
          send_buf [t] = 0x7F;
          BZRON;                               // пикаем
          _delay_ms(50);                      // ждем
          BZROFF;
          indication ();                       // показываем
          _delay_ms(50);                      // ждем
     }
     _delay_ms(500);
      for (int t = 0; t < 6; t++){             // сначала забиваем нулями ?? нужно ли это хз
           send_buf [t] = 0x00;
      }     
}

void off_key (void)                          // отслеживание кнопки выключения
{
     static uint8_t on_of = 0;                       // создаем новыую переменную счетчика
     if (PIN_OFF_OF && !boolean.non_on) {boolean.non_on = true;}     // это защита чтоб прибор не начал выключатся пока не отпустишь кнопку включения после включения
     if (PIN_OFF && boolean.non_on)  {                               // дальше все по класике отслеживаем нажатие, считаем переменную
          if (on_of < TIM_OFF) on_of ++;
          if (on_of == 1) buzers (10000);             // пикаем
     }
     else on_of = 0;                                 // если отпустили сбросили в ноль
     if (on_of >= TIM_OFF ){                         // если достигли величины дефайна на отключение
          sprintf(send_buf, " -OFF-" );               // печатаем офф
          buzers (10000);
          LMOFF;
          TENZO_OFF;
          while (1){
               sprintf(send_buf, " -OFF-" );           // печатаем офф и висим тут пока не погаснет экран
          }
     }
}

void enter_password_init (void)              // функция инициализации при вводе пароля
{
     if (INIT_KEY  )
     {
          
          static uint32_t mirror_data;
          static uint8_t  caunter_input;               // создаем счетчик
          static char itedger [10] ;                   // создаем буффер
          caunter_input = 2;
          sprintf(itedger, "%06lu"  ,(uint32_t) mirror_data );
          while (1){                                                                                                                // то количество сколько символов передали
               if (button_press (KEY_RIGHT)) { if (caunter_input < 5  ) caunter_input ++; }                                         // кнопка в право, сместить указатель
               if (button_press (KEY_LEFT))  {if (caunter_input > 2)     caunter_input --;}                                         // аналогично сместить в другую сторону если можна
               if (button_press (KEY_UP)) {
                    if (itedger[caunter_input] - '0' < 9) itedger[caunter_input]  ++;                                               // как и в функции времени проверяем какой символ у нас в элементе буфера и по возможности увеличиваем
                    else if(itedger[caunter_input] - '0' == 9) itedger[caunter_input] = '0';
               }
               if (button_press (KEY_DOWN)) {
                    if (itedger[caunter_input] - '0'  > 0) itedger[caunter_input]  --;
                    else if(itedger[caunter_input] - '0'  == 0) itedger[caunter_input]  = '9';
               }
               mirror_data = atol (itedger);                                                                                      // тут сканф заменил атол и все работает так же само
               sprintf(send_buf, "  %04lu"  ,(uint32_t) mirror_data );
               blink_simbol (caunter_input);                                                                                      // моргаем изменяемым символом
               if (button_press (KEY_ENTER))
               {
                    if (mirror_data == PASWORD_INIT)  {  init_EEprom (); error_func (NOERROR); break;}
                    if (mirror_data == PASWORD_ERR36) {  init_err36 (); error_func (NOERROR);  break;}
                    if (mirror_data != PASWORD_INIT && mirror_data != PASWORD_ERR36) {error_func (ERROR5); break;}                // если пароль не соответствует то еррор
               }
               if (button_press (KEY_ESC)) {break;}
          }
     }
}

void init_after_EEprom (void)                // инициализация после чтения еепром
{
     init_light ();                             // яркость матрицы
     init_UART_0_1 (param.baud1, param.baud2);  // скорость юартов
     eror_sens_up = 0;                          // переместимся на 0 кейс ошибок датчиокв
     tim_6 = 8000;                              // задерка на показ символов
     mein_display = 3;                          // а это чтоб не показывать при старте фигню. ерор и тд
}

