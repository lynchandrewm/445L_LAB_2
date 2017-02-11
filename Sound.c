//sound.c main drivers for sound module
#include <stdio.h>
#include <stdint.h>
#include "sound.h"
#include "Timer4a.h"
#define  PF2 							(*((volatile uint32_t *)0x40025010)) 


void makeSound(void){
	PF2 ^= 0x0004;
}

void soundInit(uint32_t frequency){
	frequency *= 1000;
	uint32_t period = 80000000/frequency;
	Timer4AInit(period, 6);
	Timer4A_AddPeriodicThread(makeSound);
}

void startBuzzer(){
	startTimer();
}

void stopBuzzer(){
	stopTimer();
	PF2 &= 0xFFFC;
}


