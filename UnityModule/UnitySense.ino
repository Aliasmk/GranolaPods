//Button Interrupt
void hitLimitSwitch(){
  stepperDisable();
  switchTriggered = true;
  setState(STATE_SWITCHTRIGGER);
}

#ifdef HALL_SENSOR
void readHallSensor(){
  int reading = analogRead(HALL_SENSOR);
  if(reading < 250 || reading > 750){
    hallSensorTriggered == true;
    state = STATE_HALLTRIGGER;
  } 
}
#endif
