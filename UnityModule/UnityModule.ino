#include <Arduino.h>
#include <Wire.h>

#define GANTRYCART
#define DISPENSER

//Gantry Cart Specific Definitions
#ifdef GANTRYCART
#define I2C_ADDRESS 0xa
#define STATE_STOPPED 0
#define STATE_HOMING 1
#define STATE_HOMED 2
#define STATE_MOVING 3
#endif

//Dispenser Specific Definitions
#ifdef DISPENSER
#define I2C_ADDRESS 0x9
#define HALL_SENSOR A0

#define STATE_IDLE 2
#define STATE_WAIT 3
#define STATE_HOLD 0
#define STATE_DISP 1

bool hallSensorTriggered = false;
#endif

int state = 0;                  //States are general machine states
int substate = 0;               //Substates are sequential steps to take within a state

byte command[32];               //All I2C commands recieved from the Pi are [0, command, arg1, arg2, arg3]. Idk why the leading zero.
        

//General Pin Definitions
#define LIMIT_SWITCH 3
#define STATUS_G 8
#define STATUS_R 9
 
#define STEPPER_DIR 10
#define STEPPER_STEP 16
#define STEPPER_SLEEP 17
#define MICROSTEPS 4
#define CW HIGH
#define CCW LOW
#define STEP_PER_REV 200
bool stepperStatus = false;
long int lastStepAt;
int currentSpeed = 0;
int targetSpeed = 0;
bool currentDir = CW;
bool targetDir = CW;

void setup() {
  //Status LED Setup
  pinMode(STATUS_G, OUTPUT);
  pinMode(STATUS_R, OUTPUT);
  setState(4);
  updateStatusIndicator();

  //E-stop/Limit Switch Interrupt Setup
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH), hitLimitSwitch, FALLING);
  
  //Stepper Motor Setup
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);
  pinMode(STEPPER_SLEEP, OUTPUT);
  stepperDisable();

  //Timer Setup for ~200uS
  TCCR2B = (TCCR2B & B11111000) | 0x02; //Timer2 - 8 divisor
  TIMSK2 = (TIMSK2 & B11111110) | 0x01; //Enable Timer2 interrupt

  //I2C Connection Setup
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(getSerialMessage);
  Wire.onRequest(sendSerialMessage);
  delay(1000);
  setState(2);
}

/* Serial Comms */

//Callback function for when the device detects i2c data
void getSerialMessage(int numBytes){
  //int result = 0;
  int i = 0;
  while(Wire.available()){
    command[i++] = (byte)Wire.read(); //must be casted or else it throws mad problems.
  }


  #ifdef GANTRYCART
  if(command[1] == 0x1){
    setState(STATE_MOVING);
    if(command[2] == 1){
      //1 on cmd2 indicates ccw
      stepperStart(command[3]);
    } else {
      //2 on cmd2 indicates cw
      stepperStart(-1*command[3]);
    }
  } else if(command[1] == 0x4){
      setState(STATE_STOPPED);
      stepperStop();
  } else if(command[1] == 0x5) {
      setState(STATE_HOMING);
  }
  #endif

  
}

//Callback function when i2c data is requested
void sendSerialMessage(){
  byte reply[32];

  reply[0] = command[1];
  reply[1] = 0;
  
  #ifdef GANTRYCART
  if(command[1] == 0xAA && command[2] == 5){ //0xaa: Request for Info 0x5: gantry calibration
    if(state == STATE_HOMED)
      reply[1] = 1;
  }
  #endif

  #ifdef DISPENSER
  if(command[1] == 170 && command[2] == 17){ //Request for hall sensor 
    if(state == STATE_HOLD){
      reply[1] = 1;
    }
  } 
  #endif

  Wire.write(reply[0]);
  Wire.write(reply[1]);
}

//Button Interrupt
void hitLimitSwitch(){
  stepperDisable();
  #ifdef GANTRYCART
    setState(STATE_HOMED);
  #endif
}

//Timer2 Interrupt (128us)
ISR(TIMER2_OVF_vect){
  processMovement();
}

