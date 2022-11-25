#include <Servo.h>

#define topThresh 4.0
#define frontThresh 60.0
#define motor 9
#define led_r 10
#define trig0 7
#define trig1 6
#define echo0 8
#define echo1 5
#define reset A5

#define openPos 90
#define closePos 6

#define closed 0
#define opened 1

#define cs 0.0332

Servo servo;

struct AutoTrash
{
  enum states {bin_full = 200};
  bool binState = closed, locked = false, sms = false;
  String Hannahs_phone = "+2349074369133", devs_phone = "+2347089182147";
  void init(void)
  {
    servo.attach(motor);

    servo.write(closePos);
    
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
    digitalWrite(led_r, 0);
    delay(10000);
    digitalWrite(led_r, 1);
    Serial.begin(9600);
    
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

  void sendSMS(String phoneNumber)
  {
    Serial.println("AT");
    delay(1000);
    Serial.println("AT+CMGF=1");
    delay(1000);
    Serial.println("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    Serial.print("Trash can is full at Hannah Orakwelu's place, please come and empty it. Thank you.");
    delay(1000);
    Serial.write(26);
    delay(1000);
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
      digitalWrite(led_r, 0);
      if(!sms)
      {
        sendSMS(devs_phone);
        delay(5000);
        sendSMS(Hannahs_phone);
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
