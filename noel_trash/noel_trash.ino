#include <Servo.h>

#define topThresh 4.0
#define frontThresh 30.0

#define motor 9

#define trig0 6
#define trig1 8

#define echo0 5
#define echo1 7

#define openPos 180
#define closePos 0

#define closed 0
#define opened 1

#define cs 0.0332

Servo servo;

String data = "";

struct AutoTrash
{
  bool binState = closed;

  void init(void)
  {
    servo.attach(motor);

    servo.write(closePos);

    Serial.begin(9600);
    pinMode(2, 1);
    for(int i = 0; i < 3; i++)
    {
      digitalWrite(2, 1);
      delay(300);
      digitalWrite(2, 0);
      delay(300);
    }
  }

  float checkBin(void)
  {
    float distance = measureDistance(trig0, echo0);
    distance = fmap(distance, 18.0, topThresh, 0, 100);
    if(distance < 0)
    {
      return 0;
    }
    else {
      return distance;
    }
  }

  bool checkPresence(void)
  {
    float distance = measureDistance(trig1, echo1);
    if (distance <= frontThresh)
    {
      return true;
    }
    else {
      return false;
    }
  }
  float measureDistance(byte trig, byte echo)
  {
    unsigned long t;
    pinMode(trig, 1);
    pinMode(echo, 0);
    digitalWrite(trig, 0);
    digitalWrite(trig, 1);
    delayMicroseconds(10);
    digitalWrite(trig, 0);
    t = pulseIn(echo, 1);
    t /= 2;
    return cs * t;
  }
  void openBin()
  {
    if (binState == closed)
    {
      servo.write(openPos);
      delay(15);
      binState = opened;
    }
  }

  void closeBin()
  {
    if (binState == opened)
    {
      servo.write(closePos);
      delay(15);
      binState = closed;
    }
  }
  void wait(uint16_t period)
  {
    delay(period * 1000);
  }
  void run()
  {
    if (checkPresence())
    {
      openBin();
      wait(5);
      closeBin();
    }
    while (Serial.available() > 0)
    {
      delay(3);
      char c = Serial.read();
      data += c;
    }
    if (data.length() > 0)
    {
      data.trim();
      if (data == "+read;")
      {
        Serial.println(checkBin());
      }
      data = "";
    }
  }
  float fmap(float val, float fromLow, float fromHigh, float toLow, float toHigh)
  {
    float norm = (val - fromLow) / (fromHigh - fromLow);
    float lerp = norm * (toHigh - toLow) + toLow;
    return lerp;
  }
} autoTrash;
void setup() {
  autoTrash.init();
}
void loop() {
  autoTrash.run();
}
