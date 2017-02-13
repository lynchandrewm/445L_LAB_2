//Alarm.c main drivers for alarm module
#include <stdio.h>
#include <stdint.h>
#include "Alarm.h"
#include "Clock.h"


static alarm alarms[20];
static uint8_t numAlarms = 0;

int8_t static findFreeAlarm(void);

uint8_t Alarm_Enable(uint8_t hour, uint8_t minute){
	//find free alarm if available 
	int8_t alarmNum = findFreeAlarm();
	if(alarmNum == -1){
		return 0; 
	}
	//set alarm 
	alarms[alarmNum].hour  = hour;
	alarms[alarmNum].minute  = minute;
	alarms[alarmNum].valid  = 1;
	return 1;
}

uint8_t Alarm_Disable(uint8_t hour, uint8_t minute){
	for(int i = 0; i < 20; i++){
		if((alarms[i].valid == 1) && (hour == alarms[i].hour) && (alarms[i].minute == minute)){
			alarms[i].valid = 0; 
			return 1;
		}
	}
	return 0;
}

uint8_t Alarm_Check(){
	//Get current time 
	uint32_t timeReg = Clock_GetTimeReg();
	uint8_t hour = Clock_ExtractHour(timeReg);
	uint8_t minute = Clock_ExtractMinute(timeReg);
	
	//check to see if alarm goes off 
	for(int i = 0; i < 20; i++){
		if((alarms[i].valid == 1) && (hour == alarms[i].hour) && (alarms[i].minute == minute)){
			alarms[i].valid = 0; 
			return 1;
		}
	}
	return 0;
}

uint8_t Alarm_Number(){
	uint8_t numAlarmsEnabled = 0;
	for(int i = 0; i < 20; i++){
		if(alarms[i].valid == 1){
			numAlarmsEnabled++;
		}
	}
	return numAlarmsEnabled;
}

uint8_t Alarm_GetString(char* string, uint8_t index){
	uint8_t count = 0;
	uint8_t found = 0;
	for(int i = 0; i < 20; i++){
		if(alarms[i].valid == 1){
			count++;
		}
		if(count > index){
			index = i;
			found = 1;
			break;
		}
	}
	if(found == 0){
    sprintf(string,"None    ");
		return 0;
	}
	
	alarm selectedAlarm = alarms[index];
	uint8_t timeSplit = 0;
	uint8_t hour = selectedAlarm.hour;
	uint8_t minute = selectedAlarm.minute;
	char* AMPMstr = "AM";
  char* leadingZeroHour = "";
  char* leadingZeroMinute = "";
  if(hour>11){ 
    AMPMstr = "PM";
    hour -= 12;
  }
  if(hour == 0){ hour = 12; }
  if(hour<10){ leadingZeroHour = " "; }
  if(minute<10){ leadingZeroMinute = "0"; }
  sprintf(string, "%s%d:%s%d %s", leadingZeroHour, hour, leadingZeroMinute, minute, AMPMstr);
	return 1;
}

int8_t static findFreeAlarm(){
	if(numAlarms == 20){
		return -1;
	} 
	else{
		numAlarms++;
		for(int i = 0; i < 20; i++){
			if(alarms[i].valid == 0){
				return i;
			}
		}
	}
	return -1;
}
