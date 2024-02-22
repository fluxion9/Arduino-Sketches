#include <EEPROM.h>
#include <Wire.h>
#include <string.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <ACS712.h>
#include "RTClib.h"


#define iSense A1
#define vSense A3
#define vSense2 A2
#define pullSwitch 2
#define triacPin 3

unsigned long diff = 0, lastBacklightUpdate = 0, lastRoutineStamp = 0, lastDisplayStamp = 0;

float iVolt = 0.0, iCurr = 0.0, iPowr = 0.0;
float avg_v = 0.0, avg_i = 0.0, energy = 0.0;

int cnt = 0, RefreshRate = 1000;

bool tokenExhausted = true;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

struct MeterParams {
  uint8_t usage[12];
  int pointer = 0;
  char Token[12][12];
  float energy = 0.00;
  uint8_t lastStamp = 0;
  uint8_t use = 0;
};

// struct MeterParams
// {
//   uint8_t usage[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
//   int pointer = -1;
//   char Token[13][13] = {
//     "012345678989",
//     "246289742379",
//     "237879802782",
//     "278337837683",
//     "412145234125",
//     "782766636626",
//     "238383833472",
//     "092937303098",
//     "144212454153",
//     "747326523567",
//     "293400238263",
//     "233836237273"
//   };
//   float energy = 0.00;
//   uint8_t lastStamp = 0;
//   uint8_t use = 0;
// }d;

MeterParams mParams;


byte rowPins[ROWS] = { 6, 7, 8, 9 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 10, 11, 12, 13 };  //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

ACS712 ACS(iSense, 5.0, 1023, 100);
RTC_DS1307 rtc;

LiquidCrystal_I2C lcd(0x27, 16, 2);

char input[13], buffer[17], strForm[17];
uint16_t inputCount = 0;

void writeToEEPROM(int address, const MeterParams& data) {
  EEPROM.put(address, data);
}

void readFromEEPROM(int address, MeterParams& data) {
  EEPROM.get(address, data);
}

uint16_t signal(uint8_t p) {
  return 512 + 400 * sin((micros() % 1000000) * (TWO_PI * 50 / 1e6));
}

void clearInput() {
  for (int i = 0; i < 13; i++) {
    input[i] = '\0';
  }
}

void handleBacklight(void) {
  if (millis() - lastBacklightUpdate >= 5000) {
    lcd.noBacklight();
  } else {
    lcd.backlight();
  }
}

void clearRow(byte row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
}

struct TokenMeter {
  void display(int mode) {
    if (millis() - lastDisplayStamp >= RefreshRate) {
      if (mode == 0) {
        lcd.setCursor(0, 0);
        lcd.print("  Energy Meter  ");
        clearRow(1);
        buffer[0] = '\0';
        strForm[0] = '\0';
        dtostrf(iPowr, 4, 1, strForm);
        sprintf(buffer, "Load: %s W", strForm);
        lcd.print(buffer);
      } else if (mode == 1) {
        lcd.setCursor(0, 0);
        lcd.print(" Token Expired! ");
        clearRow(1);
        buffer[0] = '\0';
        strForm[0] = '\0';
        dtostrf(mParams.energy, 4, 6, strForm);
        sprintf(buffer, "kwh: %s", strForm);
        lcd.print(buffer);
      }
      lastDisplayStamp = millis();
    }
  }

