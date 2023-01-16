#include "EEPROM.h"

void setup() {
  Serial.begin(115200);
  if (!EEPROM.begin(1000))
  {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  delay(1000);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  for (int i = 0; i < 1000; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  delay(500);
  digitalWrite(4, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

}
