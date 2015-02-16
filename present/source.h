#ifndef H_SOURCE
#define H_SOURCE
#include "Arduino.h"

//R  \
//M  \
//E  Returns 0 or mic input based on threshold 
int micThreshed(int mic_raw, int noise_thresh, int noise_loud,
  int smoothedVal);

//R  \
//M  power relay status, direction relay status
//E  reverses direction of motor when called
void switchDirection(bool motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down);

//R  \
//M  power relay status, direction relay status
//E  reverses motor direction if a limit is reached
void directionWrite(int pot_raw, int &lim_next_index, 
  int limit_cycle[], bool motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down,  int &next_dir);

//R  \
//M  \
//E  Calculates & returns motor speed
int speedWrite(int signal_in);

//R  \
//M  \
//E  returns true if it is time to reverse direction
bool potCompare(int pot_reading, int next_index, 
	int limit_cycle[], int motor_dir);

//R  \
//M  Serial out
//E  Writes raw mic, smoothed mic, and pot to serial out
void debugSerial(float var_values[]);

//idk wtf this doe
int smooth(int data, float filterVal, float smoothedVal);

//talk about convoluted
int digitalSmooth(int rawIn, int *sensSmoothArray, 
	int filter_size);



#endif
