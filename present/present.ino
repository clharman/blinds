////////////////////////////////////////////
//Conor Barry  04/30/14
//Original file created
//  -Performed basic smoothing on microphone input
//  -Rotated blinds at speed proportional to sound level from microphone
//  -Switched directions at limits of potentiometer
//  -Ignored noise input at high motor speeds (because of loud motor)
//
//Colin Harman 01/27/15
//Revisited and revised
//  -Reworked overall structure
//  -Simplified filter functions
//
//Blinds project for Jim Cogswell, http://www.jimcogswell.com/
////////////////////////////////////////////

#include "source.h"

//VARIABLE DECLARATIONS
//All pin variables should be const
//Potentiometer variables
const int pot_pin = 0;                   // Arduino pin for potentiometer input                                     
const int lim_cw = 140;                  // point at which the blinds change direction from clockwise to anticlockwise. Ranges from 0 - 1023
const int lim_ccw = 850;                 // point at which the blinds change direction from anticlockwise to clockwise
int pot_raw = 0;                         // potentiometer value storage
int shim_cw_out = 275;                   // outside edge of clockwise shimmer threshold
int shim_cw_in = 325;                    // inside edge of clockwise shimmer threshold
int shim_ccw_out = 725;                  // outside edge of counterclockwise shimmer threshold
int shim_ccw_in = 675;                   // inside edge of counterclockwise shimmer threshold

//changes direction at each array level and proceeds to the next
int limit_cycle[] = {lim_cw, shim_cw_in, shim_cw_out, shim_ccw_out, shim_ccw_in, lim_ccw};

//"INITIAL CONDITIONS"
int lim_next_index = 0;
bool motor_dir = false;                // direction of motor (true = clockwise) & initial direction
int next_dir = -1;

//Microphone & signal variables
int noise_thresh = 150;                   // threshold at which the microphone will begin listening. Ranges from 0 - 1023 - 75 seems to be optimal
int noise_loud = 500;                    // threshold for really LOUD noise (potentially due to motor, to ignore)
const int mic_signal_size = 21;          // mic_signal_size should  be an odd number, no smaller than 3
int mic_signal[mic_signal_size];         // array for holding raw sensor values for sensor1 //////////////////////////////////////////////////////////////////////////////////////
int mic_raw, mic_smooth;                 // pre- and post-filtering microphone data
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

  //READ microphone
  mic_raw = analogRead(mic_pin);

  //READ the potentiometer
  pot_raw = analogRead(pot_pin);

  //get micThreshed out of there
  //micThreshed() in call to digitalSmooth gives (0 or mic_raw).  not sure if this will work because of scope of mic_raw etc
  //mic_smooth = digitalSmooth(mic_raw, mic_signal, mic_signal_size);  
    // *every sensor you use with digitalSmooth needs its own array
  if(mic_raw > 60)
    mic_smooth = mic_raw;
  else
    mic_smooth = 0;

  //calculate next motor speed
  motor_spd = mic_smooth / 4;
  
  if (motor_spd > 130)
    sustain = sustain + 10;
  if(sustain > 0)
    sustain--;

  float to_display[] = {pot_raw, mic_smooth, motor_spd, sustain};
  //Writes mic_raw, motor_spd, pot_value to serial
  //debugSerial(to_display);

  //WRITE throttle
  analogWrite(motor_pin,150);

  //Change motor direction if appropriate
  directionWrite(pot_raw, lim_next_index, 
    limit_cycle, motor_dir, relay_pwr_pin, 
    relay_dir_pin, relay_delay, coast_down, next_dir);
}
