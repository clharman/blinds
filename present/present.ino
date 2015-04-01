////////////////////////////////////////////
//HISTORY
//Original file created
//  -Performed basic smoothing on microphone input
//  -Rotated blinds at speed proportional to sound level from microphone
//  -Switched directions at limits of potentiometer
//  -Ignored noise input at high motor speeds (because of loud motor)
//  [Conor Barry  04/30/14]
//
//Revisited and redone
//  -Restructured to use main, source, and header files, with complete documentation
//  -Added limit cycling (numerous direction changes at various points)
//  -Reworked filtering system
//  -Added sustain function to prolong response to short input
//  -Added 'mode change' functionality (can switch between behavior
//  [Colin Harman 01/27/15]
//
//Blinds project for Jim Cogswell, http://www.jimcogswell.com/
////////////////////////////////////////////

#include "source.h"

//VARIABLE DECLARATIONS
//All pin variables should be const
//Potentiometer variables
const int pot_pin = 0;                  // Arduino pin for potentiometer input                                     
const int lim_ccw = 145;                // point at which the blinds change direction from clockwise to anticlockwise. Ranges from 0 - 1023
const int lim_cw = 845;                 // point at which the blinds change direction from anticlockwise to clockwise
int pot_raw = 0;                        // potentiometer value storage
int shim_ccw_out = 295;                 // outside edge of clockwise shimmer threshold
int shim_ccw_in = 305;                  // inside edge of clockwise shimmer threshold
int shim_cw_out = 705;                  // outside edge of counterclockwise shimmer threshold
int shim_cw_in = 695;                   // inside edge of counterclockwise shimmer threshold

//changes direction at each array level and proceeds to the next
int limit_cycle[] = {lim_ccw, shim_ccw_in, shim_ccw_out, shim_cw_out, shim_cw_in, lim_cw};


//"INITIAL CONDITIONS"
int lim_next_index = 0;
bool motor_dir = false;                // direction of motor (true = clockwise) & initial direction
int next_dir = -1;
int mode = 1;                            //mode may be 1,2,3

//Microphone & signal variables
int noise_thresh = 150;                   // threshold at which the microphone will begin listening. Ranges from 0 - 1023 - 75 seems to be optimal
int noise_loud = 200;                    // threshold for really LOUD noise (potentially due to motor, to ignore)
int noise_soft = 100;
float noise_ambient = 0;
double noise_input = 0;
float noise_motor = 0;
const int mic_signal_size = 11;          // mic_signal_size should  be an odd number, no smaller than 3
int mic_signal[mic_signal_size];         // array for holding raw sensor values for sensor1 //////////////////////////////////////////////////////////////////////////////////////
int mic_raw, mic_smooth;                 // pre- and post-filtering microphone data
float mic_scaled;                        // scaled from 0 to 1
float filterVal = 0.001;                 // this determines smoothness, .0001 is max  1 is off (no smoothing) ////////////////////////////////////////////////////////////////////
const int mic_pin = A5;                  // ?-> [the SPL output is connected to analog pin 0] <-?

//Motor variables
const int motor_pin = 5;                 // Arduino pin for motor output
int motor_spd = 0;                       // speed of motor (0 to 255)
const int relay_pwr_pin = 8;             // Arduino pin for motor power (on/off) relay
const int relay_dir_pin = 10;            // Arduino pin for motor direction relay
const int coast_down = 1000;             // msecs to allow motor to stop
const int relay_delay = 10;              // msecs to allow relay action


int sustain;
int silence;

//INIT
void setup()  { 
  Serial.begin(9600); // turn on serial for debugging purposes only
  //Initialize pins
  pinMode(motor_pin, OUTPUT); 
  pinMode(relay_pwr_pin, OUTPUT);
  pinMode(relay_dir_pin, OUTPUT);
  //Initialize relays
  digitalWrite(relay_pwr_pin,HIGH); // set initial power state ON
  digitalWrite(relay_dir_pin,HIGH); // set initial direction to be CLOCKWISE
}


//MAIN
void loop(){

  //INPUT PROCESSING
  //READ the potentiometer
  pot_raw = analogRead(pot_pin);
  
  //READ microphone
  mic_raw = analogRead(mic_pin);
  
  //Smooth mic signal
  mic_smooth = digitalSmooth(mic_raw, mic_signal, mic_signal_size);  // *every sensor you use with digitalSmooth needs its own array
  //Apply motor noise model
  noise_motor = 260 * log(motor_spd) - 615;
  //Apply ambient noise model
  noise_ambient = 0;
  //Apply input noise model
  noise_input = 10 * log(pow(10,(mic_smooth/10)) - pow(10,(noise_motor/10)) - pow(10,(noise_ambient/10))) / log(10);
  ////////////////
  
  if(mode == 1){//Motion ~ sound

    if (noise_input > noise_soft)
      sustain = sustain + noise_input;
    motor_spd = sustain/8 + (sustain > 10)*20;
    
    
    
    if(noise_input > noise_loud || sustain > 100){
      mode = 2;
      sustain = 0;
    }
  }
  else if(mode == 2){//Motion ~ silence
    if(noise_input < noise_soft)
      sustain = sustain + (noise_soft - noise_input);
    else if(noise_input > noise_loud)
      sustain = 0;
      
    motor_spd = sustain/8 + (sustain > 10)*20;
    
    
    if(sustain > 100){
      mode = 3;
      sustain = 0;
    }
  }
  else if(mode == 3){//Slow coast down
    if(motor_spd > 0){
      motor_spd = motor_spd - 1;
      delay(10);
    }
    else
      mode = 1;
  }

  if(sustain > 0)
    sustain = sustain - 1;
    
    
  Serial.print(noise_input);
  Serial.print("\t");
  Serial.print(sustain);
  Serial.print("\t");
  Serial.print(motor_spd);
  Serial.print("\n");
  

  //Update variables to print here, for convenience [Arrays must have same length]
  //static char* var_names[] = {"next_dir","lim_index",   "lim_next",                  "pot_raw","mic_smooth","motor_spd","sustain","mic_scaled"};
  //float var_values[] =       { next_dir,  lim_next_index,limit_cycle[lim_next_index], pot_raw,  mic_smooth,  motor_spd,  sustain , mic_scaled};
  //debugSerial(var_names, var_values);
  delay(2);

  //WRITE throttle
  //analogWrite(motor_pin,250);
  analogWrite(motor_pin, 200);

  //Change motor direction if appropriate
  directionWrite(pot_raw, lim_next_index, limit_cycle, motor_dir, 
    relay_pwr_pin, relay_dir_pin, relay_delay, coast_down, next_dir);
}
