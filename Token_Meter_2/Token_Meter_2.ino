// #include <EEPROM.h>
#include <Wire.h>
// #include <string.h>
#include <LiquidCrystal_I2C.h>
// #include "RTClib.h"


#define iSense A1
#define vSense A3
#define vSense2 A2
#define pullSwitch 2
#define triacPin 3

#define ACS712_SENSITIVITY 0.185  // Sensitivity of ACS712 sensor in mV/A

// #define triacPin 13

char* mid = "OG2501";

unsigned long diff = 0, lastRoutineStamp = 0, lastDisplayStamp = 0;

float iVolt = 0.0, iCurr = 0.0, iPowr = 0.0;
float avg_v = 0.0, avg_i = 0.0, energy = 0.0;

float balance = 0.0, cnt = 0;
bool read_bal = true;

int ack = 0;

char input[25], memo[25], buffer[90], strForm0[17], strForm1[12], strForm2[12];

int i = 0, state = 0, RefreshRate = 1000;

// RTC_DS1307 rtc;

LiquidCrystal_I2C lcd(0x27, 16, 2);

struct MeterParams {
  float energy = 0.00;
};
MeterParams mParams;

// void writeToEEPROM(int address, const MeterParams& data) {
//   EEPROM.put(address, data);
// }

// void readFromEEPROM(int address, MeterParams& data) {
//   EEPROM.get(address, data);
// }


void clearRow(byte row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
}
struct TokenMeter {
  float measureCurrentAC() {
    int rawValue = analogRead(iSense);
    float i = ((rawValue - 512) * 5.0) / 1023.0 / ACS712_SENSITIVITY;
    return fabs(i);
  }

  float measureVoltageAC() {
    float voltage = (float)analogRead(vSense);
    voltage = (voltage * 5.0) / 1023.0;
    voltage *= 101.0;
    voltage /= 1.414;
    return voltage;
  }

  void display(int mode) {
    if (millis() - lastDisplayStamp >= RefreshRate) {
      if (mode == 0) {
        lcd.setCursor(0, 0);
        lcd.print("  Energy Meter  ");
        clearRow(1);
        buffer[0] = '\0';
        strForm0[0] = '\0';
        dtostrf(iPowr, 4, 1, strForm0);
        sprintf(buffer, "Load: %s W", strForm0);
        lcd.print(buffer);
      }
      lastDisplayStamp = millis();
    }
  }

  void takeReadings(void) {
    float v = measureVoltageAC();
    float i = measureCurrentAC();
    iPowr = v * i;
    if (state && balance > 0.0) {
      iVolt += v;
      iCurr += i;
      cnt++;
    } else {
      iVolt += 0.0;
      iCurr += 0.0;
      cnt++;
    }
    display(0);
  }

  // void latchIn() {
  //   digitalWrite(pullSwitch, 1);
  //   delay(1500);
  // }

  // bool IsPowerOut() {
  //   float v = (float)analogRead(vSense2);
  //   v = (v * 5.0) / 1023.0;
  //   v *= 11.0;
  //   if (v >= 1.0) {
  //     return false;
  //   } else {
  //     return true;
  //   }
  // }

  // void shutdown() {
  //   // Serial.println("Shutting Down...");
  //   delay(1000);
  //   digitalWrite(pullSwitch, 0);
  //   delay(2000);
  //   while (true)
  //     ;
  // }

  // void backupData() {
  //   // Serial.println("Backing Up...");
  //   writeToEEPROM(1, mParams);
  //   delay(1000);
  // }

  char* readStrList(char* memory, const char* strList, byte pos) {
    byte index = 0;
    memory[0] = '\0';
    for (i = 0; i < strlen(strList); i++) {
      if (strList[i] == ',') {
        index++;
      }
      if (index == pos - 1) {
        strncat(memory, &strList[i], 1);
      }
    }
    if (memory[0] == ',') {
      strcpy(memory, memory + 1);
    }
    return memory;
  }

  bool isListData(const char* data) {
    if (data[0] == '[' && data[strlen(data) - 1] == ']') {
      return true;
    } else {
      return false;
    }
  }

  void trimWhiteSpace(char* str) {
    if (str == NULL) {
      return;
    }
    int len = strlen(str);
    int start = 0;
    int end = len - 1;
    while (isspace(str[start]) && start < len) {
      start++;
    }
    while (end >= start && isspace(str[end])) {
      end--;
    }
    int shift = 0;
    for (int i = start; i <= end; i++) {
      str[shift] = str[i];
      shift++;
    }
    str[shift] = '\0';
  }

