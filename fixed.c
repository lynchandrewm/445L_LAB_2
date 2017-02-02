// fixed.c
// Andrew Lynch
// January 20, 2017
// Details implemenation of functions in the "fixed" module. The "fixed"
// module outputs fixed point numbers and plots to a ST7735 LCD
// 16360
// Dylan
// January, 23, 2017
// hardware connections
// **********ST7735 TFT and SDC*******************
// ST7735
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

// included .h files
#include <stdint.h>
#include "fixed.h"
#include "ST7735.h"

// extern reference


// define statements
#define ASCII_ZERO 48
#define ASCII_ASTRIC 42
#define ASCII_DOT 46
#define ASCII_MINUS 45
#define ASCII_SPACE 32
#define NULL 0

#define COLUMBS 21

// struct union enum statements
typedef enum {false, true} bool;

// global variables and constants
int32_t static Xmin, Xmax, Xrange, Ymin, Ymax, Yrange;

// prototypes of private functions
void static decToString(int32_t refNum, char* numString);
void static binToString(int32_t refNum, int32_t res, char* numString);
void static printTitle(char* str);
int32_t static strLen(char* str);
void static printStringToLCD(char* string);
int32_t static magnitude(int32_t n);
int32_t static decGetPlace(int32_t num, int32_t place);
char static getAsciiNum(int32_t num);
bool static decInBounds(int32_t num);
void static decBlankString(char* str_p);
bool static binInBounds(int32_t num);
void static binBlankString(char* str_p);
void static setAxes(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY);
void static plotPoint(int32_t x, int32_t y);


// implementation of functions

/**************ST7735_sDecOut3***************
passes string pointer and num to modular functions to fill and print string
*/
void ST7735_sDecOut3(int32_t n){
  char decString[7];
  decToString(n, decString);
  printStringToLCD(decString);
  return;
}

/**************ST7735_uBinOut8***************
set resolution, then passes num and res to fill and print string
*/
void ST7735_uBinOut8(uint32_t n){
  int32_t resolution = 256;
  char binString[7];
  binToString(n, resolution, binString);
  printStringToLCD(binString);
  return;
}

/**************ST7735_XYplotInit***************
Clears display, sets min, max, and range, and outputs title to display.
*/
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY,
  int32_t maxY){
  setAxes(minX, maxX, minY, maxY);
  printTitle(title);
  return;
}

/**************ST7735_XYplot***************
Must run ST7735_XYplotInit before to set mins, maxes, and ranges. 
Indexs through buffers and prints each point.
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){
  for(int i = 0; i < num; i++){
    plotPoint(bufX[i], bufY[i]);
  }
  return;
}

/**************decToString***************
deal with out-of-bounds case, and if not build string
*/
void static decToString(int32_t refNum, char* numString){
  int32_t num = refNum;                     //this number I will mess with
  if(!decInBounds(num)){
    decBlankString(numString);  
    return;
  }
  if(num<0){                                //minus or space
    numString[0] = ASCII_MINUS;
    refNum = -refNum;
  } else {
    numString[0] = ASCII_SPACE;
  }
  num = decGetPlace(refNum, 3);             //before the decimal point
  numString[1] = getAsciiNum(num);
  numString[2] = ASCII_DOT;                 //decimal point
  for(int i = 0; i < 3; i++){               //after the decimal point
    num = decGetPlace(refNum, 2-i);
    numString[3+i] = getAsciiNum(num);
  }
  numString[6] = NULL;                      //null terminated string
  return;
}

/**************binToString***************
deal with out-of-bounds, no negative case, build string
my refNum fixed point format has decimal point between
bit2 and bit1. Shift refNum 2 magnitudes (base10) before
divide by res
*/
void static binToString(int32_t refNum, int32_t res, char* numString){
  refNum = (refNum*100)/res;                 //get base10 in my format
  int32_t num = refNum;                      //this number I will mess with
  bool numInFourthPlace = false;
  if(!binInBounds(num)){
    binBlankString(numString);
    return;
  }
  for(int i = 0; i < 2; i++){                
    num = decGetPlace(refNum, 4-i);
    if(num||numInFourthPlace){               //covers case #0#.##
      numString[i] = getAsciiNum(num);
      numInFourthPlace = true;
    } else {
      numString[i] = ASCII_SPACE;            //first two spaces are blank, 
    }                                        //not zero
  }
  num = decGetPlace(refNum, 2);
  numString[2] = getAsciiNum(num);
  numString[3] = ASCII_DOT;
  for(int i = 0; i < 2; i++){
    num = decGetPlace(refNum, 1-i);
    numString[4+i] = getAsciiNum(num);
  }
  numString[6] = NULL;
  return;
}

