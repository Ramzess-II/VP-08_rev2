#include "Menu.h"
#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
#include "main.h"

//--------------переменные из других файлов---------------//
extern bool spi_stop;
extern uint8_t main_menu_bool,data_ok;
extern uint16_t tim_0,tim_1,tim_3,tim_4,tim_5, adc;
extern int32_t massa,last_massa;
extern  char send_buf [20];
extern struct DS1307 DS;
extern struct bool_date boolean;
//--------------переменные из этого файла-----------------//
bool go_func, change_item, non_repit,super_parol, admin_parol;   
uint8_t nul_enty = 0, mirror_data_8bit;
int32_t mirror_data;
double mirror_double, mirror_double2;             
void(* on_clicks)(void);                    // создаем указатель на функцию
typedef void (*StateFunc) (void);           // создаем новый тип, теперь StateFunc указатель на void (*) (void);
struct parametrs param;
struct metrologis metrology;
double *point_metrologi;                    // указатель на элемент структуры




typedef struct  PROGMEM {               // структура меню
	const char format[10];			    // Имя меню
	void(* on_click)(void);      	    // Ссылка на функцию вызова обрабоки изменения значения, параметр +1 или -1)
	uint8_t current_menu;
	uint8_t child_menu;
	uint8_t parent_menu;
	uint8_t next_menu;
	uint8_t previos_menu;
	uint8_t  key_state;
} MenuItem;

MenuItem *pointer_menu ;                 //*pointer_menu указатель на структуру MenuItem

MenuItem const MenuItems[40] = {         // создаем массив структур меню
	//  номер         название     функция                           имя меню             ребенок        родитель      след_пункт      пред_пункт     обработка клавиш
	
	[PAR1]       =  {"PAR1",          &void_func,   	               PAR1,              PAR1_1,        PAR1 ,          PAR2 ,         PAR7 ,             SKROL_MENU },
	[PAR1_1]     =  {"PAR1.1",        &ver_po,                         PAR1_1,            NUL_PTR,       PAR1 ,          PAR1_2 ,       PAR1_3 ,           DATA_INPUT },
	[PAR1_2]     =  {"PAR1.2",        &send_adc,                       PAR1_2,            NUL_PTR,       PAR1 ,          PAR1_3 ,       PAR1_1 ,           DATA_INPUT },
	[PAR1_3]     =  {"PAR1.3",        &caunter_weighing,               PAR1_3,            NUL_PTR,       PAR1 ,          PAR1_1 ,       PAR1_2 ,           DATA_INPUT },
	[PAR2]       =  {"PAR2",          &void_func,   	               PAR2,              PAR2_1,        PAR2 ,          PAR3 ,         PAR1 ,             SKROL_MENU },
	[PAR2_1]     =  {"PAR2.1",        &diskret1_new,   	               PAR2_1,            NUL_PTR,       PAR2 ,          PAR2_2 ,       PAR2_17 ,          DATA_INPUT },
	[PAR2_2]     =  {"PAR2.2",        &discret2,   	                   PAR2_2,            NUL_PTR,       PAR2 ,          PAR2_3 ,       PAR2_1 ,           DATA_INPUT },
	[PAR2_3]     =  {"PAR2.3",        &discret3,   	                   PAR2_3,            NUL_PTR,       PAR2 ,          PAR2_4 ,       PAR2_2 ,           DATA_INPUT },
	[PAR2_4]     =  {"PAR2.4",        &sill1_discrt,   	               PAR2_4,            NUL_PTR,       PAR2 ,          PAR2_5 ,       PAR2_3 ,           DATA_INPUT },
	[PAR2_5]     =  {"PAR2.5",        &sill2_discrt,   	               PAR2_5,            NUL_PTR,       PAR2 ,          PAR2_6 ,       PAR2_4 ,           DATA_INPUT },
	[PAR2_6]     =  {"PAR2.6",        &in_npv,   	                   PAR2_6,            NUL_PTR,       PAR2 ,          PAR2_7 ,       PAR2_5 ,           DATA_INPUT },
	[PAR2_7]     =  {"PAR2.7",        &sensor_numbers,   	           PAR2_7,            NUL_PTR,       PAR2 ,          PAR2_8 ,       PAR2_6 ,           DATA_INPUT },
	[PAR2_8]     =  {"PAR2.8",        &tips_cd,   	                   PAR2_8,            NUL_PTR,       PAR2 ,          PAR2_9 ,       PAR2_7 ,           DATA_INPUT },
	[PAR2_9]     =  {"PAR2.9",        &baud_rate1,   	               PAR2_9,            NUL_PTR,       PAR2 ,          PAR2_10 ,      PAR2_8 ,           DATA_INPUT },
	[PAR2_10]    =  {"PAR2.10",       &baud_rate2,   	               PAR2_10,           NUL_PTR,       PAR2 ,          PAR2_11 ,      PAR2_9 ,           DATA_INPUT },
	[PAR2_11]    =  {"PAR2.11",       &protocol_data1,                 PAR2_11,           NUL_PTR,       PAR2 ,          PAR2_12 ,      PAR2_10 ,          DATA_INPUT },
	[PAR2_12]    =  {"PAR2.12",       &protocol_data2,   	           PAR2_12,           NUL_PTR,       PAR2 ,          PAR2_13 ,      PAR2_11 ,          DATA_INPUT },
	[PAR2_13]    =  {"PAR2.13",       &filtr_data,   	               PAR2_13,           NUL_PTR,       PAR2 ,          PAR2_14 ,      PAR2_12 ,          DATA_INPUT },
	[PAR2_14]    =  {"PAR2.14",       &light,   	                   PAR2_14,           NUL_PTR,       PAR2 ,          PAR2_15 ,      PAR2_13 ,          DATA_INPUT },
    [PAR2_15]    =  {"PAR2.15",       &zero_in_start,   	           PAR2_15,           NUL_PTR,       PAR2 ,          PAR2_16 ,      PAR2_14 ,          DATA_INPUT }, 
    [PAR2_16]    =  {"PAR2.16",       &tim_stab_func,   	           PAR2_16,           NUL_PTR,       PAR2 ,          PAR2_17 ,      PAR2_15 ,          DATA_INPUT }, 
    [PAR2_17]    =  {"PAR2.17",       &of_adc_devise,   	           PAR2_17,           NUL_PTR,       PAR2 ,          PAR2_1 ,       PAR2_16 ,          DATA_INPUT },            
	[PAR3]       =  {"PAR3",          &void_func,   	               PAR3,              PAR3_1,        PAR3 ,          PAR4 ,         PAR2 ,             SKROL_MENU },
	[PAR3_1]     =  {"PAR3.1",        &enter_time,   	               PAR3_1,            NUL_PTR,       PAR3 ,          PAR3_2 ,       PAR3_2 ,           DATA_INPUT },
	[PAR3_2]     =  {"PAR3.2",        &enter_date,   	               PAR3_2,            NUL_PTR,       PAR3 ,          PAR3_1 ,       PAR3_1 ,           DATA_INPUT },        
	[PAR4]       =  {"PAR4",          &void_func,   	               PAR4,              PAR4_1,        PAR4 ,          PAR5 ,         PAR3 ,             SKROL_MENU },
	[PAR4_1]     =  {"PAR4.1",        &void_func,   	               PAR4_1,            PAR4_1_1,      PAR4 ,          PAR4_2 ,       PAR4_2 ,           SKROL_MENU },
    [PAR4_1_1]   =  {"PAR4.1.1",      &kal_koef,   	                   PAR4_1_1,          NUL_PTR,       PAR4_1 ,        PAR4_1_2 ,     PAR4_1_2 ,         DATA_INPUT },
    [PAR4_1_2]   =  {"PAR4.1.2",      &kalib_vp,   	                   PAR4_1_2,          NUL_PTR,       PAR4_1 ,        PAR4_1_1 ,     PAR4_1_1 ,         DATA_INPUT },          
	[PAR4_2]     =  {"PAR4.2",        &individual_koef,   	           PAR4_2,            NUL_PTR,       PAR4 ,          PAR4_1 ,       PAR4_1 ,           DATA_INPUT },    
	[PAR5]       =  {"PAR5",          &adress_sens,   	               PAR5,              NUL_PTR,       PAR5 ,          PAR6 ,         PAR4 ,             NO_CHILDREN },
	[PAR6]       =  {"PAR6",          &Pasword_press,   	           PAR6,              NUL_PTR,       PAR6 ,          PAR7 ,         PAR5 ,             NO_CHILDREN },     
    [PAR7]       =  {"PAR7",          &enter_no_pay,   	               PAR7,              NUL_PTR,       PAR7 ,          PAR1 ,         PAR6 ,             NO_CHILDREN },         
};

