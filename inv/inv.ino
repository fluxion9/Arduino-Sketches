int i, period = 50, duty;

void setup() {
  pinMode(3, 1);
  pinMode(A4, 0);
//  TCCR2B = TCCR2B & B11111000 | B00000010;
  //  for(i = 0; i < 3; i++)
  //  {
  //    digitalWrite(A5, 1);
  //    delay(500);
  //    digitalWrite(A5, 0);
  //    delay(500);
  //  }
  //  digitalWrite(A5, 1);
}

void loop() {
//  int val = constrain(analogRead(4), 0, 254);
//  analogWrite(3, val);
  //  for(i = 0; i < 255; i++)
  //  {
  //    analogWrite(3, i);
  //    delay(100);
  //  }
  //  for(i = 254; i >= 0; i--)
  //  {
  //    analogWrite(3, i);
  //    delay(100);
  //  }
  //  for(i = 0; i < 255; i++)
  //  {
  //    analogWrite(9, i);
  //    delayMicroseconds(20);
  //  }
  //  for(i = 254; i >= 0; i--)
  //  {
  //    analogWrite(9, i);
  //    delayMicroseconds(20);
  //  }
  //  for(i = 0; i < 255; i++)
  //  {
  //    analogWrite(10, i);
  //    delayMicroseconds(20);
  //  }
  //  for(i = 254; i >= 0; i--)
  //  {
  //    analogWrite(10, i);
  //    delayMicroseconds(20);
  //  }
  duty = readDutyPot();
  oscout(duty);
}

int readDutyPot()
{
  int val = map(analogRead(A4), 0, 1023, 0, 19);
  return val;
}

void oscout(int period)
{
  for(i=0;i<10;i++)
  {
    digitalWrite(3, 1);
    delayMicroseconds(period);
    digitalWrite(3, 0);
    delayMicroseconds(20 - period);
  }
}
