#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

String messages[3] = {"Hello World!", "I\'m Osfoce!", "Thank you!"};

struct Writer
{
  void init(void)
  {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  WRITER  TEST  ");
  }
  void clearRow(byte row)
  {
    lcd.setCursor(0, row);
    lcd.print("                ");
  }
  void Write(int row, int col, String message, int Delay)
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

void setup() {
  writer.init();
}

void loop() {
  unsigned int count = 0;
  for(int j = 0; j < 3; j++)
  {
    writer.Write(count%2, 1, messages[j], 50);
    delay(1500);
    count++;
    count = constrain(count, 0, 10);
    //writer.clearRow(1);
  }
}
