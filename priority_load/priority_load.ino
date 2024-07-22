#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address and dimensions

int turn = 0;

const int RELAY1_PIN = 3;            // Pin for controlling relay 1
const int RELAY2_PIN = 13;           // Pin for controlling relay 2
const int ACS712_1_PIN = A2;         // Analog pin for ACS712 sensor 1
const int ACS712_2_PIN = A3;         // Analog pin for ACS712 sensor 2
const int VOLTAGE_PIN = A0;          // Analog pin for ac voltage divider
const int BATTERY_VOLTAGE_PIN = A1;  // Analog pin for inverter battery voltage divider

const int KEYPAD_ROWS = 4;  // Number of rows in the keypad
const int KEYPAD_COLS = 4;  // Number of columns in the keypad

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[KEYPAD_ROWS] = { 12, 11, 10, 9 };  // Keypad row pins
byte colPins[KEYPAD_COLS] = { 8, 7, 6, 5 };     // Keypad column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

const float ACS712_SENSITIVITY = 0.066;  // Sensitivity of ACS712 sensor in mV/A

#define MAX_POWER 1000.0  // Maximum power rating in watts

#define BATTERY_CUTOFF_VOLTAGE 10.0  // Battery voltage cutoff in volts

bool BatteryIsLow = false;

int PriorityLevel = 1;

float maxPowerRating = MAX_POWER;

float cutoffVoltage = BATTERY_CUTOFF_VOLTAGE;

unsigned long lastDisplayTime = 0, lastPriorityCheckTime = 0;

float voltage = 0.0;
float current1 = 0.0;
float current2 = 0.0;
float power1 = 0.0;
float power2 = 0.0;
float power3 = 0.0;
float battery = 0.0;

float measureVoltageAC(int pin) {
  float voltage = (float)analogRead(pin);
  voltage = (voltage * 5.0) / 1023.0;
  voltage *= 101.0;
  voltage /= 1.414;
  return voltage;
}

float measureVoltageDC(int pin) {
  float voltage = (float)analogRead(pin);
  voltage = (voltage * 5.0) / 1023.0;
  voltage *= 11.0;
  return voltage;
}

float measureCurrentAC(int pin) {
  int rawValue = analogRead(pin);
  float i = ((rawValue - 512) * 5.0) / 1023.0 / ACS712_SENSITIVITY;
  return fabs(i);
}

void HandleBatteryPriority(int priority) {
  float batteryVoltage = measureVoltageDC(BATTERY_VOLTAGE_PIN);
  // Check battery voltage and cutoff low priority load if voltage is too low
  if (batteryVoltage < cutoffVoltage) {
    cutoffVoltage += 0.5;
    BatteryIsLow = true;
    switch (priority) {
      case 1:
        digitalWrite(RELAY2_PIN, 0);
        digitalWrite(RELAY1_PIN, 1);
        break;
      case 2:
        digitalWrite(RELAY1_PIN, 0);
        digitalWrite(RELAY2_PIN, 1);
        break;
      default:
        break;
    }
  } else {
    digitalWrite(RELAY1_PIN, 1);
    digitalWrite(RELAY2_PIN, 1);
    cutoffVoltage = BATTERY_CUTOFF_VOLTAGE;
    BatteryIsLow = false;
  }
}

void HandleLoadPriority(int priority) {
  if (millis() - lastPriorityCheckTime >= 1000) {
    switch (priority) {
      case 1:
        digitalWrite(RELAY1_PIN, 1);
        if (power3 > maxPowerRating || BatteryIsLow) {
          digitalWrite(RELAY2_PIN, 0);
        } else {
          digitalWrite(RELAY2_PIN, 1);
        }
        break;
      case 2:
        digitalWrite(RELAY2_PIN, 1);
        if (power3 > maxPowerRating || BatteryIsLow) {
          digitalWrite(RELAY1_PIN, 0);
        } else {
          digitalWrite(RELAY1_PIN, 1);
        }
        break;
      default:
        break;
    }
    lastPriorityCheckTime = millis();
  }
}

