void setup() {
  Serial1.begin(9600);
  pinMode(PIN_PE2, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial1.println(analogRead(PIN_PE2));
  delay(500);
}
