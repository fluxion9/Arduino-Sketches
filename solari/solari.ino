#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

#define chipSelect 10

#define Batt A3
#define LDR A2
#define LED 8
#define blue 9

#define on 1
#define off 0

#define lowBatt 6.0


String dataString = "";
String localTime = "";


unsigned long lastDumpTime = 0;
bool onStatus = off, lowStat = false;

int second, minute, hour, day;

byte inputs[2] = {Batt, LDR};


struct Solari
{
  float vBatt = 0, iDis = 0;
  void init()
  {
    for (int i = 0; i < 2; i++)
    {
      pinMode(inputs[i], 0);
    }
    pinMode(LED, 1);
    pinMode(blue, 1);
    dataString.reserve(64);
    while (!SD.begin(chipSelect))
    {
      for (int i = 0; i < 5; i++)
      {
        digitalWrite(blue, 1);
        delay(250);
        digitalWrite(blue, 0);
        delay(250);
      }
      delay(2000);
    }
    if (!ina219.begin()) {
      while (1) {
        for (int i = 0; i < 3; i++)
        {
          digitalWrite(blue, 1);
          delay(250);
          digitalWrite(blue, 0);
          delay(250);
        }
        delay(2000);
      }
    }
    dataString = "Voltage (V),Status,Current (mA),Time Stamp (DD:HH:MM:SS)";
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
    }
    else {
      do {
        for (int i = 0; i < 5; i++)
        {
          digitalWrite(blue, 1);
          delay(100);
          digitalWrite(blue, 0);
          delay(100);
        }
        delay(2000);
        File dataFile = SD.open("datalog.csv", FILE_WRITE);
      } while (!dataFile);
      dataFile.println(dataString);
      dataFile.close();
    }
  }

  void epochToLocal(unsigned long unixEpoch)
  {
    second = unixEpoch % 60;
    minute = (unixEpoch / 60) % 60;
    hour = (unixEpoch / 3600) % 24;
    day = (unixEpoch / 86400) % 365;
    localTime = "";
    localTime.concat(day);
    localTime.concat(":");
    localTime.concat(hour);
    localTime.concat(":");
    localTime.concat(minute);
    localTime.concat(":");
    localTime.concat(second);
  }
  float measureVoltage(int pin, float Max)
  {
    float voltage = analogRead(pin);
    voltage = (voltage * Max) / 1024.0;
    return voltage;
  }

  void takeReadings()
  {
    vBatt = measureVoltage(Batt, 55.0);
    iDis = ina219.getCurrent_mA();
    epochToLocal(millis() / 1000);
    if (vBatt <= 6.0)
    {
      lowStat = true;
    }
  }

  void logData()
  {
    dataString = "";
    dataString.concat(vBatt);
    dataString.concat(",");
    dataString.concat(onStatus);
    dataString.concat(",");
    dataString.concat(iDis);
    dataString.concat(",");
    dataString.concat(localTime);
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
      for (int i = 0; i < 2; i++)
        {
          digitalWrite(blue, 1);
          delay(100);
          digitalWrite(blue, 0);
          delay(100);
        }
    }
    else
    {
      for (int i = 0; i < 5; i++)
      {
        digitalWrite(blue, 1);
        delay(100);
        digitalWrite(blue, 0);
        delay(100);
      }
    }
  }
  void run()
  {
    if (analogRead(LDR) > 1012)
    {
      onStatus = on;
    }
    else {
      onStatus = off;
    }
    digitalWrite(LED, onStatus);
    takeReadings();
    if (millis() - lastDumpTime >= 30000L)
    {
      logData();
      lastDumpTime = millis();
    }
    while (lowStat)
    {
      digitalWrite(LED, 1);
      delay(1000);
      digitalWrite(LED, 0);
      delay(2000);
    }
  }
} solari;

void setup()
{
  solari.init();
}

void loop()
{
  solari.run();
}
