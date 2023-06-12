#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(A1, A0); // CE, CSN

const byte address[6] = "00001";

String Buf = "";

void setup() {
  Serial.begin(9600);
  Buf.reserve(32);
  if(!radio.begin())
  {
    Serial.println("Failed to start Radio..");
    while(1);
  }
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
}

void loop() {
  if (Serial.available())
  {
    Buf = "";
    while (Serial.available())
    {
      delay(3);
      char d = Serial.read();
      Buf += d;
    }
    Buf.trim();
  }
  if (Buf.length() > 1)
  {
    Serial.print("Got: ");
    Serial.println(Buf);
    int len = Buf.length();
    char text[len + 1];
    for (int i = 0; i < len; i++)
    {
      text[i] = Buf[i];
    }
    text[len] = '\0';
    radio.write(&text, sizeof(text));
    delay(1000);
    Buf = "";
  }
}
