// Lab2.c
// Debugs timing and PMF for ADC
// Runs on TM4C123
// Andrew Lynch
// Stathi Pakes
// February 4, 2017

#include <stdint.h>
#include "ST7735.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ADCT2ATrigger.h"
#include "Timer5A.h"
#include "UART.h"
#include "Switch.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

#define PF2   (*((volatile uint32_t *)0x40025010))
#define PF4   (*((volatile uint32_t *)0x40025040))
#define TARGET_TIMING   8000 // (1/10kHz)/(1/80Mhz)
#define ERROR_ALLOWANCE 8   // 0.1% error allowed

typedef enum uint8_t { 
  timing,
  value
} dump_value;

typedef enum int8_t {
  line,
  hist,
  jitterAndUart
} threadName;

uint32_t Dump[1000];
uint16_t Index;
dump_value DebugFlag;
uint32_t TimeDiffs[999];
uint32_t Histogram[4096];

threadName CurrentThread = 0;
int main_line(void);
int main_hist(void);
int main_jitterAndUart(void);
void static TitleFrame(void);
void static DebugTiming(void);
void static DebugValue(void);
uint32_t static CalculateJitter(void);
void static TimingFrame(uint32_t, uint32_t);
void static ValueFrame(void);
void static PortF_Init(void);
void static DelayWait10ms(uint32_t);
void static Pause(void);
void SwitchADCAveraging(void);
void static CreateHist();
void static HorizontalFrame(uint16_t color);
void static VerticalFrame(uint16_t color);
void static DiagonalFrame(uint16_t color);
void Rise(void);
void Fall(void);

// all three mains
//debug code
int main10(void){ 
  Switch_Init(&Fall,&Rise);     // initialize GPIO Port F interrupt
  EnableInterrupts();           // interrupt after all initialization
  Fall();
}

void Rise(void){
}

void Fall(void){
  switch(CurrentThread){
    case line:
      CurrentThread = hist;
      //main_line();
      break;
    case hist:
      CurrentThread = jitterAndUart;
      //main_hist();
      break;
    case jitterAndUart:
      CurrentThread = line;
      //main_jitterAndUart();
      break;
  }
}

// test ST7735_Line
int main_line(void){uint16_t color;
  PLL_Init(Bus80MHz);
  ST7735_InitR(INITR_REDTAB);
  PortF_Init();
  TitleFrame();
  Pause();
  color = ST7735_Color565(255,255,255);
  while(1){
    ST7735_FillScreen(color);
    color = ST7735_Color565(198,115,13);
    HorizontalFrame(color);
    VerticalFrame(color);
    DiagonalFrame(color);
    ST7735_FillScreen(color);
    color = ST7735_Color565(255,255,255);
    HorizontalFrame(color);
    VerticalFrame(color);
    DiagonalFrame(color);
  }
}


void static HorizontalFrame(uint16_t color){uint32_t x1,y1,x2,y2;
  x1 = 0; y1 = 0; x2 = 0; y2 = 0; 
  for(int i = 0; i < 80; i++){
    ST7735_Line(x1,y1,x2,y2,color);
    y1+=2; y2+=2;
    if(i%4){ x2+=2; }
  }
}

void static VerticalFrame(uint16_t color){uint32_t x1,y1,x2,y2;
  x1 = 127; y1 = 159; x2 = 127; y2 = 0; 
  for(int i = 0; i < 80; i++){
    ST7735_Line(x1,y1,x2,y2,color);
    y1-=2;
    if(i%4){ x1-=2; x2-=2; }
  }
}

void static DiagonalFrame(uint16_t color){uint32_t x1,y1,x2,y2,i;
  x1 = 0; y1 = 0; x2 = 0; y2 = 0; 
  for(i = 0; i < 63; i++){
    ST7735_Line(x1,y1,x2,y2,color);
    y1 += 2; x2 += 2;
  }
  ST7735_Line(x1,y1,x2,y2,color);
  y1 += 2; x2++; y2++; i++;
  for(i; i < 79; i++){
    ST7735_Line(x1,y1,x2,y2,color);
    y1 += 2; y2 += 2;
  }
  ST7735_Line(x1,y1,x2,y2,color);
  y1++; x1++; y2 += 2; 
  for(i = 0; i < 63; i++){
    ST7735_Line(x1,y1,x2,y2,color);
    x1 += 2; y2 += 2;
  }
}

// Updating ADC Histogram
int main(void){uint32_t rangeX, maxY, Xbuffer;
  PLL_Init(Bus80MHz);
  ST7735_InitR(INITR_REDTAB);
  PortF_Init();
  ADC0_InitTimer2ATriggerSeq3(0,8000); //10kHz ADC
  TitleFrame();
  DebugFlag = value;
  EnableInterrupts();
  char* Title_str = "ADC Noise";
  ST7735_XYplotInit(Title_str, 0, 0, 0, 0); //ranges set by PlotHist
  while(1){
    if(PF4==0){
      SwitchADCAveraging();
      while(PF4==0){}
    }
    Index = 0;
    while(Index<1000){
    }
    CreateHist();
    ST7735_PlotHist(Histogram, 4096);
  }
}

