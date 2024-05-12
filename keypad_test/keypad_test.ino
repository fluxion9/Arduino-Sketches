#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address and dimensions

const int KEYPAD_ROWS = 4;  // Number of rows in the keypad
const int KEYPAD_COLS = 4;  // Number of columns in the keypad

// char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
//   { '1', '2', '3', '4' },
//   { '5', '6', '7', '8' },
//   { '9', '0', 'A', 'B' },
//   { 'C', 'D', 'E', 'F' }
// };

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

// byte rowPins[KEYPAD_ROWS] = { 5, 6, 7, 8 };    // Keypad row pins
// byte colPins[KEYPAD_COLS] = { 12, 11, 10, 9 };  // Keypad column pins

byte rowPins[KEYPAD_ROWS] = { 12, 11, 10,  9};    // Keypad row pins
byte colPins[KEYPAD_COLS] = { 8, 7, 6, 5 };  // Keypad column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

unsigned long lastDisplayTime = 0;

char buffer[17];

void HandleKeypad() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    byte len = strlen(buffer);
    if(len >= 16)
    {
      buffer[0] = '\0';
      len = 0;
    }
    buffer[len] = key;
    buffer[len + 1] = '\0';
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(buffer);
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KEYPAD TEST");
  buffer[0] = '\0';
}

void loop() {
  HandleKeypad();
}
