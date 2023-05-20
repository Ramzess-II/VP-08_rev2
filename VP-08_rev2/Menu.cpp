#include "Menu.h"
#include "Interupt.h"
#include "max7219.h"
#include "KeyBoard.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
#include "main.h"

//--------------���������� �� ������ ������---------------//
extern bool spi_stop;
extern uint8_t main_menu_bool,data_ok;
extern uint16_t tim_0,tim_1,tim_3,tim_4,tim_5, adc;
extern int32_t massa,last_massa;
extern  char send_buf [20];
extern struct DS1307 DS;
extern struct bool_date boolean;
//--------------���������� �� ����� �����-----------------//
bool go_func, change_item, non_repit,super_parol, admin_parol;   
uint8_t nul_enty = 0, mirror_data_8bit;
int32_t mirror_data;
double mirror_double, mirror_double2;             
void(* on_clicks)(void);                    // ������� ��������� �� �������
typedef void (*StateFunc) (void);           // ������� ����� ���, ������ StateFunc ��������� �� void (*) (void);
struct parametrs param;
struct metrologis metrology;
double *point_metrologi;                    // ��������� �� ������� ���������




typedef struct  PROGMEM {               // ��������� ����
	const char format[10];			    // ��� ����
	void(* on_click)(void);      	    // ������ �� ������� ������ �������� ��������� ��������, �������� +1 ��� -1)
	uint8_t current_menu;
	uint8_t child_menu;
	uint8_t parent_menu;
	uint8_t next_menu;
	uint8_t previos_menu;
	uint8_t  key_state;
} MenuItem;

MenuItem *pointer_menu ;                 //*pointer_menu ��������� �� ��������� MenuItem

MenuItem const MenuItems[40] = {         // ������� ������ �������� ����
	//  �����         ��������     �������                           ��� ����             �������        ��������      ����_�����      ����_�����     ��������� ������
	
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

void main_menu (void)                // ������� ���� ����
{
	_key_strob ();                   // ������������ ������
	if (go_func) { on_clicks ();}    // ���� ������ ���� ��� �� ��������� �������, �� ��������� ������� ��������� �� ������� �������� � ������� �������� ����
	else	strcpy_P(send_buf, pointer_menu->format );     // ���� ���� ������ �� ���������� ��� ���� 
}

uint8_t not_change (void)            // �������� �� �������� ������
{
   if (super_parol) {return 1;}      // ���� ��� ����� ������, ����� ������
   else if (admin_parol && BLK) {return 1;}     // ���� ������ ������ �� ��� ��������� ����������
   else {return 0; }                 // ���� ������ �� ������� 0
}

void _key_strob (void)               // ������� ��������� ������ � ����
{
	switch ((pgm_read_byte(&pointer_menu->key_state)))      // ���� �������� �������� �� ������� �������� ���� �����, � � ����������� �� ���� ����� ��� ����� �� ������� ������������ �������
	{
		case (SKROL_MENU):                                  // ���� ��� ��������� ����
		if (button_press (KEY_UP)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->next_menu))]); }       // ��� ������� �� ������ ��������� � ����� ���� ������� ������ ��� ���������
		if (button_press (KEY_DOWN)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->previos_menu))]); }  // ��� ����������
		if (button_press (KEY_ENTER)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->child_menu))]);  }  // ��������� � �������
		if (button_press (KEY_ESC))  { if(((&MenuItems[(pgm_read_byte(&pointer_menu->parent_menu))]) == (&MenuItems[(pgm_read_byte(&pointer_menu->current_menu))])))\
            { boolean.main_menu_bool = 0;  }else {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->parent_menu))]) ;}}  
        // ��� �� ��������� ���� � ������ ��� �������� ��������� � ������ ����, �� ������� �� ����, � ���� �� ��������� �� ��������� � ��������.     
		break;
		case (DATA_INPUT):                                   // ���� ��� ���� ������
		if (!go_func){                                       // � ������� ���� �� �����������
			if (button_press (KEY_UP)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->next_menu))]); }       // ��� ������� �� ������ ��������� � ����� ���� ������� ������ ��� ���������
			if (button_press (KEY_DOWN)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->previos_menu))]); }  // ��� ����������
			if (button_press (KEY_ESC)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->parent_menu))]); boolean.led_in_menu = false;}     // ������������ � ��������
			if (button_press (KEY_ENTER)) {on_clicks = (StateFunc)  pgm_read_ptr(&MenuItems[(pgm_read_byte(&pointer_menu->current_menu))].on_click);  go_func = true;}  // �������� ��������� ������� ����
		}
		if (go_func){                                                                                                         // ���� �� ��������� � ���������� �������
			if (change_item) if (button_press (KEY_ESC)) {  change_item = false; non_repit = false; boolean.led_display = 0;*point_metrologi = mirror_double2;boolean.led_in_menu = false;} // ���� �� �������� ��������� �� �� ������� ��������� ��� ������
			if (!change_item){                                                                                                // ���� �� �������� ���������
				if (button_press (KEY_ESC)) { go_func = false; boolean.non_rep = false;boolean.led_display = 0; boolean.led_in_menu = false;}	          // ���� �� �� �������� ��������� �� �� ������� �������
			}                              // ���� ��� �������, ��� ��� �� �������� ���� ���� ��� ������� �������
		}
		break;
		case (NO_CHILDREN):                                   // ���� ��� ���� ������ ��� ������ � �������
		if (!go_func){                                       // � ������� ���� �� �����������
     		if (button_press (KEY_UP)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->next_menu))]); }       // ��� ������� �� ������ ��������� � ����� ���� ������� ������ ��� ���������
     		if (button_press (KEY_DOWN)) {pointer_menu =((MenuItem*)&MenuItems[(pgm_read_byte(&pointer_menu->previos_menu))]); }  // ��� ����������
     		if (button_press (KEY_ESC))  {boolean.main_menu_bool = 0; }                              // ���� ������ ������ �������� ����
     		if (button_press (KEY_ENTER)) {on_clicks = (StateFunc)  pgm_read_ptr(&MenuItems[(pgm_read_byte(&pointer_menu->current_menu))].on_click);  go_func = true;}  // �������� ��������� ������� ����
		}
		if (go_func){                                                                                                         // ���� �� ��������� � ���������� �������
     		if (change_item) if (button_press (KEY_ESC)) {  change_item = false; non_repit = false; }                         // ���� �� �������� ��������� �� �� ������� ��������� ��� ������
     		if (!change_item){                                                                                                // ���� �� �������� ���������
          		if (button_press (KEY_ESC)) { go_func = false; boolean.change_item_mirr = false;}	                          // ���� �� �� �������� ��������� �� �� ������� �������
     		}
		}
		break;        
	}
}

