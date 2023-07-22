#include <SPI.h>
#include <SD.h>

#define chipSelect 10

#define Irr A2
#define Vo1 A3
#define Vo2 A0
#define Th1 A4
#define Th2 A5

#define led 5

String dataString = "";
String localTime = "";

unsigned long lastDumpTime = 0;

byte inputs[5] = {Vo1, Vo2, Th1, Th2, Irr};

class Blinker
{
  int ledPin;
  long onTime;
  long offTime;

  int ledState;
  unsigned long previousMillis;
  unsigned long currentMillis;

public:
  Blinker(int pin, long on, long off)
  {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);

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
Blinker Blink(led, 300, 1500);

struct Report
{
  float tHS, tCS, tDiff, vPV, vTEG, irrad;
  void init()
  {
    pinMode(led, 1);
    for (int i = 0; i < 5; i++)
    {
      pinMode(inputs[i], 0);
      digitalWrite(led, 1);
      delay(250);
      digitalWrite(led, 0);
      delay(250);
    }
    dataString.reserve(64);
    while (!SD.begin(chipSelect))
    {
      digitalWrite(led, 1);
      delay(2000);
    }
    digitalWrite(led, 0);
    dataString = "T_HS (*C),T_CS (*C),T_Diff (*C),vTEG (V),vPV (V),Irradiance (W/M^2),Time Stamp (DD:HH:MM:SS)";
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
      blynk(led, 2);
    }
    else
    {
      blynk(led, 3);
    }
    delay(1500);
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

  float measureTemp(int pin, float R1)
  {
    float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
    float Vo = analogRead(pin);
    float R2 = R1 / ((1023.0 / Vo) - 1.0);
    float logR2 = log(R2);
    float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
    float Tc = T - 273.15;
    return Tc;
  }

  float measureVoltage(int pin, float Max)
  {
    float voltage = analogRead(pin);
    voltage = (voltage * Max) / 1024.0;
    return voltage;
  }

  float measureIrradians()
  {
    float voltage = analogRead(Irr);
    voltage = (voltage * 5.0) / 1023.0; // volts
    float illuminance = 2500 / voltage;
    illuminance -= 500;
    illuminance /= 10; // lux
    float irradiance = illuminance / 120.0;
    return irradiance;
  }
  void takeReadings()
  {
    tHS = measureTemp(Th1, 10000);
    tCS = measureTemp(Th2, 10000);
    tDiff = tHS - tCS;
    vTEG = measureVoltage(Vo1, 10.0);
    vPV = measureVoltage(Vo2, 55.0);
    irrad = measureIrradians();
    epochToLocal(millis() / 1000);
  }

  void blynk(byte stuff, int num)
  {
    for (int i = 0; i < num; i++)
    {
      digitalWrite(stuff, 1);
      delay(200);
      digitalWrite(stuff, 0);
      delay(200);
    }
  }

  void logData()
  {
    dataString = "";
    dataString.concat(tHS);
    dataString.concat(",");
    dataString.concat(tCS);
    dataString.concat(",");
    dataString.concat(tDiff);
    dataString.concat(",");
    dataString.concat(vTEG);
    dataString.concat(",");
    dataString.concat(vPV);
    dataString.concat(",");
    dataString.concat(irrad);
    dataString.concat(",");
    dataString.concat(localTime);
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
      blynk(led, 2);
    }
    else
    {
      blynk(led, 3);
    }
  }

  void run()
  {
    takeReadings();
    Blink.Update();
    if (millis() - lastDumpTime >= 30000L)
    {
      logData();
      lastDumpTime = millis();
    }
  }
} report;

void setup()
{
  report.init();
}

void loop()
{
  report.run();
}
