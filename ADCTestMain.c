// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdint.h>
#include "ADCSWTrigger.h"
#include "Fixed.h"
#include "ST7735.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"

extern void Timer1_Init(void);

#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


uint16_t dumpADC[1000];
uint32_t dumpTime[1000];
uint16_t Index = 0;
uint16_t adcHist[4096];


volatile uint32_t ADCvalue;
// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.
void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
//  PF2 ^= 0x04;                   // profile
//  PF2 ^= 0x04;                   // profile
  ADCvalue = ADC0_InSeq3();
//  PF2 ^= 0x04;                   // profile
  //debug
  if(Index>=1000){ return; }
  dumpADC[Index] = ADCvalue;
  dumpTime[Index] = 0xFFFFFFFF - TIMER1_TAR_R;
  Index += 1;
}

void printHist(uint16_t* hist, uint32_t index, char* title){int32_t minX, maxX, minY, maxY;
  int32_t xValues[index];
  minX = 0; maxX = index; minY = 0; maxY = 0;
  for(int i = 0; i < index; i++){
    if(hist[i]){
      minX = i;
      break;
    }
  }
  for(int i = index - 1; i < 0 ; i--){
    if(hist[i]){
      maxX = i;
      break;
    }
  }
  for(int i = 0; i < index; i++){
    if(hist[i]>maxY){
      maxY = hist[i];
    }
    if(hist[i]<minY){
      minY = hist[i];
    }
    xValues[i] = i;
  }
  ST7735_XYplotInit(title, minX, maxX, minY, maxY);
  ST7735_XYplot(index, xValues, (int32_t*)hist);
}

//calculate timeJitter 
uint32_t calculateJitter(){ uint16_t timeDiffs[999];
  for(int i = 0; i < 999; i++){
    timeDiffs[i] = dumpTime[i+1] - dumpTime[i];
  }
	uint32_t largestTDiff, smallestTDiff;
	largestTDiff = smallestTDiff = timeDiffs[0];
	//find smallest and largest time diffs
	for(int i = 1; i < 999; i++){
		if(timeDiffs[i] < smallestTDiff){
			smallestTDiff = timeDiffs[i];
		}
		if(timeDiffs[i] > largestTDiff){
			largestTDiff = timeDiffs[i];
		}
	}
	return largestTDiff - smallestTDiff;
}

void adcPMF(void){
  for(int i = 0; i < 1000; i++){
    adcHist[dumpADC[i]]++;
  }
  char* adcTitle = "ADC PMF";
  printHist(adcHist, 4096, adcTitle);
}

// main to process the time recordings
int main(void){
  DisableInterrupts();
  PLL_Init(Bus80MHz);                   // 80 MHz
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
  Timer1_Init();
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
  ST7735_InitR(INITR_REDTAB);
  EnableInterrupts();
  while(Index<1000){
    WaitForInterrupt();
  }
  
  uint32_t timeJitter = calculateJitter();
  adcPMF();
  while(1){}
}

int main1(void){
  PLL_Init(Bus80MHz);                   // 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      // turn off LED
  EnableInterrupts();
  while(1){
    PF1 ^= 0x02;  // toggles when running in main
  }
}