void ver_po (void)                   // �������� ������ ��. ���� �������
{
	sprintf(send_buf, "UER1.00" );   // �������� ������
}

void send_adc (void)                 // �������� ���������� ��� � �������
{
    static uint8_t caunter_filtr;
    static double filtrs_adc [10];
    static double n;        // ������� ���� ���������� ��� �������� �������������� ����������
	if (tim_1 == 0) {  
    filtrs_adc[caunter_filtr] = (double) adc / 71;   
    for (int i = 0; i < 10; i ++) { n += filtrs_adc[i];}  
    n = n / 10;  
    caunter_filtr ++;   
    if (caunter_filtr == 10 )  caunter_filtr = 0;        
	sprintf (send_buf, "BAT%04.1f" ,  n );   // ������� ������ 
	if(filtrs_adc [9] == 0) {tim_1 = 500;} else {tim_1 = 5000;}}          // ���� � ��������� ������� 0 ����� ������ �������
}

void caunter_weighing (void)         // ������� ����������� ���� �� �����������
{
	sprintf(send_buf, "%06lu" , (uint32_t) param.caunter_weight );
}

void diskret1_new (void)             // ������� ����� �������� �����, ��� ����� ������ ���������� �������� � �� �������
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
      if (!change_item){                    // ��� ���� ���� ��� �� �������� ��������� ��� ���
      if (button_press (KEY_ENTER)) {change_item = true;}         // ���� �� �������� ��������� ����������� ������ �����, ��� ������ ������ ��������� ���� ��� �� �������� ���������
           sprintf(send_buf, "   %03lu" , (uint32_t) *inbaud ); // ���� �� �������� ������� ������� �������� ��������� �� �����
           switch (*inbaud){                                    // ����� ������� �� ��� ��. ���� �������� �������� ���������� � ������ � � ����������� �� ����� ����������� ������������� ����������   
                case 1:   mirror_data_8bit = 0; break;          // �������� �� ������ ��� ���� ���� ��� ����� � ��������� ���������� ������� ����� ����� ���������� � �� �������!
                case 2:   mirror_data_8bit = 1; break;
                case 5:   mirror_data_8bit = 2; break;
                case 10:  mirror_data_8bit = 3; break;
                case 20:  mirror_data_8bit = 4; break;
                case 50:  mirror_data_8bit = 5; break;
                case 100: mirror_data_8bit = 6; break;
                case 200: mirror_data_8bit = 7; break;
           }
      }
      if (change_item){                                                // ������ ���� �� �������� ���������
           if (button_press (KEY_UP)) { if (mirror_data_8bit < 8) mirror_data_8bit ++;  }   // ����������� ������ ����� � ���� ������������� �������� ������ 8 ������� ���
           if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 0) mirror_data_8bit --; }   // �� ��� �� ��������
           switch (mirror_data_8bit){                                                       // � ����������� �� ���� �� ���� ��������� ���� ���������� ����������� ����� �������� �� �����
                case 0: mirror_data = 1; break;
                case 1: mirror_data = 2; break;
                case 2: mirror_data = 5; break;
                case 3: mirror_data = 10; break;
                case 4: mirror_data = 20; break;
                case 5: mirror_data = 50; break;
                case 6: mirror_data = 100; break;
                case 7: mirror_data = 200; break;
           }
           blink_many_simbol (0b0111000);	                                                 // ������ 3�� ���������
           sprintf(send_buf, "   %03lu"  ,(uint32_t) mirror_data );                         // ���������� ������� �������� �� ��������� ������ � 3�� �������. ���� ��� ����� ���������� ����
           if (button_press (KEY_ENTER)) {                                                      // ��� ������� ����� ��������� �������� (����� �������� ������)
                change_item = false; non_repit = false;                                         // ����� �� ������ ��������� ����������, �������� ���� ������������� ���������� ��� ����� � �������
                if (nomer == 1){ param.discret1 = mirror_data;  EEPROM_write(EE_DISCKRET1,param.discret1); }            // ��������� ���������� �������� ��� �����
                if (nomer == 2){ param.discret2 = mirror_data;  EEPROM_write(EE_DISCKRET2,param.discret2); }
                if (nomer == 3){ param.discret3 = mirror_data;  EEPROM_write(EE_DISCKRET3,param.discret3); }
           }
      }    
}

