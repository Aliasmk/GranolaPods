#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

Servo cupdisp;

#define I2C_ADDRESS 0x4
#define MICROSTEPS 1
//#define HALL_SENSOR A0

int state = 0;                  //States are general machine states
#define STATE_STOPPED 0
#define STATE_MOVING 1
#define STATE_SWITCHPOLLING 2
#define STATE_SWITCHTRIGGER 3
#define STATE_HALLPOLLING 4
#define STATE_HALLTRIGGER 5



//General Pin Definitions
#define LIMIT_SWITCH 2
#define STATUS_G 8
#define STATUS_R 9
 
#define STEPPER_DIR 10
#define STEPPER_STEP 16
#define STEPPER_SLEEP 17
#define CW HIGH
#define CCW LOW
#define STEP_PER_REV 200

bool stepperStatus = false;
long int lastStepAt;
int currentSpeed = 0;
int targetSpeed = 0;
bool currentDir = CW;
bool targetDir = CW;

byte command[32];               //All I2C commands recieved from the Pi are [0, command, arg1, arg2, arg3]. Idk why the leading zero.

bool hallSensorTriggered = false;

void setup() {
  //Pin Setup
  pinMode(STATUS_G, OUTPUT);
  pinMode(STATUS_R, OUTPUT);
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH), hitLimitSwitch, FALLING);
  
  //Timer Setup for ~200uS
  TCCR2B = (TCCR2B & B11111000) | 0x02; //Timer2 - 8 divisor
  TIMSK2 = (TIMSK2 & B11111110) | 0x01; //Enable Timer2 interrupt

  setupStepper();
  setupServo();
  setupI2C();
  
  setState(STATE_STOPPED);
}


//Timer2 Interrupt (128us)
ISR(TIMER2_OVF_vect){
  processMovement();
}

/* Main Loop */
void loop() {
  updateStatusIndicator();
  #ifdef HALL_SENSOR
    readHallSensor();
  #endif
}
