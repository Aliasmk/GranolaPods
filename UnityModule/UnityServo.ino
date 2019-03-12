void setupServo(){
  cupdisp.attach(3);

  cupdisp.write(5);
  delay(100);
  cupdisp.write(0);
  delay(100);
  cupdisp.write(5);
  delay(100);
  cupdisp.write(0);
}

void dispenseCup(){
  cupdisp.write(90);
  delay(250);
  cupdisp.write(0);
}
