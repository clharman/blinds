#ifndef H_SOURCE
#define H_SOURCE
#include "Arduino.h"


int micThreshed(int mic_raw, int noise_thresh, int noise_loud,
  int smoothedVal);

void switchDirection(bool motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down);

//Writes motor direction using following algorithm:
//NOTE: REQUIRES CORRECT INITIAL CONDITIONS TO FUNCTION.
//IF INITIAL CONDITIONS ARE WRONG MECHANISM WILL NOT STOP
//AT LIMITS AND MAY DAMAGE ITSELF
void directionWrite(int pot_raw, int lim_next_index, 
  int limit_cycle[], bool motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down);

//R  \
//M  \
//E  Calculates & returns motor speed
int speedWrite(int signal_in);

//R  \
//M  \
//E  returns true if it is time to reverse direction
bool potCompare(int pot_reading, int next_index, 
	int limit_cycle[]);

//R  \
//M  Serial out
//E  Writes raw mic, smoothed mic, and pot to serial out
void debugSerial(int mic_raw, int smoothedVal, int pot_raw);

//idk wtf this doe
int smooth(int data, float filterVal, float smoothedVal);

//talk about convoluted
int digitalSmooth(int rawIn, int *sensSmoothArray, 
	int filter_size);



#endif
