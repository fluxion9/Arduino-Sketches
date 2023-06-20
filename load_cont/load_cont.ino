void setup() {
  TCCR2B = TCCR2B & B11111000 | B00000001;
  pinMode(3, 1);
  analogWrite(3, 128);
}

void loop() {
}
