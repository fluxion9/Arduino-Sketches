#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ACS712.h>

// Constants for voltage measurement
const int voltagePin = A0;
const float voltageDividerRatio = 2.0;

// CT current sensor
const int currentPin = A1;
ACS712 currentSensor(ACS712_30A, currentPin);

// Relay control
const int relayPin = 8;
bool isOutputOn = false;

// Password settings
const int passwordLength = 4;
char password[passwordLength + 1] = "1234"; // Default password
char newPassword[passwordLength + 1] = "0000"; // Default new password
bool changingPassword = false;

// LCD display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Keypad settings
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

// Energy measurement
const float voltageReference = 5.0;
float voltage, current, power, energyConsumed;
unsigned long lastMillis = 0;

// Current limit
float currentLimit = 10.0;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

  currentSensor.begin();
}

void loop() {
  // LCD display handling
  lcd.clear();
  if (changingPassword) {
    lcd.print("Enter New Pass:");
  } else if (changingCurrentLimit) {
    lcd.print("Set Curr Limit:");
  } else {
    lcd.print("Voltage: ");
    lcd.print(voltage);
    lcd.setCursor(0, 1);
    lcd.print("Current: ");
    lcd.print(current);
  }

  // Keypad input handling
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (changingPassword) {
      handlePasswordChange(key);
    } else if (changingCurrentLimit) {
      handleCurrentLimitSetting(key);
    } else {
      handleNormalKeypadInput(key);
    }
  }

  // Read AC voltage
  int rawVoltage = analogRead(voltagePin);
  voltage = ((float)rawVoltage / 1023.0) * voltageReference * voltageDividerRatio;

  // Read AC current
  current = currentSensor.getCurrentAC();

  // Calculate power
  power = voltage * current;

  // Calculate energy consumed
  unsigned long currentMillis = millis();
  float deltaTime = (currentMillis - lastMillis) / 1000.0;
  energyConsumed += (power * deltaTime) / 3600000.0;
  lastMillis = currentMillis;

  // Check for over current protection
  if (current > currentLimit) {
    digitalWrite(relayPin, LOW);
  }

  delay(1000);
}

void handlePasswordChange(char key) {
  static byte newPasswordIndex = 0;
  
  if (key == '#') {
    newPassword[newPasswordIndex] = '\0';
    changingPassword = false;
    strcpy(password, newPassword);
    lcd.clear();
    lcd.print("Password Changed");
    delay(2000);
  } else {
    if (newPasswordIndex < passwordLength) {
      newPassword[newPasswordIndex] = key;
      lcd.setCursor(newPasswordIndex, 1);
      lcd.print('*');
      newPasswordIndex++;
    }
  }
}

void handleCurrentLimitSetting(char key) {
  static byte currentLimitIndex = 0;
  static char currentLimitStr[6];
  
  if (key == '#') {
    currentLimitStr[currentLimitIndex] = '\0';
    changingCurrentLimit = false;
    currentLimit = atof(currentLimitStr);
    lcd.clear();
    lcd.print("Limit Set");
    delay(2000);
  } else if (isdigit(key)) {
    if (currentLimitIndex < 5) {
      currentLimitStr[currentLimitIndex] = key;
      lcd.setCursor(currentLimitIndex, 1);
      lcd.print(key);
      currentLimitIndex++;
    }
  }
}

void handleNormalKeypadInput(char key) {
  if (key == '*') {
    changingPassword = true;
    lcd.clear();
    lcd.print("Enter New Pass:");
  } else if (key == '0') {
    if (strcmp(password, "0000") == 0) {
      isOutputOn = !isOutputOn;
      digitalWrite(relayPin, isOutputOn ? HIGH : LOW);
      lcd.clear();
      lcd.print(isOutputOn ? "Output On" : "Output Off");
      delay(2000);
    }
  } else if (key == '#') {
    changingCurrentLimit = true;
    lcd.clear();
    lcd.print("Set Curr Limit:");
  }
}
