#include <Servo.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <QMC5883LCompass.h>

#define GPSRXPin A3
#define GPSTXPin 9

#define GPSBaud 4800

#define topThresh 4.0
#define frontThresh 60.0

#define openPos 90
#define closePos 6

#define m0f 11
#define m0r 10
#define m1f 12
#define m1r 13

#define trig0 A1
#define trig1 7
#define echo0 A0
#define echo1 8

#define closed 0
#define opened 1

#define servomotor 3

#define cs 0.0332 // cm/uS

Servo servo;
TinyGPSPlus gpsr;
SoftwareSerial gps(GPSRXPin, GPSTXPin);
QMC5883LCompass compass;

String Buffer = "", data = "", mem = "";
String ltd = "", lgd = "";

struct ATC
{
  enum states {bin_full = 200};
  enum motions {idle, stopped, Forward, Backward, turnleft, turnright};
  bool binState = closed, locked = false, gpsStat = false;
  int status = idle, azimuthal = 0, bearing = 0, comp_x, comp_y, comp_z;

  void init(void)
  {
    Serial.begin(9600);
    gps.begin(GPSBaud);
    compass.init();
    pinMode(m0f, OUTPUT);
    pinMode(m0r, OUTPUT);
    pinMode(m1f, OUTPUT);
    pinMode(m1r, OUTPUT);
    stop();

    servo.attach(servomotor);
    servo.write(closePos);

    Buffer.reserve(100);
    data.reserve(100);
    mem.reserve(100);
    ltd.reserve(10);
    lgd.reserve(10);
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

  void readGPS()
  {
    unsigned long Time = millis(), timeout = 1500;
    while (true)
    {
      while (gps.available() > 0)
      {
        if (gpsr.encode(gps.read()))
        {
          if (gpsr.location.isValid())
          {
            gpsStat = true;
            ltd = String(gpsr.location.lat(), 6);
            lgd = String(gpsr.location.lng(), 6);
          }
          else {
            gpsStat = false;
          }
        }
      }
      if (millis() - Time >= timeout)
      {
        return;
      }
    }
  }

  int readCompass()
  {
    compass.read();
    azimuthal = compass.getAzimuth();
    bearing = compass.getBearing(azimuthal);
    comp_x = compass.getX();
    comp_y = compass.getY();
    comp_z = compass.getZ();
    return azimuthal;
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

  void forward()
  {
    stop();
    digitalWrite(m0f, 1);
    digitalWrite(m1f, 1);
    status = Forward;
  }

  void backward()
  {
    stop();
    digitalWrite(m0r, 1);
    digitalWrite(m1r, 1);
    status = Backward;
  }

  void turnRight()
  {
    stop();
    digitalWrite(m0f, 1);
    digitalWrite(m1r, 1);
    status = turnright;
  }

  void turnLeft()
  {
    stop();
    digitalWrite(m1f, 1);
    digitalWrite(m0r, 1);
    status = turnleft;
  }

  void stop()
  {
    digitalWrite(m0f, 0);
    digitalWrite(m0r, 0);
    digitalWrite(m1f, 0);
    digitalWrite(m1r, 0);
    status = stopped;
  }

  void turnCW(int angle)
  {
    int initAngle = readCompass();
    turnRight();
    while(abs(readCompass() - initAngle) < angle);
    stop();
  }

  void turnCCW(int angle)
  {
    int initAngle = readCompass();
    turnLeft();
    while(abs(readCompass() - initAngle) < angle);
    stop();
  }

  void react()
  {

  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"stat\":");
    Buffer.concat(status);
    Buffer.concat("\"gStat\":");
    Buffer.concat(gpsStat);
    Buffer.concat(",\"lat\":");
    Buffer.concat(ltd);
    Buffer.concat(",\"lng\":");
    Buffer.concat(lgd);
    Buffer.concat(",\"azm\":");
    Buffer.concat(azimuthal);
    Buffer.concat(",\"brg\":");
    Buffer.concat(bearing);
    Buffer.concat(",\"cx\":");
    Buffer.concat(comp_x);
    Buffer.concat(",\"cy\":");
    Buffer.concat(comp_y);
    Buffer.concat(",\"cz\":");
    Buffer.concat(comp_z);
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

  void wait(uint16_t period)
  {
    delay(period * 1000);
  }

  void run()
  {
    readCompass();
    while (Serial.available() > 0)
    {
      delay(3);
      char c = Serial.read();
      data += c;
    }
    if (data.length() > 0)
    {
      data.trim();
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[')+1, data.indexOf(']'));
        String command = readStrList(&mem, data, 1);
        if(command == "cw")
        {
          int val = readStrList(&mem, data, 2).toInt();
          turnCW(val);
        }
        if(command == "ccw")
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
      else if (data == "+read;")
      {
        load_buffer();
        Serial.println(Buffer);
      }
      data = "";
    }
  }
}atc;

void setup()
{
  atc.init();
}

void loop()
{
  atc.run();
}
