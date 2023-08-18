#include <TinyGPSPlus.h>

TinyGPSPlus gps;

#define trig0 13
#define trig1 23
#define trig2 49
#define trig3 24

#define echo0 12
#define echo1 22
#define echo2 48
#define echo3 25

#define motor0_F 8
#define motor0_R 9
#define motor1_F 10
#define motor1_R 11

#define obsThresh 6.0

#define cs 0.0332

String Buffer = "", data = "", mem = "";

bool temp = false;

double latitude = 0.0, longitude = 0.0;

byte trigs[4] = {trig0, trig1, trig2, trig3};
byte echos[4] = {echo0, echo1, echo2, echo3};

float distances[4] = {0.0, 0.0, 0.0, 0.0};

enum motions
{
  idle,
  Forward,
  Backward,
  turnleft,
  turnright
};

int Status = idle;

long speed = 128;

struct RBV
{
  void init(void)
  {
    Serial3.begin(9600);
    Serial.begin(9600);
    Buffer.reserve(94);
    data.reserve(32);
    mem.reserve(20);

    pinMode(motor0_F, 1);
    pinMode(motor0_R, 1);
    pinMode(motor1_F, 1);
    pinMode(motor1_R, 1);
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

  bool isListData(String *data)
  {
    if (data->startsWith("[") && data->endsWith("]"))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  String readStrList(String *memory, String strList, byte position)
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

  void checkObstacle()
  {
    for (int i = 0; i < 4; i++)
    {
      distances[i] = measureDistance(trigs[i], echos[i]);
    }
  }

  void readGPS()
  {
    while (Serial3.available() > 0)
    {
      gps.encode(Serial3.read());
      if (gps.location.isUpdated())
      {
        getData();
      }
      else
      {
        if (!temp)
        {
          getData();
          temp = true;
        }
      }
    }
  }

  void getData()
  {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  }

  void run(void)
  {
    checkObstacle();
    readGPS();
    react();
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
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        speed = readStrList(&mem, data, 1).toInt();
      }
      else if (data == "+fwd;")
      {
        forward();
      }
      else if (data == "+bwd;")
      {
        backward();
      }
      else if (data == "+tr;")
      {
        turnRight();
      }
      else if (data == "+tl;")
      {
        turnLeft();
      }
      else if (data == "+stop;")
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

  void forward()
  {
    checkObstacle();
    if (distances[0] < obsThresh)
    {
      stop();
    }
    else
    {
      stop();
      analogWrite(motor0_F, speed);
      analogWrite(motor1_F, speed);
      Status = Forward;
    }
  }

  void react()
  {
    checkObstacle();
    if (Status == Forward && distances[0] < obsThresh)
    {
      stop();
    }
    else if (Status == Backward && distances[1] < obsThresh)
    {
      stop();
    }
  }

  void backward()
  {
    checkObstacle();
    if (distances[1] < obsThresh)
    {
      stop();
    }
    else
    {
      stop();
      analogWrite(motor0_R, speed);
      analogWrite(motor1_R, speed);
      Status = Backward;
    }
  }

  void turnRight()
  {
    stop();
    analogWrite(motor0_F, speed);
    analogWrite(motor1_R, speed);
    Status = turnright;
  }

  void turnLeft()
  {
    stop();
    analogWrite(motor1_F, speed);
    analogWrite(motor0_R, speed);
    Status = turnleft;
  }

  void stop()
  {
    analogWrite(motor0_F, 0);
    analogWrite(motor0_R, 0);
    analogWrite(motor1_F, 0);
    analogWrite(motor1_R, 0);
    Status = idle;
  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"front\":");
    Buffer.concat(distances[0]);
    Buffer.concat(",\"back\":");
    Buffer.concat(distances[1]);
    Buffer.concat(",\"left\":");
    Buffer.concat(distances[2]);
    Buffer.concat(",\"right\":");
    Buffer.concat(distances[3]);
    Buffer.concat(",\"speed\":");
    Buffer.concat(speed);
    Buffer.concat(",\"lng\":");
    Buffer.concat(longitude);
    Buffer.concat(",\"lat\":");
    Buffer.concat(latitude);
    Buffer.concat(",\"stat\":");
    Buffer.concat(Status);
    Buffer.concat("}");
  }

} rbv;

void setup()
{
  rbv.init();
}

void loop()
{
  rbv.run();
}
