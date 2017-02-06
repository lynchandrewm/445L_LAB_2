// Lab1.c
// Runs on LM4F120/TM4C123
// Contains testing for different modules in Lab1
// UART, ADC, interpreter, display
// Matthew DeKoning 
// Andrew Lynch
// January 31, 2017
// January 31, 2017

#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"
#include "ADCT2ATrigger.h"
#include "interpreter.h"
#include "UART.h"
#include "Timer4A.h"
#include "Clock.h"
#include "PLL.h"
#include "OS.h"
#include "../inc/tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void OutCRLF(void);

void timer4TestFunction(){
  ST7735_ds_OutString(0,"TEST");
}

void timer4TestFunction2(){
  ST7735_ds_OutString(1,"TEST");
}

//final main
int main(void){
  PLL_Init(Bus50MHz);       // set system clock to 50 MHz
  UART_Init();              // initialize UART
  INTERPRETER_initArray();
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  ADC0_InitTimer2ATriggerSeq3(0, 8000000); // ADC channel 0, 10 Hz sampling
  Timer4A_Init(2000,7); //init the timer
  Timer4A_AddPeriodicThread(OS_Scheduler);
  Clock_Init();
  EnableInterrupts();
  char string[100];
  while(1){
    UART_OutString(">");
    UART_InString(string, 99);
    INTERPRETER_parseMessage(string);
    OutCRLF();
    if(interpreter_device == -1){
      UART_OutString(interpreter_msg);
    }
    else{
      //UART_OutString(interpreter_msg);
      ST7735_ds_SetCursor(interpreter_device, 0, interpreter_line);
      ST7735_ds_OutString(interpreter_device, "                    ");
      ST7735_ds_SetCursor(interpreter_device, 0, interpreter_line);
      ST7735_ds_OutString(interpreter_device, interpreter_msg);
    }
    OutCRLF();
  }
}

//test clock
int main9(void){
  PLL_Init(Bus50MHz);       // set system clock to 50 MHz
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  DisableInterrupts();
  Clock_Init();
  Clock_SetClock(2017,2,1,0,4,0);
  char time_str[20];
  char date_str[30];
  EnableInterrupts();
  while(1){
    Clock_GetTime(time_str);
    Clock_GetDate(date_str);
    ST7735_ds_SetCursor(0,0,0);
    ST7735_ds_OutString(0,time_str);
    ST7735_ds_SetCursor(0,0,1);
    ST7735_ds_OutString(0,date_str);
  }
}

//test timer4
int main8(void){
  PLL_Init(Bus50MHz);       // set system clock to 50 MHz
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  Timer4A_Init(2000,4);
  Timer4A_AddPeriodicThread(timer4TestFunction);
  Timer4A_AddPeriodicThread(timer4TestFunction2);
  EnableInterrupts();
  while(1){
    WaitForInterrupt();
  }
}

//interupt driven UART
//UART and Interpreter
int main7(void){char string[100];
  PLL_Init(Bus50MHz);       // set system clock to 50 MHz
  UART_Init();              // initialize UART
  INTERPRETER_initArray();
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  ADC0_InitTimer2ATriggerSeq3(0, 8000000); // ADC channel 0, 10 Hz sampling
  Timer4A_Init(2000,4); //init the timer
  EnableInterrupts();
  while(1){
    WaitForInterrupt();
  }
}


//UART and Interpreter
int main01(void){char string[100];
  PLL_Init(Bus50MHz);       // set system clock to 50 MHz
  UART_Init();              // initialize UART
  INTERPRETER_initArray();
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  ADC0_InitTimer2ATriggerSeq3(0, 8000000); // ADC channel 0, 10 Hz sampling
  Timer4A_Init(2000,4); //init the timer
  EnableInterrupts();
  while(1){
    UART_OutString(">");
    UART_InString(string, 99);
    INTERPRETER_parseMessage(string);
    OutCRLF();
    if(interpreter_device == -1){
      UART_OutString(interpreter_msg);
    }
    else{
      //UART_OutString(interpreter_msg);
      ST7735_ds_SetCursor(interpreter_device, 0, interpreter_line);
      ST7735_ds_OutString(interpreter_device, "                    ");
      ST7735_ds_SetCursor(interpreter_device, 0, interpreter_line);
      ST7735_ds_OutString(interpreter_device, interpreter_msg);
    }
    OutCRLF();
  }
}

//UART testing
int main4(void){
  char i;
  char string[20];  // global to assist in debugging
  uint32_t n;

  PLL_Init(Bus50MHz);       // set system clock to 50 MHz
  UART_Init();              // initialize UART
  OutCRLF();
  for(i='A'; i<='Z'; i=i+1){// print the uppercase alphabet
    UART_OutChar(i);
  }
  OutCRLF();
  UART_OutChar(' ');
  for(i='a'; i<='z'; i=i+1){// print the lowercase alphabet
    UART_OutChar(i);
  }
  OutCRLF();
  UART_OutChar('-');
  UART_OutChar('-');
  UART_OutChar('>');
  while(1){
    UART_OutString("InString: ");
    UART_InString(string,19);
    UART_OutString(" OutString="); UART_OutString(string); OutCRLF();

    UART_OutString("InUDec: ");  n=UART_InUDec();
    UART_OutString(" OutUDec="); UART_OutUDec(n); OutCRLF();

    UART_OutString("InUHex: ");  n=UART_InUHex();
    UART_OutString(" OutUHex="); UART_OutUHex(n); OutCRLF();

  }
}
//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}


//adc test (with display)
int main2(void){
  PLL_Init(Bus80MHz);                      // 80 MHz system clock
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  SYSCTL_RCGCGPIO_R |= 0x00000020;         // activate port F
  ADC0_InitTimer2ATriggerSeq3(0, 8000000); // ADC channel 0, 10 Hz sampling
  GPIO_PORTF_DIR_R |= 0x04;                // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;             // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;                // enable digital I/O on PF2
                                           // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;                  // disable analog functionality on PF
  GPIO_PORTF_DATA_R &= ~0x04;              // turn off LED
  EnableInterrupts();
  char* adcMessage = "Voltage:";
  int32_t volts;
  while(1){
    WaitForInterrupt();
    GPIO_PORTF_DATA_R ^= 0x04;             // toggle LED
    volts = ADC_GetVoltage();
    ST7735_ds_Message(0, 0, adcMessage, volts);
    ST7735_ds_OutString(0,"mV");
  }
}


//display driver test
int main1(void){
  PLL_Init(4);
  uint8_t lines = 8;
  ST7735_ds_InitR(INITR_REDTAB, 4, 4, 4, 4);
  ST7735_ds_SetTextColor(2, ST7735_RED);
  ST7735_ds_SetTextColor(1, ST7735_GREEN);
  ST7735_ds_SetTextColor(3, ST7735_BLUE);
  
  ST7735_ds_OutString(1, "Hello World\n");
  ST7735_ds_SetCursor(1, 0, 0);
  ST7735_ds_OutString(1, "Test2");
  
  ST7735_ds_OutString(0, "Test1");
  ST7735_ds_OutString(2, "Test3");
  ST7735_ds_OutString(3, "Test4");
  
}