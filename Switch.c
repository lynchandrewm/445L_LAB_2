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
#include "../driverlib/gpio.h"
#include "../driverlib/pin_map.h"
#include "../driverlib/sysctl.h"
#include "../driverlib/interrupt.h"
#define PB3                     (*((volatile uint32_t *)0x40005020))
#define PC6                     (*((volatile uint32_t *)0x40006100))
#define PF4                     (*((volatile uint32_t *)0x40025040))

/*
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
*/

volatile static unsigned long PreviousPB3;
volatile static unsigned long PreviousPC6;
volatile static unsigned long PreviousPF4;
void (*TouchTaskPB3)(void);
void (*TouchTaskPC6)(void);
void (*TouchTaskPF4)(void);
uint32_t static Bus1ms;

void static PortB_handler(void);
void static PortC_handler(void);
void static PortF_handler(void);


static void Timer0Arm(void){long sr;
//  sr = StartCritical();
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
  TIMER0_TAILR_R = Bus1ms*10;      // 4) 100ms reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
//  EndCritical(sr);
}
static void GPIOArm(void){
  GPIOIntClear(0x40005000,GPIO_PIN_3);
  GPIOIntClear(0x40006000,GPIO_PIN_6);
  GPIOIntClear(0x40025000,GPIO_PIN_4);
  GPIOIntEnable(0x40005000,GPIO_PIN_3);
  GPIOIntEnable(0x40006000,GPIO_PIN_6);
  GPIOIntEnable(0x40025000,GPIO_PIN_4);
  PreviousPB3 = GPIOPinRead(0x40005000,GPIO_PIN_3);
  PreviousPC6 = GPIOPinRead(0x40006000,GPIO_PIN_6);
  PreviousPF4 = GPIOPinRead(0x40025000,GPIO_PIN_4);
}
// Initialize switch interface on PF4 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Switch_Init(uint32_t freq){Bus1ms = freq;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
  GPIOPinTypeGPIOInput(0x40005000,GPIO_PIN_3);
  GPIOPinTypeGPIOInput(0x40006000,GPIO_PIN_6);
  GPIOPinTypeGPIOInput(0x40025000,GPIO_PIN_4);
  GPIOPadConfigSet(0x40005000, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  
  GPIOPadConfigSet(0x40006000, GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  
  GPIOPadConfigSet(0x40025000, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  
  GPIOIntRegister(0x40005000,PortB_handler);
  GPIOIntRegister(0x40006000,PortC_handler);
  GPIOIntRegister(0x40025000,PortF_handler);
  GPIOIntTypeSet(0x40005000,GPIO_PIN_3,GPIO_BOTH_EDGES);
  GPIOIntTypeSet(0x40006000,GPIO_PIN_6,GPIO_BOTH_EDGES);
  GPIOIntTypeSet(0x40025000,GPIO_PIN_4,GPIO_BOTH_EDGES);
  IntPrioritySet(INT_GPIOB,0xE0);
  IntPrioritySet(INT_GPIOC,0xE0);
  IntPrioritySet(INT_GPIOF,0xE0);

    
  GPIOArm();

  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
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
  GPIOIntDisable(0x40005000,GPIO_PIN_3);
  GPIOIntDisable(0x40006000,GPIO_PIN_6);
  GPIOIntDisable(0x40025000,GPIO_PIN_4);
}
  
//interrupts on edge triggered switch 
void static PortB_handler(void){
  GPIOIntClear(0x40005000,GPIO_PIN_3);
  CommonHandler();
  if(PreviousPB3){
    (*TouchTaskPB3)();
  }
  Timer0Arm();
}

//interrupts on edge triggered switch 
void static PortC_handler(void){
  GPIOIntClear(0x40006000,GPIO_PIN_6);
  CommonHandler();
  if(PreviousPC6){
    (*TouchTaskPC6)();
  }
  Timer0Arm();
}

//interrupts on edge triggered switch 
void static PortF_handler(void){
  GPIOIntClear(0x40025000,GPIO_PIN_4);
  CommonHandler();
  if(PreviousPF4){
    (*TouchTaskPF4)();
  }
  Timer0Arm();
}

// Interrupt 10 ms after rising edge of PF4
void Timer0A_Handler(void){uint8_t i;
  TIMER0_IMR_R = 0x00000000;    // disarm timeout interrupt  
  GPIOArm();   // start GPIO
}