void main_menu (void)                // функция меин меню
{
	_key_strob ();                   // отслеживание кнопок
	if (go_func) { on_clicks ();}    // если поднят флаг что мы выполняем функции, то выполнять функцию указатель на которую хранится в массиве структур меню
	else	strcpy_P(send_buf, pointer_menu->format );     // если флаг опущен то показывать имя меню 
}

uint8_t not_change (void)            // проверка на введеные пароли
{
   if (super_parol) {return 1;}      // если это супер пароль, сразу доступ
   else if (admin_parol && BLK) {return 1;}     // если пароль админа то еще проверяем блокировку
   else {return 0; }                 // если ничего то вернуть 0
}

void _key_strob (void)               // функция обработки клавиш в меню
{
	switch ((pgm_read_byte(&pointer_menu->key_state)))      // сюда передаем считаный из массива структур меню номер, и в зависимости от того какой это номер по разному обрабатываем клавиши
	{
		case (SKROL_MENU):                                  // если это прокрутка меню
		if (button_press (KEY_UP)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->next_menu))]); }       // при нажатии на кнопку переходим в пункт меню который указан как следующий
		if (button_press (KEY_DOWN)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->previos_menu))]); }  // как предыдущий
		if (button_press (KEY_ENTER)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->child_menu))]);  }  // переходим к ребенку
		if (button_press (KEY_ESC))  { if(((&MenuItems[(pgm_read_byte(&pointer_menu->parent_menu))]) == (&MenuItems[(pgm_read_byte(&pointer_menu->current_menu))])))\
            { boolean.main_menu_bool = 0;  }else {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->parent_menu))]) ;}}  
        // тут мы проверяем если у пункта имя родителя совпадает с именем меню, то выходим из меню, а если не совпадает то переходим к родителю.     
		break;
		case (DATA_INPUT):                                   // если это ввод данных
		if (!go_func){                                       // и функция меню не выполняется
			if (button_press (KEY_UP)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->next_menu))]); }       // при нажатии на кнопку переходим в пункт меню который указан как следующий
			if (button_press (KEY_DOWN)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->previos_menu))]); }  // как предыдущий
			if (button_press (KEY_ESC)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->parent_menu))]); boolean.led_in_menu = false;}     // возвращаемся к родителю
			if (button_press (KEY_ENTER)) {on_clicks = (StateFunc)  pgm_read_ptr(&MenuItems[(pgm_read_byte(&pointer_menu->current_menu))].on_click);  go_func = true;}  // начинаем выполнять функцию меню
		}
		if (go_func){                                                                                                         // если мы находимся в выполнении функции
			if (change_item) if (button_press (KEY_ESC)) {  change_item = false; non_repit = false; boolean.led_display = 0;*point_metrologi = mirror_double2;boolean.led_in_menu = false;} // если мы изменяли параметры то по ескейпу перестаем это делать
			if (!change_item){                                                                                                // если мы изменяем параметры
				if (button_press (KEY_ESC)) { go_func = false; boolean.non_rep = false;boolean.led_display = 0; boolean.led_in_menu = false;}	          // если мы не изменяем параметры то по ескейпу выходим
			}                              // плюс два костыля, без них не работает ввод коэф для каждого датчика
		}
		break;
		case (NO_CHILDREN):                                   // если это ввод данных без захода в функцию
		if (!go_func){                                       // и функция меню не выполняется
     		if (button_press (KEY_UP)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->next_menu))]); }       // при нажатии на кнопку переходим в пункт меню который указан как следующий
     		if (button_press (KEY_DOWN)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->previos_menu))]); }  // как предыдущий
     		if (button_press (KEY_ESC))  {boolean.main_menu_bool = 0; }                              // если нажали ескейп покинули меню
     		if (button_press (KEY_ENTER)) {on_clicks = (StateFunc)  pgm_read_ptr(&MenuItems[(pgm_read_byte(&pointer_menu->current_menu))].on_click);  go_func = true;}  // начинаем выполнять функцию меню
		}
		if (go_func){                                                                                                         // если мы находимся в выполнении функции
     		if (change_item) if (button_press (KEY_ESC)) {  change_item = false; non_repit = false; }                         // если мы изменяли параметры то по ескейпу перестаем это делать
     		if (!change_item){                                                                                                // если мы изменяем параметры
          		if (button_press (KEY_ESC)) { go_func = false; boolean.change_item_mirr = false;}	                          // если мы не изменяем параметры то по ескейпу выходим
     		}
		}
		break;        
	}
}

