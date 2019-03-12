/* Serial Comms */

void setupI2C(){
  //I2C Connection Setup
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(getSerialMessage);
  Wire.onRequest(sendSerialMessage);
  
  //Flash I2C Address
  digitalWrite(STATUS_R,LOW);
  for(int i = 0; i<I2C_ADDRESS; i++){
    digitalWrite(STATUS_G,HIGH);
    delay(100);
    digitalWrite(STATUS_G,LOW);
    delay(200);
  }
}

//Callback function for when the device detects i2c data
void getSerialMessage(int numBytes){
  int i = 0;
  while(Wire.available()){
    command[i++] = (byte)Wire.read(); //must be casted or else it throws mad problems.
  }

  switch (command[1]){
    case 0x1:     //Free move stepper
      setState(STATE_MOVING);
      if(command[2] == 1)
        stepperStart(command[3]); //1 on cmd2 indicates ccw
      else
        stepperStart(-1*command[3]); //2 on cmd2 indicates cw
    break;
    case 0x4:     //Stop stepper
      setState(STATE_STOPPED);
      stepperStop();
    break;
    case 0x5:     //Wait for Limit Switch
      setState(STATE_SWITCHPOLLING);
    break;
    case 0x11:    //Wait for Hall Sensor
      setState(STATE_HALLPOLLING);
    break;
    case 0x20:    //Dispense Cup
      //setState(STATE_MOVING);
      cupDispenseReady = true;
      //setState(STATE_STOPPED);
    break;
  }
}

//Callback function when i2c data is requested
void sendSerialMessage(){
  byte reply[32];

  for(int i = 0; i<32; i++){
    reply[i] = 0;
  }
  reply[0] = command[1];
  reply[1] = 0;
  
  if(command[1] == 0xAA){   //Request for Info
    switch(command[2]){
      case 0x5:     //Gantry Calibration
        if(state == STATE_SWITCHTRIGGER){
          reply[1] = 1;
          switchTriggered = false;
        } 
      break;
      case 0x11:    //Request Hall
        if(state == STATE_HALLTRIGGER)
          reply[1] = 1;
          hallSensorTriggered = false;
      break;  
    }
  }

  for(int i = 0; i<32; i++){
    Wire.write(reply[i]);
  }
}
