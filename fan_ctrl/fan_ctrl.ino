#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPIN 4
#define buzzer A1
#define relay 5
#define doplr_sense A5
#define btn_1 6
#define btn_2 7
#define btn_3 8
#define btn_4 9
#define backlight 10
#define off 0
#define on 1
const int rs = A4, en = A2, d4 = A0, d5 = 13, d6 = 12, d7 = 11;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
DHT dht(DHTPIN, DHTTYPE);

struct FCtrl
{
  float temperature, upper, lower, avg;
  int16_t count, posense, scan_rate, dev_mode;
  bool presence, fan_state;
  uint32_t prev_sense, prev_scan, last_millis, refresh_rate, last_count, tick_rate, count_period, max_period;
  void init(void)
  {

    pinMode(buzzer, 1);
    pinMode(relay, 1);
    pinMode(doplr_sense, 0);
    pinMode(btn_1, 2);
    pinMode(btn_2, 2);
    pinMode(btn_3, 2);
    pinMode(btn_4, 2);
    pinMode(backlight, 1);

    lcd.begin(16, 2);
    dht.begin();
    lcd.clear();
    lcd.setCursor(0, 0);
    digitalWrite(buzzer, 1);
    delay(300);
    digitalWrite(buzzer, 0);
    digitalWrite(backlight, 1);

    presence = false;
    prev_sense = 0;
    posense = 10000;
    prev_scan = 0;
    last_millis = 0;
    refresh_rate = 500;
    scan_rate = 2000;
    fan_state = off;
    count = 0;
    avg = 0;
    upper = 50.0;
    lower = 25.0;
    dev_mode = 0;
    last_count = 0;
    tick_rate = 1000;
    count_period = 0;
    max_period = 180;

    stop_fan();
    
    greet();

  }
  void countdown(void)
  {
    if (millis() - last_count >= tick_rate)
    {
      if(digitalRead(doplr_sense))
      {
        count_period = max_period;
      }
      else {
        count_period--;
        count_period = constrain(count_period, 0, max_period);
      }
      if(count_period > 0)
      {
        presence = true;
      }
      else {
        presence = false;
      }
      last_count = millis();
    }
  }
  void start_fan(void)
  {
    digitalWrite(relay, 1);
  }
  void stop_fan(void)
  {
    digitalWrite(relay, 0);
  }
  void measure_temp(void)
  {
    float t = dht.readTemperature();
    if (isnan(t))
    {
      temperature = 0.0;
    }
    else {
      temperature = t;
    }
  }
  void greet()
  {
    lcd.print("    Power ON    ");
    lcd.setCursor(0, 1);
    delay(500);
    lcd.print("  Starting....  ");
    delay(1000);
    lcd.clear();
  }

  void display(byte mode)
  {
    if (millis() - last_millis >= refresh_rate)
    {
      if (mode == 0)
      {
        lcd.setCursor(0, 0);
        lcd.print("  *FAN CONTROL  ");
        lcd.setCursor(0, 1);
        lcd.print("Temp: " + String(temperature, 1));
      }
      else if (mode == 1)
      {
        lcd.setCursor(0, 0);
        lcd.print("  SET HIGH INT  ");
        lcd.setCursor(0, 1);
        lcd.print("High: " + String(upper, 1));
      }
      else if (mode == 2)
      {
        lcd.setCursor(0, 0);
        lcd.print("  SET LOWR INT  ");
        lcd.setCursor(0, 1);
        lcd.print("Low: " + String(lower, 1));
      }
      last_millis = millis();
    }
  }
  void beep()
  {
    digitalWrite(buzzer, 1);
    delay(100);
    digitalWrite(buzzer, 0);
    delay(200);
  }

  byte read_keys()
  {
    if (!digitalRead(btn_1))
    {
      beep();
      return 1;
    }
    else if (!digitalRead(btn_2))
    {
      beep();
      return 2;
    }
    else if (!digitalRead(btn_3))
    {
      beep();
      return 3;
    }
    else if (!digitalRead(btn_4))
    {
      beep();
      return 4;
    }
    else {
      return 0;
    }
  }

  void control_fan(void)
  {
    if (presence == true)
    {
      if (temperature >= upper && fan_state == off)
      {
        start_fan();
        fan_state = on;
      }
      else if (temperature <= lower && fan_state == on)
      {
        stop_fan();
        fan_state = off;
      }
    }
    else {
      stop_fan();
      fan_state = off;
    }
  }
  void update_upper(void)
  {

  }
  void update_lower()
  {

  }
  void run(void)
  {
    if (dev_mode == 0)
    {
      measure_temp();
      countdown();
      control_fan();
      display(0);
      byte val = read_keys();
      switch (val)
      {
        case 2:
          dev_mode = 1;
          break;
        case 3:
          dev_mode = 2;
          break;
        default:
          break;
      }
    }
    else if (dev_mode == 1)
    {
      measure_temp();
      countdown();
      control_fan();
      display(1);
      byte val = read_keys();
      switch (val)
      {
        case 1:
          update_upper();
          dev_mode = 0;
          break;
        case 2:
          upper += 0.1;
          upper = constrain(upper, 0, 50.0);
          break;
        case 3:
          upper -= 0.1;
          upper = constrain(upper, 0, 50.0);
          break;
        case 4:
          dev_mode = 0;
          break;
        default:
          break;
      }
    }
    else if (dev_mode == 2)
    {
      measure_temp();
      display(2);
      byte val = read_keys();
      switch (val)
      {
        case 1:
          update_lower();
          dev_mode = 0;
          break;
        case 2:
          lower += 0.1;
          lower = constrain(lower, 0, 50.0);
          break;
        case 3:
          lower -= 0.1;
          lower = constrain(lower, 0, 50.0);
          break;
        case 4:
          dev_mode = 0;
          break;
        default:
          break;
      }
    }
  }


}fctrl;
void setup() {
  fctrl.init();
}

void loop() {
  fctrl.run();

}
