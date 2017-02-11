// Lab3.c
// TM4c123
// Runs alarm clock, and tests components
// Andrew Lynch
// Stathi Pakes
// February 8, 2017

#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "AlarmClock.h"
#include "DisplayClock.h"
#include "Alarm.h"
#include "Clock.h"
#include "ST7735.h"
#include "../driverlib/gpio.h"
#include "Switch.h"
#include "SysTick.h"

#define PB3                     (*((volatile uint32_t *)0x40005020))
#define PC6                     (*((volatile uint32_t *)0x40006100))
#define PF4                     (*((volatile uint32_t *)0x40025040))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint32_t CountLeft, CountMiddle, CountRight;

// prototypes of private functions

// ----------Test Main----------
int main1(void){
  AlarmClock_Init(PLL_Init(Bus80MHz));
  while(1){
    AlarmClock_Start();
  }
}


int main(void){
  Clock_Init(PLL_Init(Bus80MHz));
  ST7735_InitR(INITR_REDTAB);
  char timeString[20];
  uint32_t localTime, previousTime;
  localTime = Clock_GetTimeReg();
  previousTime = localTime;
  Clock_GetTime(timeString);
  DisplayClock_PrintFullClockFace("Test",timeString,"Test","Test",Clock_ExtractSecond(localTime),Clock_ExtractMinute(localTime),5*Clock_ExtractHour(localTime));
  while(1){
    if(Clock_ExtractSecond(localTime)!=Clock_ExtractSecond(previousTime)){
      Clock_GetTime(timeString);
      DisplayClock_UpdateClock("Test",timeString,"Test","Test",Clock_ExtractSecond(localTime),Clock_ExtractMinute(localTime),5*Clock_ExtractHour(localTime));
    }
    previousTime = localTime;
    localTime = Clock_GetTimeReg();
  }
}

void switchTestTaskLeft(void){
  CountLeft++;
}

void switchTestTaskMiddle(void){
  CountMiddle++;
}

void switchTestTaskRight(void){
  CountRight++;
}

// ----------Test Switch----------
int main324(void){
  PLL_Init(Bus80MHz);
  ST7735_InitR(INITR_REDTAB);
  GPIOIntEnable(0x40005000,GPIO_INT_PIN_3);
  GPIOIntRegister(0x40005000,switchTestTaskMiddle);
  GPIOIntEnable(0x40006000,GPIO_INT_PIN_6);
  GPIOIntRegister(0x40006000,switchTestTaskMiddle);
  GPIOIntEnable(0x40025000,GPIO_INT_PIN_4);
  GPIOIntRegister(0x40025000,switchTestTaskMiddle);
  EnableInterrupts();
  CountLeft = 0; CountMiddle = 0; CountRight = 0;
  while(1){
    ST7735_SetCursor(0,0);
    ST7735_OutString("Left: ");
    ST7735_OutUDec(CountLeft);
    ST7735_SetCursor(0,1);
    ST7735_OutString("Middle: ");
    ST7735_OutUDec(CountMiddle);
    ST7735_SetCursor(0,2);
    ST7735_OutString("Right: ");
    ST7735_OutUDec(CountRight);
  }
}