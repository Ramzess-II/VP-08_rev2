

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <avr/io.h>
void skan_key (void);
uint8_t button_skan (uint8_t button, uint8_t state);
uint8_t button_press (uint8_t button);
uint8_t button_long_press (uint8_t button);





#endif /* KEYBOARD_H_ */