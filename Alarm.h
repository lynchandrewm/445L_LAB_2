/*Alarm.h - this file contains the functions for module Alarm*/
#include <stdint.h>

typedef struct alarm{
	uint8_t hour;
	uint8_t minute;
	uint8_t valid;
} alarm;

/****************Alarm_Enable***************
 If possible, set an alarm to the hour and minute specified. 
 If sucessful return 1, else if not sucessful or no alarm available return 0
 Inputs:  unsined 8-bit integer hours and unsined 8-bit integer minutes 
 Outputs: 1 if successful, 0 if not 
 */ 
uint8_t Alarm_Enable(uint8_t hours, uint8_t minutes);

/****************Alarm_Disable***************
 If possible, remove an alarm set to the hour and minute specified. 
 If sucessful return 1, else if not sucessful or no alarm available return 0
 Inputs:  unsined 8-bit integer hours and unsined 8-bit integer minutes 
 Outputs: 1 if successful, 0 if not 
 */ 
uint8_t Alarm_Disable(uint8_t hours, uint8_t minutes);

/****************Alarm_Check***************
 Checks to see if any alarms are triggered. If so, return 1 and 
 remove triggered alarm. If not, do nothing and return 0;
 Inputs:  none
 Outputs: 1 if an alarm triggered, 0 if not
 */ 
uint8_t Alarm_Check(void);

/****************Alarm_Number***************
 Checks to see if any alarms are enabled. If so, return the number enabled 
 If not, do nothing and return 0;
 Inputs:  none
 Outputs: number of alarms if alarms enabled, 0 if not
 */ 
uint8_t Alarm_Number(void);

/****************Alarm_GetString***************
 Checks to see if any alarms are enabled. If so, print the string representation of the enabled one at the specified index
 If not, do nothing and return 0;
 Inputs:   
 Outputs: 1 if successful , 0 if not
 */ 
uint8_t Alarm_GetString(char* string, uint8_t index);





