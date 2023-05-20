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
	key = PINL;                                  // ��������� � ���������� ���� ������  0x3F - ����� �������
	if (key < 0x3F) {                            // ���� ���� ����� �� ������� ������ �� ���������� �������
		key_caunter ++;                          // ����������� ������� ������������
	}
	else{                                        // ���� ������ �� ������ �� ���������� ���
		key_caunter = 0;
		push = false;
		press_one = false;
		speed_button = false;
	}
	if (last_key != key) {key_caunter = 0;push = false;super_speed = 0; press_one = false;speed_button = false;}   // ���� ���������� ���� ������ � ����� ������ ������, �� �������� ��� �������� � �������, ������ ������ ������
	last_key = key;                               // ������������ ����� �������� � �������
	if (key_caunter == BOUNCE && !push) {push = true; buzers (1000); }       // ���� ������� ������ �������� ������������ �������, � ������� ��� ����������� �������
	if (long_state > 0) long_state --;            // �������� �������� ���������� ����������� �����
	
}


uint8_t button_press (uint8_t button)            // ������� �������� �� ������� 
{
	if 	(button == key && push && !press_one){   // ���� ��� ������ ��������� � ������� ������� � ������ ������ � �� ��� ��� �� ������������
		press_one = true;                         // ��������� ���� ���� ������ ���� �� �������� ���� �� �������� ������
		long_state = 2500;                       // ������� ������ ������� ��� ��������� ������
		return 1;                                // ���������� 1
	}
	else return 0;
	
}

uint8_t button_long_press (uint8_t button)                 // ���������� �������
{
	if 	(button == key && push && long_state == 0 && !speed_button){       // ���� ��� ������ ��������� � ������� ������� � ������� ��������� ������� = 0 � ��������� ��������
		long_state = LONG_BOUNCE;                                          // ������� ������ ������� ��� ��������� ������     
		if (super_speed == TIM_TO_SPEED) speed_button = true;              // ���� ��������� �-��� ���������� �������, ��������� ���� ��� ����� ���������
		super_speed ++;                                                    // ������� ���������� �������
		return 1;
	}
	else if (button == key && push && long_state == 0 && speed_button){    // ���� ��� ���� ����� � ������ ���� ����������� ��������������
		long_state = SPEED_BOUNCE;                                         // � ������ ��������� ������ �������� � �������� ������� ����������� 
		return 1;
	}
	else return 0;
	
}