void sill1_discrt (void)              // ������� ����� ������   
{
	if (!change_item){                // ���� �� �� ������ ������   
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // ����������� ����� � ���� ��� ��������� � ���������� �����
		sprintf(send_buf, "%06lu" , (uint32_t) param.sill1 );mirror_data = param.sill1;    // ���������� �������� ��������� � ���� ��� ����� �������� ������
	}
	if (change_item){                                                                      // ���� �� ������ ������ 
		enter_numbers (5);                                                                 // �������� � ������� ����� ������ ���� ���������� � ���������� ������ ��� �����
		if (button_press (KEY_ENTER)) {                                                    // ����������� ����� (������� ������)
			change_item = false; non_repit = false;                                        // ������� �� ������� ���������
			param.sill1 = mirror_data;  EEPROM_write_32t(EE_SILL1,param.sill1); }          // ������������ ������������� � ��������
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

void in_npv (void)                     // ����������� ������� ��� ����� ���
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

void sensor_numbers (void)             // ������� ��� ����� ���������� ��������
{
	if (!change_item){                 // ���� �� �������� ���������
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}         // ����������� �����
		sprintf(send_buf, "    %02lu" , (uint32_t) param.sensor_number );mirror_data_8bit = param.sensor_number;   // � ������� ������� �������� �� �����
	}
	if (change_item){                                               // ����� �������� ��������
		if (button_press (KEY_UP)) { if (mirror_data_8bit < 12) mirror_data_8bit += 2;  }             // ���� ������ ����� +2      
		if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 2) mirror_data_8bit -=2; }               // ������ ���� -2   � ��� ��� ��� ������� � �� ��� ������� ���� ���� �� ����� ����� �� �����������
		blink_many_simbol (0x30);                                                                     // ������ ����� ������� ���������
		sprintf(send_buf, "    %02lu"  ,(uint32_t) mirror_data_8bit );                                // ������� ������� ���� �������� ��� �� ��������
		if (button_press (KEY_ENTER)) {                                                               // � ������ ������� �� ����� ��������� ����� ��������
        param.sensor_number = mirror_data_8bit; EEPROM_write(EE_SENNUMBER,param.sensor_number);
        change_item = false; non_repit = false;
        }   
	}
}

void tips_cd (void)                    // ����������� ������� ��� ����� ���� �������
{
     enter_8bit (1,2,&param.tip_cd,EE_TIPCD,1); 
}

void baud_rate1 (void)                 // ������ ��� ���������� ������� ����� ����� ������� ���������� �������, ��� �������� ������
{
  input_baud (&param.baud1,1);
}

void baud_rate2 (void)
{
     input_baud (&param.baud2,2);
}

