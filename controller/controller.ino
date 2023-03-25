#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define Relay 12
#define Vin A1

struct Writer
{
  void init(void)
  {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
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

struct Controller
{
  void init() 
  {
    pinMode(Relay, 1);
    pinMode(Vin, 0);
    writer.init();
    writer.write(0, 3, "Inverter", 100);
    delay(500);
    writer.write(1, 0, "Initializing...", 100);
    delay(1000);
    digitalWrite(Relay, 1);
  }
}control;

float measureVoltageDC(byte pin, float Max)
{
  float voltage = analogRead(pin);
  voltage = (voltage * Max) / 1023.0;
  return voltage;
}

unsigned long last_millis = 0;

void setup() {
  control.init();
}

void loop() {
  float Voltage = measureVoltageDC(Vin, 505.0);
  if((millis() - last_millis) >= 1000)
  {
    if (Voltage > 240.0)
    {
      Voltage = 240.0;
    }
    writer.write(1, 0, "Voltage: " + String(Voltage), 100);
    last_millis = millis();
  }
}
