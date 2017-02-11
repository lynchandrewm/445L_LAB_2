// AlarmClock.c
// TM4C
// Runs a fully functional alarm clock 
// fit with 20 alarms, 1 timer, and auto-updating time
// Andrew Lynch
// Stathi Pakes
// February 8, 2017

#include <stdint.h>
#include "AlarmClock.h"
#include "DisplayClock.h"
#include "Alarm.h"
#include "Timer.h"
#include "Clock.h"
#include "ST7735.h"
#include "Switch.h"
#include "SysTick.h"

extern void EnableInterrupts(void);
extern void DisableInterrupts(void);
extern long StartCritical(void);
extern void EndCritical(long sr);
extern void WaitForInterrupt(void);

uint32_t static tempDisplay[10];
uint8_t static cursor;

char* MainClockScrollString[4] = {" Alarm Set ", " Timer Set ", "View Alarms", " Set Time  "};

//prototypes of private functions
void static TitleFrame(void);
void static SetTime(void);
void static SetTimeRecursive(void);
void static SetTime_Left(void);
void static SetTime_Middle(void);
void static SetTime_Right(void);
void static MainClock(void);
void static MainClock_GetAlarms(void);
void static MainClockRecursive(void);
void static MainClock_Left(void);
void static MainClock_Middle(void);
void static MainClock_Right(void);
void static AlarmSet(void);
void static AlarmSet_Right(void);
void static TimerSet(void);
void static TimerSetRecursive(void);
void static TimerSet_Left(void);
void static TimerSet_Right(void);
void static ViewActiveAlarms(void);
void static ActiveAlarmsRecursive(void);
void static ActiveAlarms_Left(void);
void static ActiveAlarms_Middle(void);
void static ActiveAlarms_Right(void);


void AlarmClock_Init(uint32_t freq){
  ST7735_InitR(INITR_REDTAB);
  Clock_Init(freq);
  Switch_Init();
}

void AlarmClock_Start(){
  TitleFrame();
}

void static TitleFrame(void){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Lab 3\n  Andrew and Stathi";
  x = 8; y = 7;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  SetTime();
}

void static SetTime(void){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Set Time:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_OutString("+    >    Done");
  Switch_AssignTask(0,SetTime_Left);
  Switch_AssignTask(1,SetTime_Middle);
  Switch_AssignTask(2,SetTime_Right);
  cursor = 0;
  tempDisplay[0] = 12;
  for(int i = 1; i < 3; i++){
    tempDisplay[i] = 0;
  }
  SetTimeRecursive();
}

//doesn't handle hours and min being one or two characters
void static SetTimeRecursive(void){int32_t x,y;
  char minSpacer = '0';
  char hourSpacer = ' ';
  x = 8; y = 7;
  ST7735_SetCursor(x,y);
  if(tempDisplay[0]<10){
    ST7735_OutChar(hourSpacer);
  }
  ST7735_OutUDec(tempDisplay[0]);
  ST7735_OutChar(':');
  if(tempDisplay[1]<10){
    ST7735_OutChar(minSpacer);
  }
  ST7735_OutUDec(tempDisplay[1]);
  if(tempDisplay[2]){
    ST7735_OutString(" PM");
  }
  else{
    ST7735_OutString(" AM");
  }
  switch(cursor){
    case 0:
      x = 8; y = 7;  //position of hour
    case 1:
      x = 11; y = 7; //position of minute
    case 2:
      x = 14; y = 7; //position of AM/PM
  }
  SysTick_Wait10ms(50);
  ST7735_OutString("  ");
  SysTick_Wait10ms(50);
}

void SetTime_Left(void){
  switch(cursor){
    case 0:
      if(tempDisplay[cursor]==12){
        tempDisplay[cursor] = 1;
        break;
      }
      tempDisplay[cursor]++;
      break;
    case 1:
      if(tempDisplay[cursor]==59){
        tempDisplay[cursor] = 0;
        break;
      }
      tempDisplay[cursor]++;
      break;
    case 2:
      tempDisplay[cursor] ^= 0x1;
  }
}

