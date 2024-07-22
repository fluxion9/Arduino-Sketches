#include <Servo.h>

#define s1 10
#define s2 9
#define s3 11

#define upsensor A3
#define downsensor A0

#define led 8

//servo1 80->0 (ccw) open - close
//servo2 0->80 (ccw) open - close
//servo3 0->70 (ccw) open - close

Servo tank, tank1, valve;

void setup() {
  pinMode(led, 1);
  pinMode(upsensor, 0);
  pinMode(downsensor, 0);
  
  tank.attach(s1);
  tank1.attach(s3);
  valve.attach(s2);

  tank.write(80);
  delay(15);
  tank1.write(70);
  delay(15);
  valve.write(80);
  delay(15);

  delay(2000);
  while(digitalRead(upsensor))
  {
    digitalWrite(led, 1);
    delay(100);
    digitalWrite(led, 0);
    delay(100);
  }
}
void loop() {
  if(!digitalRead(upsensor))
  {
    tank.write(0);
    tank1.write(0);
    delay(100);
  }
  else {
    tank.write(80);
    tank1.write(70);
    delay(100);
  }
  if(!digitalRead(downsensor))
  {
    valve.write(0);
    delay(100);
  }
  else {
    valve.write(80);
    delay(100);
  }
}
