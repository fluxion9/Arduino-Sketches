#include <SoftwareSerial.h>

// The serial connection to the GPS module
//SoftwareSerial ss(4, 3);

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
//  ss.begin(9600);
}

void loop(){
  while (Serial1.available() > 0){
    // get the byte data from the GPS
    byte gpsData = Serial1.read();
    Serial.write(gpsData);
  }
}
