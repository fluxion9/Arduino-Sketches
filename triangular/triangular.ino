void setup() {
  // put your setup code here, to run once:
  pinMode(11, 1);
  TCCR2B = TCCR2B & B11111000 | B00000010;
  analogWrite(11, 128);
}

void loop() {
//  for(int i = 0; i < 255; i++)
//  {
//    analogWrite(11, i);
//    delayMicroseconds(2);
//  }
//  for(int i = 255; i >= 0; i--)
//  {
//    analogWrite(11, i);
//    delayMicroseconds(2);
//  }
}
