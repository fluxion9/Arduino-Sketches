// unsigned long time_us = 0;

void setup() {
  Serial.begin(9600); //Begin Serial Communication at 9600 Bauds
  // pinMode(9, 1); // Set Trig pin as output
  // pinMode(10, 0); // Set Echo Pin as input
};

void loop() {
  float adcVal = analogRead(34);
  float voltage = (adcVal * 3.3) / 4095;
  Serial.println(voltage);
  // digitalWrite(9, 0); // pull trig pin low
  // digitalWrite(9, 1); // Start a pulse by pulling trig pin High
  // delayMicroseconds(10); // delay for pulse width (or duration) = 10uS
  // digitalWrite(9, 0); // //End the pulse by pulling trig pin low
  // time_us = pulseIn(10, HIGH); // wait and measure the response pulse from the sensor
  // Serial.print(time_us); //print the time measure
  // Serial.println(" uS"); // print the unit and go to a new line
  // time_us = time_us / 2;
  // float distance = 0.0332f * time_us;
  // Serial.print(distance);
  // Serial.println(" cm");
  delay(1000); //wait for 1 second (1000 milliseconds) before restarting the loop
}
