#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define fan 9
#define temp_bus 10
#define btn_1 20
#define btn_2 21
#define btn_3 5
#define buzzer 11

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
  uint16_t fanSpeed;
  uint32_t last_millis = 0, refresh_rate = 1000;
  enum button_states {plus = 1, start_stop, minus, normal_speed = 128};
  void init(void)
  {
    pinMode(buzzer, 1);
    pinMode(btn_1, 2);
    pinMode(btn_2, 2);
    pinMode(btn_3, 2);

    digitalWrite(buzzer, 1);
    delay(300);
    digitalWrite(buzzer, 0);
    TCCR1B = TCCR1B & B11111000 | B00000011;
    fanSpeed = 128;
    sensors.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      for (;;);
    }
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    welcome();
  }

  void welcome(void)
  {
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
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

  void Display(byte slide)
  {
    if (millis() - last_millis >= refresh_rate)
    {
      if (slide == 1)
      {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("    *Speed Cool*    ");
        display.println("Set Temp -> " + String(set_temperature) + " C");
        display.println("Temp -> " + String(temperature) + " C");
        display.println("Status -> Idle");
        display.display();
      }
      else if (slide == 2)
      {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("    *Speed Cool*    ");
        display.println("Set Temp -> " + String(set_temperature) + " C");
        display.println("Temp -> " + String(temperature) + " C");
        display.println("Status -> Cooling");
        display.display();
      }
      last_millis = millis();
    }
    if (slide == 3)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("    *Speed Cool*    ");
      display.println("Set Temp -> " + String(set_temperature) + " C");
      display.println("Temp -> " + String(temperature) + " C");
      display.println("Status -> Start Cool");
      display.display();
      delay(500);
    }
    if(slide == 4)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("    *Speed Cool*    ");
      display.println("Set Temp -> " + String(set_temperature) + " C");
      display.println("Temp -> " + String(temperature) + " C");
      display.println("Status -> Stop Cool");
      display.display();
      delay(500);
    }
    if(slide == 5)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("    *Speed Cool*    ");
      display.println("Set Temp -> " + String(set_temperature) + " C");
      display.println("Temp -> " + String(temperature) + " C");
      display.println("Speed -> " + String(int(fanSpeed / 51)));
      display.display();
      delay(500);
    }
  }

  void run(void)
  {
    if (dev_state == idle)
    {
      stopFan();
      measureTemperature();
      Display(1);
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
    if (dev_state == busy)
    {
      measureTemperature();
      Display(2);
      byte val = read_keys();
      switch (val)
      {
        case plus:
          fanSpeed += 51;
          fanSpeed = constrain(fanSpeed, 51, 255);
          Display(5);
          break;
        case minus:
          fanSpeed -= 51;
          fanSpeed = constrain(fanSpeed, 51, 255);
          Display(5);
          break;
        case start_stop:
          dev_state = idle;
          break;
        default:
          break;
      }
      if (set_temperature < temperature && fanState == off)
      {
        beep();
        Display(3);
        fanSpeed = normal_speed;
        runFan(fanSpeed);
      }
      else if (set_temperature >= temperature && fanState == on)
      {
        beep();
        Display(4);
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
  speedCool.init();
}

void loop() {
  speedCool.run();
}