void ver_po (void)                   // показать версию ПО. пока топорно
{
	sprintf(send_buf, "UER1.00" );   // печатаем версию
}

void send_adc (void)                 // показать напряжение АКБ в вольтах
{
    static uint8_t caunter_filtr;
    static double filtrs_adc [10];
    static double n;        // создали дабл переменную для хранения промежуточного вычисления
	if (tim_1 == 0) {  
    filtrs_adc[caunter_filtr] = (double) adc / 71;   
    for (int i = 0; i < 10; i ++) { n += filtrs_adc[i];}  
    n = n / 10;  
    caunter_filtr ++;   
    if (caunter_filtr == 10 )  caunter_filtr = 0;        
	sprintf (send_buf, "BAT%04.1f" ,  n );   // функция вывода 
	if(filtrs_adc [9] == 0) {tim_1 = 500;} else {tim_1 = 5000;}}          // пока в последнем символе 0 очень быстро набрать
}

void caunter_weighing (void)         // счетчик взвешиваний пока не реализовано
{
	sprintf(send_buf, "%06lu" , (uint32_t) param.caunter_weight );
}

void diskret1_new (void)             // функция ввода дискрета новая, где можно просто перебирать значения а не вводить
{
   input_discret (&param.discret1,1);
}

void discret2 (void)
{
   input_discret (&param.discret2,2);
}

void discret3 (void)
{
   input_discret (&param.discret3,3);
}

void input_discret (uint8_t *inbaud, uint8_t nomer)
{
      if (!change_item){                    // это флаг того что мы изменяем параметры или нет
      if (button_press (KEY_ENTER)) {change_item = true;}         // пока не изменяем параметры отслеживаем кнопку ентер, как только нажали поднимаем флаг что мы изменяем параметры
           sprintf(send_buf, "   %03lu" , (uint32_t) *inbaud ); // пока не изменяем выводим текущее значение параметра на экран
           switch (*inbaud){                                    // тупая функция но все же. сюда передаем значение хранящиеся в памяти и в зависимости от этого присваиваем промежуточной переменной   
                case 1:   mirror_data_8bit = 0; break;          // значение из кейсов для того чтоб при входе в изменение показывали текущий пункт перед изменением а не нулевой!
                case 2:   mirror_data_8bit = 1; break;
                case 5:   mirror_data_8bit = 2; break;
                case 10:  mirror_data_8bit = 3; break;
                case 20:  mirror_data_8bit = 4; break;
                case 50:  mirror_data_8bit = 5; break;
                case 100: mirror_data_8bit = 6; break;
                case 200: mirror_data_8bit = 7; break;
           }
      }
      if (change_item){                                                // теперь если мы изменяем параметры
           if (button_press (KEY_UP)) { if (mirror_data_8bit < 8) mirror_data_8bit ++;  }   // отслеживаем кнопку вверх и пока промежуточное значение меньше 8 плюсуем его
           if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 0) mirror_data_8bit --; }   // ну или же минусуем
           switch (mirror_data_8bit){                                                       // в зависимости от того до куда доклацали нашу переменную присваиваем новое значение из кейса
                case 0: mirror_data = 1; break;
                case 1: mirror_data = 2; break;
                case 2: mirror_data = 5; break;
                case 3: mirror_data = 10; break;
                case 4: mirror_data = 20; break;
                case 5: mirror_data = 50; break;
                case 6: mirror_data = 100; break;
                case 7: mirror_data = 200; break;
           }
           blink_many_simbol (0b0111000);	                                                 // мигаем 3мя символами
           sprintf(send_buf, "   %03lu"  ,(uint32_t) mirror_data );                         // показываем текущее значение со смещением вправо и 3мя знаками. если там пусто показывать нули
           if (button_press (KEY_ENTER)) {                                                      // при нажатии ентер применяем значение (нужно добавить еепром)
                change_item = false; non_repit = false;                                         // выйти из режима изменения параметров, сбросить флаг единоразового выполнения при входе в функцию
                if (nomer == 1){ param.discret1 = mirror_data;  EEPROM_write(EE_DISCKRET1,param.discret1); }            // присвоить измененное значение как новое
                if (nomer == 2){ param.discret2 = mirror_data;  EEPROM_write(EE_DISCKRET2,param.discret2); }
                if (nomer == 3){ param.discret3 = mirror_data;  EEPROM_write(EE_DISCKRET3,param.discret3); }
           }
      }    
}

void sill1_discrt (void)              // функция ввода порога   
{
	if (!change_item){                // пока мы не вводим данные   
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // отслеживаем ентер и если что переходим в выполнение ввода
		sprintf(send_buf, "%06lu" , (uint32_t) param.sill1 );mirror_data = param.sill1;    // показываем значение параметра и если там пусто добавить нулями
	}
	if (change_item){                                                                      // если мы вводим данные 
		enter_numbers (5);                                                                 // передаем в функцию ввода данных нашу переменную и количество знаков для ввода
		if (button_press (KEY_ENTER)) {                                                    // отслеживаем ентер (добавит еепром)
			change_item = false; non_repit = false;                                        // выходим из функции изменения
			param.sill1 = mirror_data;  EEPROM_write_32t(EE_SILL1,param.sill1); }          // приравниваем новоезначение в параметр
	}
}

void sill2_discrt (void)
{
	if (!change_item){
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}
		sprintf(send_buf, "%06lu" , (uint32_t) param.sill2 );mirror_data = param.sill2;
	}
	if (change_item){
		enter_numbers (5);
		if (button_press (KEY_ENTER)) {
			change_item = false; non_repit = false;
			param.sill2 = mirror_data; EEPROM_write_32t(EE_SILL2,param.sill2); }
	}
}

