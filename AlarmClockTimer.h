/*timer.h - this file contains the functions for module Timer*/
#include <stdint.h>

/****************Timer_SetTimer***************
 Set the timer to the hour and minute specified. 
 Inputs:  unsined 8-bit integer hours and unsined 8-bit integer minutes 
 Outputs: none
 */ 
void AlarmClockTimer_Enable(uint8_t hour, uint8_t minute, uint8_t second);

/****************Timer_Disable***************
 Disables the timer. 
 Inputs: none
 Outputs: none 
 */ 
void AlarmClockTimer_Disable(void);

/****************Timer_Check***************
 Checks to see if timer triggered. If so, return 1 and 
 disable timer. If not, do nothing and return 0;
 Inputs:  none
 Outputs: 1 if an timer triggered, 0 if not
 */ 
uint8_t AlarmClockTimer_Check(void);


/****************Timer_GetString***************
 Checks to see if timer is enabled. If so, print the string representation of the timer
 If not, do nothing and return 0;
 Inputs:   
 Outputs: 1 if successful , 0 if not
 */ 
uint8_t AlarmClockTimer_GetString(char* string);


