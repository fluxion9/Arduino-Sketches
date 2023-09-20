#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EmonLib.h>

#define voltagePin A1
#define currentPin A3
#define isolator A0

LiquidCrystal_I2C lcd(0x27, 20, 4);
EnergyMonitor CTsense;

RF24 radio(9, 10); // CE, CSN
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
Blinker NorBadRadio(6, 8, 200, 400);

String Buffer = "", data = "", mem = "";
unsigned long lastSendTime = 0, lastDisplayTime = 0, lastEnergyRead = 0;

float gridV = 0, invV = 0, current = 0, power = 0, energy = 0;

struct GTI
{
  void init()
  {
    initRadio();
    Serial.begin(9600);
    TCCR2B = TCCR2B & B11111000 | B00000010;
    analogWrite(3, 128);
    lcd.init();
    lcd.backlight();
    CTsense.current(currentPin, 111.1);
    Buffer.reserve(64);
    mem.reserve(20);
    data.reserve(20);
    pinMode(currentPin, 0);
    pinMode(voltagePin, 0);
    pinMode(isolator, 1);
  }

  void initRadio()
  {
    if(!radio.begin())
    {
      while(1) {
        NorBadRadio.Update();
      }
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
      text[i] = command[i];
    }
    text[len] = '\0';
    radio.write(&text, sizeof(text));
  }

  void load_buffer(void)
  {
   Buffer = "";
   Buffer.concat("{\"grid\":");
   Buffer.concat(gridV);
   Buffer.concat(",\"ivtr\":");
   Buffer.concat(invV);
   Buffer.concat(",\"curr\":");
   Buffer.concat(current);
   Buffer.concat(",\"powr\":");
   Buffer.concat(power);
   Buffer.concat(",\"enrg\":");
   Buffer.concat(String(energy, 5));
   Buffer.concat("}");
  }

  float measureVoltageAC()
  {
    float vol = analogRead(voltagePin);
    vol = (vol * 5.0) / 1023.0;
    vol = vol * 101.0;
    gridV = vol / 1.414;
    return gridV;
  }

  float measureCurrentAC()
  {
    return CTsense.calcIrms(1480);
  }

  // bool isListData(String *data)
  // {
  //   if (data->startsWith("[") && data->endsWith("]"))
  //   {
  //     return true;
  //   }
  //   else
  //   {
  //     return false;
  //   }
  // }

  // String readStrList(String *memory, String strList, byte position)
  // {
  //   byte index = 0;
  //   *memory = "";
  //   for (int i = 0; i < strList.length(); i++)
  //   {
  //     if (strList[i] == ',')
  //     {
  //       index++;
  //     }
  //     if (index == position - 1)
  //     {
  //       memory->concat(strList[i]);
  //     }
  //   }
  //   if (memory->startsWith(","))
  //   {
  //     *memory = memory->substring(memory->indexOf(',') + 1);
  //   }
  //   return *memory;
  // }

  void checkSerial()
  {
    if (Serial.available())
    {
      data = "";
      while (Serial.available() > 0)
      {
        delay(3);
        char c = Serial.read();
        data += c;
      }
    }
    if (data.length() > 0)
    {
      data.trim();
      if (data == "+on")
      {
        switchON();
      }
      else if (data == "+off")
      {
        switchOFF();
      }
      else if (data == "+cla")
      {
        clamp();
      }
      else if (data == "+ucla")
      {
        unclamp();
      }
      data = "";
    }
  }

  void switchON()
  {
    sendToRadio("/on/all");
  }

  void switchOFF()
  {
    sendToRadio("/off/all");
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

  void display()
  {
    if (millis() - lastDisplayTime >= 1000)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Grid Tied Inverter ");
      lcd.setCursor(0, 1);
      lcd.print("Voltage: " + String(invV) + " V");
      lcd.setCursor(0, 2);
      lcd.print("Current: " + String(current) + " A");
      lcd.setCursor(0, 3);
      lcd.print("Power: " + String(power) + " W");
      lastDisplayTime = millis();
    }
  }
  void takeReadings()
  {
    invV = measureVoltageAC();
    current = measureCurrentAC();
    power = invV * current;
    if(millis() - lastEnergyRead >= 1000)
    {
      energy += (power / 1000) * 0.00028;
      lastEnergyRead = millis();
    }
  }
   void clamp()
   {
    digitalWrite(isolator, 1);
   }

   void unclamp()
   {
    digitalWrite(isolator, 0);
   }

  void run()
  {
    takeReadings();
    display();
    sendData();
    checkSerial();
    led.Update();
  }
}gti;


void setup() {
  gti.init();
}

void loop() {
  gti.run();
}
