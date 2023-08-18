#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

#define chipSelect 10

#define Batt A2
#define LDR A3
#define LED 8

#define on 1
#define off 0



String dataString = "";
String localTime = "";

unsigned long lastDumpTime = 0;
bool onStatus = off;

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
    dataString.reserve(64);
    while (!SD.begin(chipSelect))
    {
    }
    if (!ina219.begin()) {
      while (1) {
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
    }
  delay(1500);
}

bool isDark()
{
  if (analogRead(LDR) <= 150)
  {
    return 1;
  }
  else {
    return 0;
  }
}

void epochToLocal(unsigned long unixEpoch)
{
  long second = unixEpoch % 60;
  long minute = (unixEpoch / 60) % 60;
  long hour = (unixEpoch / 3600) % 24;
  long day = (unixEpoch / 86400) % 365;
  localTime = "";
  localTime += String(day) + ':';
  localTime += String(hour) + ':';
  localTime += String(minute) + ':';
  localTime += String(second);
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
  }
  else
  {
  }
}

void run()
{
  if (isDark())
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
