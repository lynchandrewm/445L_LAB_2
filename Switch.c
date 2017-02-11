// Switch.c
// Runs on TMC4C123
// Use GPIO in edge time mode to request interrupts on any
// edge of PF4 and start Timer0B. In Timer0B one-shot
// interrupts, record the state of the switch once it has stopped
// bouncing.
// Daniel Valvano
// May 3, 2015

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

// PF4 connected to a negative logic switch using internal pull-up (trigger on both edges)
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Switch.h"
#define PB3                     (*((volatile uint32_t *)0x40005020))
#define PC6                     (*((volatile uint32_t *)0x40006100))
#define PF4                     (*((volatile uint32_t *)0x40025040))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile static unsigned long PreviousPB3;
volatile static unsigned long PreviousPC6;
volatile static unsigned long PreviousPF4;
void (*TouchTaskPB3)(void);
void (*TouchTaskPC6)(void);
void (*TouchTaskPF4)(void);
static void Timer0Arm(void){long sr;
  sr = StartCritical();
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
  TIMER0_TAILR_R = 1600000;      // 4) 10ms reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
  EndCritical(sr);
}
static void GPIOArm(void){
  // clear flags
  GPIO_PORTB_ICR_R = 0x8;      // (e) clear flag3
  GPIO_PORTC_ICR_R = 0x40;      // (e) clear flag6
  GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
  // arm interrupts
  GPIO_PORTB_IM_R |= 0x8;      // (f) arm interrupt on PB3 *** No IME bit as mentioned in Book ***
  GPIO_PORTC_IM_R |= 0x40;      // (f) arm interrupt on PC6 *** No IME bit as mentioned in Book ***
  GPIO_PORTF_IM_R |= 0x10;      // (f) arm interrupt on PF4 *** No IME bit as mentioned in Book ***

  // NVIC PB 1, PC 2, PF 3
  NVIC_PRI0_R = (NVIC_PRI0_R&0x000000FF)|0xA0A0A000; // (g) priority 5
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x4000000E;      // (h) enable interrupt 1,2,3 in NVIC  
}
// Initialize switch interface on PF4 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Switch_Init(void){uint8_t i;
  // **** general initialization ****
  SYSCTL_RCGCGPIO_R |= 0x0000002E; // (a) activate clock for port F
  while((SYSCTL_PRGPIO_R & 0x0000002E) == 0){
  }
  
  GPIO_PORTB_DIR_R &= ~0x8;    // (c) make PB3 in (built-in button)
  GPIO_PORTB_AFSEL_R &= ~0x8;  //     disable alt funct on PB3
  GPIO_PORTB_DEN_R |= 0x8;     //     enable digital I/O on PB3   
  GPIO_PORTB_PCTL_R &= ~0x0000F000; // configure PB3 as GPIO
  GPIO_PORTB_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTB_PUR_R |= 0x8;     //     enable weak pull-up on PB3
  GPIO_PORTB_IS_R &= ~0x8;     // (d) PB3 is edge-sensitive
  GPIO_PORTB_IBE_R |= 0x8;     //     PB3 is both edges

  GPIO_PORTC_DIR_R &= ~0x40;    // (c) make PC6 in (built-in button)
  GPIO_PORTC_AFSEL_R &= ~0x40;  //     disable alt funct on PC6
  GPIO_PORTC_DEN_R |= 0x40;     //     enable digital I/O on PC6   
  GPIO_PORTC_PCTL_R &= ~0x0F000000; // configure PC6 as GPIO
  GPIO_PORTC_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTC_PUR_R |= 0x40;     //     enable weak pull-up on PC6
  GPIO_PORTC_IS_R &= ~0x40;     // (d) PC6 is edge-sensitive
  GPIO_PORTC_IBE_R |= 0x40;     //     PC6 is both edges

  GPIO_PORTF_DIR_R &= ~0x10;    // (c) make PF4 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x10;  //     disable alt funct on PF4
  GPIO_PORTF_DEN_R |= 0x10;     //     enable digital I/O on PF4   
  GPIO_PORTF_PCTL_R &= ~0x000F0000; // configure PF4 as GPIO
  GPIO_PORTF_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x10;     //     enable weak pull-up on PF4
  GPIO_PORTF_IS_R &= ~0x10;     // (d) PF4 is edge-sensitive
  GPIO_PORTF_IBE_R |= 0x10;     //     PF4 is both edges
    
  GPIOArm();

  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  
  PreviousPB3 = PB3;
  PreviousPC6 = PC6;
  PreviousPF4 = PF4;
}


void Switch_AssignTask(uint8_t switchNumber, void(*touchtask)(void)){
  switch(switchNumber){
    case 0:
      TouchTaskPB3 = touchtask;
      break;
    case 1:
      TouchTaskPC6 = touchtask;
      break;
    case 2:
      TouchTaskPF4 = touchtask;
      break;
  }
}
 
void static CommonHandler(void){
  // clear flags
  GPIO_PORTB_ICR_R = 0x8;      // (e) clear flag3
  GPIO_PORTC_ICR_R = 0x40;      // (e) clear flag6
  GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
  // disarm interrupts
  GPIO_PORTB_IM_R &= ~0x8;      // (f) arm interrupt on PB3 *** No IME bit as mentioned in Book ***
  GPIO_PORTC_IM_R &= ~0x40;      // (f) arm interrupt on PC6 *** No IME bit as mentioned in Book ***
  GPIO_PORTF_IM_R &= ~0x10;      // (f) arm interrupt on PF4 *** No IME bit as mentioned in Book ***
}
  
//interrupts on edge triggered switch 
void GPIOPortB_Handler(void){
  CommonHandler();
  if(PreviousPB3){
    (*TouchTaskPB3)();
  }
  Timer0Arm();
}

//interrupts on edge triggered switch 
void GPIOPortC_Handler(void){
  CommonHandler();
  if(PreviousPC6){
    (*TouchTaskPC6)();
  }
  Timer0Arm();
}

//interrupts on edge triggered switch 
void GPIOPortF_Handler(void){
  CommonHandler();
  if(PreviousPF4){
    (*TouchTaskPF4)();
  }
  Timer0Arm();
}

// Interrupt 10 ms after rising edge of PF4
void Timer0A_Handler(void){uint8_t i;
  TIMER0_IMR_R = 0x00000000;    // disarm timeout interrupt  
  PreviousPB3 = PB3;
  PreviousPF4 = PF4;
  PreviousPC6 = PC6;
  GPIOArm();   // start GPIO
}