void in_npv (void)                     // аналогичная функция для ввода НПВ
{
	if (!change_item){
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}
		sprintf(send_buf, "%06lu" , (uint32_t) param.NPV );mirror_data = param.NPV;
	}
	if (change_item){
		enter_numbers (5);
		if (button_press (KEY_ENTER)) {
             change_item = false; non_repit = false;
             if (mirror_data > 0) {
             param.NPV = mirror_data; EEPROM_write_32t(EE_NPV, param.NPV); 
             }
             else { error_func(ERROR5);}             
        }
	}
}

void sensor_numbers (void)             // функция для ввода количества датчиков
{
	if (!change_item){                 // пока не изменяем параметры
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}         // отслеживаем ентер
		sprintf(send_buf, "    %02lu" , (uint32_t) param.sensor_number );mirror_data_8bit = param.sensor_number;   // и выводим текущее значение на экран
	}
	if (change_item){                                               // когда изменяем значение
		if (button_press (KEY_UP)) { if (mirror_data_8bit < 12) mirror_data_8bit += 2;  }             // если кнопка вверх +2      
		if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 2) mirror_data_8bit -=2; }               // кнопка вниз -2   и это все для зеркала а не для функции чтоб если не нажму ентер не сохранилось
		blink_many_simbol (0x30);                                                                     // мигаем двумя правыми символами
		sprintf(send_buf, "    %02lu"  ,(uint32_t) mirror_data_8bit );                                // выводим зеркало чтоб показать что мы изменили
		if (button_press (KEY_ENTER)) {                                                               // в случае нажатия на ентер сохраняем новое значение
        param.sensor_number = mirror_data_8bit; EEPROM_write(EE_SENNUMBER,param.sensor_number);
        change_item = false; non_repit = false;
        }   
	}
}

void tips_cd (void)                    // аналогичная функция для ввода типа датчика
{
     enter_8bit (1,2,&param.tip_cd,EE_TIPCD,1); 
}

void baud_rate1 (void)                 // короче для одинаковых функций ввода решил сделать обобщающую функцию, это экономит память
{
  input_baud (&param.baud1,1);
}

void baud_rate2 (void)
{
     input_baud (&param.baud2,2);
}

void input_baud (uint8_t *inbaud, uint8_t nomer)       // сюда передаем ссылку на переменную
{
     if (!change_item){                    // это флаг того что мы изменяем параметры или нет
          if (button_press (KEY_ENTER)) {change_item = true;}         // пока не изменяем параметры отслеживаем кнопку ентер, как только нажали поднимаем флаг что мы изменяем параметры
          switch (*inbaud){                                           // тупая функция но все же. сюда передаем значение хранящиеся в памяти и в зависимости от этого присваиваем промежуточной переменной
               // значение из кейсов для того чтоб при входе в изменение показывали текущий пункт перед изменением а не нулевой!
               case 0:    mirror_data_8bit = 0;sprintf (send_buf, "   600"    ); break;             
               case 1:    mirror_data_8bit = 1;sprintf (send_buf, "  2400"   ); break;          //плюс выводим по разному для красивого отображения
               case 2:    mirror_data_8bit = 2;sprintf (send_buf, "  9600"   ); break;
               case 3:    mirror_data_8bit = 3;sprintf (send_buf, " 19200"  ); break;
          }
     }
     if (change_item){                                                // теперь если мы изменяем параметры
          if (button_press (KEY_UP)) { if (mirror_data_8bit < 3) mirror_data_8bit ++;  }   // отслеживаем кнопку вверх и пока промежуточное значение меньше 8 плюсуем его
          if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 0) mirror_data_8bit --; }   // ну или же минусуем
          switch (mirror_data_8bit){                                                       // в зависимости от того до куда доклацали нашу переменную присваиваем новое значение из кейса
               case 0: mirror_data = 0;   blink_many_simbol (0b0111111); sprintf(send_buf, "   600"    );  break;         
               case 1: mirror_data = 1;   blink_many_simbol (0b0111111); sprintf(send_buf, "  2400"   );  break;
               case 2: mirror_data = 2;   blink_many_simbol (0b0111111); sprintf(send_buf, "  9600"   );  break;
               case 3: mirror_data = 3;   blink_many_simbol (0b0111111); sprintf(send_buf, " 19200"  );  break;
          }
          if (button_press (KEY_ENTER)) {                                                       // при нажатии ентер применяем значение (нужно добавить еепром)
               change_item = false; non_repit = false;                                          // выйти из режима изменения параметров, сбросить флаг единоразового выполнения при входе в функцию
               if (nomer == 1){ param.baud1 = mirror_data_8bit;  EEPROM_write(EE_BAUD1,param.baud1); }            // присвоить измененное значение как новое
               if (nomer == 2){ param.baud2 = mirror_data_8bit;  EEPROM_write(EE_BAUD2,param.baud2); } 
               init_UART_0_1 (param.baud1, param.baud2);    
          }
     }
}

void protocol_data1 (void)              // простая функция как ввод количества цд
{
     enter_8bit (1,2,&param.protokol1,EE_PROTOCOL1,1); 
}

void protocol_data2 (void)              // аналогично первой
{
    enter_8bit (1,2,&param.protokol2,EE_PROTOCOL2,1);  
}

void filtr_data (void)                  // и это тоже. в дальнейшем нужно написать функцию которая объеденит их 
{
    enter_8bit (1,5,&param.filtr,EE_FILTR,1);  
}

void light (void)                       // настройка яркости
{
	if (!change_item){
		if (button_press (KEY_ENTER)) {change_item = true;}
		sprintf(send_buf, "     %01lu" , (uint32_t) param.lighte );mirror_data_8bit = param.lighte;
	}
	if (change_item){
		if (button_press (KEY_UP)) { if (mirror_data_8bit < 2) mirror_data_8bit ++;  }
		if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 0) mirror_data_8bit --; }
		blink_simbol (5);
		sprintf(send_buf, "     %01lu"  ,(uint32_t) mirror_data_8bit );
		if (button_press (KEY_ENTER)) {param.lighte = mirror_data_8bit;
			change_item = false; non_repit = false;
			switch (param.lighte){
				case 0:if (!spi_stop) send_data ( 0x0A,  0x04); break;                // сразу отправляем в микру новое значение яркости
				case 1:if (!spi_stop) send_data ( 0x0A,  0x07); break;
				case 2:if (!spi_stop) send_data ( 0x0A,  0x0F); break;
			}
		}
	}
}