void input_baud (uint8_t *inbaud, uint8_t nomer)       // ���� �������� ������ �� ����������
{
     if (!change_item){                    // ��� ���� ���� ��� �� �������� ��������� ��� ���
          if (button_press (KEY_ENTER)) {change_item = true;}         // ���� �� �������� ��������� ����������� ������ �����, ��� ������ ������ ��������� ���� ��� �� �������� ���������
          switch (*inbaud){                                           // ����� ������� �� ��� ��. ���� �������� �������� ���������� � ������ � � ����������� �� ����� ����������� ������������� ����������
               // �������� �� ������ ��� ���� ���� ��� ����� � ��������� ���������� ������� ����� ����� ���������� � �� �������!
               case 0:    mirror_data_8bit = 0;sprintf (send_buf, "   600"    ); break;             
               case 1:    mirror_data_8bit = 1;sprintf (send_buf, "  2400"   ); break;          //���� ������� �� ������� ��� ��������� �����������
               case 2:    mirror_data_8bit = 2;sprintf (send_buf, "  9600"   ); break;
               case 3:    mirror_data_8bit = 3;sprintf (send_buf, " 19200"  ); break;
          }
     }
     if (change_item){                                                // ������ ���� �� �������� ���������
          if (button_press (KEY_UP)) { if (mirror_data_8bit < 3) mirror_data_8bit ++;  }   // ����������� ������ ����� � ���� ������������� �������� ������ 8 ������� ���
          if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 0) mirror_data_8bit --; }   // �� ��� �� ��������
          switch (mirror_data_8bit){                                                       // � ����������� �� ���� �� ���� ��������� ���� ���������� ����������� ����� �������� �� �����
               case 0: mirror_data = 0;   blink_many_simbol (0b0111111); sprintf(send_buf, "   600"    );  break;         
               case 1: mirror_data = 1;   blink_many_simbol (0b0111111); sprintf(send_buf, "  2400"   );  break;
               case 2: mirror_data = 2;   blink_many_simbol (0b0111111); sprintf(send_buf, "  9600"   );  break;
               case 3: mirror_data = 3;   blink_many_simbol (0b0111111); sprintf(send_buf, " 19200"  );  break;
          }
          if (button_press (KEY_ENTER)) {                                                       // ��� ������� ����� ��������� �������� (����� �������� ������)
               change_item = false; non_repit = false;                                          // ����� �� ������ ��������� ����������, �������� ���� ������������� ���������� ��� ����� � �������
               if (nomer == 1){ param.baud1 = mirror_data_8bit;  EEPROM_write(EE_BAUD1,param.baud1); }            // ��������� ���������� �������� ��� �����
               if (nomer == 2){ param.baud2 = mirror_data_8bit;  EEPROM_write(EE_BAUD2,param.baud2); } 
               init_UART_0_1 (param.baud1, param.baud2);    
          }
     }
}

void protocol_data1 (void)              // ������� ������� ��� ���� ���������� ��
{
     enter_8bit (1,2,&param.protokol1,EE_PROTOCOL1,1); 
}

void protocol_data2 (void)              // ���������� ������
{
    enter_8bit (1,2,&param.protokol2,EE_PROTOCOL2,1);  
}

void filtr_data (void)                  // � ��� ����. � ���������� ����� �������� ������� ������� ��������� �� 
{
    enter_8bit (1,5,&param.filtr,EE_FILTR,1);  
}

void light (void)                       // ��������� �������
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
				case 0:if (!spi_stop) send_data ( 0x0A,  0x04); break;                // ����� ���������� � ����� ����� �������� �������
				case 1:if (!spi_stop) send_data ( 0x0A,  0x07); break;
				case 2:if (!spi_stop) send_data ( 0x0A,  0x0F); break;
			}
		}
	}
}

void enter_time (void)                  // ������� ����� �������    
{
    enter_1307(&DS.hour, &DS.min, &DS.sec, 0);
}

void enter_date (void)                  // ������� ����� ����
{
   enter_1307(&DS.date, &DS.month, &DS.year, 1);    
}

