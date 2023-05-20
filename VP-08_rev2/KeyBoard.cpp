#include "KeyBoard.h"

#include "Interupt.h"
#include "max7219.h"
#include "Menu.h"
#include "UART.h"
#include "RTC1307.h"
#include "EEprom.h"
#include "main.h"

volatile static uint16_t long_state;
volatile static uint8_t key,super_speed;
volatile static bool push,press_one,speed_button;




void skan_key (void)
{
	volatile static uint8_t last_key;
	volatile static uint16_t key_caunter;
	key = PINL;                                  // считываем в переменную порт кнопок  0x3F - везде плюсики
	if (key < 0x3F) {                            // если хоть какая то клавиша нажата то выполнится условие
		key_caunter ++;                          // увеличиваем счетчик антидребезга
	}
	else{                                        // если ничего не нажато то сбрасываем все
		key_caunter = 0;
		push = false;
		press_one = false;
		speed_button = false;
	}
	if (last_key != key) {key_caunter = 0;push = false;super_speed = 0; press_one = false;speed_button = false;}   // если удерживали одну кнопку а затем нажали другую, то сбросить все счетчики и таймера, начать отсчет заново
	last_key = key;                               // приравниваем новое значение к старому
	if (key_caunter == BOUNCE && !push) {push = true; buzers (1000); }       // если счетчик достиг значения антидребезга пикнуть, и сказать что произведено нажатие
	if (long_state > 0) long_state --;            // отнимаем значение переменной ускоренного счета
	
}


uint8_t button_press (uint8_t button)            // функция проверки на нажатие 
{
	if 	(button == key && push && !press_one){   // если код кнопки совпадает с нажатой кнопкой и кнопка нажата и мы это еще не использовали
		press_one = true;                         // поднимаем флаг чтоб больше сюда не заходить пока не отпустим кнопку
		long_state = 2500;                       // взводим таймер отсчета для удержания клавиш
		return 1;                                // возвращаем 1
	}
	else return 0;
	
}

uint8_t button_long_press (uint8_t button)                 // длительное нажатие
{
	if 	(button == key && push && long_state == 0 && !speed_button){       // если код кнопки совпадает с нажатой кнопкой и счетчик обратного отсчета = 0 и отключена скорость
		long_state = LONG_BOUNCE;                                          // взводим таймер отсчета для удержания клавиш     
		if (super_speed == TIM_TO_SPEED) speed_button = true;              // если насчитали н-ное количество нажатий, поднимаем флаг что нужно ускорится
		super_speed ++;                                                    // считаем количество нажатий
		return 1;
	}
	else if (button == key && push && long_state == 0 && speed_button){    // если все тоже самое и поднят флаг ускоренного перелистывания
		long_state = SPEED_BOUNCE;                                         // в таймер загружаем другое значение и начинаем быстрее перемещатся 
		return 1;
	}
	else return 0;
	
}