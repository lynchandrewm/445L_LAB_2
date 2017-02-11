// Clock.h
// Runs on TM4C123
// Use Timer4B in periodic mode to create clock functions
// Andrew Lynch
// January 31, 2017

#ifndef _CLOCK
#define _CLOCK

void Clock_GetTime(char string[12]);

void Clock_GetDate(char string[19]);

void Clock_Init(uint32_t freq);

void Clock_SetClock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

void Clock_SetYear(uint16_t year);

void Clock_SetMonth(uint8_t month);

void Clock_SetDay(uint8_t day);

void Clock_SetHour(uint8_t hour);

void Clock_SetMinute(uint8_t minute);

void Clock_SetSecond(uint8_t second);

void Clock_BuildTimeString(uint8_t hour, uint8_t minute, char string[12]);

void Clock_ParseStringToTime(char* string, uint8_t* hour_ptr, uint8_t* minute_ptr);

// To get the numeric form of a time or date data, first get a Time register
// or a Date register. Then using this register, use the functions that follow
// to extract the time and date data desired
uint32_t Clock_GetTimeReg(void);

uint32_t Clock_GetDateReg(void);

uint16_t Clock_ExtractMillisecond(uint32_t localTime);

uint8_t  Clock_ExtractSecond(uint32_t localTime);

uint8_t  Clock_ExtractMinute(uint32_t localTime);

uint8_t  Clock_ExtractHour(uint32_t localTime);

uint8_t  Clock_ExtractDay(uint32_t localDate);

uint8_t  Clock_ExtractDaysInCurrentMonth(uint32_t localDate);

uint8_t  Clock_ExtractMonth(uint32_t localDate);

uint16_t  Clock_ExtractYear(uint32_t localDate);

uint8_t  Clock_ExtractLeapYearEnable(uint32_t localDate);


#endif