void enter_1307 (unsigned int *number1, unsigned int *number2, unsigned int *number3, uint8_t option)    // ��������� ������� ����� ������ � ����
{
    if (tim_5 == 0) { tim_5 = 15000;  DS_Read(); } 
	if (!change_item){
     	if (admin_parol|| super_parol || option != 1) if (button_press (KEY_ENTER)) {change_item = true; mirror_data_8bit = 0;}      // ��� ������ �� ����� � ��������� ���� ��� ������
     	sprintf(send_buf, "%02d.%02d.%02d." ,  *number1,*number2,*number3);                                                          // ��� ���������� ��� ����� ��� ����. �������� ���� ��������� �� ������
	}
	if (change_item){
     	tim_3 = 1000;                                                                                 // ���������� ������ ���� � ��� ����� �� ������������ ����
     	static char itedger [10] ;                                                                    // ������ ��� �������� �������� ������  // �� �������� ����� ������� ���� ������� ������ ��� ����
     	if (!non_repit){ non_repit = true;  sprintf(itedger, "%02d%02d%02d" ,  *number1,*number2,*number3 ); mirror_data_8bit = 0 ; }     // ��� ������ ����� �������� ������ �������� �������
     	else {
          	if (button_press (KEY_RIGHT)) { if (mirror_data_8bit < 5  ) mirror_data_8bit ++; }           // ������ � ����� ����������� ���������� ������� �������� �� ����� ��������� �������
          	if (button_press (KEY_LEFT)) {if (mirror_data_8bit > 0) mirror_data_8bit --;}                // ������ � ���� ���������
          	if (button_press (KEY_UP)) {                                                                 // ������ ����� 
               	if (itedger[mirror_data_8bit] - '0' < 9) itedger[mirror_data_8bit]  ++;                  // ��������� ����� ������ � �������� �������. ���� ��� �����, � ��� ������ 9 �� ����� �� ���������
               	else if(itedger[mirror_data_8bit] - '0' == 9) itedger[mirror_data_8bit] = '0';           // ���� �� ��� 9��, �� �������� � ����. ���� ���������. ����� ������ ���� ��� ��� �������� �� ����
          	}          // ��� �������, �� �������� ����������, � ��� ������� � ����, ����� ������� ���� ���� ��� ����� ����� �� �� ��� ���������
          	if (button_press (KEY_DOWN)) {                                                               // ������ ����� ������ ���� ����� �� � ������ �����������
               	if (itedger[mirror_data_8bit] - '0'  > 0) itedger[mirror_data_8bit]  --;
               	else if(itedger[mirror_data_8bit] - '0'  == 0) itedger[mirror_data_8bit]  = '9';
          	}
          	sscanf(itedger,"%02u%02u%02u" ,  number1, number2, number3 );                                 // ��� �� ���������� ������� � ���������� ��� �����
          	blink_simbol (mirror_data_8bit);                                                              // ������� ��� ��������, �� ������� ��������� ���������� ��������� �������
          	sprintf(send_buf, "%02d.%02d.%02d." ,  *number1,*number2,*number3);                           // ��� ���������� �� ������ ��� ��������� �� ������� ����������
          	if (button_press (KEY_ENTER)) {                                                               // �� ������� ������� �����
               	change_item = false; non_repit = false;                                                   // ������� �� ��������� ����������, � ���������� ���� ���������� ����������
                if (option == 0)   {                                                                      // ���� ���������� 0 �� ������ ��� ���� ����
               	  if ( DS.hour < 25 && DS.min < 60 && DS.sec < 60) {DS_Write_time ();}                    // ���� �� ��������� � ��������� ���������� ��������, �������� �� � ����� �����
                  else { error_func(ERROR5);}                                                             // ���� �������� �� ���������� �����
                }  
                if (option == 1)   {                                                                      // ���������� ��� ����
                  if ( DS.date < 31 && DS.month < 13 && DS.year < 99) {DS_Write_date ();}
                  else { error_func(ERROR5);}                
                } 
                if (option == 2)   {                                                                      // ���������� ��� ���� err36
                     if ( *number1 < 31 && *number2 < 13 && *number3 < 99) {write_no_pay ();massa = 2;}   // ����� ���������������� 
                     else { error_func(ERROR5);}
                }                                                       
          	}
     	}
	}       
}

void kal_koef (void)                    // ���� �������������� ����
{
	if (!change_item){
		if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // ������ �� ����� ��� ������, ��� �������������
		sprintf(send_buf, "%f" ,  param.calib_koef ); mirror_double = param.calib_koef;                      // ������� �� ����� �����, ����
	}
	if (change_item){
		enter_float ();                                                                                      // ��������� � ������� ����� ����� �����
		if (button_press (KEY_ENTER)) {
			change_item = false; non_repit = false;                                                          // ������� �� ������� �����, ���������� ����������� ����������
			if (mirror_double < 0.001 || mirror_double > 500) {                                                // ���� ����� ����������� �������� �����
				error_func (ERROR5);
			}
			else{ param.calib_koef = mirror_double;                                                           // ���� ���������� �� ���������
            EEPROM_write_float(EE_KALIBKOEF,param.calib_koef);}
		}
	}
}

void init_menu (void)                   // ������� ������������� ����. ������ ��������� ����������� ������� ����� ����.
{
	pointer_menu = ((MenuItem*)&MenuItems[0]);      // ��� ������� ���������� �����
     #ifdef DEBUG_MOD                               // ���� � ������� �� ������� ��������� ������ ��������
     super_parol = true;
     #endif
}

void error_func (uint8_t error_number)  // ������ ��� ����������� 
{
	tim_0 = 0;                          // ���������� ������ ��������
	tim_1 = 60000;                      // ������� ������ ����������� �����
	while (!tim_1 == 0)                 // � ������ � ����, � � ��� �������� ����� ��������� ����� �� �����
	{strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[error_number])));}     // ���� ���������� �� ����� ����� ������ ������ � ���� ������������ �� �� �� ������ ���������� ��������. � �� ����� ����� ���������� ��� ����� ����
}

