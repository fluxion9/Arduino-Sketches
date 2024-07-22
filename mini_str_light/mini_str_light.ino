#define ldrpin A0
#define outpin 9
#define pirpin 10

#define mid 128
#define full 255

unsigned long lastMotionTime = 0;

void setup() {
  pinMode(ldrpin, 0);
  pinMode(outpin, 1);
  pinMode(pirpin, 0);
  Serial.begin(9600);
}

void loop() {
  if(analogRead(ldrpin) >= 900)
  {
    if(millis() - lastMotionTime <= 10000)
    {
      analogWrite(outpin, full);
    }
    else {
      analogWrite(outpin, mid);
    }
    if(digitalRead(pirpin))
    {
      lastMotionTime = millis();
    }
  }
  else {
    analogWrite(outpin, 0);
  }
}
