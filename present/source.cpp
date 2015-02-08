#include "source.h"
#include "Arduino.h"

//R  \
//M  \
//E  Returns 0 or mic input based on threshold 
int micThreshed(int mic_raw, int noise_thresh, int noise_loud,
  int smoothedVal){ 
  if(mic_raw > noise_thresh && ~(mic_raw > noise_loud && smoothedVal > 60)){ // if mic exceeds room noise BUT NOT if mic is loud while motor is high (ignores motor noise)
    return mic_raw;
  }
  else{                       //if mic does not exceed room noise, then don't read mic
    return 0;
  }
}

//changes motor direction
void switchDirection(bool motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down){
  if(motor_dir==true){ //if pot reads that its reached the edge and the motor is moving clockwise
    digitalWrite(relay_pwr_pin, LOW); //turn off power
    delay(coast_down + relay_delay); //wait for it to slow down
    digitalWrite(relay_dir_pin,LOW); //change direction
    delay(relay_delay);
    digitalWrite(relay_pwr_pin,HIGH); //turn on motor
    motor_dir = false; //the direction is now anti-clockwise
  }
  else if(motor_dir==false){ //if the pot reaches far left, and motor direction is counterclockwise
    digitalWrite(relay_pwr_pin, LOW); //turn off power
    delay(coast_down + relay_delay); // wait for it to slow down
    digitalWrite(relay_dir_pin,HIGH); //change direction
    delay(relay_delay);
    digitalWrite(relay_pwr_pin,HIGH); //turn on motor
    motor_dir = true; //the direction is now clockwise
  }
}

//R  \
//M  
//E   
//Writes motor direction using following algorithm:
//NOTE: REQUIRES CORRECT INITIAL CONDITIONS TO FUNCTION.
//IF INITIAL CONDITIONS ARE WRONG MECHANISM WILL NOT STOP
//AT LIMITS AND MAY DAMAGE ITSELF
void directionWrite(int pot_raw, int lim_next_index, 
  int limit_cycle[], bool motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down){
  if(potCompare(pot_raw, lim_next_index, limit_cycle)){
      switchDirection(motor_dir, relay_pwr_pin, relay_dir_pin, 
                        relay_delay, coast_down);
  }  
}

//R  \
//M  \
//E  Calculates & returns motor speed
int speedWrite(int signal_in){
  return signal_in * 0.25; //gain
}

//R  \
//M  \
//E  returns true if motor should reverse direction.  specifically:
//  returns true if:
//   - index mod 2 = 0 AND pot < limit
//   - index mod 2 = 1 AND pot > limit
bool potCompare(int pot_reading, int next_index, int limit_cycle[]){
  if(next_index % 2 == 0 && pot_reading < limit_cycle [next_index])
    return true;
  else if(next_index % 2 == 1 && pot_reading > limit_cycle [next_index])
    return true;
  else
    return false;
}

//Outputs debugging information in the serial window in the following format:
// Uncomment the following Serial lines of code and open the serial monitor while running to see the raw mic value, the motor speed (smoothedVal), and potentiometer value
//  mic_raw       motor_spd          POT VALUE  
void debugSerial(int mic_raw, int smoothedVal, int pot_raw){
  Serial.print(mic_raw);
  Serial.print("           ");
  Serial.print(smoothedVal);
  Serial.print("           ");
  Serial.println(pot_raw);
}

//ALTERNATIVE SMOOTHING FUNCTION
int smooth(int data, float filterVal, float smoothedVal){
  if (filterVal > 1){      // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }
  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);
  return (int)smoothedVal;
}

//DIGITAL SMOOTHING FUNCTION
int digitalSmooth(int rawIn, int *sensSmoothArray, 
int filter_size){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  // static int raw[filter_size];
  int sorted[filter_size];
  bool done;
  i = (i + 1) % filter_size;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot
  // Serial.print("raw = ");
  for (j=0; j<filter_size; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }
  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filter_size - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }
  /*
  for (j = 0; j < (filter_size); j++){    // print the array to debug
   Serial.print(sorted[j]); 
   Serial.print("   "); 
   }
   Serial.println();
   */
  // throw out top and bottom 25% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filter_size * 25)  / 100), 1); 
  top = min((((filter_size * 75) / 100) + 1  ), (filter_size - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }
  //  Serial.println();
  //  Serial.print("average = ");
  //  Serial.println(total/k);
  return total / k;    // divide by number of samples
}
