/*displayClock.h - this file contains the functions for module DisplayClock*/

#ifndef _DISPLAYCLOCK
#define _DISPLAYCLOCK

/****************displayClock***************
 Prints the main clock display to the screen
 Inputs:  none
 Outputs: Clock on the LCD screen
 */ 
void DisplayClock_PrintFullClockFace(char* alarm, char* digitalTime, char* timer, char* selectionBar, uint8_t second, uint8_t minute, uint8_t hour);

void DisplayClock_UpdateClock(char* alarm, char* digitalTime, char* timer, char* selectionBar, uint8_t second, uint8_t minute, uint8_t hour);

#endif