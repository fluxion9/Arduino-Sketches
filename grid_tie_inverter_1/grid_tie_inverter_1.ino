#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

RF24 radio(A1, A0); // CE, CSN
const byte address[6] = "00001";

class Blinker
{
    int ledPin;
    int gndPin;
    long onTime;
    long offTime;

    int ledState;
    unsigned long previousMillis;
    unsigned long currentMillis;
  public:
    Blinker(int pin, int gnd, long on, long off)
    {
      ledPin = pin;
      gndPin = gnd;
      pinMode(ledPin, OUTPUT);
      pinMode(gndPin, 1);
      digitalWrite(gndPin, 0);

      onTime = on;
      offTime = off;
      ledState = LOW;
      previousMillis = 0;
    }
    void Update()
    {
      currentMillis = millis();
      if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
      {
        ledState = LOW;
        previousMillis = currentMillis;
        digitalWrite(ledPin, ledState);
      }
      else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime))
      {
        ledState = HIGH;
        previousMillis = currentMillis;
        digitalWrite(ledPin, ledState);
      }
    }
};

Blinker led(6, 8, 300, 3000);

String Buffer = "", data = "", mem = "";
unsigned long lastSendTime = 0, lastDisplayTime = 0;

struct GTI
{
  void init()
  {
    TCCR2B = TCCR2B & B11111000 | B00000010;
    analogWrite(3, 128);
    lcd.init();
    lcd.backlight();
  }

  void initRadio()
  {
    if(!radio.begin())
    {
      while(1);
    }
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MAX);
    radio.stopListening();
  }

  void sendToRadio(String command)
  {
    int len = command.length();
    char text[len + 1];
    for (int i = 0; i < len; i++)
    {
      text[i] = Buf[i];
    }
    text[len] = '\0';
    radio.write(&text, sizeof(text));
  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"grid\":");
    Buffer.concat(voltage);
    Buffer.concat(",\"ivtr\":");
    Buffer.concat(current);
    Buffer.concat(",\"curr\":");
    Buffer.concat(power);
    Buffer.concat(",\"powr\":");
    Buffer.concat(power);
    Buffer.concat(",\"enrg\":");
    Buffer.concat(String(energy, 5));
    Buffer.concat(",\"ps\":");
    Buffer.concat(isOutputOn);
    Buffer.concat("}");
  }

  void sendData()
  {
    if (millis() - lastSendTime >= 1500)
    {
      load_buffer();
      Serial.println(Buffer);
      lastSendTime = millis();
    }
  }

  void display(int mode)
  {
    if (millis() - lastDisplayTime >= 1000)
    {

      lastDisplayTime = millis();
    }
  }

  void run()
  {
  }
};


void setup() {
  TCCR2B = TCCR2B & B11111000 | B00000010;
  analogWrite(3, 128);
}

void loop() {
  led.Update();
}
