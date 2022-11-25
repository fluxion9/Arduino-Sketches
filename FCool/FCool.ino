#include <OneWire.h>
#include <DallasTemperature.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define fan 1
#define temp_bus 2
#define btn_1 3
#define btn_2 4
#define btn_3 5
#define buzzer 6

#define off 0
#define on 1

#define busy 1
#define idle 0

OneWire oneWire(temp_bus);
DallasTemperature sensors(&oneWire);

struct SpeedCool
{
  bool fanState = off, dev_state = idle;
  float temperature, set_temperature = 20.0;
  byte fanSpeed;
  enum button_states {plus = 1, start_stop, minus, normal_speed = 128};
  void init(void)
  {
    pinMode(fan, 1);
    pinMode(buzzer, 1);
    pinMode(btn_1, 2);
    pinMode(btn_2, 2);
    pinMode(btn_3, 2);

    digitalWrite(buzzer, 1);
    delay(300);
    digitalWrite(buzzer, 0);

    sensors.begin();

  }

  void measureTemperature(void)
  {
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
  }

  void runFan(byte speed)
  {
    analogWrite(fan, speed);
    fanState = on;
  }
  void stopFan(void)
  {
    analogWrite(fan, 0);
    fanState = off;
  }
  void beep()
  {
    digitalWrite(buzzer, 1);
    delay(100);
    digitalWrite(buzzer, 0);
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
    else {
      return 0;
    }
  }

  void greet()
  {

  }
  void display(byte mode)
  {

  }

  void run(void)
  {
    if (dev_state == idle)
    {
      stopFan();
      measureTemperature();
      byte val = read_keys();
      switch (val)
      {
        case plus:
          set_temperature += 0.1;
          break;
        case minus:
          set_temperature -= 0.1;
          break;
        case start_stop:
          dev_state = busy;
          break;
        default:
          break;
      }
    }
    if(dev_state == busy)
    {
      measureTemperature();
      byte val = read_keys();
      switch (val)
      {
        case plus:
          fanSpeed += 51;
          constrain(fanSpeed, 0, 255);
          break;
        case minus:
          fanSpeed -= 51;
          constrain(fanSpeed, 0, 255);
          break;
        case start_stop:
          dev_state = idle;
          break;
        default:
          break;
      }
      if(set_temperature < temperature && fanState == off)
      {
        beep();
        fanSpeed = normal_speed;
        runFan(fanSpeed);
      }
      else if(set_temperature >= temperature && fanState == on) 
      {
        beep();
        stopFan();
        dev_state = idle;
      }
      else {
        runFan(fanSpeed);
      }
    }
  }
} speedCool;




void setup() {
}

void loop() {
}