void Pasword_press (void)               // ������� ����� ������
{
    static int32_t xor_pas;
	if (!change_item){
     	if (button_press (KEY_ENTER)) {change_item = true; xor_pas = 0; xor_pas = (DS.date * 100); xor_pas += DS.month; }   // ��� �� ��������� ����� ��� ��� 
     	sprintf(send_buf, "  0000" ); mirror_data = 0;       // ���������� ���� ������
	}
	if (change_item){
     	enter_numbers (3);                                   // ������� � ������� ����� �����
     	if (button_press (KEY_ENTER)) {                      // �� ������
              change_item = false; non_repit = false;        // ���������� ���������� �������, � ���� ������������� �����
              if (mirror_data == PASWORD_ADMIN) { admin_parol = true; error_func (NOERROR); go_func = false;}   // ���� ����� ����� ������ ������ ������, ������� ���� ������, �������� ��� ��, � ����� ����� �� ������� �����
              if (mirror_data == PASWORD_SUPER) { super_parol = true; error_func (NOERROR); go_func = false;}
              if (mirror_data == (xor_pas ^ 0xFFF)) { super_parol = true; error_func (NOERROR); go_func = false;}         // ��� ���� ������ � ����������� �� ����  0xFFF
              if (mirror_data != PASWORD_SUPER && mirror_data != PASWORD_ADMIN) {error_func (ERROR5); }         // ���� ������ �� ������������� �� �����
        }
	}	
}

void void_func (void)                   // ������ �������-������� ��� ��������� ����
{
	
}

void enter_numbers (uint8_t size )               // ����� ���������� �������� �� 0 �� 5. ��� ���������� �������� ����
{
	static uint8_t  caunter_input;               // ������� �������
	static char itedger [10] ;                   // ������� ������
	if (!non_repit){ non_repit = true; mirror_data = labs(mirror_data);  sprintf(itedger, "%06ld"  ,(int32_t) mirror_data ); caunter_input = 5 - size; }    // ������ ����������, ��������� �������� ����� �� �������� � ������, � ��� �� ������ �������� �� 
	else {                                                                                                                  // �� ���������� ������� �������� ��������
		if (button_press (KEY_RIGHT)) { if (caunter_input < 5  ) caunter_input ++; }                                        // ������ � �����, �������� ���������
		if (button_press (KEY_LEFT)) {if (caunter_input >( 5 - size)) caunter_input --;}                                    // ���������� �������� � ������ ������� ���� �����
		if (button_press (KEY_UP)) {
			if (itedger[caunter_input] - '0' < 9) itedger[caunter_input]  ++;                                               // ��� � � ������� ������� ��������� ����� ������ � ��� � �������� ������ � �� ����������� �����������
			else if(itedger[caunter_input] - '0' == 9) itedger[caunter_input] = '0';
		}
		if (button_press (KEY_DOWN)) {
			if (itedger[caunter_input] - '0'  > 0) itedger[caunter_input]  --;
			else if(itedger[caunter_input] - '0'  == 0) itedger[caunter_input]  = '9';
		}
		mirror_data = atol (itedger);                                                                                      // ��� ����� ������� ���� � ��� �������� ��� �� ����
		switch (size){                                                                                                     // ��� ������� ��� ������ ���������� ��� ������ ����� �����
			case 0: sprintf(send_buf, "     %01ld"   ,(int32_t) mirror_data ); break; 
			case 1: sprintf(send_buf, "    %02ld"  ,(int32_t) mirror_data ); break;
			case 2: sprintf(send_buf, "   %03ld"  ,(int32_t) mirror_data ); break;
			case 3: sprintf(send_buf, "  %04ld"  ,(int32_t) mirror_data ); break;
			case 4: sprintf(send_buf, " %05ld"  ,(int32_t) mirror_data ); break;
			case 5: sprintf(send_buf, "%06ld"  ,(int32_t) mirror_data ); break;
		}
		blink_simbol (caunter_input);                                                                                     // ������� ���������� ��������
	}
}

void enter_float (void)                          // �� ����������� ������� ����� ����� ��������. ����� ������ ����� ������ �����. ����� �������
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
    get_zemik_sensors(param.sensor_number);                                                 //  ���������� �������
    boolean.led_in_menu = true; 
    if (tim_5 == 0 && !boolean.errors){ if (massa < last_massa + param.discret1 *2  && massa > last_massa - param.discret1 *2 ) \
    {send_point(3);}  else {send_non_point(3);}  tim_5 = (param.tim_stab * 1000); last_massa = massa;}     
    (massa == 0 && !boolean.errors ) ?   send_point (2) : send_non_point(2);   // ���� ����� ����� 0 �� ������ ������      
	if (!change_item){                // ���� �� �� ������ ������
    if (boolean.data_ok)  {boolean.data_ok = false; converting_cod_to_massa (); massa = ((massa * param.calib_koef)+0.5);}   // ��� �������� �������� �����, ��� �������   
     	if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // ����������� ����� � ���� ��� ��������� � ���������� �����
        if (button_press (KEY_LEFT))  {zero_funk(1);}    
        if (!boolean.errors) { 
     	 sprintf(send_buf, "%6ld" , massa );                                                                // ���������� �������� �����, ���� ���� �� �������� ������ 
         mirror_data = massa; }   else {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR4])));  }      
	}
	if (change_item){                                                                      // ���� �� ������ ������
        if (boolean.data_ok)  {boolean.data_ok = false; converting_cod_to_massa ();}       // ��� �������� ���� �������� ��� �.�
     	enter_numbers (5);                                                                 // �������� � ������� ����� ������ ���� ���������� � ���������� ������ ��� �����
     	if (button_press (KEY_ENTER)) {                                                    // ����������� ����� (������� ������)
        change_item = false; non_repit = false; boolean.led_in_menu = false;               // ������� �� ������� ���������
        if (mirror_data < param.NPV ){                                                     // ���� ������ ��� �� ���������
        param.calib_koef = (double) mirror_data / (double) labs(massa);
        EEPROM_write_float(EE_KALIBKOEF,param.calib_koef); }                                // ������������ ������������� � ��������
        else {error_func (ERROR5);}                             
        }             
	}     
}

