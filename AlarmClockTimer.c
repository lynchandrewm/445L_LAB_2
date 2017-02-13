#include <stdio.h>
#include <stdint.h>
#include "AlarmClockTimer.h"
#include "Clock.h"

static uint8_t timerSecond;
static uint8_t timerMinute;
static uint8_t timerHour;
static uint8_t isOn; 

void AlarmClockTimer_Enable(uint8_t hour, uint8_t minute, uint8_t second){
	//set timer
	timerHour = hour;
	timerMinute  = minute;
  timerSecond = second;
	isOn  = 1;
}

void AlarmClockTimer_Disable(){
	isOn = 0;
}

uint8_t AlarmClockTimer_Check(){
	uint32_t timeReg = Clock_GetTimeReg();
	uint8_t hour = Clock_ExtractHour(timeReg);
	uint8_t minute = Clock_ExtractMinute(timeReg);
  uint8_t second = Clock_ExtractSecond(timeReg);
	//check to see if timer goes off 
	if((timerHour == hour) && (timerMinute == minute) && (timerSecond == second)){
		isOn = 0;
		return 1;
	}
	return 0;
} 

uint8_t AlarmClockTimer_GetString(char* string){
	if(isOn == 0){
    sprintf(string,"");
		return 0;
	}
	//Get current time 
	uint8_t second, minute, hour;
	uint32_t timeReg = Clock_GetTimeReg();
  second = Clock_ExtractSecond(timeReg);
  minute = Clock_ExtractMinute(timeReg);
  hour   = Clock_ExtractHour(timeReg);
	//calculate hour
	int8_t hourChange = -1;
	if(hour <= timerHour){
		hour = timerHour - hour;
	} else{
		hour =  (24 - hour) + timerHour;
	}
	//calculate minute + hour change
	if(minute <= timerMinute){
		minute = timerMinute - minute;
		hourChange++;
	} else{
		minute = (60 - minute) + timerMinute;
	}
	while(minute >= 60){
			minute -= 60;
			hourChange++;
	}
	hour += hourChange;
	//calculate second + minute change 
	second = 60 - second;
	if(second == 60){
		second -= 60;
	} else{
		minute--;
	}
	//print time left 
  char* leadingZeroHour = "";
  char* leadingZeroMinute = "";
  char* leadingZeroSecond = "";
  if(hour<10){ leadingZeroHour = " "; }
  if(minute<10){ leadingZeroMinute = "0"; }
  if(second<10){ leadingZeroSecond = "0"; }
  sprintf(string, "%s%d:%s%d:%s%d", leadingZeroHour, hour, leadingZeroMinute, minute, leadingZeroSecond, second);
	return 1;
}
