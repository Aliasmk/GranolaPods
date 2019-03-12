//Sets the machine state
void setState(int newState){
  digitalWrite(STATUS_G,LOW);
  digitalWrite(STATUS_R,LOW);
  state = newState;
}

//Asynchronously updates the LED Indicator
void updateStatusIndicator(){
  long int frame = millis();
  
  switch(state){
    case STATE_STOPPED:                 //Stopped: Quick Green Flash
      if(frame%2000 < 50)
        digitalWrite(STATUS_G,HIGH);
      else
        digitalWrite(STATUS_G,LOW);
    break;
    case STATE_MOVING:                  //Moving: Flash Green
      if(frame%1000 < 500)
        digitalWrite(STATUS_G,HIGH);
      else
        digitalWrite(STATUS_G,LOW);
    break; 
      
    case STATE_HALLPOLLING:             //Hall Poll: Red/Green Alternate
      if(frame%500 < 250){
        digitalWrite(STATUS_G,LOW);
        digitalWrite(STATUS_R,HIGH);
      } else {
        digitalWrite(STATUS_G,HIGH);
        digitalWrite(STATUS_R,LOW);
      }
    break;
    case STATE_HALLTRIGGER:             //Hall Trigger: Solid Green
        digitalWrite(STATUS_G,HIGH);
        digitalWrite(STATUS_R,LOW);
    break;
    
    case STATE_SWITCHPOLLING:           //Switch Poll: Flash Red
      if(frame%1000 < 500)
        digitalWrite(STATUS_R,HIGH);
      else
        digitalWrite(STATUS_R,LOW);
    break;
    case STATE_SWITCHTRIGGER:           //Switch Trigger: Solid Red
      digitalWrite(STATUS_R,HIGH);
      digitalWrite(STATUS_G,LOW);
    break;

  }
}
