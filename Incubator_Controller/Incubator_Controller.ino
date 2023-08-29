#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "DHT.h"

#define servoPin 10
#define dhtPin 13
#define lamp 9
#define fan 3
#define dsbPin 5

#define leftAngle 45
#define rightAngle 135

#define dhtType DHT11

#define interval 1800000UL
#define adjustmentSpeed 15


LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(dsbPin);
DallasTemperature t_sense(&oneWire);
DHT dht(dhtPin, dhtType);

Servo servo;

String Buffer = "", data = "", mem = "";

float airTemp = 0.0, trayTemp = 0.0, humidity = 0.0;

float set_temp = 0.0;

byte trayPos = 0;


struct Scroll_W
{
  void init(void)
  {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    servo.attach(servoPin);
  }

  void clearRow(byte row)
  {
    lcd.setCursor(0, row);
    lcd.print("                ");
  }

  void write(int row, int col, String message, int Delay)
  {
    clearRow(row);
    lcd.setCursor(col, row);
    for (int i = 0; i < message.length(); i++)
    {
      lcd.setCursor(col + i, row);
      lcd.print(message[i]);
      delay(Delay);
    }
  }
} sWriter;


struct Incubator
{
  unsigned long lastDisplay = 0, lastSendTime = 0, lastTimeMoved = 0;
  byte paramNum = 0, screenPos = 0;

  void init()
  {
    Serial.begin(9600);
    sWriter.init();
    dht.begin();
    Buffer.reserve(20);
    data.reserve(32);
    mem.reserve(20);
    moveTo(leftAngle);
  }

  void display()
  {
    if (millis() - lastDisplay >= 1000)
    {
      paramNum++;
      screenPos++;

      if (screenPos > 1)
      {
        screenPos = 0;
      }
      else if (screenPos < 0)
      {
        screenPos = 0;
      }

      if (paramNum > 2)
      {
        paramNum = 0;
      }
      else if (paramNum < 0)
      {
        paramNum = 0;
      }

      if (paramNum == 0)
      {
        sWriter.write(screenPos, 0, "Tray Temp: " + String(trayTemp), 50);
      }
      else if (paramNum == 1)
      {
        sWriter.write(screenPos, 0, "Air Temp: " + String(airTemp), 50);
      }
      else if (paramNum == 2)
      {
        sWriter.write(screenPos, 0, "Humidity: " + String(humidity), 50);
      }
      lastDisplay = millis();
    }
  }

  void measureTemperatureAndHumidity()
  {
    t_sense.requestTemperatures();
    trayTemp = t_sense.getTempCByIndex(0);
    trayTemp = fabs(trayTemp);
    airTemp = dht.readTemperature();
    humidity = dht.readHumidity();
  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"tTemp\":");
    Buffer.concat(trayTemp);
    Buffer.concat(",\"aTemp\":");
    Buffer.concat(airTemp);
    Buffer.concat(",\"humd\":");
    Buffer.concat(humidity);
    Buffer.concat("}");
  }

  void sendData()
  {
    if (millis() - lastSendTime > 1500)
    {
      load_buffer();
      Serial.println(Buffer);
      lastSendTime = millis();
    }
  }

  void checkSerial()
  {
    if (Serial.available())
    {
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
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        set_temp = readStrList(&mem, data, 1).toFloat();
      }
      data = "";
    }
  }

  void moveTo(byte pos)
  {
    int posNow = servo.read();
    if (posNow < pos)
    {
      for (int i = posNow; i >= pos; i--)
      {
        servo.write(i);
        delay(adjustmentSpeed);
      }
    }
    else if (posNow > pos)
    {

      for (int i = posNow; i <= pos; i++)
      {
        servo.write(i);
        delay(adjustmentSpeed);
      }
    }
  }

  void adjustTrayPosition()
  {
    if (millis() - lastTimeMoved >= interval)
    {
      if (trayPos == 0)
      {
        moveTo(rightAngle);
        trayPos = 1;
      }
      else if (trayPos == 1)
      {
        moveTo(leftAngle);
        trayPos = 0;
      }
      lastTimeMoved = millis();
    }
  }

  void startHeating()
  {
    digitalWrite(lamp, 1);
  }

  void stopHeating()
  {
    digitalWrite(lamp, 0);
  }

  void adjustTemperature()
  {
    if (airTemp <= set_temp - 0.5)
    {
      startHeating();
    }
    else if (airTemp > set_temp)
    {
      stopHeating();
    }
  }

  bool isListData(String* data)
  {
    if (data->startsWith("[") && data->endsWith("]"))
    {
      return true;
    }
    else {
      return false;
    }
  }

  String readStrList(String* memory, String strList, byte position)
  {
    byte index = 0;
    *memory = "";
    for (int i = 0; i < strList.length(); i++)
    {
      if (strList[i] == ',')
      {
        index++;
      }
      if (index == position - 1)
      {
        memory->concat(strList[i]);
      }
    }
    if (memory->startsWith(","))
    {
      *memory = memory->substring(memory->indexOf(',') + 1);
    }
    return *memory;
  }

  void run()
  {
    measureTemperatureAndHumidity();
    display();
    sendData();
    checkSerial();
    adjustTrayPosition();
    adjustTemperature();
  }
} incubator;

void setup()
{
  incubator.init();
}

void loop()
{
  incubator.run();
}
