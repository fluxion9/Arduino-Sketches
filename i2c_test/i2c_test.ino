#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial debug(6, 7);
String buf;
void setup() {
  Wire.begin();
  debug.begin(9600);
}

void loop() {
  Wire.requestFrom(53, 6); 
  while(Wire.available())
  {
    char d = Wire.read();
    buf += d;
  }
  if(buf.length() > 1)
  {
    buf = buf.substring(0, buf.indexOf(';'));
    debug.print("Got: ");
    debug.println(buf);
  }
  buf = "";
  delay(1000);
}
