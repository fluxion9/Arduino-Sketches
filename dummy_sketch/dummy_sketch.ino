#include <SoftwareSerial.h>
SoftwareSerial lora(5, 6);

#define M0 3
#define M1 4

void setup() {
  lora.begin(9600);
  Serial.begin(9600);
  pinMode(M0, 1);
  pinMode(M1, 1);
  digitalWrite(M0, 0);
  digitalWrite(M1, 0);
  
}

void loop() {
  if(Serial.available() > 0)
  {
    String input = Serial.readString();
    lora.println(input);
  }
  if(lora.available() > 0)
  {
    String input = lora.readString();
    Serial.println(input);
  }
  delay(20);
}
