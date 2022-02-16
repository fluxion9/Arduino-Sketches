#include <SPI.h>
#include <SD.h>
#define chipSelect = 8;

void setup() {
  if (!SD.begin(chipSelect)) {
    while (1);
  }
}

void loop() {
  String dataString = "";
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }
}
