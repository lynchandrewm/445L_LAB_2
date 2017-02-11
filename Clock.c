// Clock.c
// Runs on TM4C123
// Use Timer4B in periodic mode to create clock functions
// Andrew Lynch
// January 31, 2017

#include <stdint.h>
#include <stdio.h>
#include "Timer5A.h"
#include "ST7735.h"
#include "Clock.h"
#include "../inc/tm4c123gh6pm.h"

void UpdateClock(void);

/*
uint16_t static Year = 0;
uint8_t static LeapYear = 0;
uint8_t static Month = 0;
uint8_t static DaysInMonth = 0;
uint8_t static Day = 0;
uint8_t static Hour = 0;
uint8_t static Minute = 0;
uint8_t static Second = 0;
uint16_t static Millisecond = 0;
*/

#define MILLISECOND    0x000003FF
#define SECOND         0x0001F800
#define MINUTE         0x01F80000
#define HOUR           0xF8000000
#define DAY            0x0000001F
#define DAYS_PER_MONTH 0x000007C0
#define MONTH          0x0000F000
#define YEAR           0x3FFE0000
#define LEAPYEAR_EN    0x80000000

//milliseconds            bits 0-9
//seconds                 bits 11-16
//minutes                 bits 19-24
//hours                   bits 27-31
uint32_t static Time = 0x00000000;

//day                     bits 0-4
//days in current month   bits 6-10
//month                   bits 12-15
//year                    bits 17-29
//leap year enable        bit  31
uint32_t static Date = 0x00000000;

void static FullChangeClock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond);


void Clock_Init(uint32_t freq){
  Timer5A_Init(freq); //1ms 
}

uint32_t Clock_GetTimeReg(void){
  return Time;
}

uint32_t Clock_GetDateReg(void){
  return Date;
}

uint16_t Clock_ExtractMillisecond(uint32_t localTime){
  return (localTime&MILLISECOND);
}

uint8_t  Clock_ExtractSecond(uint32_t localTime){
  return (localTime&SECOND)>>11;
}

uint8_t  Clock_ExtractMinute(uint32_t localTime){
  return (localTime&MINUTE)>>19;
}

uint8_t  Clock_ExtractHour(uint32_t localTime){
  return (localTime&HOUR)>>27;
}

uint8_t  Clock_ExtractDay(uint32_t localDate){
  return (localDate&DAY);
}

uint8_t  Clock_ExtractDaysInCurrentMonth(uint32_t localDate){
  return (localDate&DAYS_PER_MONTH)>>6;
}

uint8_t  Clock_ExtractMonth(uint32_t localDate){
  return (localDate&MONTH)>>12;
}

uint16_t  Clock_ExtractYear(uint32_t localDate){
  return (localDate&YEAR)>>17;
}

uint8_t  Clock_ExtractLeapYearEnable(uint32_t localDate){
  return (localDate&LEAPYEAR_EN)>>31;
}



uint32_t static EncodeMillisecond(uint16_t num, uint32_t localTime){ num &= 0x3FF;
  return (localTime&(~MILLISECOND))|(num);
}

uint32_t static EncodeSecond(uint8_t num, uint32_t localTime){ num &= 0x3F;
  return (localTime&(~SECOND))|(num<<11);
}

uint32_t static EncodeMinute(uint8_t num, uint32_t localTime){ num &= 0x3F;
  return (localTime&(~MINUTE))|(num<<19);
}

uint32_t static EncodeHour(uint8_t num, uint32_t localTime){ num &= 0x1F;
  return (localTime&(~HOUR))|(num<<27);
}

uint32_t static EncodeDay(uint8_t num, uint32_t localDate){ num &= 0x1F;
  return (localDate&(~DAY))|(num);
}

uint32_t static EncodeDaysInCurrentMonth(uint8_t num, uint32_t localDate){ num &= 0x1F;
  return (localDate&(~DAYS_PER_MONTH))|(num<<6);
}

uint32_t static EncodeMonth(uint8_t num, uint32_t localDate){ num &= 0xF;
  return (localDate&(~MONTH))|(num<<12);
}