void enter_time (void)                  // функция ввода времени    
{
    enter_1307(&DS.hour, &DS.min, &DS.sec, 0);
}

void enter_date (void)                  // функция ввода даты
{
   enter_1307(&DS.date, &DS.month, &DS.year, 1);    
}

void enter_1307 (unsigned int *number1, unsigned int *number2, unsigned int *number3, uint8_t option)    // обобщеная функция ввода времни и даты
{
    if (tim_5 == 0) { tim_5 = 15000;  DS_Read(); } 
	if (!change_item){
     	if (admin_parol|| super_parol || option != 1) if (button_press (KEY_ENTER)) {change_item = true; mirror_data_8bit = 0;}      // это защита от входа в изменение даты без пароля
     	sprintf(send_buf, "%02d.%02d.%02d." ,  *number1,*number2,*number3);                                                          // тут показываем или время или дату. передаем сюда указатель на данные
	}
	if (change_item){
     	tim_3 = 1000;                                                                                 // сбрасываем таймер чтоб в это время не опрашивались часы
     	static char itedger [10] ;                                                                    // буффер для хранения разбитых данных  // по хорошему нужно сделать один большой буффер для всех
     	if (!non_repit){ non_repit = true;  sprintf(itedger, "%02d%02d%02d" ,  *number1,*number2,*number3 ); mirror_data_8bit = 0 ; }     // при первом входе забиваем буффер текущими данными
     	else {
          	if (button_press (KEY_RIGHT)) { if (mirror_data_8bit < 5  ) mirror_data_8bit ++; }           // кнопка в право увеличивает переменную которая отвечает за номер вводимого символа
          	if (button_press (KEY_LEFT)) {if (mirror_data_8bit > 0) mirror_data_8bit --;}                // кнопка в лево уменьшает
          	if (button_press (KEY_UP)) {                                                                 // кнопка вверх 
               	if (itedger[mirror_data_8bit] - '0' < 9) itedger[mirror_data_8bit]  ++;                  // проверяет какой символ в элементе буффера. если это цифра, и она меньше 9 то можно ее увеличить
               	else if(itedger[mirror_data_8bit] - '0' == 9) itedger[mirror_data_8bit] = '0';           // если же это 9ка, то сбросить в ноль. типа зациклить. минус символ нуля это для перевода из аски
          	}          // щас подумал, по хорошему переделать, и все указать в аски, таким образом даже если там будет мусор то мы его обнаружим
          	if (button_press (KEY_DOWN)) {                                                               // кнопка вверх делает тоже самое но в другом направлении
               	if (itedger[mirror_data_8bit] - '0'  > 0) itedger[mirror_data_8bit]  --;
               	else if(itedger[mirror_data_8bit] - '0'  == 0) itedger[mirror_data_8bit]  = '9';
          	}
          	sscanf(itedger,"%02u%02u%02u" ,  number1, number2, number3 );                                 // тут мы складываем обратно в переменные наш буфер
          	blink_simbol (mirror_data_8bit);                                                              // моргаем тем символом, на который указывает переменная вводимого символа
          	sprintf(send_buf, "%02d.%02d.%02d." ,  *number1,*number2,*number3);                           // тут отображаем на экране уже собранные из буффера переменные
          	if (button_press (KEY_ENTER)) {                                                               // по нажатию клавиши ентер
               	change_item = false; non_repit = false;                                                   // выходим из изменения параметров, и сбрасываем флаг единичного выполнения
                if (option == 0)   {                                                                      // если передавали 0 то значит это была дата
               	  if ( DS.hour < 25 && DS.min < 60 && DS.sec < 60) {DS_Write_time ();}                    // если мы находимся в диапазоне правильных значений, записать их в микру часов
                  else { error_func(ERROR5);}                                                             // если значения не правильные еррор
                }  
                if (option == 1)   {                                                                      // аналогично для даты
                  if ( DS.date < 31 && DS.month < 13 && DS.year < 99) {DS_Write_date ();}
                  else { error_func(ERROR5);}                
                } 
                if (option == 2)   {                                                                      // аналогично для даты err36
                     if ( *number1 < 31 && *number2 < 13 && *number3 < 99) {write_no_pay ();massa = 2;}   // масса переиспользуется 
                     else { error_func(ERROR5);}
                }                                                       
          	}
     	}
	}       
}

void kal_koef (void)                    // ввод калибровочного коэф
{
	if (!change_item){
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // защита от ввода без пароля, или разблокировки
		sprintf(send_buf, "%f" ,  param.calib_koef ); mirror_double = param.calib_koef;                      // выводим на экран калиб, коэф
	}
	if (change_item){
		enter_float ();                                                                                      // переходим в функцию ввода флоат числа
		if (button_press (KEY_ENTER)) {
			change_item = false; non_repit = false;                                                          // выходим из функции ввода, сбрасываем одноразовое выполнение
			if (mirror_double < 0.001 || mirror_double > 500) {                                                // если ввели некоректные значения еррор
				error_func (ERROR5);
			}
			else{ param.calib_koef = mirror_double;                                                           // если корректные то сохраняем
            EEPROM_write_float(EE_KALIBKOEF,param.calib_koef);}
		}
	}
}

void init_menu (void)                   // функция инициализации меню. просто указателю присваиваем нулевой пункт меню.
{
	pointer_menu = ((MenuItem*)&MenuItems[0]);      // так получаю правильный адрес
     #ifdef DEBUG_MOD                               // чтоб в отладке не вводить постоянно пароли конченые
     super_parol = true;
     #endif
}

void error_func (uint8_t error_number)  // фунция для отображения 
{
	tim_0 = 0;                          // сбрасываем таймер моргания
	tim_1 = 60000;                      // взводим таймер отображения ерора
	while (!tim_1 == 0)                 // и падаем в вайл, а в нем вызываем показ различных строк из флеша
	{strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[error_number])));}     // если компилятор не знает какие именно данные я буду использовать то он не сможет подставить значение. и по этому нужно передавать вот такую дичь
}

