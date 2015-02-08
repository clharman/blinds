////////////////////////////////////////////
//Conor Barry  04/30/14
//Original file created
//  -Performed basic smoothing on microphone input
//  -Rotated blinds at speed proportional to sound level from microphone
//  -Switched directions at limits of potentiometer
//  -Ignored noise input at high motor speeds (because of loud motor)

//Colin Harman 01/27/15
//Revisited and revised
//  -Reworked overall structure
//  -Simplified filter functions

//Blinds project for Jim Cogswell, http://www.jimcogswell.com/

////////////////////////////////////////////

#include "source.h"

//VARIABLE DECLARATIONS
//All pin variables should be const
//Potentiometer variables
const int pot_pin = 0;                   // Arduino pin for potentiometer input                                     
int pot_raw = 0;                         // potentiometer value storage
const int lim_cw = 140;                  // point at which the blinds change direction from clockwise to anticlockwise. Ranges from 0 - 1023
const int lim_ccw = 850;                 // point at which the blinds change direction from anticlockwise to clockwise
int shim_cw_out = 275;                   // outside edge of clockwise shimmer threshold
int shim_cw_in = 325;                    // inside edge of clockwise shimmer threshold
int shim_ccw_out = 725;                  // outside edge of counterclockwise shimmer threshold
int shim_ccw_in = 675;                   // inside edge of counterclockwise shimmer threshold

//changes direction at each array level and proceeds to the next
int limit_cycle[] = {lim_cw, shim_cw_in, shim_cw_out, shim_ccw_out, shim_ccw_in, lim_ccw};



//starts going clockwise until lim_cw
int lim_next_index = 0;
bool motor_dir = true;                // direction of motor (true = clockwise) & initial direction




//Microphone & signal variables
int noise_thresh = 70;                   // threshold at which the microphone will begin listening. Ranges from 0 - 1023 - 75 seems to be optimal
int noise_loud = 300;                    // threshold for really LOUD noise (potentially due to motor, to ignore)
const int filter_size = 21;              // filter_size should  be an odd number, no smaller than 3
int sensSmoothArray1 [filter_size];      // array for holding raw sensor values for sensor1 //////////////////////////////////////////////////////////////////////////////////////
int mic_raw, mic_smooth;                 // variables for sensor1 data
float filterVal = 0.001;                 // this determines smoothness, .0001 is max  1 is off (no smoothing) ////////////////////////////////////////////////////////////////////
float smoothedVal;                       // this holds the last loop value just use a unique variable for every different sensor that needs smoothing ////////////////////////////
const int mic_pin = A5;                  // ?-> [the SPL output is connected to analog pin 0] <-?

//Motor variables
const int motor_pin = 5;                 // Arduino pin for motor output
int motor_spd = 0;                       // speed of motor (0 to 255)
const int relay_pwr_pin = 8;             // Arduino pin for motor power (on/off) relay
const int relay_dir_pin = 10;            // Arduino pin for motor direction relay
int motor_scaled;                        // value storage for the scaled motor value
int coast_down = 1000;                   // msecs to allow motor to stop
int relay_delay = 10;                    // msecs to allow relay action




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
  
  mic_raw = analogRead(mic_pin);


  //micThreshed() in call to digitalSmooth gives (0 or mic_raw).  not sure if this will work because of scope of mic_raw etc
  mic_smooth = digitalSmooth(micThreshed(mic_raw, noise_thresh, 
    noise_loud, smoothedVal), sensSmoothArray1, filter_size);  // every sensor you use with digitalSmooth needs its own array

  //Writes mic_raw, motor_spd, pot_value to serial
  debugSerial(mic_raw, smoothedVal, pot_raw);

  //Abstraction to write speed to motor based on whatever
  speedWrite(motor_pin, mic_smooth);
  
  pot_raw = analogRead(pot_pin); //read the value for the potentiometer

  //Abstraction to determine direction based on whatever
  directionWrite(pot_raw, lim_next_index, 
  limit_cycle, motor_dir, relay_pwr_pin, 
  relay_dir_pin, relay_delay, coast_down);
}