uint32_t static EncodeYear(uint16_t num, uint32_t localDate){ num &= 0x1FFF;
  return (localDate&(~YEAR))|(num<<17);
}

uint32_t static EncodeLeapYearEnable(uint8_t num, uint32_t localDate){ num &= 0x1;
  return (localDate&(~LEAPYEAR_EN))|(num<<31);
}

void Clock_GetTime(char string[12]){uint32_t localTime = Time;
  uint8_t second, minute, hour;
  second = Clock_ExtractSecond(localTime);
  minute = Clock_ExtractMinute(localTime);
  hour   = Clock_ExtractHour(localTime);
  char* AMPMstr = "AM";
  char* leadingZeroHour = "";
  char* leadingZeroMinute = "";
  char* leadingZeroSecond = "";
  if(hour>11){ 
    AMPMstr = "PM";
    hour -= 12;
  }
  if(hour == 0){ hour = 12; }
  if(hour<10){ leadingZeroHour = "0"; }
  if(minute<10){ leadingZeroMinute = "0"; }
  if(second<10){ leadingZeroSecond = "0"; }
  sprintf(string, "%s%d:%s%d:%s%d %s", leadingZeroHour, hour, leadingZeroMinute, minute, leadingZeroSecond, second, AMPMstr);
}

void Clock_BuildTimeString(uint8_t hour, uint8_t minute, char string[12]){
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
}

void Clock_ParseStringToTime(char* string, uint8_t* hour_ptr, uint8_t* minute_ptr){uint8_t hour, minute;
  //parse here :)
  
}

void NameOfMonth(char* string, uint8_t month){
  switch(month){
    case 1: 
      sprintf(string, "January");
      break;
    case 2:
      sprintf(string, "February");
      break;
    case 3: 
      sprintf(string, "March");
      break;
    case 4: 
      sprintf(string, "April");
      break;
    case 5: 
      sprintf(string, "May");
      break;
    case 6: 
      sprintf(string, "June");
      break;
    case 7: 
      sprintf(string, "July");
      break;
    case 8: 
      sprintf(string, "August");
      break;
    case 9: 
      sprintf(string, "September");
      break;
    case 10: 
      sprintf(string, "October");
      break;
    case 11: 
      sprintf(string, "November");
      break;
    case 12: 
      sprintf(string, "December");
      break;
  }
}

void Clock_GetDate(char string[19]){uint32_t localDate = Date;
  uint8_t day, month; uint16_t year;
  day   = Clock_ExtractDay(localDate);
  month = Clock_ExtractMonth(localDate);
  year  = Clock_ExtractYear(localDate);
  char monthName[10];
  NameOfMonth(monthName, month);
  sprintf(string, "%s %d, %d", monthName, day, year); 
}  

void Timer5A_Handler(void){
  TIMER5_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER4B timeout
  UpdateClock();
}

uint8_t UpdateDaysInMonth(uint8_t month, uint8_t leapyearEn){
  if(leapyearEn&&(month==2)){
    return 29;
  }
  return 28 + (month + month/8) % 2 + 2 % month + 2 * 1/month;
}

uint8_t UpdateLeapYear(uint16_t year){
  if(year%4){ //if not divisible by 4, leapyear
    return 0;
  }
  if(year%100){ //else if not divisible by 100, not leapyear 
    return 1;
  }
  if(year%400){ //else if not divisible by 400, leapyear
    return 0;
  }
  return 1; //if divisible by 400, leapyear
}

void UpdateClock(void){ uint32_t localTime, localDate;
  uint16_t millisecond, year; 
  uint8_t second, minute, hour, day, daysInCurrentMonth, month;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  //debug
  if(millisecond>510){
    //important
  }
  //debug
  second             = Clock_ExtractSecond(localTime);
  minute             = Clock_ExtractMinute(localTime);
  hour               = Clock_ExtractHour(localTime);
  day                = Clock_ExtractDay(localDate);
  daysInCurrentMonth = Clock_ExtractDaysInCurrentMonth(localDate);
  month              = Clock_ExtractMonth(localDate);
  year               = Clock_ExtractYear(localDate);
  millisecond++;
  if(millisecond == 1000){
    second++; 
    millisecond = 0;
  }
  if(second == 60){
    minute++;
    second = 0;
  }
  if(minute == 60){
    hour++;
    minute = 0;
  }
  if(hour == 24){
    day++;
    hour = 0;
  }
  if(day > daysInCurrentMonth){
    month++;
    day = 1;
  }
  if(month > 12){
    year++;
    month = 1;
  }
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

void Clock_SetClock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second){
  FullChangeClock(year, month, day, hour, minute, second, 0);
}

