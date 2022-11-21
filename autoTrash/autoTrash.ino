#include <Servo.h>


#define motor
#define led_r
#define trig0
#define trig1
#define echo0
#define echo1
#define reset 

#define cs 0.0332

struct AutoTrash
{
  void init(void)
  {
    Servo servo;
    servo.attach(motor);

    servo.write(0);
    
    pinMode(motor, 1);
    pinMode(led_r, 1);
    
    
    pinMode(reset, 2);

    digitalWrite(led_r, 1);
    
  }

  void measureDistance(byte trig, byte echo)
  {
    pinMode(trig, 1);
    pinMode(echo, 0);

    digitalWrite(trig, 0);
    digitalWrite(trig, 1);
    delayMicroseconds(10);
    digitalWrite(trig, 0);

  }
};
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