void HandleKeypad() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    // Process keypress
    if (key >= '0' && key <= '9') {
      int prio = key - '0';
      if (prio > 0 && prio < 3) {
        PriorityLevel = prio;
        lcd.clear();
        lcd.print("Priority set to:");
        lcd.setCursor(0, 1);
        lcd.print(PriorityLevel);
        delay(1000);
        lcd.clear();
      }
    } else if (key == 'A') {
      // Process setting maximum power rating
      lcd.clear();
      lcd.print("Enter max power:");
      String powerStr = "";
      while (true) {
        char key2 = keypad.getKey();
        if (key2 >= '0' && key2 <= '9') {
          powerStr += key2;
          lcd.setCursor(powerStr.length() - 1, 1);
          lcd.print(key2);
        } else if (key2 == '#') {
          maxPowerRating = powerStr.toFloat();
          lcd.clear();
          lcd.print("Power set to:");
          lcd.setCursor(0, 1);
          lcd.print(maxPowerRating);
          delay(1000);
          lcd.clear();
          break;
        }
      }
    } else if (key == 'B') {
      // Process setting maximum power rating
      lcd.clear();
      lcd.print("Enter Batt Volt:");
      String volStr = "";
      int dec_cnt = 0;
      while (true) {
        char key2 = keypad.getKey();
        if (key2 == '*' && dec_cnt < 1 && volStr.length() >= 1) {
          volStr += ".";
          dec_cnt++;
          lcd.setCursor(volStr.length() - 1, 1);
          lcd.print('.');
        } else if (key2 >= '0' && key2 <= '9') {
          volStr += key2;
          lcd.setCursor(volStr.length() - 1, 1);
          lcd.print(key2);
        } else if (key2 == '#') {
          cutoffVoltage = volStr.toFloat();
          lcd.clear();
          lcd.print("Min Volt set to:");
          lcd.setCursor(0, 1);
          lcd.print(cutoffVoltage);
          delay(1000);
          lcd.clear();
          break;
        }
      }
    }
  }
}

void updateDisplay(int RefreshRate) {
  if (millis() - lastDisplayTime >= RefreshRate) {
    // Update LCD with relevant information
    switch (turn) {
      case 0:
        lcd.clear();
        lcd.print("Load 1:");
        lcd.setCursor(0, 1);
        lcd.print("V:");
        lcd.print(voltage, 0);
        lcd.print("V P:");
        lcd.print(power1, 0);
        lcd.print("W");
        turn = 1;
        lastDisplayTime = millis();
        break;
      case 1:
        lcd.clear();
        lcd.print("Load 2:");
        lcd.setCursor(0, 1);
        lcd.print("V:");
        lcd.print(voltage, 0);
        lcd.print("V P:");
        lcd.print(power2, 0);
        lcd.print("W");
        turn = 2;
        lastDisplayTime = millis();
        break;
      case 2:
        lcd.clear();
        lcd.print("Battery:");
        lcd.setCursor(0, 1);
        lcd.print("Volt:");
        lcd.print(battery);
        lcd.print("V");
        turn = 3;
        lastDisplayTime = millis();
        break;
      case 3:
        lcd.clear();
        lcd.print("Power 3:");
        lcd.setCursor(0, 1);
        lcd.print("Power3:");
        lcd.print(power3, 0);
        lcd.print("W");
        turn = 0;
        lastDisplayTime = millis();
        break;
    }
  }
}

void takeReadings() {
  voltage = measureVoltageAC(VOLTAGE_PIN);
  current1 = measureCurrentAC(ACS712_1_PIN);
  current2 = measureCurrentAC(ACS712_2_PIN);
  battery = measureVoltageDC(BATTERY_VOLTAGE_PIN);
  power1 = voltage * current1;
  power2 = voltage * current2;
  power3 = power1 + power2;
}

void setup() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.print("Priority Load");
  lcd.setCursor(0, 1);
  lcd.print("Control Device");
  delay(2000);
  lcd.clear();
  lastDisplayTime = millis();
  lastPriorityCheckTime = millis();
}

void loop() {
  HandleKeypad();
  takeReadings();
  updateDisplay(1000);
  HandleBatteryPriority(PriorityLevel);
  HandleLoadPriority(PriorityLevel);
}