void adress_sens (void)                             // ��������� ��������
{
     static uint8_t last_adr;                       // ���������� �����
     if (!change_item){                              // ���� �� ��������
          if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}      // ���� ������ �������
          sprintf(send_buf, " START" );                                              // � ���������� �����
     }
     if (change_item){
          if (!non_repit){ non_repit = true;  mirror_data_8bit = get_number_sensor (); last_adr = mirror_data_8bit; }  // ��� ������ ������ ���������, ������� ������
          if (mirror_data_8bit == 100) {error_func (ERROR4); change_item = false; non_repit = false; }                 // ���� ��� 100 ������ ����� �� ������
          else {
               if (button_press (KEY_UP)) { if (mirror_data_8bit < 12) mirror_data_8bit ++;  }                              // ��� ������� ��� ��������
               if (button_press (KEY_DOWN)) {if (mirror_data_8bit > 1) mirror_data_8bit --; }                               //
               blink_many_simbol (0x30);                                                                     // ������ ����� ������� ���������
               sprintf(send_buf, "    %02lu"  ,(uint32_t) mirror_data_8bit );                                               // �������� �� ��� ������� (����� �������)
               if (button_press (KEY_ENTER)) {                                                                              // �� ������� ����� ��������� ����� ��������
                    send_new_adr (last_adr,mirror_data_8bit);
                    change_item = false; non_repit = false;
                    error_func (NOERROR);                                                                                     // ��� ���� �������� �����, �� ����� ��� ����� ���������� ��
               }
          }
     }
}

void tim_stab_func (void)                  // ������� ����� ������� ������������
{
     enter_8bit (1,15,&param.tim_stab,EE_FILTR,2);
}

void of_adc_devise (void)                  // ������� ����� ���������� � ����� ������� ���
{
  enter_8bit (0,1,&param.of_adc,EE_OF_ADC,2);
}

void enter_8bit (uint8_t min, uint8_t max, uint8_t *paramet, uint16_t eeprom, uint8_t size)    // ������� ����� ����� 8���
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