void SetTime_Middle(void){
  switch(cursor){
    case 0:
      cursor++;
      break;
    case 1:
      cursor++;
      break;
    case 2:
      cursor = 0;
  }
}

void SetTime_Right(void){
  if(tempDisplay[2]){
    Clock_SetHour(tempDisplay[0]+12);
  }
  else{
    Clock_SetHour(tempDisplay[0]);
  }
  Clock_SetMinute(tempDisplay[1]);
  MainClock();
}

char* AlarmStrings[20];
uint8_t AlarmIndex = 0;

void static MainClock(void){
  ST7735_FillScreen(ST7735_BLACK);
  Switch_AssignTask(0,MainClock_Left);
  Switch_AssignTask(2,MainClock_Middle);
  Switch_AssignTask(3,MainClock_Right);
  cursor = 0;
  tempDisplay[0] = 0; tempDisplay[1] = 0; tempDisplay[2] = 0;
  MainClock_GetAlarms();
  MainClockRecursive();
}

void static MainClock_GetAlarms(void){
  alarm alarms[20];
  Alarm_GetAlarms(alarms);
  for(int i = 0; i < 20; i++){
    if(alarms[i].valid){
      Clock_BuildTimeString(alarms[i].hours, alarms[i].minutes, AlarmStrings[AlarmIndex++]);
    }
  }
}

/*
  DisplayClock_UpdateClock(char* alarms, uint8_t numAlarms, char* digitalTime, uint8_t numDigitalTime, char* timer, uint8_t numTimer,
    char* scrollingString, uint8_t numScrollingString, uint8_t secondsDegree, uint8_t minutesDegree, uint8_t hoursDegree);
*/

void static MainClockRecursive(void){uint8_t secondsDegree, minutesDegree, hoursDegree;
  char time_string[12]; Clock_GetTime(time_string);
  char timer_string[12]; Timer_GetRemainingTime(timer_string);
  uint32_t localTime = Clock_GetTimeReg();
  secondsDegree = Clock_ExtractSecond(localTime); minutesDegree = Clock_ExtractMinute(localTime); hoursDegree = Clock_ExtractHour(localTime);
  if(hoursDegree>11){ 
    hoursDegree -= 12;
  }
  if(hoursDegree == 0){ hoursDegree = 12; }
  hoursDegree *= 5;
  if(tempDisplay[0]%800000000){
    tempDisplay[1] = 1;
    tempDisplay[0] = 0;
  }
  if(tempDisplay[1]&&(tempDisplay[0]%80000000)){
    DisplayClock_UpdateClock(AlarmStrings[cursor], time_string, timer_string, 
      MainClockScrollString[tempDisplay[2]], secondsDegree, minutesDegree, hoursDegree);
    cursor++;
    if(cursor>=AlarmIndex){
      cursor = 0;
      tempDisplay[1] = 0;
    }
    tempDisplay[0] = 0;
    MainClockRecursive();
  }
  DisplayClock_UpdateClock(AlarmStrings[0], time_string, timer_string, 
      MainClockScrollString[tempDisplay[2]], secondsDegree, minutesDegree, hoursDegree);
  tempDisplay[0]++;
}

void static MainClock_Left(void){
  if(tempDisplay[2]==0){
    tempDisplay[2] = 3;
  } 
  else{
    tempDisplay[2]--;
  }
}

void static MainClock_Middle(void){
  switch(tempDisplay[2]){
    case 0:
      AlarmSet();
    case 1:
      TimerSet();
    case 2:
      ViewActiveAlarms();
    case 3:
      SetTime();
  }
}

void static MainClock_Right(void){
  if(tempDisplay[2]==3){
    tempDisplay[2] = 0;
  } 
  else{
    tempDisplay[2]++;
  }
}

void static AlarmSet(void){uint8_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Set Alarm:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_OutString("+    >    Done");
  Switch_AssignTask(0,SetTime_Left);
  Switch_AssignTask(1,SetTime_Middle);
  Switch_AssignTask(2,AlarmSet_Right);
  cursor = 0;
  tempDisplay[0] = 12;
  for(int i = 1; i < 3; i++){
    tempDisplay[i] = 0;
  }
  SetTimeRecursive();
}

