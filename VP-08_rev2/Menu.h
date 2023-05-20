
#ifndef MENU_H_
#define MENU_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>

enum {                  // перечисления разных обработчиков клавиш в меню
     NUL_PTR,            
     SKROL_MENU = 1,
     DATA_INPUT,
     NO_CHILDREN,
     SPECIAL,

};

enum {                    // перечисление пунктов меню
     PAR1,
     PAR1_1,
     PAR1_2,
     PAR1_3,
     PAR2,
     PAR2_1,
     PAR2_2,
     PAR2_3,
     PAR2_4,
     PAR2_5,
     PAR2_6,
     PAR2_7,
     PAR2_8,
     PAR2_9,
     PAR2_10,
     PAR2_11,
     PAR2_12,
     PAR2_13,
     PAR2_14,
     PAR2_15,
     PAR2_16,
     PAR2_17,
     PAR3,
     PAR3_1,
     PAR3_2,    
     PAR4,
     PAR4_1,
     PAR4_1_1,
     PAR4_1_2,
     PAR4_2,
     PAR5,
     PAR6,
     PAR7,
};

enum                              // перечисление ерроров
{
     ERROR0,
     ERROR1,
     ERROR2,
     ERROR3,
     ERROR4,
     ERROR5,
     ERROR6,
     ERROR36,
     NOERROR,
     NO,
     BAT,
};

struct parametrs {                // структура с параметрами
     uint32_t caunter_weight;
     uint8_t discret1 ;
     uint8_t discret2 ;
     uint8_t discret3 ;
     int32_t sill1;
     int32_t sill2;
     uint8_t sensor_number ;
     uint8_t tip_cd ;
     int32_t NPV;
     uint8_t baud1;
     uint8_t baud2;
     uint8_t protokol1;
     uint8_t protokol2;
     uint8_t filtr;
     uint8_t lighte ;
     uint8_t zero_start;
     uint8_t err36;
     uint8_t tim_stab;
     uint8_t of_adc;
     double calib_koef;
     int32_t pasw_err;
     unsigned int data_err;
     unsigned int mount_err;
     unsigned int year_err;
} ;

struct metrologis {
    int32_t tara;
    double koef_sensor1;
    int32_t cod_zero_sensor1;
    int32_t cod_sensor1;
    double koef_sensor2;
    int32_t cod_zero_sensor2;
    int32_t cod_sensor2;   
    double koef_sensor3;
    int32_t cod_zero_sensor3;
    int32_t cod_sensor3;    
    double koef_sensor4;
    int32_t cod_zero_sensor4;
    int32_t cod_sensor4;  
    double koef_sensor5;
    int32_t cod_zero_sensor5;
    int32_t cod_sensor5;
    double koef_sensor6;
    int32_t cod_zero_sensor6;
    int32_t cod_sensor6;
    double koef_sensor7;
    int32_t cod_zero_sensor7;
    int32_t cod_sensor7;
    double koef_sensor8;
    int32_t cod_zero_sensor8;
    int32_t cod_sensor8;
    double koef_sensor9;
    int32_t cod_zero_sensor9;
    int32_t cod_sensor9;
    double koef_sensor10;
    int32_t cod_zero_sensor10;
    int32_t cod_sensor10;
    double koef_sensor11;
    int32_t cod_zero_sensor11;
    int32_t cod_sensor11;
    double koef_sensor12;
    int32_t cod_zero_sensor12;
    int32_t cod_sensor12;      
              
     };

// строки с выводимыми ерорами
const char er_string0[] PROGMEM = "ERR0";    // 
const char er_string1[] PROGMEM = "ERR1";    // превышение нпв
const char er_string2[] PROGMEM = "ERR2";
const char er_string3[] PROGMEM = "ERR3";
const char er_string4[] PROGMEM = "ERR4";    // ошибка датчиков (адресация. калибровка)
const char er_string5[] PROGMEM = "ERR5";    // не верное значение
const char er_string6[] PROGMEM = "ERR6";    // 
const char er_string7[] PROGMEM = "ERR36";   //оплата
const char er_string8[] PROGMEM = " -YES-";
const char er_string9[] PROGMEM = " -NO- ";
const char er_string10[] PROGMEM = " BAT  ";
// а это массив указателей на строки
const char *const error_string[] PROGMEM = {
     [ERROR0] = er_string0,
     [ERROR1] = er_string1,
     [ERROR2] = er_string2,
     [ERROR3] = er_string3,
     [ERROR4] = er_string4,
     [ERROR5] = er_string5,
     [ERROR6] = er_string6,
     [ERROR36] = er_string7,
     [NOERROR] = er_string8,
     [NO] = er_string9,
     [BAT] = er_string10,
  };

// функции из меню, разбиты по группам, но это не точно
void main_menu (void);
void _key_strob (void);
void enter_numbers (uint8_t size );
void enter_float (void  ) ;
void set_nubers (uint8_t min, uint8_t max);
void error_func (uint8_t error_number);
void void_func (void);
uint8_t not_change (void);

void ver_po (void);
void send_adc (void);
void caunter_weighing (void);

void diskret1_new (void);
void discret2 (void);
void discret3 (void);
void input_discret (uint8_t *inbaud, uint8_t nomer);
void sill1_discrt (void);
void sill2_discrt (void);
void in_npv (void);
void tips_cd (void);
void sensor_numbers (void);
void baud_rate1 (void);
void baud_rate2 (void);
void input_baud (uint8_t *inbaud, uint8_t nomer);
void protocol_data1 (void);
void protocol_data2 (void);
void filtr_data (void);
void light (void);
void zero_in_start (void);
void tim_stab_func (void);
void of_adc_devise (void);
void individual_koef (void);
void enter_no_pay (void);

void enter_time (void);
void enter_date (void);
void enter_1307 (unsigned int *number1, unsigned int *number2, unsigned int *number3, uint8_t option);
void enter_8bit (uint8_t min, uint8_t max, uint8_t *paramet, uint16_t eeprom, uint8_t size);

void Pasword_press (void);
void kal_koef (void);
void kalib_vp (void);
void adress_sens (void);
void case_up_down (uint8_t up_down, double increment);

void init_menu (void);




#endif /* MENU_H_ */