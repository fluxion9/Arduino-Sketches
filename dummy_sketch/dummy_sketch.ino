#include <SoftwareSerial.h>
SoftwareSerial sim(11, 12);

void setup() {
  sim.begin(9600);
  Serial.begin(9600);
}

String data = "";

void loop() {
  if(Serial.available())
  {
    char c = Serial.read();
    if(c == 's')
    {
      sendSMS("+2347089182147");
    }
  }
}

void sendSMS(String phoneNumber)
  {
    sim.println("AT");
    delay(1000);
    sim.println("AT+CMGF=1");
    delay(1000);
    sim.println("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    sim.print("Trash Can is full at Hannah Orakwelu's place, please come and empty it. Thank you.");
    delay(1000);
    sim.write(26);
    delay(1000);
  }