  void substr(const char* input, char* output, int start, int stop) {
    if (start < 0 || start >= strlen(input) || stop < 0 || stop >= strlen(input) || start > stop) {
      output[0] = '\0';
    } else {
      strncpy(output, input + start, stop - start + 1);
      output[stop - start + 1] = '\0';
    }
  }

  void dumpData() {
    buffer[0] = '\0';
    strForm0[0] = '\0';
    dtostrf(avg_v, 4, 1, strForm0);
    float avg_p = avg_v * avg_i;
    strForm1[0] = '\0';
    dtostrf(avg_p, 4, 1, strForm1);
    strForm2[0] = '\0';
    dtostrf(mParams.energy, 4, 6, strForm2);
    sprintf(buffer, "{\"mid\":\"%s\",\"vol\":%s,\"pow\":%s,\"eng\":%s,\"ack\":%d}", mid, strForm0, strForm1, strForm2, ack);
    Serial.print(buffer);
    if (ack == 1) {
      ack = 0;
    }
  }

  void checkSerial(char* res) {
    if (Serial.available()) {
      res[0] = '\0';
      int count;
      while (Serial.available() > 0) {
        delay(3);
        count++;
        if(count > 20)
        {
          res[0] = '\0';
          break;
        }
        char c = Serial.read();
        strncat(res, &c, 1);
      }
    }
    if (strlen(res) > 0) {
      trimWhiteSpace(res);
      if (isListData(res)) {
        // Serial.println(res);
        mParams.energy = 0.0;
        memo[0] = '\0';
        substr(res, memo, 1, strlen(res) - 1);
        res[0] = '\0';
        strcpy(res, memo);
        memo[0] = '\0';
        state = atoi(readStrList(memo, res, 1));
        int top = atoi(readStrList(memo, res, 3));
        if (top) {
          balance = atof(readStrList(memo, res, 2));
          ack = 1;
          if (read_bal) {
            read_bal = false;
          }
        } else if (read_bal) {
          balance = atof(readStrList(memo, res, 2));
          read_bal = false;
        }
      } else {
        res[0] = '\0';
      }
    }
  }

  void DoRoutine() {
    diff = millis() - lastRoutineStamp;
    if (diff >= 30000UL) {
      avg_v = iVolt / cnt;
      avg_i = iCurr / cnt;
      energy = avg_v * avg_i;
      energy /= 1000.0;
      float t = (float)diff / 3600000.0;
      energy *= t;
      mParams.energy += energy;
      balance -= energy;
      if (balance < 0.0) {
        balance = 0.0;
      }
      dumpData();
      iVolt = 0;
      iCurr = 0;
      cnt = 0;
      lastRoutineStamp = millis();
    }
  }

  void Actions() {
    // if (IsPowerOut()) {
    //   // backupData();
    //   // shutdown();
    // }
    if (balance > 0.0) {
      if (state) {
        ActivateIsolator();
      } else {
        DeactivateIsolator();
      }
    } else {
      DeactivateIsolator();
    }
  }

  void ActivateIsolator() {
    digitalWrite(triacPin, 1);
  }
  void DeactivateIsolator() {
    digitalWrite(triacPin, 0);
  }
  void init(void) {
    // latchIn();

    Serial.begin(9600);

    pinMode(iSense, 0);
    pinMode(vSense, 0);
    pinMode(vSense2, 0);
    pinMode(pullSwitch, 1);
    pinMode(triacPin, 1);
    DeactivateIsolator();

    // if (EEPROM.read(0) != 0) {
    //   // Serial.println("EEPROM not empty, reading data...");
    //   readFromEEPROM(1, mParams);
    //   // Serial.println("Done!");
    //   //Serial.println(mParams.energy, 8);
    // } else {
    //   while (1)
    //     ;
    // }

    // if (!rtc.begin()) {
    //   Serial.println("Couldn't find RTC");
    //   Serial.flush();
    //   while (1) delay(10);
    // }

    lcd.init();
    lcd.init();
    lcd.backlight();
    lcd.clear();

    memo[0] = '\0';
    input[0] = '\0';
    buffer[0] = '\0';
    strForm0[0] = '\0';
    strForm1[0] = '\0';
    strForm2[0] = '\0';
  }

  void run() {
    takeReadings();
    checkSerial(input);
    DoRoutine();
    Actions();
  }

} tokenMeter;

void setup() {
  tokenMeter.init();
}

void loop() {
  tokenMeter.run();
}
