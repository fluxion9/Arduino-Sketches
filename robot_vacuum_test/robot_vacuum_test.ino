#define trig0 7
#define trig1 3
#define trig2 5
#define trig3 22

#define echo0 6
#define echo1 2
#define echo2 4
#define echo3 12

#define motor0_F 8
#define motor0_R 9
#define motor1_F 10
#define motor1_R 11

#define cs 0.0332

String Buffer = "", data = "", mem = "";

byte trigs[4] = {trig0, trig1, trig2, trig3};
byte echos[4] = {echo0, echo1, echo2, echo3};

float distances[4] = {0.0, 0.0, 0.0, 0.0};

class Blinker
{
    int ledPin;
    long onTime;
    long offTime;

    int ledState;
    unsigned long previousMillis;
    unsigned long currentMillis;
  public:
    Blinker(int pin, long on, long off)
    {
      ledPin = pin;
      pinMode(ledPin, OUTPUT);

      onTime = on;
      offTime = off;
      ledState = LOW;
      previousMillis = 0;
    }
    void Update()
    {
      currentMillis = millis();
      if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
      {
        ledState = LOW;
        previousMillis = currentMillis;
        digitalWrite(ledPin, ledState);
      }
      else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime))
      {
        ledState = HIGH;
        previousMillis = currentMillis;
        digitalWrite(ledPin, ledState);
      }
    }
};

Blinker blinkBlue(13, 300, 5000);

long speed = 128;

struct RBV
{
  void init(void)
  {
    Serial1.begin(9600);
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

  void run(void)
  {
    for(int i = 0; i < 4; i++) 
    {
        distances[i] = measureDistance(trigs[i], echos[i]);
    }
    while (Serial1.available() > 0)
    {
      delay(3);
      char c = Serial1.read();
      data += c;
    }
    if (data.length() > 0)
    {
      data.trim();
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[')+1, data.indexOf(']'));
        speed = readStrList(&mem, data, 1).toInt();
      }
      else if (data == "+fwd")
      {
        stop();
        forward();
      }
      else if (data == "+bwd")
      {
        stop();
        backward();
      }
      else if (data == "+tr")
      {
        stop();
        turnRight();
      }
      else if (data == "+tl")
      {
        stop();
        turnLeft();
      }
      else if (data == "+stop")
      {
        stop();
      }
      else if (data == "+read;")
      {
        load_buffer();
        Serial1.println(Buffer);
      }
      data = "";
    }
  }

  void forward()
  {
    analogWrite(motor0_F, speed);
    analogWrite(motor1_F, speed);
  }

  void backward()
  {
    analogWrite(motor0_R, speed);
    analogWrite(motor1_R, speed);
  }

  void turnRight()
  {
    analogWrite(motor0_F, speed);
    analogWrite(motor1_R, speed);
  }

  void turnLeft()
  {
    analogWrite(motor1_F, speed);
    analogWrite(motor0_R, speed);
  }

  void stop()
  {
    analogWrite(motor0_F, 0);
    analogWrite(motor0_R, 0);
    analogWrite(motor1_F, 0);
    analogWrite(motor1_R, 0);
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
    Buffer.concat("}");
  }

}rbv;

void setup() {
  rbv.init();
}

void loop() {
  rbv.run();
}