void individual_koef (void)  // ������� ����� �������������� �����
{
    get_zemik_sensors(param.sensor_number);                                                                  //  ���������� �������
    if (boolean.data_ok)  {boolean.data_ok = false; converting_cod_to_massa (); massa = ((massa * param.calib_koef)+0.5);}   // ��� �������� �������� �����, 
    if (!boolean.non_rep){ boolean.non_rep = true; boolean.led_in_menu = true; mirror_data_8bit = 1; mirror_double2 = 0; }    // ��� ����� �������� �������� � ��������� ������� �������
    if (tim_5 == 0 && !boolean.errors){ if (massa < last_massa + param.discret1 *2  && massa > last_massa - param.discret1 *2 ) \
    {send_point(3);}  else {send_non_point(3);}  tim_5 = (param.tim_stab * 1000); last_massa = massa;}    // ������������
    (massa == 0 && !boolean.errors ) ?   send_point (2) : send_non_point(2);   // ���� ����� ����� 0 �� ������ ������    
         if (!change_item){                // ���� �� �� ������ ������
              if (not_change()) if (button_press (KEY_ENTER)) {change_item = true;}                                // ����������� ����� � ���� ��� ��������� � ���������� �����
              if (button_press (KEY_UP))   { if(mirror_data_8bit < param.sensor_number) mirror_data_8bit ++; }     // ������������ �����-���� �� ������� ��������
              if (button_press (KEY_DOWN)) { if(mirror_data_8bit > 1) mirror_data_8bit --; }
              sprintf(send_buf, "    %2lu"  ,(uint32_t) mirror_data_8bit );                                        // � ���������� ����� �������
         }
         if (change_item){                                                                              // ���� �� ������ ������
              if (!boolean.errors && boolean.led_display == 0x03) {sprintf(send_buf, "%6ld" ,massa ); *point_metrologi = mirror_double;}  // ���� ����� 3 ������ � ��� ������, �� ��� �����
              if ( boolean.errors && boolean.led_display == 0x03) {strcpy_P(send_buf, (PGM_P)pgm_read_word(&(error_string[ERROR4]))); }   // ���� ����� 3 ������ � ���� ������ �� ��� ���
              if(boolean.led_display < 4){ if (button_press (KEY_ENTER)) {                        // ����������� ����� 
              change_item = false;  boolean.led_in_menu = false; eeprom_calib_sens();  boolean.led_display = 0x00; *point_metrologi = mirror_double;}} // ������� �� ������� ���������, ���������� � ������, ���������� ������, ��������� ����������� ����� ��������
              if(boolean.led_display == 4){ if (button_press (KEY_ENTER)) {                        // ����������� ����� 
                 boolean.led_display = 0x00;                               // ���������� ����� ������
                 if (mirror_double < 10){
                 *point_metrologi = mirror_double;                         // ��������� ����������� ����� ��������
                 eeprom_calib_sens(); } else {error_func (ERROR5); non_repit = false;}  }}    // ������� �� ������� ���������, ���������             
              if(boolean.led_display == 4) {enter_float (); }              // ���� ��� 4 �����, �� ���� ����� � ������ ���������� ������
              else {                                                       // � ���� ����� ������
                if (button_press (KEY_LEFT)) {
                     if (boolean.led_display == 2) boolean.led_display = 0x04;  // ������ � ���� ������ �������� 4 �����
                }
                if (button_press (KEY_RIGHT)) {
                     boolean.led_display ^= 0x01;                          // ������ � ����� ����������� ����� ������� ����� � �����
                }
                if (button_press (KEY_UP)) {
                     case_up_down (1, 0.00001);                            // ������� ��������� ������
                }
                if (button_press (KEY_DOWN)) {
                     case_up_down(2, 0.00001);
                }
                if (button_long_press (KEY_UP)) {
                     case_up_down (1, 0.00011);                            // ��������� ��������� ������
                }
                if (button_long_press (KEY_DOWN)) {
                     case_up_down(2, 0.00011);
                }
              }              
              if(boolean.led_display == 0)case_up_down(0,0);               // ��� 0 �������, ���� ���� ��� ��������� � �������� � ������������� ���������� �����
              if(boolean.led_display == 2)case_up_down(0,0);               // ��� 2 �������, ���� ���������� ������� �����
         }        
}

void case_up_down (uint8_t up_down, double increment)                      // ������������� ������� ����� �����
{
     switch (mirror_data_8bit)                                             // � ���������� �� ���� ����� ����� ������� ������
     {
          case 1: point_metrologi  = &metrology.koef_sensor1; break;       // ��������� ����������� ����� � ������ ������� �����
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
          // mirror_double2 ����� ��� ���� ���� ��� ��������� ���� �� ����� �����(� ������ �����) ��� �������� � �������� �������, � � ������ ������� ��� ����� ��� ����������, ������� �������� �������� � ���� ����������
          if (up_down == 1){ if (mirror_double < 10) mirror_double += increment; } if(up_down == 2){if (mirror_double > 0) mirror_double -= increment; }  // ���� ������ � 1 �� ������� � 2 �� ��������
          if(boolean.led_display == 0){sprintf(send_buf, "%f" , mirror_double ); mirror_double = *point_metrologi;boolean.led_display = 2; mirror_double2 = *point_metrologi;}  // ���� ��� �������� ���������� �������� �����
          if(boolean.led_display == 2){sprintf(send_buf, "%f" , mirror_double );}     // � ����� ���������� ��� ������ ������� ����� ��������
}

void enter_no_pay (void)                                                   // ������ ���������� ���36
{
     if (!boolean.change_item_mirr){                                       // ����� ������� ����������/�� ���������� (��� ��� ������ ������������� � ����� �����)
           if (button_press (KEY_ENTER)) {boolean.change_item_mirr = true;}
          sprintf(send_buf, "000000"  );massa = 0;mirror_data = 0; }
	 if (boolean.change_item_mirr){
       switch (massa) {                                                    // ���������� ����� �� �� ����������))
           case 0: enter_numbers (5);   
           if (button_press (KEY_ENTER)) {
             if (mirror_data ==  param.pasw_err) {massa = 1;non_repit = false;mirror_data = 0;  EEPROM_write(EE_ERRPR36, 0); param.err36 = 0;  }   // ���� ������ ��������� � ��� ��� ����� ������� ����������
          	 else { error_func(ERROR5);boolean.change_item_mirr = false; non_repit = false;}                                                       // ��� ���5   
           } break;
           case 1: enter_1307(&param.data_err, &param.mount_err, &param.year_err, 2);   break;                                                     // ������ ����� ���� ����������            
           case 2: enter_numbers (5); 
           if (button_press (KEY_ENTER)) {
            param.pasw_err = mirror_data; EEPROM_write_32t(EE_PSWRD1,param.pasw_err);boolean.change_item_mirr = false; go_func = false;non_repit = false;} break;   // ������ ����� ������ � ���������� ��� � ������      
       }              
    }
}