void SwitchADCAveraging(void){
  switch(ADC0_SAC_R){
    case 0x0:
      ADC0_SAC_R = 0x2;
    break;
    case 0x2:
      ADC0_SAC_R = 0x4;
    break;
    case 0x4:
      ADC0_SAC_R = 0x6;
    break;
    default:
      ADC0_SAC_R = 0x0;
  }
}

void static CreateHist(){
  for(int i = 0; i < 4096; i++){
    Histogram[i] = 0;
  }
  for(int i = 0; i < Index; i++){
    Histogram[Dump[i]]++;
  }
}
// Jitter and UART ADC Noise
int main_jitterAndUart(void){
  PLL_Init(Bus80MHz);
  ST7735_InitR(INITR_REDTAB);
  PortF_Init();
  UART_Init();
  Timer5A_Init();
  ADC0_InitTimer2ATriggerSeq3(0,8000); //10kHz ADC
  EnableInterrupts();
  TitleFrame();
  Pause();
  while(1){
    DebugTiming();
    //Pause();
    //DebugValue();
    //Pause();
  }
}

/**************TitleFrame***************
Prints title page of lab 2
*/
void static TitleFrame(){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* titleStr = "Lab 2\n  Andrew and Stathi";
  x = 8; y = 7;
  ST7735_SetCursor(x,y);
  ST7735_OutString(titleStr);
}

void static DebugTiming(void){uint32_t jitter; uint16_t errors = 0;
  DebugFlag = timing;
  Index = 0;
  while(Index<1000){
    jitter = ((jitter+55)*12345678)/1234567+0x02; //this line causes jitter
  }
  for(int i = 0; i < 999; i++){
    TimeDiffs[i] = Dump[i+1] - Dump[i];
    if((TimeDiffs[i]+ERROR_ALLOWANCE<TARGET_TIMING)|(TimeDiffs[i]-ERROR_ALLOWANCE>TARGET_TIMING)){
      errors++;
    }
  }
  jitter = CalculateJitter();
  TimingFrame(errors, jitter);
}

uint32_t static CalculateJitter(void){uint32_t largestTimeDiff, smallestTimeDiff;
	largestTimeDiff = smallestTimeDiff = TimeDiffs[0];
	//find smallest and largest time diffs
	for(int i = 1; i < 999; i++){
		if(TimeDiffs[i] < smallestTimeDiff){
			smallestTimeDiff = TimeDiffs[i];
		}
		if(TimeDiffs[i] > largestTimeDiff){
			largestTimeDiff = TimeDiffs[i];
		}
	}
	return largestTimeDiff - smallestTimeDiff;
}

void static TimingFrame(uint32_t errors, uint32_t jitter){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* string = "Timing Analysis\n";
  char* error_str =  "Number of errors :\n";
  char* jitter_str = "\nJitter :\n";
  x = 3; y = 0; //cursor
  ST7735_SetCursor(x, y);
  ST7735_OutString(string);
  ST7735_OutString(error_str);
  ST7735_OutUDec(errors);
  ST7735_OutString(jitter_str);
  ST7735_OutUDec(jitter);
}

void static DebugValue(void){
  ValueFrame();
  DebugFlag = value;
  Index = 0;
  while(Index<1000){
    WaitForInterrupt();
  }
  for(int i = 0; i < Index; i++){
    UART_OutUDec(Dump[i]);
    UART_OutString("\n\r");
  }
}

void static ValueFrame(void){int32_t x,y;
  ST7735_FillScreen(ST7735_BLACK);
  char* start_str = "ACD Noise\n     Measurement\n\n  Clear the termial\n\n";
  char* uart_str = "Clear the hyperterminal\n\r";
  char* sending_str = "Sending...";
  x = 6; y = 0; //cursor
  ST7735_SetCursor(x, y);
  ST7735_OutString(start_str);
  UART_OutString(uart_str);
  Pause();
  ST7735_OutString(sending_str);
}

void ADC0Seq3_Handler(void){int32_t previous, current;
  PF2 = 1; PF2 = 0; PF2 = 1; PF2 = 0;
  
  ADC0_ISC_R = 0x08;          // acknowledge ADC sequence 3 completion
  ADCvalue = ADC0_SSFIFO3_R;  // 12-bit result
  if((DebugFlag==timing)&&(Index<1000)){
    Dump[Index] = 0xFFFFFFFF - TIMER5_TAR_R;
    Index++;
  } else if((DebugFlag==value)&&(Index<1000)){
    Dump[Index] = ADCvalue;
    Index++;
  } 
  
  PF2 = 1; PF2 = 0;
}





// PF4 is input
// Make PF2 an output, enable digital I/O, ensure alt. functions off
void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTF_PCTL_R &= ~0x000F0F00; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x14;      // 4) disable analog function on PF2, PF4
  GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set direction to output
  GPIO_PORTF_AFSEL_R &= ~0x14;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x14;         // 7) enable digital port
}

// Subroutine to wait 10 msec
// Inputs: None
// Outputs: None
// Notes: ...
void static DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

void static Pause(void){
  while(PF4==0x00){ 
    DelayWait10ms(10);
  }
  while(PF4==0x10){
    DelayWait10ms(10);
  }
}
