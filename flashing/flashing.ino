#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ACS712.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
ACS712  ACS(A1, 5.0, 1023, 100);

#include <SPI.h>
#include <SD.h>

#define cs 10
String buf = "";
String params1 = "api_key=ZVE893GCAYIE1SPT&field1=";
String params2 = "api_key=ZVE893GCAYIE1SPT&field2=";
struct Writer
{
  void init(void)
  {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    ACS.autoMidPoint();
    lcd.print("  DATA LOGGER   ");
    while (!SD.begin(cs)) {
      write(1, 0, "No or Bad SD :-(", 20);
      delay(1000);
    }
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile) {
    dataFile.println("VOLTAGE(V),CURRENT(A),POWER(W)");
    dataFile.close();
    Serial.begin(9600);
  }
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

  void log(float param1, float param2, float param3)
  {
    buf = "";
    buf.concat(param1);
    buf.concat(",");
    buf.concat(param2);
    buf.concat(",");
    buf.concat(param3);
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile) {
    dataFile.println(buf);
    dataFile.close();
    Serial.println(params1+String(param1));
    delay(10000);
    Serial.println(params2+String(param2));
    delay(10000);
  }
  }

  
}writer;

unsigned long last_millis = 0;

float measureVoltageDC(byte pin, float Max)
{
  float voltage = analogRead(pin);
  voltage = (voltage * Max) / 1023.0;
  return voltage;
}

void setup() {
  writer.init();
}

void loop() {
  float v = measureVoltageDC(A6, 55.0);
  float i = ACS.mA_DC() / 1000;
  float p = v * i;
  writer.write(1, 0, "Voltage: " + String(v) + "V", 50);
  delay(500);
  writer.write(1, 0, "Current: " + String(i) + "A", 50);
  delay(500);
  writer.write(1, 0, "Logging Data... ", 50);
  delay(500);
  if(millis() - last_millis >= 3000)
  {
    writer.log(v, i, p);
    last_millis = millis();
  }
  delay(2000);
}
