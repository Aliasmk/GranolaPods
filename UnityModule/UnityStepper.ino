void setupStepper(){
  //Stepper Motor Setup
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);
  pinMode(STEPPER_SLEEP, OUTPUT);
  stepperDisable();
}

void processMovement(){
  if(stepperStatus && micros() - lastStepAt > RPMtoStepDelay(currentSpeed)){
    stepperStep();
  } 
}

//Convert an RPM to step delay in uS. 
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
