#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

Servo cupdisp;

#define I2C_ADDRESS 0x4
#define MICROSTEPS 1
#define HALL_SENSOR A0

volatile int state = 0;                  //States are general machine states
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
#define CW LOW
#define CCW HIGH
#define STEP_PER_REV 200

volatile bool stepperStatus = false;
volatile long int lastStepAt;
volatile int currentSpeed = 0;
volatile int targetSpeed = 0;

volatile int i2c_data_ready = false;
volatile int i2c_data_request = false;
volatile byte command[5];               //All I2C commands recieved from the Pi are [registerpointer, command, arg1, arg2, arg3].
volatile byte reply[4];

volatile bool hallSensorTriggered = false;
volatile bool switchTriggered = false;
volatile bool cupDispenseReady = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Beginning Startup...");
  
  //Pin Setup
  pinMode(STATUS_G, OUTPUT);
  pinMode(STATUS_R, OUTPUT);
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH), hitLimitSwitch, FALLING);
  Serial.println("Pins Initialized"); 
  
  //Timer Setup for ~200uS
  //TCCR2B = (TCCR2B & B11111000) | 0x02; //Timer2 - 8 divisor
  //TIMSK2 = (TIMSK2 & B11111110) | 0x01; //Enable Timer2 interrupt
  Serial.println("Timer Initialized");
  
  setupStepper();
  Serial.println("Stepper Initialized");
  setupServo();
  Serial.println("Servo Initialized");
  setupI2C();
  Serial.println("I2C Initialized");

  Serial.println("Setup complete");
  setState(STATE_STOPPED);
}


//Timer2 Interrupt (128us)
/*ISR(TIMER2_OVF_vect){
  processMovement();
}*/

/* Main Loop */
void loop() {
  updateStatusIndicator();
  
  #ifdef HALL_SENSOR
   int hallValue = readHallSensor();
  #endif

  if(millis()%2000 == 0){
    Serial.print("Heartbeat. Hall value: ");
    Serial.print(hallValue);
    Serial.print(" (");
    Serial.print(hallSensorTriggered,DEC);
    Serial.println(")");
    
  }

  if(cupDispenseReady){
    dispenseCup();
    cupDispenseReady = false;
  }

  if(i2c_data_ready){
    processI2C();
    i2c_data_ready = false;
  }

  if(i2c_data_request){
    sendI2CReply();
    i2c_data_request = false;
  }

  processMovement();
}