/* Main Loop */
void loop() {
  updateStatusIndicator();

  #ifdef DISPENSER
    readHallSensor();
  #endif
  
}



void processMovement(){
  if(stepperStatus && micros() - lastStepAt > RPMtoStepDelay(currentSpeed)){
    stepperStep();
  } 
}





#ifdef DISPENSER

void readHallSensor(){
  int reading = analogRead(HALL_SENSOR);
  if(reading < 250 || reading > 750){
    hallSensorTriggered == true;
    state = STATE_HOLD;
  } 
}

#endif



//Called when status is changed
void onStateChange(){
  #ifdef GANTRYCART
    if(state == STATE_HOMING){
      stepperStart(-60);
    }
  #endif
}

//Sets the machine state
void setState(int newState){
  digitalWrite(STATUS_G,LOW);
  digitalWrite(STATUS_R,LOW);
  state = newState;
  onStateChange();
}

//Asynchronously updates the LED Indicator
void updateStatusIndicator(){
  long int frame = millis();
  
  switch(state){
  case 2:
  //solid green
      digitalWrite(STATUS_G,HIGH);
      digitalWrite(STATUS_R,LOW);
  break;
  case 3:
  //flash green
    if(frame%1000 < 500){
      digitalWrite(STATUS_G,HIGH);
    } else {
      digitalWrite(STATUS_G,LOW);
    }
  break;   
  case 1:
  //alternate green/red
    if(frame%500 < 250){
      digitalWrite(STATUS_G,LOW);
      digitalWrite(STATUS_R,HIGH);
    } else {
      digitalWrite(STATUS_G,HIGH);
      digitalWrite(STATUS_R,LOW);
    }
     
  break;
  case 0:
  //flash red
    if(frame%1000 < 500){
      digitalWrite(STATUS_R,HIGH);
    } else {
      digitalWrite(STATUS_R,LOW);
    }
  break;
  case 4:
  //solid red
    digitalWrite(STATUS_R,HIGH);
    digitalWrite(STATUS_G,LOW);
  break;
  default:
  //quick flash red
    if(frame%1500 < 100)
      digitalWrite(STATUS_R,HIGH);
    else
      digitalWrite(STATUS_R,LOW);
  }
}


/* STEPPER MOTOR CONTROL */
long int RPMtoStepDelay(double RPM){
  RPM = max(abs(RPM), 2);
  double result = 300000.0/(double)(RPM*MICROSTEPS);
  return (long int)result;
  //300000uS per step for 200step/rev TODO parametric 
}

//Start the stepper motor motion. 
void stepperStart(int stepperRPM){
  stepperEnable();
  targetSpeed = stepperRPM;
}

//Decel stepper to stop
void stepperStop(){
  targetSpeed = 0;  
}

//Perform one step of the stepper. Also accel or decel to target speed.
void stepperStep(){
  //Update the previous step time
  lastStepAt = micros();

  //Accel and Decel to target
  if(currentSpeed < targetSpeed)
    currentSpeed++;
  else if(currentSpeed > targetSpeed)
    currentSpeed--;

  //set direction
  if(currentSpeed < 0 && currentDir == CCW ){
    digitalWrite(STEPPER_DIR,CW);
    currentDir = CW;
  } else if(currentSpeed > 0 && currentDir == CW){
    digitalWrite(STEPPER_DIR,CCW);
    currentDir = CCW;
  }

  //Make step
  digitalWrite(STEPPER_STEP,HIGH); 
  //delayMicroseconds(10);
  digitalWrite(STEPPER_STEP,LOW);

  //Disable if stopped
  if(targetSpeed == 0 && abs(currentSpeed) < 10)
    stepperDisable();
}

//Wake up the stepper motor driver
void stepperEnable(){
  if(!stepperStatus){
    //Wake up the stepper driver before stepping
    digitalWrite(STEPPER_SLEEP,HIGH);
    delay(1); //delay 1ms to allow charge pump to stabilize
    stepperStatus = true;
  }
}

//Put the stepper motor driver to sleep
void stepperDisable(){
  currentSpeed = 0;
  digitalWrite(STEPPER_SLEEP,LOW);
  stepperStatus = false;
  delay(1);
}