void Pasword_press (void)               // функция ввода пароля
{
    static int32_t xor_pas;
	if (!change_item){
     	if (button_press (KEY_ENTER)) {change_item = true; xor_pas = 0; xor_pas = (DS.date * 100); xor_pas += DS.month; }   // так мы формируем число для ХОР 
     	sprintf(send_buf, "  0000" ); mirror_data = 0;       // показываем нули справа
	}
	if (change_item){
     	enter_numbers (3);                                   // заходим в функцию ввода числа
     	if (button_press (KEY_ENTER)) {                      // по ентеру
              change_item = false; non_repit = false;        // сбрасываем выполнение функции, и флаг единоразового входа
              if (mirror_data == PASWORD_ADMIN) { admin_parol = true; error_func (NOERROR); go_func = false;}   // если ввели число равное паролю админа, поднять флаг пароля, показать что ок, и сразу выйти из функции ввода
              if (mirror_data == PASWORD_SUPER) { super_parol = true; error_func (NOERROR); go_func = false;}
              if (mirror_data == (xor_pas ^ 0xFFF)) { super_parol = true; error_func (NOERROR); go_func = false;}         // это ввод пароля в зависимости от даты  0xFFF
              if (mirror_data != PASWORD_SUPER && mirror_data != PASWORD_ADMIN) {error_func (ERROR5); }         // если пароль не соответствует то еррор
        }
	}	
}

void void_func (void)                   // пустая функция-затычка для структуры меню
{
	
}

void enter_numbers (uint8_t size )               // можно передавать значение от 0 до 5. это количество значимых цифр
{
	static uint8_t  caunter_input;               // создаем счетчик
	static char itedger [10] ;                   // создаем буффер
	if (!non_repit){ non_repit = true; mirror_data = labs(mirror_data);  sprintf(itedger, "%06ld"  ,(int32_t) mirror_data ); caunter_input = 5 - size; }    // первое выполнение, разбиваем входящее число на элементы в буффер, а так же делаем смещение на 
	else {                                                                                                                  // то количество сколько символов передали
		if (button_press (KEY_RIGHT)) { if (caunter_input < 5  ) caunter_input ++; }                                        // кнопка в право, сместить указатель
		if (button_press (KEY_LEFT)) {if (caunter_input >( 5 - size)) caunter_input --;}                                    // аналогично сместить в другую сторону если можна
		if (button_press (KEY_UP)) {
			if (itedger[caunter_input] - '0' < 9) itedger[caunter_input]  ++;                                               // как и в функции времени проверяем какой символ у нас в элементе буфера и по возможности увеличиваем
			else if(itedger[caunter_input] - '0' == 9) itedger[caunter_input] = '0';
		}
		if (button_press (KEY_DOWN)) {
			if (itedger[caunter_input] - '0'  > 0) itedger[caunter_input]  --;
			else if(itedger[caunter_input] - '0'  == 0) itedger[caunter_input]  = '9';
		}
		mirror_data = atol (itedger);                                                                                      // тут сканф заменил атол и все работает так же само
		switch (size){                                                                                                     // это функция для вывода информации без лишних нулей слева
			case 0: sprintf(send_buf, "     %01ld"   ,(int32_t) mirror_data ); break; 
			case 1: sprintf(send_buf, "    %02ld"  ,(int32_t) mirror_data ); break;
			case 2: sprintf(send_buf, "   %03ld"  ,(int32_t) mirror_data ); break;
			case 3: sprintf(send_buf, "  %04ld"  ,(int32_t) mirror_data ); break;
			case 4: sprintf(send_buf, " %05ld"  ,(int32_t) mirror_data ); break;
			case 5: sprintf(send_buf, "%06ld"  ,(int32_t) mirror_data ); break;
		}
		blink_simbol (caunter_input);                                                                                     // моргаем изменяемым символом
	}
}

void enter_float (void)                          // не доработаная функция ввода флоат значения. точка только возле одного знака. нужно править
{
    static uint8_t poz_point;
	static uint8_t  caunter_input;
	static char itedgers [20] ;
	if (!non_repit){ non_repit = true;  sprintf(itedgers, "%f"  , mirror_double ); caunter_input = 0 ; }
	else {
        for (int i = 0; i < 10; i ++)
        { if (itedgers[i] == '.') {poz_point = i; break;} }              
     	if (button_press (KEY_RIGHT)) { if (itedgers[caunter_input+1]  == '.' ){ caunter_input ++;}if (caunter_input < 6  ) caunter_input ++; }
     	if (button_press (KEY_LEFT)) {if (itedgers[caunter_input-1]  == '.'  ){  caunter_input --;}if (caunter_input > 0 ) caunter_input --;}
     	if (button_press (KEY_UP)) {
          	if (itedgers[caunter_input ] - '0' < 9) itedgers[caunter_input ]  ++;
          	else if(itedgers[caunter_input ] - '0' == 9) itedgers[caunter_input ] = '0';
     	}

		if (button_press (KEY_DOWN)) {
			if (itedgers[caunter_input ] - '0'  > 0) itedgers[caunter_input ]  --;
			else if(itedgers[caunter_input ] - '0'  == 0) itedgers[caunter_input ]  = '9';
		}
		mirror_double = atof(itedgers);
        sprintf(send_buf, "%s"  ,itedgers );
        if (poz_point == 1 && PIN_OFF && caunter_input == 2 ){itedgers[1] = itedgers[2]; itedgers[2] = '.'; caunter_input = 1; }
        if (poz_point == 2 && PIN_OFF && caunter_input == 0 ){itedgers[2] = itedgers[1]; itedgers[1] = '.'; caunter_input = 0; }
        if (poz_point == 1 && PIN_OFF && caunter_input == 3 ){itedgers[1] = itedgers[2]; itedgers[2] = itedgers[3]; itedgers[3] = '.'; caunter_input = 2; }     
        if (poz_point == 2 && PIN_OFF && caunter_input == 3 ){itedgers[2] = itedgers[3]; itedgers[3] = '.'; caunter_input = 2; }  
        if (poz_point == 3 && PIN_OFF && caunter_input == 0 ){itedgers[3] = itedgers[2]; itedgers[2] = itedgers[1]; itedgers[1] = '.'; caunter_input = 0; } 
        if (poz_point == 3 && PIN_OFF && caunter_input == 1 ){itedgers[3] = itedgers[2]; itedgers[2] = '.'; caunter_input = 1; }              
        if (poz_point < caunter_input){blink_simbol (caunter_input - 1);}
        else {blink_simbol (caunter_input);}     
	}       
}

