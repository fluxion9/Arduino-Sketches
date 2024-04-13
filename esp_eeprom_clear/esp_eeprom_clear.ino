#include "EEPROM.h"

#define EEPROM_SIZE 1024


void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.print("Starting EEPROM...");
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("Done!");
  int records = EEPROM.read(0);
  Serial.print("Found ");
  Serial.print(records);
  Serial.println(" records in EEPROM.");
  int readpos = 1;
  char card[9];
  for (int i = 0; i < records; i++) {
      int readlen = EEPROM.read(readpos);
      for (int j = 0; j < readlen; j++) {
        card[j] = (char)EEPROM.read(readpos + j + 1);
      }
      card[8] = '\0';
      Serial.print(i+1);
      Serial.print(".");
      Serial.println(card);
      readpos += 9;
    }
  delay(2000);
  Serial.print("Clearing EEPROM...");
  for(int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("Done!");
}

void loop() {
}
