#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "DHT.h"


#define bulb 4
#define fan 3


#define DHTPIN 2
#define DHTTYPE DHT11

#define on 1
#define off 0



DHT dht(DHTPIN, DHTTYPE);

struct Controller
{
  float setTemp = 30.0;
  bool fanState = off;
  unsigned long lastMillis = 0;
  bool toggle = false;

  void init(void)
  {
    pinMode(bulb, 1);
    pinMode(fan, 1);
    dht.begin();
    lcd.init();
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
  }

  float fmap(float val, float fromLow, float fromHigh, float toLow, float toHigh)
  {
    float norm = (val - fromLow) / (fromHigh - fromLow);
    float lerp = norm * (toHigh - toLow) + toLow;
    return lerp;
  }

  float measureTemp()
  {
    return dht.readTemperature();
  }

  void startFan()
  {
    digitalWrite(fan, 1);
  }

  void stopFan()
  {
    digitalWrite(fan, 0);
  }

  void OnLights()
  {
    digitalWrite(bulb, 1);
  }

  void OffLights()
  {
    digitalWrite(bulb, 0);
  }

  void clearLcdRow(int row)
  {
    lcd.setCursor(0, row);
    lcd.print("                ");
  }

  void run()
  {
    float temperature = measureTemp();
    float setTemp = fmap((float)analogRead(A0), 0, 1023.0, 0.0, 100.0);
    if (temperature > setTemp)
    {
      OffLights();
      startFan();
      if (!toggle)
      {
        clearLcdRow(1);
        lcd.setCursor(0, 1);
        lcd.print("Fan On!");
        delay(200);
        toggle = !toggle;
      }
    }
    if (temperature <= setTemp)
    {
      OnLights();
      stopFan();
      if (toggle)
      {
        clearLcdRow(1);
        lcd.setCursor(0, 1);
        lcd.print("Fan Off!");
        delay(200);
        toggle = !toggle;
      }
    }

    if (millis() - lastMillis >= 500)
    {
      clearLcdRow(0);
      lcd.setCursor(0, 0);
      lcd.print("SetTemp:" + String(setTemp) + " *C");
      clearLcdRow(1);
      lcd.setCursor(0, 1);
      lcd.print("Temp:" + String(temperature) + " *C");
      lastMillis = millis();
    }
  }

} controller;

void setup() {
  controller.init();
}

void loop() {
  controller.run();
}