void zero_in_start (void)
{
    enter_8bit (0,1,&param.zero_start,EE_ZEROSTART,1);  
}

void kalib_vp (void)
{
    get_zemik_sensors(param.sensor_number);                                                 //  опрашивать датчики
    boolean.led_in_menu = true; 
    if (tim_5 == 0 && !boolean.errors){ if (massa < last_massa + param.discret1 *2  && massa > last_massa - param.discret1 *2 ) \
    {send_point(3);}  else {send_non_point(3);}  tim_5 = (param.tim_stab * 1000); last_massa = massa;}     
    (massa == 0 && !boolean.errors ) ?   send_point (2) : send_non_point(2);   // если масса равна 0 то зажечь светик      
	if (!change_item){                // пока мы не вводим данные
    if (boolean.data_ok)  {boolean.data_ok = false; converting_cod_to_massa (); massa = ((massa * param.calib_koef)+0.5);}   // тут получаем реальную массу, без фильтра   
     	if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // отслеживаем ентер и если что переходим в выполнение ввода
        if (button_press (KEY_LEFT))  {zero_funk(1);}    
        if (!boolean.errors) { 
     	 sprintf(send_buf, "%6ld" , massa );                                                                // показываем значение массы, если ерор то показать ошибку 
         mirror_data = massa; }   else {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR4])));  }      
	}
	if (change_item){                                                                      // если мы вводим данные
        if (boolean.data_ok)  {boolean.data_ok = false; converting_cod_to_massa ();}       // тут получаем коды датчиков без К.К
     	enter_numbers (5);                                                                 // передаем в функцию ввода данных нашу переменную и количество знаков для ввода
     	if (button_press (KEY_ENTER)) {                                                    // отслеживаем ентер (добавит еепром)
        change_item = false; non_repit = false; boolean.led_in_menu = false;               // выходим из функции изменения
        if (mirror_data < param.NPV ){                                                     // если больше НПВ не сохранять
        param.calib_koef = (double) mirror_data / (double) labs(massa);
        EEPROM_write_float(EE_KALIBKOEF,param.calib_koef); }                                // приравниваем новоезначение в параметр
        else {error_func (ERROR5);}                             
        }             
	}     
}

void adress_sens (void)                             // адресация датчиков
{
     static uint8_t last_adr;                       // предыдущий адрес
     if (!change_item){                              // пока не изменяем
          if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}      // ждем пароль доступа
          sprintf(send_buf, " START" );                                              // и показываем старт
     }
     if (change_item){
          if (!non_repit){ non_repit = true;  mirror_data_8bit = get_number_sensor (); last_adr = mirror_data_8bit; }  // при первом заходе сканируем, находим датчик
          if (mirror_data_8bit == 100) {error_func (ERROR4); change_item = false; non_repit = false; }                 // если там 100 значит какая то ошибка
          else {
               if (button_press (KEY_UP)) { if (mirror_data_8bit < 12) mirror_data_8bit ++;  }                              // или плюсуем или отнимаем
               if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 1) mirror_data_8bit --; }                               //
               blink_many_simbol (0x30);                                                                     // мигаем двумя правыми символами
               sprintf(send_buf, "    %02lu"  ,(uint32_t) mirror_data_8bit );                                               // показать то что выбрали (номер датчика)
               if (button_press (KEY_ENTER)) {                                                                              // по нажатию ентер присвоить новое значение
                    send_new_adr (last_adr,mirror_data_8bit);
                    change_item = false; non_repit = false;
                    error_func (NOERROR);                                                                                     // тут нету обратной связи, по этому вот такое подставное ок
               }
          }
     }
}

void tim_stab_func (void)                  // функция ввода времени стабилизации
{
     enter_8bit (1,15,&param.tim_stab,EE_FILTR,2);
}

void of_adc_devise (void)                  // функция ввода отключения и показ низкого акб
{
  enter_8bit (0,1,&param.of_adc,EE_OF_ADC,2);
}

void enter_8bit (uint8_t min, uint8_t max, uint8_t *paramet, uint16_t eeprom, uint8_t size)    // функция ввода числа 8бит
{
     if (!change_item){
          if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}
          sprintf(send_buf, "    %2lu" , (uint32_t) *paramet );mirror_data_8bit = *paramet;
     }
     if (change_item){
          if (button_press (KEY_UP)) { if (mirror_data_8bit < max) mirror_data_8bit ++;  }
          if (button_press (KEY_DOWN)) {if (mirror_data_8bit > min) mirror_data_8bit --; }
          if (size == 1) {blink_simbol (5); }
          if (size == 2) {blink_many_simbol (0x30); }
          sprintf(send_buf, "    %2lu"  ,(uint32_t) mirror_data_8bit );
          if (button_press (KEY_ENTER)) {
           change_item = false; non_repit = false;
          *paramet = mirror_data_8bit;  EEPROM_write(eeprom, *paramet); }
     }     
}

