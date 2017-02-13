//sound.c main drivers for sound module
#include <stdio.h>
#include <stdint.h>
#include "Sound.h"
#include "AlarmClockSysTick.h"
#define  PF2 							(*((volatile uint32_t *)0x40025010)) 


uint32_t static notePeriod_A;

void Sound_Init(uint32_t freq){
  notePeriod_A = (freq*1000)/440;
  
  //init port
  //ensure port low
}

void Sound_FlagEnabledSound(uint8_t* flag){
  while(*flag){
    //toggle port
    SysTick_Wait(notePeriod_A);
  }
  //ensure port low
}


