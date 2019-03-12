void setupServo(){
  cupdisp.attach(3);

  /*cupdisp.write(5);
  delay(100);
  cupdisp.write(0);
  delay(100);
  cupdisp.write(5);
  delay(100);
  cupdisp.write(0);*/

  dispenseCup();
}

void dispenseCup(){
  cupdisp.write(90);
  delay(1000);
  cupdisp.write(50);
  delay(1000);
}
