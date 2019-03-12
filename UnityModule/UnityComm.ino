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

void processI2C(){
  Serial.print("RX: [");
  Serial.print(command[0]);
  for(int i = 1; i<5; i++){
    Serial.print(", ");
    Serial.print(command[i]);
  }
  Serial.println("]");

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
      cupDispenseReady = true;
    break;
  }
}

void sendI2CReply(){
 

  Serial.print("TX: [");
  Serial.print((byte)reply[0]);
  for(int i = 1; i<4; i++){
    Serial.print(", ");
    Serial.print((byte)reply[i]);
  }
  Serial.println("]");
  
}

//Callback function for when the device detects i2c data
void getSerialMessage(int numBytes){
  int i = 0;
  while(Wire.available()){
    command[i] = (byte)Wire.read(); //must be casted or else it throws mad problems.
    i++;
  }
  i2c_data_ready = true;
}

//Callback function when i2c data is requested
void sendSerialMessage(){
  int result = 0;

  if(command[1] == 0xAA){   //Request for Info
    switch(command[2]){
      case 0x5:     //Gantry Calibration
        if(switchTriggered){
          result = 1;
          switchTriggered = false;
        } 
      break;
      case 0x11:    //Request Hall
        if(hallSensorTriggered){
          result = 1;
          hallSensorTriggered = false;
        }
      break;  
    }
  }
  
  //reply[0] = 0x0;
  //reply[1] = 0x7;
  reply[0] = command[1];
  reply[1] = result;  //Wow, setting the first byte to zeroand or making the second byte not 0x7 makes it disappear from the bus
  
  for(int i = 0; i < 2; i++){
    Wire.write((byte)reply[i]);
  }

  i2c_data_request = true;
}
