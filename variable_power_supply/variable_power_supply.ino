#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

const int maxInputPin = A0;
const int outputVoltagePin = A1;
const float maxInputVoltageDividerRatio = 11.0;
const float outputVoltageDividerRatio = 11.0;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {A2, A3, A4, A5};
byte colPins[COLS] = {A6, A7, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


const int pwmPin = 9;
unsigned int pwmDutyCycle = 0;

const unsigned long displayRefreshInterval = 1000;
unsigned long lastDisplayUpdate = 0;

float maxInputVoltage;
float outputVoltage = 0.0;
bool settingVoltage = false;
String enteredVoltage = "";

void setup() {
  pinMode(pwmPin, OUTPUT);
  analogWriteFrequency(pwmPin, pwmFrequency);
  analogWriteResolution(pwmResolution);
  
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  
  maxInputVoltage = readMaxInputVoltage();

  lcd.clear();
  lcd.print("Input: ");
  lcd.print(maxInputVoltage);
  lcd.setCursor(0, 1);
  lcd.print("Output: ");
  lcd.print(outputVoltage);
  delay(1000);

}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeypadInput(key);
  }

  if (settingVoltage) {
    updateDisplay();
  }
}

void handleKeypadInput(char key) {
  if (key == '#') {
    if (enteredVoltage.length() > 0) {
      float desiredVoltage = enteredVoltage.toFloat();
      if (desiredVoltage <= maxInputVoltage) {
        settingVoltage = false;
        pwmDutyCycle = map(desiredVoltage, 0, maxInputVoltage, 0, 255);
        analogWrite(pwmPin, pwmDutyCycle);
        lcd.clear();
        lcd.print("Voltage Set:");
        lcd.setCursor(0, 1);
        lcd.print(desiredVoltage);
        delay(2000);
      } else {
        lcd.clear();
        lcd.print("Out of Range");
        delay(2000);
        settingVoltage = false;
        enteredVoltage = "";
        pwmDutyCycle = 0;
        analogWrite(pwmPin, pwmDutyCycle);
        updateDisplay();
      }
    }
  } else if (key == '*') {
    enteredVoltage = "";
    settingVoltage = false;
    updateDisplay();
  } else if (isdigit(key)) {
    settingVoltage = true;
    enteredVoltage += key;
    updateDisplay();
  }
}

void updateDisplay() {
  unsigned long currentTime = millis();
  if (currentTime - lastDisplayUpdate >= displayRefreshInterval) {
    lastDisplayUpdate = currentTime;
    lcd.clear();
    lcd.print("Desired: ");
    lcd.print(enteredVoltage);
    lcd.setCursor(0, 1);
    lcd.print("Output: ");
    lcd.print(outputVoltage);
  }
}

float readMaxInputVoltage() {
  int rawValue = analogRead(maxInputPin);
  return ((float)rawValue / 1023.0) * 5.0 * maxInputVoltageDividerRatio;
}