void static FullChangeClock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond){
  uint32_t localTime, localDate = 0x0;
  uint8_t daysInCurrentMonth, leapyearEn;
  leapyearEn = UpdateLeapYear(year);
  daysInCurrentMonth = UpdateDaysInMonth(month, leapyearEn);
  
  localTime = EncodeMillisecond(millisecond, localTime);
  localTime = EncodeSecond(second, localTime);
  localTime = EncodeMinute(minute, localTime);
  localTime = EncodeHour(hour, localTime);
  localDate = EncodeDay(day, localDate);
  localDate = EncodeDaysInCurrentMonth(daysInCurrentMonth, localDate);
  localDate = EncodeMonth(month, localDate);
  localDate = EncodeYear(year, localDate);
  localDate = EncodeLeapYearEnable(leapyearEn, localDate);
  Time = localTime; Date = localDate; //atomic, not critical
}

void Clock_SetYear(uint16_t year){uint32_t localTime, localDate;
  uint16_t millisecond; 
  uint8_t second, minute, hour, day, month;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  second             = Clock_ExtractSecond(localTime);
  minute             = Clock_ExtractMinute(localTime);
  hour               = Clock_ExtractHour(localTime);
  day                = Clock_ExtractDay(localDate);
  month              = Clock_ExtractMonth(localDate);
  
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

void Clock_SetMonth(uint8_t month){uint32_t localTime, localDate;
  uint16_t millisecond, year; 
  uint8_t second, minute, hour, day;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  second             = Clock_ExtractSecond(localTime);
  minute             = Clock_ExtractMinute(localTime);
  hour               = Clock_ExtractHour(localTime);
  day                = Clock_ExtractDay(localDate);
  year               = Clock_ExtractYear(localDate);
  
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

void Clock_SetDay(uint8_t day){uint32_t localTime, localDate;
  uint16_t millisecond, year; 
  uint8_t second, minute, hour, month;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  second             = Clock_ExtractSecond(localTime);
  minute             = Clock_ExtractMinute(localTime);
  hour               = Clock_ExtractHour(localTime);
  month              = Clock_ExtractMonth(localDate);
  year               = Clock_ExtractYear(localDate);
  
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

void Clock_SetHour(uint8_t hour){uint32_t localTime, localDate;
  uint16_t millisecond, year; 
  uint8_t second, minute, day, month;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  second             = Clock_ExtractSecond(localTime);
  minute             = Clock_ExtractMinute(localTime);
  day                = Clock_ExtractDay(localDate);
  month              = Clock_ExtractMonth(localDate);
  year               = Clock_ExtractYear(localDate);
  
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

void Clock_SetMinute(uint8_t minute){uint32_t localTime, localDate;
  uint16_t millisecond, year; 
  uint8_t second, hour, day, month;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  second             = Clock_ExtractSecond(localTime);
  hour               = Clock_ExtractHour(localTime);
  day                = Clock_ExtractDay(localDate);
  month              = Clock_ExtractMonth(localDate);
  year               = Clock_ExtractYear(localDate);
  
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

void Clock_SetSecond(uint8_t second){uint32_t localTime, localDate;
  uint16_t millisecond, year; 
  uint8_t minute, hour, day, month;
  localTime          = Time; localDate = Date;
  millisecond        = Clock_ExtractMillisecond(localTime);
  minute             = Clock_ExtractMinute(localTime);
  hour               = Clock_ExtractHour(localTime);
  day                = Clock_ExtractDay(localDate);
  month              = Clock_ExtractMonth(localDate);
  year               = Clock_ExtractYear(localDate);
  
  FullChangeClock(year, month, day, hour, minute, second, millisecond);
}

