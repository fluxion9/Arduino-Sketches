#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "ACS712.h"

LiquidCrystal_I2C lcd(0x27,16,2);
ACS712  ACS(A1, 5.0, 1023, 100);

String messages[3] = {"Hello World!", "Engine guys!", "Thank you!"};

struct Writer
{
  void init(void)
  {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    ACS.autoMidPoint();
    lcd.print("  WRITER  TEST  ");
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
      lcd.setCursor(col+i, row);
      lcd.print(message[i]);
      delay(Delay);
    }
  }
}writer;

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
  writer.write(1, 0, "Voltage: " + String(measureVoltageDC(A0, 55.0)), 100);
  delay(500);
  writer.write(1, 0, "Current: " + String(ACS.mA_DC()), 100);
  delay(500);
}
