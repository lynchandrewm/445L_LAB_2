//Alarm.c main drivers for alarm module
#include <stdio.h>
#include <stdint.h>
#include "Alarm.h"
#include "clock.h"

#define MAX_NUM_ALARMS 20

static alarm alarms[MAX_NUM_ALARMS];
static uint8_t numAlarms = 0;

uint8_t static FindFreeAlarm(void);

uint8_t Alarm_SetNewAlarm(uint8_t hours, uint8_t minutes){
	//find free alarm if available 
	uint8_t alarmNum = FindFreeAlarm();
	if(alarmNum == 0xFF){
		return 0; 
	}
	//set alarm 
	alarms[alarmNum].hours  = hours;
	alarms[alarmNum].minutes  = minutes;
	alarms[alarmNum].valid  = 1;
	return 1;
}

uint8_t static FindFreeAlarm(){
	if(numAlarms == MAX_NUM_ALARMS){
		return 0xFF;
	} 
	else{
		numAlarms++;
		for(int i = 0; i < MAX_NUM_ALARMS; i++){
			if(alarms[i].valid == 0){
				return i;
			}
		}
	}
}

uint8_t Alarm_AlarmTriggerd(){
	//Get current time 
	uint32_t timeReg = Clock_GetTimeReg();
	uint8_t hours = Clock_ExtractHour(timeReg);
	uint8_t minutes = Clock_ExtractMinute(timeReg);
	//check to see if alarm goes off 
	for(int i = 0; i < 20; i++){
		if((alarms[i].valid == 1) && (hours == alarms[i].hours) && (alarms[i].minutes == minutes)){
			alarms[i].valid = 0; 
			return 1;
		}
	}
	return 0;
}

uint8_t Alarm_AlarmsEnabled(){
		
	uint8_t numAlarmsEnabled = 0;
	for(int i = 0; i < 20; i++){
		if(alarms[i].valid == 1){
			numAlarmsEnabled++;
		}
	}
	return numAlarmsEnabled;
}

void Alarm_GetAlarms(alarm* alarmsCopy){
	
	for(int i = 0; i < 20; i++){
		if(alarms[i].valid == 1){
			alarmsCopy[i].hours = alarms[i].hours;
			alarmsCopy[i].minutes = alarms[i].minutes;
			alarmsCopy[i].valid = alarms[i].valid;
		}
	}
}

void Alarm_DeleteAlarm(uint8_t hour, uint8_t minute){
  
}
