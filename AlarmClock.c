// AlarmClock.c
// TM4C
// Runs a fully functional alarm clock 
// fit with 20 alarms, 1 timer, and auto-updating time
// Andrew Lynch
// Stathi Pakes
// February 8, 2017

#include <stdint.h>
#include <stdio.h>
#include "AlarmClock.h"
#include "DisplayClock.h"
#include "Alarm.h"
#include "AlarmClockTimer.h"
#include "Clock.h"
#include "ST7735.h"
#include "Switch.h"
#include "Sound.h"
#include "AlarmClockSysTick.h"
#include "Timer4A.h"

/*
extern void EnableInterrupts(void);
extern void DisableInterrupts(void);
extern long StartCritical(void);
extern void EndCritical(long sr);
extern void WaitForInterrupt(void);
*/

enum uint8_t {
  Left,
  Middle,
  Right
};

uint8_t static SwitchFlag[3];
uint32_t static tempDisplay[10];
uint8_t static cursor;

char* MainClockScrollString[4] = {" Alarm Set ", " Timer Set ", "View Alarms", " Set Time  "};

//prototypes of private functions
uint8_t static NextState(uint8_t next);
void static TitleFrame(void);
void static SetTime(void);
void static SetTimeRecursive(void);
void static SetTime_Left(void);
void static SetTime_Middle(void);
void static SetTime_Right(void);
uint8_t static MainClock(void);
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
void static InitiateAlarm(void);
void static CheckAlarmsAndTimer(void);


void AlarmClock_Init(uint32_t freq){
  ST7735_InitR(INITR_REDTAB);
  Clock_Init(freq);
  Switch_Init(freq);
  SysTick_Init(freq);
  Timer4A_Init(freq*1000,5,CheckAlarmsAndTimer);
}

void AlarmClock_Start(){uint8_t state = 0;
  while(1){
    state = NextState(state);
  }
}

uint8_t static NextState(uint8_t next){
  switch(next){
    case 0:
      TitleFrame();
      return 1;
    case 1:
      SetTime();
      break;
    case 2:
      return MainClock();
    case 3:
      AlarmSet();
      break;
    case 4:
      TimerSet();
      break;
    case 5:
      ViewActiveAlarms();
      break;
  }
  return 2;
}

void static TitleFrame(void){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Lab 3\n  Andrew and Stathi";
  x = 8; y = 7;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  SysTick_Wait10ms(200);
}

void static SetTime(void){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Set Time:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_SetCursor(x,y);
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
  SwitchFlag[Right] = 1;
  while(SwitchFlag[Right]){
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
        break;
      case 1:
        x = 11; y = 7; //position of minute
        break;
      case 2:
        x = 14; y = 7; //position of AM/PM
    }
    ST7735_SetCursor(x,y);
    SysTick_Wait10ms(50);
    ST7735_OutString("  ");
    SysTick_Wait10ms(50);
  }
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
  if(tempDisplay[0]==12){
    tempDisplay[0] = 0;
  }
  if(tempDisplay[2]){
    Clock_SetHour(tempDisplay[0]+12);
  }
  else{
    Clock_SetHour(tempDisplay[0]);
  }
  Clock_SetMinute(tempDisplay[1]);
  SwitchFlag[Right] = 0;
}

uint8_t static MainClock(void){
  ST7735_FillScreen(ST7735_BLACK);
  Switch_AssignTask(0,MainClock_Left);
  Switch_AssignTask(1,MainClock_Middle);
  Switch_AssignTask(2,MainClock_Right);
  cursor = 0;
  tempDisplay[0] = 0; tempDisplay[1] = 0; tempDisplay[2] = 0;
  MainClockRecursive();
  return cursor;
}

/*
  DisplayClock_UpdateClock(char* alarms, uint8_t numAlarms, char* digitalTime, uint8_t numDigitalTime, char* timer, uint8_t numTimer,
    char* scrollingString, uint8_t numScrollingString, uint8_t secondsDegree, uint8_t minutesDegree, uint8_t hoursDegree);
*/

void static MainClock_GetDegrees(uint32_t localTime, uint8_t* seconds, uint8_t* minutes, uint8_t* hours){uint8_t twelvesCount, twelvesTotal;
  twelvesCount = 0; twelvesTotal = 12;
  *seconds = Clock_ExtractSecond(localTime); *minutes = Clock_ExtractMinute(localTime); *hours = Clock_ExtractHour(localTime);
  if(*hours>11){ *hours-=12; }
  *hours *= 5;
  while(twelvesTotal<*minutes){
    twelvesCount++;
    twelvesTotal += 12;
  }
  *hours += twelvesCount;
} 

void static MainClock_SelectionBar(char string[9], uint8_t index){
  switch(cursor){
    case 0:
      sprintf(string,"<-- Title Screen -->");
      break;
    case 1:
      sprintf(string,"<--  Reset Time  -->");
      break;
    case 3:
      sprintf(string,"<--   Set Alarm  -->");
      break;
    case 4:
      sprintf(string,"<--   Set Timer  -->");
      break;
    case 5:
      sprintf(string,"<-- View Alarms  -->");
      break;
  }

}