void static AlarmSet_Right(void){uint8_t hour, minute;
  if(tempDisplay[2]){
    hour = tempDisplay[0]+12;
  }
  else{
    hour = tempDisplay[0];
  }
  minute = tempDisplay[1];
  Alarm_SetNewAlarm(hour, minute);
  MainClock();
}

void static TimerSet(void){uint8_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Set Timer:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_OutString("+    >    Done");
  Switch_AssignTask(0,TimerSet_Left);
  Switch_AssignTask(1,SetTime_Middle);
  Switch_AssignTask(2,TimerSet_Right);
  cursor = 0;
  for(int i = 0; i < 3; i++){
    tempDisplay[i] = 0;
  }
  TimerSetRecursive();
}

void static TimerSetRecursive(void){int32_t x,y;
  char minSpacer = '0';
  char hourSpacer = ' ';
  x = 8; y = 7;
  ST7735_SetCursor(x,y);
  if(tempDisplay[0]<10){
    ST7735_OutChar(hourSpacer);
  }
  ST7735_OutUDec(tempDisplay[0]);
  ST7735_OutChar(':');
  if(tempDisplay[1]<10){
    ST7735_OutChar(minSpacer);
  }
  ST7735_OutUDec(tempDisplay[1]);
  if(tempDisplay[2]<10){
    ST7735_OutChar(minSpacer);
  }
  ST7735_OutUDec(tempDisplay[2]);
  switch(cursor){
    case 0:
      x = 8; y = 7;  //position of hour
    case 1:
      x = 11; y = 7; //position of minute
    case 2:
      x = 14; y = 7; //position of second
  }
  SysTick_Wait10ms(50);
  ST7735_OutString("  ");
  SysTick_Wait10ms(50);
}

void static TimerSet_Left(void){  
  switch(cursor){
    case 0:
      if(tempDisplay[cursor]==12){
        tempDisplay[cursor] = 1;
        break;
      }
      tempDisplay[cursor]++;
      break;
    case 1:
      if(tempDisplay[cursor]==59){
        tempDisplay[cursor] = 0;
        break;
      }
      tempDisplay[cursor]++;
      break;
    case 2:
      if(tempDisplay[cursor]==59){
        tempDisplay[cursor] = 0;
        break;
      }
      tempDisplay[cursor]++;
  }
}

void static TimerSet_Right(void){uint8_t hour, minute, second;
  if(tempDisplay[2]){
    hour = tempDisplay[0]+12;
  }
  else{
    hour = tempDisplay[0];
  }
  minute = tempDisplay[1];
  second = tempDisplay[2];
  Timer_CreateTimer(hour, minute, second);
  MainClock();
}

void static ViewActiveAlarms(void){uint8_t x,y; 
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Active Alarms:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_OutString("Delete  >  Done");
  Switch_AssignTask(0,ActiveAlarms_Left);
  Switch_AssignTask(1,ActiveAlarms_Middle);
  Switch_AssignTask(2,ActiveAlarms_Right);
  cursor = 0;
  ActiveAlarmsRecursive();
}

void static ActiveAlarmsRecursive(void){uint8_t x,y; 
  x = 8; y = 7;
  if(AlarmIndex){
    x = 4; 
    ST7735_SetCursor(x,y);
    ST7735_OutString("No alarms set");
    WaitForInterrupt();
  }
  ST7735_SetCursor(x,y);
  ST7735_OutString(AlarmStrings[cursor]);
  WaitForInterrupt();
}

void static ActiveAlarms_Left(void){uint8_t hour, minute;
  Clock_ParseStringToTime(AlarmStrings[cursor], &hour, &minute);
  Alarm_DeleteAlarm(hour, minute);
  MainClock_GetAlarms();
  ActiveAlarmsRecursive();
}

void static ActiveAlarms_Middle(void){
  cursor++;
  if(cursor>=AlarmIndex){
    cursor = 0;
  }
  ActiveAlarmsRecursive();
}

void static ActiveAlarms_Right(void){
  MainClock();
}

