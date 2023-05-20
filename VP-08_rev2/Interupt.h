
#ifndef INTERUPT_H_
#define INTERUPT_H_


#include <avr/io.h>
#include <avr/interrupt.h>

void buzers (uint16_t tim);
void error_buzzer (bool sig);


#endif /* INTERUPT_H_ */