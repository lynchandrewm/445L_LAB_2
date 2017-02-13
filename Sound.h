/*Sound.h - this file contains the functions for module Sound*/

/****************startBuzzer***************
 Starts the alarm buzzer
 Inputs:  frequency of buzzer
 Outputs: none
 */ 
void Sound_Init(uint32_t freq);


/****************startBuzzer***************
 Runs the sound until flag is made false
 Inputs:  none
 Outputs: none
 */ 
void Sound_FlagEnabledSound(uint8_t* flag);