  void handleInput(int mode) {
    if (mode == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Enter  Token  ");
      lcd.setCursor(0, 1);
      clearInput();
      while (1) {
        handleBacklight();
        if (IsPowerOut()) {
          shutdown();
        }
        char key = keypad.getKey();
        if (key) {
          lastBacklightUpdate = millis();
          handleBacklight();
          if (isDigit(key)) {
            byte len = strlen(input);
            input[len] = key;
            input[len+1] = '\0';
            buffer[0] = '\0';
            sprintf(buffer, " %c%c%c%c-%c%c%c%c-%c%c%c%c ", input[0], input[1], input[2], input[3], input[4], input[5], input[6], input[7], input[8], input[9], input[10], input[11]);
            lcd.setCursor(0, 1);
            lcd.print(buffer);
            inputCount++;
            if (inputCount >= 12) {
              for (int i = 0; i < 12; i++) {
                char stored[13];
                stored[0] = '\0';
                for(int j = 0; j < 12; j++)
                {
                  byte s_len = strlen(stored);
                  stored[s_len] = mParams.Token[i][j];
                  stored[s_len + 1] = '\0';
                }
                if (strcmp(input, stored) == 0) {
                  if (mParams.usage[i] == 0) {
                    //valid
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Valid Token!");
                    lcd.setCursor(0, 1);
                    lcd.print("Activating...");
                    delay(500);
                    mParams.pointer = i;
                    mParams.usage[i] = 1;
                    mParams.use += 1;
                    clearInput();
                    inputCount = 0;
                    DateTime now = rtc.now();
                    mParams.lastStamp = now.month();
                    clearRow(1);
                    lcd.print("Done!");
                    backupData();
                    delay(1000);
                    lcd.clear();
                    return;
                  } else {
                    //used
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Used Token!");
                    lcd.setCursor(0, 1);
                    lcd.print("Enter a new one.");
                    delay(1500);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("  Enter  Token  ");
                    lcd.setCursor(0, 1);
                    clearInput();
                    inputCount = 0;
                    break;
                  }
                }
              }
              //invalid
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Invalid Token!");
              lcd.setCursor(0, 1);
              lcd.print("Try Again.");
              delay(1500);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("  Enter  Token  ");
              lcd.setCursor(0, 1);
              inputCount = 0;
              clearInput();
            }
          }
        }
      }
    } else if (mode == 1) {
      handleBacklight();
      char key = keypad.getKey();
      if (key) {
        lastBacklightUpdate = millis();
        handleBacklight();
        if (key == 'A') {
          handleInput(2);
        }
      }
    } else if (mode == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Enter  Token  ");
      lcd.setCursor(0, 1);
      clearInput();
      while (1) {
        handleBacklight();
        char key = keypad.getKey();
        if (key) {
          lastBacklightUpdate = millis();
          handleBacklight();
          if (isDigit(key)) {
            byte len = strlen(input);
            input[len] = key;
            input[len+1] = '\0';
            buffer[0] = '\0';
            sprintf(buffer, " %c%c%c%c-%c%c%c%c-%c%c%c%c ", input[0], input[1], input[2], input[3], input[4], input[5], input[6], input[7], input[8], input[9], input[10], input[11]);
            lcd.setCursor(0, 1);
            lcd.print(buffer);
            inputCount++;
            if (inputCount >= 12) {
              for (int i = 0; i < 12; i++) {
                char stored[13];
                stored[0] = '\0';
                for(int j = 0; j < 12; j++)
                {
                  byte s_len = strlen(stored);
                  stored[s_len] = mParams.Token[i][j];
                  stored[s_len + 1] = '\0';
                }
                if (strcmp(input, stored) == 0) {
                  if (mParams.usage[i] == 0) {
                    //valid
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Valid Token!");
                    lcd.setCursor(0, 1);
                    lcd.print("Activating...");
                    delay(500);
                    mParams.pointer = i;
                    mParams.usage[i] = 1;
                    mParams.use += 1;
                    mParams.energy = 0.0;
                    tokenExhausted = false;
                    clearInput();
                    inputCount = 0;
                    DateTime now = rtc.now();
                    mParams.lastStamp = now.month();
                    clearRow(1);
                    lcd.print("Done!");
                    backupData();
                    delay(1000);
                    lcd.clear();
                    return;
                  } else {
                    //used
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Used Token!");
                    lcd.setCursor(0, 1);
                    lcd.print("Enter a new one.");
                    delay(1500);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("  Enter  Token  ");
                    lcd.setCursor(0, 1);
                    clearInput();
                    inputCount = 0;
                    break;
                  }
                }
              }
              //invalid
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Invalid Token!");
              lcd.setCursor(0, 1);
              lcd.print("Try Again.");
              delay(1500);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("  Enter  Token  ");
              lcd.setCursor(0, 1);
              inputCount = 0;
              clearInput();
            }
          } else if (key == 'A') {
            lcd.clear();
            break;
          }
        }
      }
    } else if (mode == 3) {
      handleBacklight();
      char key = keypad.getKey();
      if (key) {
        lastBacklightUpdate = millis();
        handleBacklight();
      }
    }
  }

