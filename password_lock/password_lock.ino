#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define LOOP 100

#define lock 5
#define len 4

#define led_r A3
#define led_b A0

#define INPUT 0
#define LAST_INPUT 1
#define NEW_INPUT 2
#define CONFIRM_INPUT 3
#define SET_KEY 7

#define GRANTED 4
#define DENIED 5
#define UNMATCH 6

#define NORMAL 0
#define CHANGE_PASSWORD 1
#define NEW_PASSWORD 2
#define CONFIRM_NEW_PASSWORD 3

#define C0 9
#define C1 8
#define C2 7
#define C3 6

#define R0 13
#define R1 12
#define R2 11
#define R3 10


const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', '/'},
  {'4', '5', '6', 'x'},
  {'7', '8', '9', '+'},
  {'*', '0', '#', '-'}
};
byte rowPins[ROWS] = {R0, R1, R2, R3};
byte colPins[COLS] = {C0, C1, C2, C3};


Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Locker
{
  char code[4];
  char input[4] = {'$', '$', '$', '$'};
  char temp[4] = {'$', '$', '$', '$'};
  int pos = 0;
  byte MODE = NORMAL;
  unsigned long last_millis, refresh_rate = 500;
  void init()
  {
    pinMode(lock, 1);

    pinMode(led_r, 1);
    pinMode(led_b, 1);
    for(byte i = 0; i < 3; ++i)
    {
      digitalWrite(led_b, 1);
      delay(300);
      digitalWrite(led_r, 0);
      delay(300);
    }
    delay(1000);
    digitalWrite(led_r, 1);
    keypad.setDebounceTime(250);

    lcd.init();
    lcd.backlight();

    check_EEPROM();
  }

  void open_lock(int d)
  {
    digitalWrite(led_r, 0);
    digitalWrite(led_b, 1);
    digitalWrite(lock, 1);
    delay(d * 1000);
    digitalWrite(lock, 0);
    digitalWrite(led_r, 1);
    digitalWrite(led_b, 0);
  }
  void check_EEPROM(void)
  {
    int ROM[4];
    for(byte i = 0; i < 4; i++)
    {
      ROM[i] = EEPROM.read(i);
    }
    if(ROM[0] && ROM[1] && ROM[2] && ROM[3])
    {
      display(7);
      read_keys();
      if(pos >= len)
      {
        for(byte i = 0; i < 4; i++)
        {
          code[i] = input[i];
        }
        store_password();
      }
      set_input('$');
      pos = 0;
    }
    else {
      recover_password();
    }
  }
  void welcome(void)
  {
  }
  void match_open(void)
  {
    if (pos >= len)
    {
      if (input[0] == code[0] && input[1] == code[1] && input[2] == code[2] && input[3] == code[3])
      {
        display(GRANTED);
        open_lock(5);
      }
      else {
        display(DENIED);
        delay(1000);
      }
      set_input('$');
      pos = 0;
    }
  }
  void set_input(char value)
  {
    for (byte i = 0; i < 4; ++i)
    {
      input[i] = value;
    }
  }
  void run_mode(byte mode_)
  {
    if (mode_ == NORMAL)
    {
      display(INPUT);
      read_keys();
      match_open();
    }
    else if (mode_ == CHANGE_PASSWORD)
    {
      display(LAST_INPUT);
      read_keys();
      if (pos >= len)
      {
        switch (match_compare())
        {
          case true:
            MODE = NEW_PASSWORD;
            break;
          case false:
            MODE = CHANGE_PASSWORD;
            break;
        }
        set_input('$');
        pos = 0;
      }
    }
    else if (mode_ == NEW_PASSWORD)
    {
      display(NEW_INPUT);
      read_keys();
      if (pos >= len)
      {
        for (byte i = 0; i < 4; i++)
        {
          temp[i] = input[i];
        }
        set_input('$');
        pos = 0;
        MODE = CONFIRM_NEW_PASSWORD;
      }
    }
    else if (mode_ == CONFIRM_NEW_PASSWORD)
    {
      display(CONFIRM_INPUT);
      read_keys();
      if (pos >= len)
      {
        if (input[0] == temp[0] && input[1] == temp[1] && input[2] == temp[2] && input[3] == temp[3])
        {
          for (byte i = 0; i < 4; i++)
          {
            code[i] = temp[i];
          }
          store_password();
          MODE = NORMAL;
        }
        else
        {
          display(UNMATCH);
          delay(500);
          MODE = CHANGE_PASSWORD;
        }
        set_input('$');
        pos = 0;
      }
    }
  }
  bool match_compare()
  {
    if (input[0] == code[0] && input[1] == code[1] && input[2] == code[2] && input[3] == code[3])
    {
      pos = 0;
      return true;
    }
    else {
      pos = 0;
      return false;
    }
  }
  void read_keys()
  {
    char key = keypad.getKey();
    if (key != NO_KEY)
    {
      if (isAlphaNumeric(key))
      {
        input[pos++] = key;
      }
      else if (key == '+')
      {
        MODE = CHANGE_PASSWORD;
      }
      else if (key == '-')
      {
        pos = 0;
        MODE = NORMAL;
      }
      else if (key == '/')
      {
        pos--;
        input[pos] = '$';
      }
    }
  }
  void store_password()
  {
    for (byte i = 0; i < 4; i++)
    {
      EEPROM.write(i, code[i]);
      delay(5);
    }
  }
  void recover_password(void)
  {
    for (byte i = 0; i < 4; i++)
    {
      code[i] = EEPROM.read(i);
      delay(5);
    }
  }
  void display(byte slide)
  {
    if (millis() - last_millis >= refresh_rate)
    {
      if (slide == INPUT)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Enter Password ");
        display_input(input, 4);
      }
      else if (slide == LAST_INPUT)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Enter Last Key ");
        display_input(input, 4);
      }
      else if (slide == NEW_INPUT)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Enter New Key  ");
        display_input(input, 4);
      }
      else if (slide == CONFIRM_INPUT)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Confirm Pass Key");
        display_input(input, 4);
      }
      else if (slide == SET_KEY)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set the Pass Key");
        display_input(input, 4);
      }
      last_millis = millis();
    }
    if (slide == GRANTED)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("#ACCESS GRANTED#");
    }
    else if (slide == DENIED)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("#ACCESS DENIED#");
    }
    else if (slide == UNMATCH)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Match Error!  ");
    }
  }
  void display_input(char inp[], int size)
  {
    for (byte i = 6; i <= 9; i++)
    {
      if (inp[i - 6] != '$')
      {
        lcd.setCursor(i, 1);
        lcd.print("*");
      }
      else {
        lcd.setCursor(i, 1);
        lcd.print(" ");
      }
    }
  }
  void run(int times)
  {
    if (times != LOOP)
    {
      for (int i = 0; i < times; i++)
      {
        run_mode(MODE);
      }
    }
    else {
      for (;;)
      {
        run_mode(MODE);
      }
    }
  }
} locker;
void setup() {
  locker.init();
  locker.run(LOOP);
}
void loop() {
}
