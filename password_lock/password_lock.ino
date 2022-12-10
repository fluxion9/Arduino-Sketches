#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>


#define lock 5

#define led_r A3
#define led_b A0

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
  {'7', '8', '9', '-'},
  {'*', '0', '#', '+'}
};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {9, 8, 7, 6};

char code[4];
//char code[] = {'1', '2', '1', '2'}; //The default code

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Locker
{
  void init()
  {
    pinMode(lock, 1);

    pinMode(led_r, 1);
    pinMode(led_b, 1);

    pinMode(C0, 1);
    pinMode(C1, 1);
    pinMode(C2, 1);
    pinMode(C3, 1);

    pinMode(R0, 0);
    pinMode(R1, 0);
    pinMode(R2, 0);
    pinMode(R3, 0);

    lcd.init();
    lcd.backlight();
    
  }
  
  void welcome(void)
  {
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
  }
  void input_key()
  {
    int i = 4,j = 4;
    char keypressed = keypad.getKey();
    char input_key[];
    while(keypressed != '#')
    {
      lcd.setCursor(j, 1);
      lcd.print("*");                
      char input_key[i] = keypressed;   
      i++;
      j++;
    }

    
  }

  void set_eeprom()//once
  {
     char code[] = {'1', '2', '1', '2'};
     for (unsigned int i = 0 ; i < EEPROM.length() ; i++ )
     EEPROM.write(i, 0);
     digitalWrite(13, HIGH);

     for(int i=0; <sizeof(code) ; i++)
      EEPROM.put(i, code[i]); 
  } 

  void get_stored_password()
  {
    EEPROM.get(0, code[0]);
    EEPROM.get(1, code[1]);
    EEPROM.get(2, code[2]);
    EEPROM.get(3, code[3]);
  }

  void reset_password()
  {
    char reset_key = keypad.getKey();
    if(key == '\')
    {
      
    }
  }
  void display(byte slide)
  {
    if(slide == 1)
    {
      lcd.clear();
      lcd.setCursor(4,4);
      lcd.print("INPUT PASSWORD");
      lcd.setCursor(10,3);
      lcd.print("RESET PASSWORD");
    }
  }
  void run(int times)
  {
    if (times != LOOP)
    {
      for (int i = 0; i < times; i++)
      {

      }
    }
    else {
      for (;;)
      {

      }
    }
  }

} locker;
void setup() {
}

void loop() {
}
