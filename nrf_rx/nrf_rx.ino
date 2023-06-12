#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(3, 2); // CE, CSN

const byte address[6] = "00001";

char text[32] = "";
String str = "";

void setup() {
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
  str.reserve(64);
  pinMode(6, 1);
}

void loop() {
  if (radio.available()) {
    radio.read(&text, sizeof(text));
    str = String(text);
    str.trim();
  }
  if(str.length() > 1)
  {
    if(str == "/on/pr2")
    {
      digitalWrite(6, 1);
    }
    else if(str == "/off/pr2")
    {
      digitalWrite(6, 0);
    }
    else if(str == "/on/all")
    {
      digitalWrite(6, 1);
    }
    else if(str == "/off/all")
    {
      digitalWrite(6, 0);
    }
    str = "";
  }
}
