#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define led 7

const int maxInputPin = A3;
const int outputVoltagePin = A2;
const float VoltageDividerRatio = 11.0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {10, 11, 12, 13};
byte colPins[COLS] = {4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


const int pwmPin = 9;
unsigned int pwmDutyCycle = 254;

const unsigned long displayRefreshInterval = 1000;

unsigned long lastDisplayUpdate = 0;

float maxInputVoltage = 0.0;
float outputVoltage = 0.0;
bool settingVoltage = false;
String enteredVoltage = "";

float desiredVoltage = 5.0;

void setup() {
  pinMode(pwmPin, OUTPUT);
  TCCR1B = TCCR1B & B11111000 | B00000001;
  analogWrite(pwmPin, pwmDutyCycle);
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  pinMode(led, 1);

  lcd.clear();
  lcd.print("Input: ");
  lcd.print(readMaxInputVoltage());
  lcd.setCursor(0, 1);
  lcd.print("Output: ");
  lcd.print(readOutputVoltage());

  for (int i = 0; i < 3; i++)
  {
    digitalWrite(led, 1);
    delay(200);
    digitalWrite(led, 0);
    delay(200);
  }
  delay(1000);
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeypadInput(key);
  }
  if (settingVoltage) {
    updateDisplay(1);
  }
  else {
    updateDisplay(0);
  }
  adjustOutput();
}

void handleKeypadInput(char key) {
  if (key == '#') {
    if (enteredVoltage.length() > 0) {
      desiredVoltage = enteredVoltage.toFloat();
      maxInputVoltage = readMaxInputVoltage();
      if (desiredVoltage <= maxInputVoltage) {
        settingVoltage = false;
        if(desiredVoltage <= 5.0)
        {
          desiredVoltage = 5.0;
        }
        pwmDutyCycle = map(desiredVoltage, 0, maxInputVoltage, 0, 255);
        analogWrite(pwmPin, pwmDutyCycle);
        enteredVoltage = "";
      } else {
        lcd.clear();
        lcd.print("Out of Range");
        delay(2000);
        settingVoltage = false;
        enteredVoltage = "";
        pwmDutyCycle = 0;
        desiredVoltage = 5.0;
        analogWrite(pwmPin, pwmDutyCycle);
      }
    }
  } else if (key == '*') {
    pwmDutyCycle = 254;
    analogWrite(pwmPin, pwmDutyCycle);
    enteredVoltage += ".";
  } else if (isdigit(key)) {
    pwmDutyCycle = 254;
    analogWrite(pwmPin, pwmDutyCycle);
    settingVoltage = true;
    enteredVoltage += key;
    
  }
}

void updateDisplay(int mode) {
  if (millis() - lastDisplayUpdate >= displayRefreshInterval) {
    if (mode == 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Vset: ");
      lcd.print(desiredVoltage);
      lcd.setCursor(0, 1);
      lcd.print("Vout: ");
      lcd.print(outputVoltage);
    }
    else if (mode == 1)
    {
      lcd.clear();
      lcd.print("Input:");
      lcd.setCursor(0, 1);
      lcd.print(enteredVoltage);
    }
    lastDisplayUpdate = millis();
  }
}

void adjustOutput()
{
  outputVoltage = readOutputVoltage();
  if (outputVoltage < desiredVoltage)
  {
    pwmDutyCycle--;
    pwmDutyCycle = constrain(pwmDutyCycle, 0, 254);
    analogWrite(pwmPin, pwmDutyCycle);
  }
  else if (outputVoltage > desiredVoltage)
  {
    pwmDutyCycle++;
    pwmDutyCycle = constrain(pwmDutyCycle, 0, 254);
    analogWrite(pwmPin, pwmDutyCycle);
  }
}

float readOutputVoltage()
{
  int rawValue = analogRead(outputVoltagePin);
  return ((float)rawValue / 1023.0) * 5.0 * VoltageDividerRatio;
}

float readMaxInputVoltage() {
  int rawValue = analogRead(maxInputPin);
  return ((float)rawValue / 1023.0) * 5.0 * VoltageDividerRatio;
}