  void takeReadings(void) {
    float v = measureVoltageAC();
    float i = measureCurrentAC();
    iVolt += v;
    iCurr += i;
    cnt++;
    iPowr = v*i;
    display(0);
  }

  float measureCurrentAC() {
    // return (float)ACS.mA_AC_sampling();
    return 0.1;
  }

  float measureVoltageAC() {
    float voltage = (float)analogRead(vSense);
    voltage = (voltage * 5.0) / 1023.0;
    voltage *= 101.0;
    voltage /= 1.414;
    return voltage;
  }

  void latchIn() {
    digitalWrite(pullSwitch, 1);
    delay(1500);
  }
  bool IsPowerOut() {
    float v = (float)analogRead(vSense2);
    v = (v * 5.0) / 1023.0;
    v *= 11.0;
    if (v >= 1.0) {
      return false;
    } else {
      return true;
    }
  }

  void backupData() {
    writeToEEPROM(1, mParams);
    delay(1000);
  }

  void shutdown() {
    delay(1000);
    digitalWrite(pullSwitch, 0);
    delay(2000);
  }

  void dumpData() {
  }

  void DoRoutine() {
    diff = millis() - lastRoutineStamp;
    if (diff >= 30000U) {
      avg_v = iVolt / cnt;
      avg_i = iCurr / cnt;
      energy = (avg_v * avg_i) / 1000.0;
      energy *= (diff / 3600000UL);
      mParams.energy += energy;
      cnt = 0;
      dumpData();
      lastRoutineStamp = millis();
    }
  }

  void Actions() {
    DateTime now = rtc.now();
    if (now.month() - mParams.lastStamp >= 1) {
      tokenExhausted = true;
      DeactivateIsolator();
      dumpData();
      while (tokenExhausted) {
        display(1);
        handleInput(1);
      }
    } else {
      ActivateIsolator();
    }
    if (IsPowerOut()) {
      backupData();
      shutdown();
    }
    handleInput(3);
  }

  void ActivateIsolator() {
    digitalWrite(triacPin, 1);
  }
  void DeactivateIsolator() {
    digitalWrite(triacPin, 0);
  }
  void init(void) {
    latchIn();

    Serial.begin(115200);

    pinMode(iSense, 0);
    pinMode(vSense, 0);
    pinMode(vSense2, 0);
    pinMode(pullSwitch, 1);
    pinMode(triacPin, 1);
    DeactivateIsolator();

    if (!rtc.begin()) {
      Serial.println("Couldn't find RTC");
      Serial.flush();
      while (1) delay(10);
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();

    ACS.setADC(signal, 5, 1024);

    input[0] = '\0';
    buffer[0] = '\0';
    strForm[0] = '\0';

    lastBacklightUpdate = millis();

    if (EEPROM.read(0) != 0) {
      Serial.println("EEPROM not empty, reading data...");
      readFromEEPROM(1, mParams);
      Serial.println("Done!");
    } else {
      while (1)
        ;
    }
    if (mParams.pointer == -1) {
      handleInput(0);
    } else if (mParams.pointer >= 0) {
      tokenExhausted = false;
    } else {
      while (1)
        ;
    }
  }

  void run() {
    takeReadings();
    DoRoutine();
    Actions();
  }
}tokenMeter;

void setup() {
  tokenMeter.init();
}

void loop() {
  tokenMeter.run();
}
