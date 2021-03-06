#include "source.h"

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

//R  \
//M  power relay status, direction relay status
//E  reverses direction of motor when called
void switchDirection(bool &motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down){
  if(motor_dir==false){                //If motor direction is cw, go ccw
    digitalWrite(relay_pwr_pin, LOW); //turn off power
    delay(coast_down + relay_delay);  //wait for system to stop
    digitalWrite(relay_dir_pin,LOW);  //change direction
    delay(relay_delay);               //allow relay actuation
    digitalWrite(relay_pwr_pin,HIGH); //turn on motor
    motor_dir = true; //the direction is now anti-clockwise
  }
  else if(motor_dir==true){          //If motor direction is ccw, go cw
    digitalWrite(relay_pwr_pin, LOW); //turn off power
    delay(coast_down + relay_delay);  //wait for system to stop
    digitalWrite(relay_dir_pin,HIGH); //change direction
    delay(relay_delay);               //allow relay actuation
    digitalWrite(relay_pwr_pin,HIGH); //turn on motor
    motor_dir = false; //the direction is now clockwise
  }
}

//R  \
//M  power relay status, direction relay status
//E  reverses motor direction if a limit is reached
//
//WARNING: REQUIRES CORRECT INITIAL CONDITIONS TO FUNCTION.
//IF INITIAL CONDITIONS ARE WRONG MECHANISM WILL NOT STOP
//AT LIMITS AND MAY DAMAGE ITSELF
void directionWrite(int pot_raw, int &lim_next_index, 
  int *limit_cycle, bool &motor_dir, int relay_pwr_pin, 
  int relay_dir_pin, int relay_delay, int coast_down, int &next_dir){
  if(potCompare(pot_raw, lim_next_index, limit_cycle)){
    
      switchDirection(motor_dir, relay_pwr_pin, relay_dir_pin, 
                        relay_delay, coast_down);
                        
                        
      
      if(lim_next_index == 0 || lim_next_index == 5){
        delay(10);
        //Serial.print("flop next dir \n\n\n");
        if(next_dir == 1)
          next_dir = -1;
        else
          next_dir = 1;
      }
      lim_next_index = lim_next_index + next_dir;
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
bool potCompare(int pot_reading, const int next_index, int* limit_cycle){
  if(next_index % 2 == 0 && pot_reading < limit_cycle [next_index]){
        //Serial.print("first switch case \n");
    return true;
  }
  else if(next_index % 2 == 1 && pot_reading > limit_cycle [next_index]){
        //Serial.print("second switch case \n");
    return true;
  }
  else
    return false;
}

//R  var_names is an array of c-strings containing only & all names of variables held in array of floats var_values
//M  Serial out
//E  Prints a line with the name of each variable followed by its value
void debugSerial(char* var_names[], const float var_values[]){
  for(int i = 0; i < sizeof(var_values)+6; i++){
    Serial.print(var_names[i]);
    Serial.print("\t");
    Serial.print(var_values[i]);
    Serial.print("\t");
  }
  Serial.print("\n");
}


//R
//M
//E
int digitalSmooth(int rawIn, int *sensSmoothArray, int filter_size){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
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
  // throw out top and bottom 25% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filter_size * 25)  / 100), 1); 
  top = min((((filter_size * 75) / 100) + 1  ), (filter_size - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
  }
  return total / k;    // divide by number of samples
}
