#include <rdm6300.h>

void setup() {
  Serial.begin(9600);
  rdm6300.begin(6);
  Serial.println("\nPlace RFID tag near the rdm6300...");
}

void loop() {
  if (rdm6300.update())
    Serial.println(rdm6300.get_tag_id(), HEX);
  delay(10);
}