/**************printTitle***************
Prints title in space above graph. Does not wrap. 
Starts at top left of display.
*/
void static printTitle(char* str){int32_t x,y,len;
  len = strLen(str);
  x = (COLUMBS-len)/2; y = 1;
  ST7735_SetCursor(x,y);
  printStringToLCD(str);
  return;
}

int32_t static strLen(char* str){int32_t len;
  for(len = 0; str[len] != 0x0; len++);
  return len;
}

/**************printStringToLCD***************
clears the screen, then prints null terminated string. 
prints in yellow because that appears to be the init default. it's negotiable.
*/
void static printStringToLCD(char* str_p){
  ST7735_OutString(str_p);
}

/**************decGetPlace***************
base-ten "place" finder.
e.g. num = 123456, place = 2, return 4.
num = 99900, place = 0, return 0.
*/
int32_t static decGetPlace(int32_t num, int32_t place) {
	int32_t greaterMod = magnitude(place + 1);
	int32_t lessMod = magnitude(place);
	int32_t shiftToOnesPlace = magnitude(place);
	num = ((num%greaterMod)-(num%lessMod))/shiftToOnesPlace;
	return num;
}

/**************magnitude***************
returns 10^n
*/
int32_t static magnitude(int32_t n){
  int32_t magNum = 1;
  for(int i = 0; i < n; i++){
    magNum *= 10;
  }
  return magNum;
}

/**************getAsciiNum***************
indexes through ascii table from zero to obtain ascii code for num.
assert (if available) checks that num is between 0 and 9.
*/
char static getAsciiNum(int32_t num){
  //assert((num>=0)&&(num<10));
  return (char)(ASCII_ZERO + num);
}

/**************decInBounds***************
check that the num is between -9999 and 9999
*/
bool static decInBounds(int32_t num){
  return (num>=-9999)&&(num<=9999);
}

/**************decBlankString***************
uses referenced string pointer to fill string with " *.***"
null terminated.
*/
void static decBlankString(char* str_p){
  str_p[0] = ASCII_SPACE; str_p[1] = ASCII_ASTRIC; str_p[2] = ASCII_DOT;
  str_p[3] = ASCII_ASTRIC; str_p[4] = ASCII_ASTRIC; str_p[5] = ASCII_ASTRIC;
  str_p[6] = NULL;
  return;
}


/**************binInBounds***************
returns true if num is in bounds. In bounds is positive less than 255997.
256000 isn't the upper bound due to rounding.
*/
bool static binInBounds(int32_t num){
  return (num>=0)&&(num<=99999); 
}

/**************binBlankString***************
uses referenced string pointer to fill string with "***.**"
null terminated.
*/
void static binBlankString(char* str_p){
  str_p[0] = ASCII_ASTRIC; str_p[1] = ASCII_ASTRIC; str_p[2] = ASCII_ASTRIC;
  str_p[3] = ASCII_DOT; str_p[4] = ASCII_ASTRIC; str_p[5] = ASCII_ASTRIC;
  str_p[6] = NULL;
  return;
}

/**************setAxis***************
Some copied and adapted code from ST7753_PlotClear. This function has been 
adapted to keep track of a variable X-axis. 
*/
void static setAxes(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
  if(maxY>minY){
    Ymax = maxY;
    Ymin = minY;
  } else{
    Ymax = minY;
    Ymin = maxY;
  }
  if(maxX>minX){
    Xmax = maxX;
    Xmin = minX;
  } else{
    Xmax = minX;
    Xmin = maxX;
  }
  Yrange = Ymax - Ymin;
  Xrange = Xmax - Xmin;
}

/**************plotPoint***************
Outputs to display single (x,y) coordinate. Uses Ymin/max and Xmin/max
Ymax maps to j=32, Ymin to j=159
Xmax maps to i=0,  Xmin to i=127
*/
void static plotPoint(int32_t x, int32_t y){int32_t i,j;
  if(y<Ymin){y = Ymin;}
  if(y>Ymax){y = Ymax;}
  if(x<Xmin){x = Xmin;}
  if(x>Xmax){x = Xmax;}
  j = ((((Ymax-y)*127)+(32*Yrange))+(Yrange/2))/Yrange;
  i = (((x-Xmin)*127)+(Xrange/2))/Xrange;
  //j = (((Ymax-y)/Yrange)*127)+32;  //truncation error
  //i = ((x-Xmin)/Xrange)*127;
  if(j<32){  j=32;  }
  if(j>159){ j=159; }
  if(i<0){   i=0;   }
  if(i>127){ i=127; }
  ST7735_DrawPixel(i,   j,   ST7735_BLUE);
  ST7735_DrawPixel(i+1, j,   ST7735_BLUE);
  ST7735_DrawPixel(i,   j+1, ST7735_BLUE);
  ST7735_DrawPixel(i+1, j+1, ST7735_BLUE);
}