void static MainClockRecursive(void){uint8_t secondsDegree, minutesDegree, hoursDegree, numAlarms, currentAlarm, previousSecond;
  char time_string[12], timer_string[12], alarm_string[9], selectionBar_string[21]; 
  uint32_t localTime; 
  Clock_GetTime(time_string); AlarmClockTimer_GetString(timer_string);
  localTime = Clock_GetTimeReg();
  MainClock_GetDegrees(localTime, &secondsDegree, &minutesDegree, &hoursDegree);
  numAlarms = Alarm_Number();
  currentAlarm = 0;
  Alarm_GetString(alarm_string, currentAlarm++);
  cursor = 3;
  MainClock_SelectionBar(selectionBar_string,cursor);
  DisplayClock_PrintFullClockFace(alarm_string,time_string,timer_string,selectionBar_string,secondsDegree,minutesDegree,hoursDegree);
  SwitchFlag[Middle] = 1;
  while(SwitchFlag[Middle]){
    previousSecond = secondsDegree;
    do{
      localTime = Clock_GetTimeReg();
    }while(previousSecond==Clock_ExtractSecond(localTime));
    if(numAlarms){
      Alarm_GetString(alarm_string, currentAlarm++);
    }
    if(currentAlarm==numAlarms){
      currentAlarm = 0;
    }
    Clock_GetTime(time_string); AlarmClockTimer_GetString(timer_string); MainClock_SelectionBar(selectionBar_string,cursor);
    MainClock_GetDegrees(localTime, &secondsDegree, &minutesDegree, &hoursDegree);
    DisplayClock_UpdateClock(alarm_string,time_string,timer_string,selectionBar_string,secondsDegree,minutesDegree,hoursDegree);
  }
}

void static MainClock_Left(void){
  switch(cursor){
    case 0:
      cursor = 5;
      break;
    case 1:
      cursor = 0;
      break;
    case 3:
      cursor = 1;
      break;
    case 4:
      cursor = 3;
      break;
    case 5:
      cursor = 4;
      break;
  }
}

void static MainClock_Middle(void){
  SwitchFlag[Middle] = 0;
}

void static MainClock_Right(void){
  switch(cursor){
    case 0:
      cursor = 1;
      break;
    case 1:
      cursor = 3;
      break;
    case 3:
      cursor = 4;
      break;
    case 4:
      cursor = 5;
      break;
    case 5:
      cursor = 0;
      break;
  }
}


void static AlarmSet(void){uint8_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Set Alarm:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_SetCursor(x,y);
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
  Alarm_Enable(hour, minute);
  SwitchFlag[Right] = 0;
}

void static TimerSet(void){uint8_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Set Timer:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_SetCursor(x,y);
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
  SwitchFlag[Right] = 1;
  while(SwitchFlag[Right]){
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
    ST7735_OutChar(':');
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
    ST7735_SetCursor(x,y);
    SysTick_Wait10ms(50);
    ST7735_OutString("  ");
    SysTick_Wait10ms(50);
  }
}

void static TimerSet_Left(void){  
  switch(cursor){
    case 0:
      if(tempDisplay[cursor]==99){
        tempDisplay[cursor] = 0;
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
  hour = tempDisplay[0];
  minute = tempDisplay[1];
  second = tempDisplay[2];
  AlarmClockTimer_Enable(hour, minute, second);
  SwitchFlag[Right] = 0;
}

void static ViewActiveAlarms(void){uint8_t x,y,numAlarms;
  char alarm_string[9];
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Active Alarms:";
  x = 6; y = 0;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
  x = 4; y = 15;
  ST7735_SetCursor(x,y);
  ST7735_OutString("Delete  >  Done");
  Switch_AssignTask(0,ActiveAlarms_Left);
  Switch_AssignTask(1,ActiveAlarms_Middle);
  Switch_AssignTask(2,ActiveAlarms_Right);
  numAlarms = Alarm_Number();
  cursor = 0;
  SwitchFlag[Right] = 1;
  while(numAlarms&&SwitchFlag[Right]){
    if(cursor==numAlarms){
      cursor = 0;
    }
    x = 8; y = 7;
    ST7735_SetCursor(x,y);
    Alarm_GetString(alarm_string,cursor);
      }
  x = 4; y = 7;
  ST7735_SetCursor(x,y);
  ST7735_OutString("No alarms set");
  while(SwitchFlag[Right]){
  };
}

void static ActiveAlarms_Left(void){uint8_t hour, minute;
  //Alarm_Disable(cursor);
  cursor++;
}

void static ActiveAlarms_Middle(void){
  cursor++;
}

void static ActiveAlarms_Right(void){
  SwitchFlag[Right] = 0;
}

void static InitiateAlarm(void){
  cursor = 2; // return to MainClock after
  Switch_AssignTask(0,MainClock_Middle);
  Switch_AssignTask(1,MainClock_Middle);
  Switch_AssignTask(2,MainClock_Middle);
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_SetCursor(7,7);
  ST7735_OutString("Alarm!");
  SwitchFlag[Middle] = 1;
  Sound_FlagEnabledSound(&SwitchFlag[Middle]);
}

void static CheckAlarmsAndTimer(void){
  if(Alarm_Check()||AlarmClockTimer_Check()){
    InitiateAlarm();
  }
}