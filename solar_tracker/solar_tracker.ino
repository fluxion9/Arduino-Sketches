#include <Servo.h> // includes the servo library
Servo myServo; // Creating an instance of the Servo Class called myServo

#define ldr1 A5 //defining A5 as ldr1 pin
#define ldr2 A4 //defining A4 as ldr2 pin


int servoPosition = 20; // Variable that stores servo motor position in the RAM



void setup() {
  myServo.attach(9); // servo will be attached to pin 9
  myServo.write(servoPosition); // set servo(solar tracker) to it's resting position i.e 20 degrees
}

void loop() {
  int ldrVal1 = analogRead(ldr1); // set ldrVal1 to current analog value at ldr1 i.e A5 pin
  int ldrVal2 = analogRead(ldr2);  // set ldrVal2 to current analog value at ldr2 i.e A4 pin
  
  if(ldrVal1 > ldrVal2) // check if ldrVal1 is greater than ldrVal2, if true, then run this block
  {
    servoPosition++; //increment servo position by 1
    servoPosition = constrain(servoPosition, 0, 40); // make sure servoPosition never exceeds the boundaries of 0 and 40 degrees
  }
  if(ldrVal1 < ldrVal2) // check if ldrVal1 is less than ldrVal2, if true, then run this block
  {
    servoPosition--; //decrement servo position by 1
    servoPosition = constrain(servoPosition, 0, 40); // make sure servoPosition never exceeds the boundaries of 0 and 40 degrees
  }
  myServo.write(servoPosition); // set servo to the current value of servoPosition
  delay(15); // wait for 15ms to improve stability
}