void individual_koef (void)  // функция ввода индивидуальных коэфф
{
    get_zemik_sensors(param.sensor_number);                                                                  //  опрашивать датчики
    if (boolean.data_ok)  {boolean.data_ok = false; converting_cod_to_massa (); massa = ((massa * param.calib_koef)+0.5);}   // тут получаем реальную массу, 
    if (!boolean.non_rep){ boolean.non_rep = true; boolean.led_in_menu = true; mirror_data_8bit = 1; mirror_double2 = 0; }    // при входе сбросить счетчики и показания второго зеркала
    if (tim_5 == 0 && !boolean.errors){ if (massa < last_massa + param.discret1 *2  && massa > last_massa - param.discret1 *2 ) \
    {send_point(3);}  else {send_non_point(3);}  tim_5 = (param.tim_stab * 1000); last_massa = massa;}    // стабилизация
    (massa == 0 && !boolean.errors ) ?   send_point (2) : send_non_point(2);   // если масса равна 0 то зажечь светик    
         if (!change_item){                // пока мы не вводим данные
              if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // отслеживаем ентер и если что переходим в выполнение ввода
              if (button_press (KEY_UP))   { if(mirror_data_8bit < param.sensor_number) mirror_data_8bit ++; }     // перемещаемся вверх-вниз по номерам датчиков
              if (button_press (KEY_DOWN)) { if(mirror_data_8bit > 1) mirror_data_8bit --; }
              sprintf(send_buf, "    %2lu"  ,(uint32_t) mirror_data_8bit );                                        // и показываем номер датчика
         }
         if (change_item){                                                                              // если мы вводим данные
              if (!boolean.errors && boolean.led_display == 0x03) {sprintf(send_buf, "%6ld" ,massa ); *point_metrologi = mirror_double;}  // если показ 3 экрана и нет ошибок, то это масса
              if ( boolean.errors && boolean.led_display == 0x03) {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR4]))); }   // если показ 3 экрана и есть ошибки то это ерр
              if(boolean.led_display < 4){ if (button_press (KEY_ENTER)) {                        // отслеживаем ентер 
              change_item = false;  boolean.led_in_menu = false; eeprom_calib_sens();  boolean.led_display = 0x00; *point_metrologi = mirror_double;}} // выходим из функции изменения, сохранаяем в память, сбрасываем экраны, указателю присваиваем новое значение
              if(boolean.led_display == 4){ if (button_press (KEY_ENTER)) {                        // отслеживаем ентер 
                 boolean.led_display = 0x00;                               // сбрасываем номер экрана
                 if (mirror_double < 10){
                 *point_metrologi = mirror_double;                         // указателю присваиваем новое значение
                 eeprom_calib_sens(); } else {error_func (ERROR5); non_repit = false;}  }}    // выходим из функции изменения, сохраняем             
              if(boolean.led_display == 4) {enter_float (); }              // если это 4 экран, то ввод флоат и другой обработчик кнопок
              else {                                                       // а если любой другой
                if (button_press (KEY_LEFT)) {
                     if (boolean.led_display == 2) boolean.led_display = 0x04;  // кнопка в лево делает активным 4 экран
                }
                if (button_press (KEY_RIGHT)) {
                     boolean.led_display ^= 0x01;                          // кнопка в право переключает между выводом массы и коэфф
                }
                if (button_press (KEY_UP)) {
                     case_up_down (1, 0.00001);                            // нажатие добавляет чучуть
                }
                if (button_press (KEY_DOWN)) {
                     case_up_down(2, 0.00001);
                }
                if (button_long_press (KEY_UP)) {
                     case_up_down (1, 0.00011);                            // удержание добавляет больше
                }
                if (button_long_press (KEY_DOWN)) {
                     case_up_down(2, 0.00011);
                }
              }              
              if(boolean.led_display == 0)case_up_down(0,0);               // для 0 дисплея, чтоб один раз сработать и записать в промежуточную переменную число
              if(boolean.led_display == 2)case_up_down(0,0);               // для 2 дисплея, чтоб показывать текущий коэфф
         }        
}

void case_up_down (uint8_t up_down, double increment)                      // промежуточная функция ввода коэфф
{
     switch (mirror_data_8bit)                                             // в завиимости от того какой номер датчика выбран
     {
          case 1: point_metrologi  = &metrology.koef_sensor1; break;       // указателю присваиваем адрес в памяти нужного коэфф
          case 2: point_metrologi  = &metrology.koef_sensor2; break;
          case 3: point_metrologi  = &metrology.koef_sensor3; break;
          case 4: point_metrologi  = &metrology.koef_sensor4; break;
          case 5: point_metrologi  = &metrology.koef_sensor5; break;
          case 6: point_metrologi  = &metrology.koef_sensor6; break;  
          case 7: point_metrologi  = &metrology.koef_sensor7; break;
          case 8: point_metrologi  = &metrology.koef_sensor8; break;
          case 9: point_metrologi  = &metrology.koef_sensor9; break;
          case 10: point_metrologi = &metrology.koef_sensor10; break;
          case 11: point_metrologi = &metrology.koef_sensor11; break;
          case 12: point_metrologi = &metrology.koef_sensor12; break;                  
     }
          // mirror_double2 нужен для того чтоб при изменении коэф во время ввода(и показа массы) все менялось в реальном времени, а в случае нажатия ЕСК выйти без сохранения, прошлое значение хранится в этой переменной
          if (up_down == 1){ if (mirror_double < 10) mirror_double += increment; } if(up_down == 2){if (mirror_double > 0) mirror_double -= increment; }  // если вывали с 1 то плюсуем с 2 то минусуем
          if(boolean.led_display == 0){sprintf(send_buf, "%f" , mirror_double ); mirror_double = *point_metrologi;boolean.led_display = 2; mirror_double2 = *point_metrologi;}  // один раз обновить переписать значения коэфф
          if(boolean.led_display == 2){sprintf(send_buf, "%f" , mirror_double );}     // а потом показывать уже просто зеркало этого значения
}

void enter_no_pay (void)                                                   // снятие блокировки ерр36
{
     if (!boolean.change_item_mirr){                                       // новая функция выполнения/не выполнения (так как другая задействована в вводе часов)
           if (button_press (KEY_ENTER)) {boolean.change_item_mirr = true;}
          sprintf(send_buf, "000000"  );massa = 0;mirror_data = 0; }
	 if (boolean.change_item_mirr){
       switch (massa) {                                                    // используем массу не по назначению))
           case 0: enter_numbers (5);   
           if (button_press (KEY_ENTER)) {
             if (mirror_data ==  param.pasw_err) {massa = 1;non_repit = false;mirror_data = 0;  EEPROM_write(EE_ERRPR36, 0); param.err36 = 0;  }   // если пароль совпадает с тем что ввели снимаем блокировку
          	 else { error_func(ERROR5);boolean.change_item_mirr = false; non_repit = false;}                                                       // нет ерр5   
           } break;
           case 1: enter_1307(&param.data_err, &param.mount_err, &param.year_err, 2);   break;                                                     // вводим новую дату блокировки            
           case 2: enter_numbers (5); 
           if (button_press (KEY_ENTER)) {
            param.pasw_err = mirror_data; EEPROM_write_32t(EE_PSWRD1,param.pasw_err);boolean.change_item_mirr = false; go_func = false;non_repit = false;} break;   // вводим новый пароль и записываем его в память      
       }              
    }
}

