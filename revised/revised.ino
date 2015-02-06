////////////////////////////////////////////
//Conor Barry  04/30/14
//Original file created
//  -Performed basic smoothing on microphone input
//  -Rotated blinds at speed proportional to sound level from microphone
//  -Switched directions at limits of potentiometer
//  -Ignored noise input at high motor speeds (because of loud motor)

//Colin Harman 01/27/15
//Revisited and revised
//  -Simplified filter functions

//Blinds project for Jim Cogswell, http://www.jimcogswell.com/

////////////////////////////////////////////
//VARIABLE DECLARATIONS

//Potentiometer variables
const int clockwiseThreshold = 140;      // point at which the blinds change direction from clockwise to anticlockwise. Ranges from 0 - 1023
const int antiClockwiseThreshold = 850;  // point at which the blinds change direction from anticlockwise to clockwise

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
int motor_pin = 5;                       // Arduino pin for motor output
int motor_spd = 0;                       // speed of motor (0 to 255)
int relay_pwr_pin = 8;                   // Arduino pin for motor power (on/off) relay
int relay_dir_pin = 10;                  // Arduino pin for motor direction relay
int pot_pin = 0;                         // Arduino pin for potentiometer input
int pot_raw = 0;                         // potentiometer value storage
boolean motor_dir = true;                // direction of motor (true = clockwise)
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
  
  //put a MIC_READ here instead of in a function
  
  //micThreshed() in call to digitalSmooth gives (0 or mic_raw).  not sure if this will work because of scope of mic_raw etc
  mic_smooth = digitalSmooth(micThreshed(), sensSmoothArray1);  // every sensor you use with digitalSmooth needs its own array

  //Writes mic_raw, motor_spd, pot_value to serial
  debugSerial(mic_raw, smoothedVal, pot_raw);

  //Abstraction to write speed to motor based on whatever
  speedWrite();
  
  pot_raw = analogRead(pot_pin); //read the value for the potentiometer

  //Abstraction to determine direction based on whatever
  directionWrite();
}
//ENDLOOP//





////////////////////////////////////////////
//DIGITAL SMOOTHING FUNCTION

int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  // static int raw[filter_size];
  static int sorted[filter_size];
  boolean done;

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

////////////////////////////////////////////
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


// reads mic pin
// returns (0 or mic_raw)
int micThreshed(){
  
  mic_raw = analogRead(mic_pin);   // read sensor 1
  if(mic_raw > noise_thresh && ~(mic_raw > noise_loud && smoothedVal > 60)){ // if mic exceeds room noise BUT NOT if mic is loud while motor is high (ignores motor noise)
    return mic_raw;
  }
  else{                       //if mic does not exceed room noise, then don't read mic
    return 0;
  }
}

void speedWrite(){
  smoothedVal = mic_smooth * 0.25; // divide the smoothed values by 4 so that they fit within a suitable output range for the motor
  analogWrite(motor_pin,smoothedVal); //set the motor to go at this speed
}

void directionWrite(){
  if(pot_raw < clockwiseThreshold && motor_dir==true){ //if pot reads that its reached the edge and the motor is moving clockwise
    digitalWrite(relay_pwr_pin, LOW); //turn off power
    delay(coast_down + relay_delay); //wait for it to slow down
    digitalWrite(relay_dir_pin,LOW); //change direction
    delay(relay_delay);
    digitalWrite(relay_pwr_pin,HIGH); //turn on motor
    motor_dir = false; //the direction is now anti-clockwise
  }
  else if(pot_raw > antiClockwiseThreshold && motor_dir==false){ //if the pot reaches far left, and motor direction is anti-clockwise
    digitalWrite(relay_pwr_pin, LOW); //turn off power
    delay(coast_down + relay_delay); // wait for it to slow down
    digitalWrite(relay_dir_pin,HIGH); //change direction
    delay(relay_delay);
    digitalWrite(relay_pwr_pin,HIGH); //turn on motor
    motor_dir = true; //the direction is now clockwise
  }
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
