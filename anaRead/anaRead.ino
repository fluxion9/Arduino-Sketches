#include <SPI.h>
#include <SD.h>
#define chipSelect 8
void setup() {
  if (!SD.begin(chipSelect)) {
    while (1);
  }

}

void loop() {
  File dataFile = SD.open("analog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("A0: " + String(analogRead(A0)));
    dataFile.println("A1: " + String(analogRead(A1)));
    dataFile.println("A2: " + String(analogRead(A2)));
    dataFile.println("A3: " + String(analogRead(A3)));
    dataFile.println("A4: " + String(analogRead(A4)));
    dataFile.println("A5: " + String(analogRead(A5)));
    dataFile.println();
    dataFile.close();
  }
  else {
    ;
  }
 delay(1000);
}
