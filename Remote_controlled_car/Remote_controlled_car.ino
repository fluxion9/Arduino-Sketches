#define m0f 10
#define m0r 11
#define m1f 3
#define m1r 9

String data = "";
int speed = 200;

void setup() {
  Serial.begin(9600);
  pinMode(m0f, OUTPUT);
  pinMode(m0r, OUTPUT);
  pinMode(m1f, OUTPUT);
  pinMode(m1r, OUTPUT);
  delay(2000);
}

void loop() {
  while (Serial.available() > 0)
  {
    delay(3);
    char c = Serial.read();
    data += c;
  }
  if (data.length() > 0)
  {
    data.trim();
    if (isDigit(data.charAt(0)))
    {
      speed = data.toInt();
      speed = (speed * 255) / 10;
    }
    else if (data == "F")
    {
      stop();
      forward();
    }
    else if (data == "B")
    {
      stop();
      backward();
    }
    else if (data == "R")
    {
      stop();
      turnRight();
    }
    else if (data == "L")
    {
      stop();
      turnLeft();
    }
    else if (data == "S")
    {
      stop();
    }
    else if (data == "q")
    {
      speed = 255;
    }
    data = "";
  }
}

void forward()
{
  stop();
  analogWrite(m0f, speed);
  analogWrite(m1f, speed);
}

void backward()
{
  stop();
  analogWrite(m0r, speed);
  analogWrite(m1r, speed);
}

void turnLeft()
{
  stop();
  analogWrite(m0r, speed);
  analogWrite(m1f, speed);
}

void turnRight()
{
  stop();
  analogWrite(m0f, speed);
  analogWrite(m1r, speed);
}

void stop()
{
  analogWrite(m0f, 0);
  analogWrite(m0r, 0);
  analogWrite(m1f, 0);
  analogWrite(m1r, 0);
}
