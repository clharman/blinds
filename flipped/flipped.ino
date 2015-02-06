////////////////////////////////////////////
//Conor Barry 04/30/14
//Blinds project for Jim Cogswell

////////////////////////////////////////////
//VARIABLE DECLARATIONS

int clockwiseThreshold = 145; // change this value to set the point at which the blinds will change direction from clockwise to anticlockwise. Ranges from 0 - 1023
int antiClockwiseThreshold = 845;// change this value to set the point at which the blinds will change direction from anticlockwise to clockwise
int roomNoiseThreshold = 35; //change this value to set the threshold at which the microphone will begin listening. Ranges from 0 - 1023 - 75 seems to be optimal






#define filterSamples   21              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1 
int rawData1, smoothData1;  // variables for sensor1 data
float filterVal = 0.001;       // this determines smoothness  - .0001 is max  1 is off (no smoothing)
float smoothedVal;     // this holds the last loop value just use a unique variable for every different sensor that needs smoothing


int motor = 5;  // the pin that the motor is attached to
int motorSpeed = 0;  // how bright the motor is
int powerRelay = 8;  //the pin for the Power Relay  
int directionRelay = 10;  // the pin for the Direction Relay
int pot = 0;  // the pin for the potentiometer
int potValue = 0; // the potentiometer value storage
boolean motorDirection = true;  // the motor direction is true (i.e. clockwise) from the start

const int micInput = A5;   // the SPL output is connected to analog pin 0
int mappedMotor; // value storage for the scaled motor value
int inputData;

////////////////////////////////////////////
//SETUP ARDUINO

void setup()  { 

  Serial.begin(9600); // turn on serial for debugging purposes only
  pinMode(motor, OUTPUT); 
  pinMode(powerRelay, OUTPUT);
  pinMode(directionRelay, OUTPUT);


  digitalWrite(powerRelay,HIGH); //turn power on at start
  
  digitalWrite(directionRelay,HIGH); //set direction to be CLOCKWISE

}

////////////////////////////////////////////
//CONTINUOUSLY RUNNING CODE

void loop(){

  ////////////////////////////////////////////////////
  //MICROPHONE SMOOTHING

  rawData1 = analogRead(micInput);   // read sensor 1

  if(rawData1 > roomNoiseThreshold){ // if mic does not exceed room noise, don't read
    inputData = rawData1; //read the mic
  }
  if(rawData1 < roomNoiseThreshold){ //if mic exceeds room noise
    inputData = 0;
  }
  //   
  //if(rawData1 > 300 && smoothedVal > 60){ // if mic is reading loud values and the motor is running quickly, then don't read the mic (otherwise it would get into a feedback loop
  //  inputData = 200;
  //}
  
  smoothData1 = digitalSmooth(inputData, sensSmoothArray1);  // every sensor you use with digitalSmooth needs its own array

  //    Serial.print(rawData1);
  //    Serial.print("   ");
  //    Serial.println(smoothData1);


  /////////////////////////////////////////////////////
  //MOTOR SPEED CONTROL////////////////////////////////

  smoothedVal = (25 - (smoothData1)) * ((25 - (smoothData1)) > 0); // divide the smoothed values by 4 so that they fit within a suitable output range for the motor
  analogWrite(motor,smoothedVal); //set the motor to go at this speed

  potValue = analogRead(0); //read the value for the potentiometer


  /////////////////////////////////////////////////////
  //  DEBUG SECTION
  // Uncomment the following Serial lines of code and open the serial monitor while running to see the raw mic value, the motor speed (smoothedVal), and potentiometer value
  //  RAW MIC       MOTORSPEED          POT VALUE //////    
//
//  Serial.print(rawData1);
//  Serial.print("           ");
//  Serial.print(smoothedVal);
//  Serial.print("           ");
//  Serial.println(potValue);

///////////////////////////////////////////////////////
// SWITCHING DIRECTION MECHANISM
  if(potValue < clockwiseThreshold && motorDirection==true){ //if pot reads that its reached the edge and the motor is moving clockwise
    digitalWrite(powerRelay, LOW); //turn off power
    delay(1000); //wait for it to slow down
    digitalWrite(directionRelay,LOW); //change direction
    delay(10);
    digitalWrite(powerRelay,HIGH); //turn on motor
    motorDirection = false; //the direction is now anti-clockwise
  }

  if(potValue > antiClockwiseThreshold && motorDirection==false){ //if the pot reaches far left, and motor direction is anti-clockwise
    digitalWrite(powerRelay, LOW); //turn off power
    delay(1000); // wait for it to slow down
    digitalWrite(directionRelay,HIGH); //change direction
    delay(10);
    digitalWrite(powerRelay,HIGH); //turn on motor
    motorDirection = true; //the direction is now clockwise
  }
}

////////////////////////////////////////////
//DIGITAL SMOOTHING FUNCTION

int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  /*
  for (j = 0; j < (filterSamples); j++){    // print the array to debug
   Serial.print(sorted[j]); 
   Serial.print("   "); 
   }
   Serial.println();
   */

  // throw out top and bottom 25% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 25)  / 100), 1); 
  top = min((((filterSamples * 75) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
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

