#include <Servo.h>

#define topThresh 4.0
#define frontThresh 30.0
#define motor 3
#define led_r 4
#define trig0 5
#define trig1 6
#define echo0 7
#define echo1 8
#define reset 9

#define openPos 90
#define closePos 0

#define closed 0
#define opened 1

#define cs 0.0332

Servo servo;

struct AutoTrash
{
  enum states {bin_full = 200};
  bool binState = closed, locked = false, sms = false;
  
  void init(void)
  {
    servo.attach(motor);

    servo.write(0);
    
    pinMode(motor, 1);
    pinMode(led_r, 1);
    
    digitalWrite(led_r, 1);
    pinMode(reset, 2);

    for(int i = 0; i < 3; i++)
    {
      digitalWrite(led_r, 0);
      delay(300);
      digitalWrite(led_r, 1);
      delay(300);
    }

    digitalWrite(led_r, 1);
    
  }

  int checkBin(void)
  {
    float distance = measureDistance(trig0, echo0);
    if (distance <= topThresh)
    {
      return bin_full;
    }
    else {
      return 0 - bin_full;
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
    float d;
    pinMode(trig, 1);
    pinMode(echo, 0);

    digitalWrite(trig, 0);
    digitalWrite(trig, 1);
    delayMicroseconds(10);
    digitalWrite(trig, 0);
    t = pulseIn(echo, 1);
    t /= 2;
    d = cs * t;
    return d;
  }

  void sendSMS()
  {
    
  }

  void openBin()
  {
    if(binState == closed)
    {
      for(int i = servo.read(); i <= openPos; i++)
      {
        servo.write(i);
        delay(15);
      }
      binState = opened;
    }
  }

  void closeBin()
  {
    if(binState == opened)
    {
      for(int i = servo.read(); i >= closePos; i--)
      {
        servo.write(i);
        delay(15);
      }
      binState = closed;
    }
  }

  void wait(uint16_t period)
  {
    delay(period * 1000);
  }

  void run()
  {
    while (!locked)
    {
      digitalWrite(led_r, 1);
      if(checkPresence())
      {
        openBin();
        wait(5);
        closeBin();
        if(checkBin() == bin_full)
        {
          locked = true;
        }
        else{
          locked = false;
        }
      }
      else {
        if(checkBin() == bin_full)
        {
          locked = true;
        }
        else{
          locked = false;
        }
      }
    }
    while(locked)
    {
      digitalWrite(led_r, 1);
      if(!sms)
      {
        sendSMS();
        sms = true;
      }
      if(!digitalRead(reset) && binState == closed)
      {
        delay(100);
        openBin();
      }
      if(!digitalRead(reset) && binState == opened)
      {
        delay(100);
        closeBin();
        if(checkBin() == bin_full)
        {
          locked = true;
        }
        else{
          locked = false;
          sms = false;
        }
      }
    }
  }
}autoTrash;
void setup() {
  autoTrash.init();
}

void loop() {
  autoTrash.run();
}
