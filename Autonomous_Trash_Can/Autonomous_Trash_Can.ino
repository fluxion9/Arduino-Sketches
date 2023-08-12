#include <Servo.h>
#include <Wire.h>
#include <QMC5883LCompass.h>

#define topThresh 4.0
#define frontThresh 60.0

#define openPos 90
#define closePos 0

#define trig0 7
#define trig1 A1
#define echo0 8
#define echo1 A0

#define m0f 11
#define m0r 10
#define m1f 12
#define m1r 13

#define closed 0
#define opened 1

#define servomotorpin 9

#define cs 0.0332

Servo servo;
QMC5883LCompass compass;

enum states {bin_full = 200};

enum motions {idle, stopped, Forward, Backward, turnleft, turnright};

bool binState = closed, Locked = false, gpsStat = false;

int Status = idle, azimuthal = 0;

float dist = 0.0;

String data = "", Buffer = "";
String ltd = "", lgd = "", mem = "";


struct ATC
{
  void init(void)
  {
    Serial.begin(9600);

    pinMode(trig0, 1);
    pinMode(echo0, 0);
    pinMode(trig1, 1);
    pinMode(echo1, 0);

    pinMode(m0f, OUTPUT);
    pinMode(m0r, OUTPUT);
    pinMode(m1f, OUTPUT);
    pinMode(m1r, OUTPUT);
    stop();

    Wire.begin();
    compass.init();

    digitalWrite(trig0, 0);
    digitalWrite(trig1, 0);

    servo.attach(servomotorpin);
    servo.write(closePos);

    data.reserve(64);
    Buffer.reserve(100);

    mem.reserve(100);

    ltd.reserve(15);
    lgd.reserve(15);
  }

  int checkBin(void)
  {
    // float distance = measureDistance(trig0, echo0);
    // if (distance <= topThresh)
    // {
    //   return bin_full;
    // }
    // else {
    //   return 0 - bin_full;
    // }
    return 0 - bin_full;
  }

  bool checkPresence(void)
  {
    float distance = measureDistance(trig1, echo1);
    dist = distance;
    if (distance > 0.0 && distance <= frontThresh)
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
    digitalWrite(trig, 0);
    digitalWrite(trig, 1);
    delayMicroseconds(10);
    digitalWrite(trig, 0);
    t = pulseIn(echo, 1);
    t /= 2;
    d = cs * t;
    return d;
  }

  void openBin()
  {
    if (binState == closed)
    {
      for (int i = servo.read(); i <= openPos; i++)
      {
        servo.write(i);
        delay(15);
      }
      binState = opened;
      while (checkPresence());
    }
  }

  void closeBin()
  {
    if (binState == opened)
    {
      for (int i = servo.read(); i >= closePos; i--)
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

  void readGPS()
  {
    data = "";
    Wire.requestFrom(35, 25);
    while (Wire.available())
    {
      char d = Wire.read();
      data += d;
    }
    if (data.length() > 1)
    {
      data = data.substring(0, data.indexOf(';'));
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        ltd = readStrList(&mem, data, 1);
        lgd = readStrList(&mem, data, 2);
        if (ltd.toFloat() == 0.0 || lgd.toFloat() == 0.0)
        {
          gpsStat = 0;
        }
        else {
          gpsStat = 1;
        }
      }
    }
    data = "";
  }

  int readCompass()
  {
    compass.read();
    azimuthal = compass.getAzimuth();
    return azimuthal;
  }

  void checkSerial()
  {
    if (Serial.available())
    {
      data = Serial.readStringUntil(';');
    }
    if (data.length() > 0)
    {
      data.trim();
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        String command = readStrList(&mem, data, 1);
        if (command == "cw")
        {
          int val = readStrList(&mem, data, 2).toInt();
          turnCW(val);
        }
        if (command == "ccw")
        {
          int val = readStrList(&mem, data, 2).toInt();
          turnCCW(val);
        }
      }
      else if (data == "+fwd")
      {
        forward();
      }
      else if (data == "+bwd")
      {
        backward();
      }
      else if (data == "+tr")
      {
        turnRight();
      }
      else if (data == "+tl")
      {
        turnLeft();
      }
      else if (data == "+stop")
      {
        stop();
      }
      else if (data == "+read")
      {
        load_buffer();
        Serial.println(Buffer);
      }
      data = "";
    }
  }

  void forward()
  {
    stop();
    digitalWrite(m0f, 1);
    digitalWrite(m1f, 1);
    Status = Forward;
  }

  void backward()
  {
    stop();
    digitalWrite(m0r, 1);
    digitalWrite(m1r, 1);
    Status = Backward;
  }

  void turnRight()
  {
    stop();
    digitalWrite(m0f, 1);
    digitalWrite(m1r, 1);
    Status = turnright;
  }

  void turnLeft()
  {
    stop();
    digitalWrite(m1f, 1);
    digitalWrite(m0r, 1);
    Status = turnleft;
  }

  void stop()
  {
    digitalWrite(m0f, 0);
    digitalWrite(m0r, 0);
    digitalWrite(m1f, 0);
    digitalWrite(m1r, 0);
    Status = stopped;
  }

  void turnCW(int angle)
  {
    int initAngle = readCompass();
    turnRight();
    while (abs(readCompass() - initAngle) < angle);
    stop();
  }

  void turnCCW(int angle)
  {
    int initAngle = readCompass();
    turnLeft();
    while (abs(readCompass() - initAngle) < angle);
    stop();
  }

  //  void react()
  //  {
  //
  //  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"stat\":");
    Buffer.concat(Status);
    Buffer.concat(",\"gStat\":");
    Buffer.concat(gpsStat);
    Buffer.concat(",\"lat\":");
    Buffer.concat(ltd);
    Buffer.concat(",\"lng\":");
    Buffer.concat(lgd);
    Buffer.concat(",\"dist\":");
    Buffer.concat(dist);
    Buffer.concat(",\"azm\":");
    Buffer.concat(azimuthal);
    Buffer.concat("}");
  }

  bool isListData(String* data)
  {
    if (data->startsWith("[") && data->endsWith("]"))
    {
      return true;
    }
    else {
      return false;
    }
  }

  String readStrList(String* memory, String strList, byte position)
  {
    byte index = 0;
    *memory = "";
    for (int i = 0; i < strList.length(); i++)
    {
      if (strList[i] == ',')
      {
        index++;
      }
      if (index == position - 1)
      {
        memory->concat(strList[i]);
      }
    }
    if (memory->startsWith(","))
    {
      *memory = memory->substring(memory->indexOf(',') + 1);
    }
    return *memory;
  }

  void run()
  {
    readCompass();
    readGPS();
    checkSerial();
    if (!Locked && checkPresence())
    {
      openBin();
      wait(5);
      closeBin();
      if (checkBin() == bin_full)
      {
        Locked = true;
      }
      else {
        Locked = false;
      }
    }
    else {
      if (checkBin() == bin_full)
      {
        Locked = true;
      }
      else {
        Locked = false;
      }
    }
  }
};

ATC atc;

void setup() {
  atc.init();
}

void loop() {
  atc.run();
}
