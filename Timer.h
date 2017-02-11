// Timer.h
// TM4C123
// Public functions for Timer module
// Andrew Lynch
// Stathi Pakes
// February 9, 2017

#ifndef _TIMER
#define _TIMER

void Timer_CreateTimer(uint8_t hour, uint8_t minute, uint8_t second);

void Timer_GetRemainingTime(char* timer_string);

#endif
