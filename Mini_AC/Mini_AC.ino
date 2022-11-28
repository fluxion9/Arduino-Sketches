#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"


#define humidifier 9
#define fan 13
#define buzzer A0
#define temp_sense 2
#define temp_type DHT11
#define start_stop 8
#define led_b A2

#define off 0
#define on 1

#define LOOP 0


DHT temp(temp_sense, temp_type);

LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Air_Cooler
{
  uint32_t last_millis, refresh_rate = 1000;
  float temperature, humidity;
  bool fan_state = off, humidifier_state = off;


  void init(void)
  {
    pinMode(humidifier, 1);
    pinMode(fan, 1);
    pinMode(buzzer, 1);
    pinMode(start_stop, 2);
    pinMode(led_b, 1);
    lcd.init();
    lcd.init();
    lcd.backlight();
    temp.begin();

    blink(3);
    sayHi();
    beep();

    start_humidifier();

    last_millis = millis();

  }

  void run(int times)
  {
    if (times == LOOP)
    {
      for (;;)
      {
        measure_temperature();
        measure_humidity();
        display(1);
        read_button();
        read_button();
      }
    }
    else if (times > LOOP)
    {
      for (int i = 0; i < times; i++)
      {
        measure_temperature();
        measure_humidity();
        display(1);
        read_button();
        read_button();
      }
    }
  }

  void start_humidifier(void)
  {
    digitalWrite(humidifier, 1);
    humidifier_state = on;
  }

  void stop_humidifier(void)
  {
    digitalWrite(humidifier, 0);
    humidifier_state = off;
  }

  void start_fan(void)
  {
    digitalWrite(fan, 1);
    fan_state = on;
  }

  void stop_fan(void)
  {
    digitalWrite(fan, 0);
    fan_state = off;
  }

  void read_button(void)
  {
    if (!digitalRead(start_stop) && fan_state == off)
    {
      beep();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("      INFO      ");
      lcd.setCursor(0, 1);
      lcd.print("Fan ON!");
      start_fan();
      digitalWrite(led_b, 1);
      delay(500);
    }
    if (!digitalRead(start_stop) && fan_state == on)
    {
      beep();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("      INFO      ");
      lcd.setCursor(0, 1);
      lcd.print("Fan OFF!");
      stop_fan();
      digitalWrite(led_b, 0);
      delay(500);
    }
  }

  void measure_temperature(void)
  {
    float t = temp.readTemperature();
    if (isnan(t))
    {
      temperature = 0.0;
    }
    else {
      temperature = t;
    }
  }

  void measure_humidity(void)
  {
    float h = temp.readHumidity();
    if (isnan(h))
    {
      humidity = 0.0;
    }
    else {
      humidity = h;
    }
  }

  void display(byte mode)
  {
    if (millis() - last_millis >= refresh_rate)
    {
      if (mode == 1)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: " + String(temperature));
        lcd.setCursor(0, 1);
        lcd.print("Humidity: " + String(humidity));
      }
      last_millis = millis();
    }
  }

  void blink(int times)
  {
    for (int i = 0; i < times; i++)
    {
      digitalWrite(led_b, 1);
      delay(500);
      digitalWrite(led_b, 0);
      delay(500);
    }
  }

  void beep(void)
  {
    digitalWrite(buzzer, 1);
    delay(300);
    digitalWrite(buzzer, 0);
  }

  void sayHi(void)
  {
    lcd.print("    Power ON    ");
    lcd.setCursor(0, 1);
    delay(500);
    lcd.print("  Starting....  ");
    delay(1000);
    lcd.clear();
  }
} air_cooler;


void setup() {
  air_cooler.init();
  air_cooler.run(LOOP);
}

void loop() {
}
