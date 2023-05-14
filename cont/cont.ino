#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define Relay1 7
#define Relay2 8
#define Relay3 12

String input;

struct Writer
{
  void init(void)
  {
    pinMode(Relay1, 1);
    pinMode(Relay2, 1);
    pinMode(Relay3, 1);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Controller ");
    Serial.begin(9600);
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

  void run(void)
  {
    if(Serial.available())
    {
      while(Serial.available() > 0)
      {
        delay(3);
        char d = Serial.read();
        input += d;
      }
      if(input.length() > 0)
      {
        input.trim();
        if(input == "A")
        {
          activate(0);
        }
        else if(input == "a")
        {
          deactivate(0);
        }
        else if(input == "B")
        {
          activate(1);
        }
        else if(input == "b")
        {
          deactivate(1);
        }
        else if(input == "C")
        {
          activate(2);
        }
        else if(input == "c")
        {
          deactivate(2);
        }
        input = "";
      }
    }
  }

  void activate(byte pin)
  {
    Serial.println("activating...");
    if(pin == 0)
    {
      digitalWrite(Relay1, 1);
      write(1, 0, "Output1 Active  ", 50);
      delay(700);
      clearRow(1);
    }
    else if(pin == 1)
    {
      digitalWrite(Relay2, 1);
      write(1, 0, "Output2 Active  ", 50);
      delay(700);
      clearRow(1);
    }
    else if(pin == 2)
    {
      digitalWrite(Relay3, 1);
      write(1, 0, "Output3 Active  ", 50);
      delay(700);
      clearRow(1);
    }
  }

  void deactivate(byte pin)
  {
    Serial.println("deactivating...");
    if(pin == 0)
    {
      digitalWrite(Relay1, 0);
      write(1, 0, "Output1 Inactive ", 50);
      delay(700);
      clearRow(1);
    }
    else if(pin == 1)
    {
      digitalWrite(Relay2, 0);
      write(1, 0, "Output2 Inactive ", 50);
      delay(700);
      clearRow(1);
    }
    else if(pin == 2)
    {
      digitalWrite(Relay3, 0);
      write(1, 0, "Output3 Inactive ", 50);
      delay(700);
      clearRow(1);
    }
  }
} writer;

void setup()
{
  writer.init();
}

void loop()
{
  writer.run();
}
