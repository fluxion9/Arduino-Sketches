#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#define C0 11
#define C1 12
#define C2 13

#define R0 7
#define R1 8
#define R2 9
#define R3 10

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {R0, R1, R2, R3};
byte colPins[COLS] = {C0, C1, C2};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct VarPow
{
  void init()
  {
  }

  void run()
  {
  }
}varPow;

void setup() {
}

void loop() {
}
