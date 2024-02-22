#include <EmonLib.h>
#include <Arduino.h>
#include <avr/wdt.h>

// #define switch0 2
// #define AUX 3
// #define M0 4
// #define M1 5
// #define switch1 6
// #define buzzer 13
// #define ledActive 8
// #define ledWorking 13
// #define iSense A5
// #define vSense A1

#define switch0 PIN_PC2
#define AUX PIN_PC0
#define M0 PIN_PD4
#define M1 PIN_PD5
#define switch1 PIN_PC3
#define buzzer PIN_PE1
#define ledActive PIN_PD6
#define ledWorking PIN_PD7
#define iSense PIN_PE2
#define vSense PIN_PE3

#define addr_h 0xFF
#define addr_l 0x02

#define NORMAL_MODE 0
#define POWER_SAVING_MODE 1
#define WAKEUP_MODE 2
#define SLEEP_MODE 3


EnergyMonitor ct;

const char* meterId = "MET001";

int i;

char Resp[11], Bufr[197], Memo[33], Usage[65], Temp[65];

char eStr[17], vStr[7], iStr[7];

bool Registered = false;

byte gateway_channel = 0x1D;

uint8_t addr_buffer[3] = { 0xFF, 0xFF, gateway_channel };

struct MeterParams {
  float avg_current = 0.00,
        avg_voltage = 0.00,
        balance = 0.0;

  double energy = 0, last_energy = 0;

  unsigned long interval = 10000;

  int is_active = 1,
      is_on = 0,
      status = 204;

} meterParams;

struct Meter {
  bool ledstate = false;
  bool beep_active = false;
  bool beep_on = false;

  float count = 0;
  int dumpcount = 0;
  int chargecount = 0;

  unsigned long lastTime = 0, lastblinktime = 0;

  const int onblinkinterval = 200;
  const int offblinkinterval = 3000;

  void blynk() {
    if (ledstate && (millis() - lastblinktime >= onblinkinterval)) {
      digitalWrite(ledWorking, 0);
      ledstate = false;
      lastblinktime = millis();
    } else if (!ledstate && (millis() - lastblinktime >= offblinkinterval)) {
      digitalWrite(ledWorking, 1);
      ledstate = true;
      lastblinktime = millis();
    }
  }

  void initializeIsolator() {
    pinMode(switch0, OUTPUT);
    pinMode(switch1, OUTPUT);
  }

  void activateIsolator() {
    digitalWrite(switch0, HIGH);
    digitalWrite(switch1, HIGH);
  }

  void deactivateIsolator() {
    digitalWrite(switch0, LOW);
    digitalWrite(switch1, LOW);
  }

  void sendMode(int mode) {
    switch (mode) {
      case 0:
        digitalWrite(M0, LOW);
        digitalWrite(M1, LOW);
        break;
      case 1:
        digitalWrite(M0, HIGH);
        digitalWrite(M1, LOW);
        break;
      case 2:
        digitalWrite(M0, LOW);
        digitalWrite(M1, HIGH);
        break;
      case 3:
        digitalWrite(M0, HIGH);
        digitalWrite(M1, HIGH);
        break;
      default:
        digitalWrite(M0, LOW);
        digitalWrite(M1, LOW);
        break;
    }
  }

  float measureVoltageAC() {
    float temp = analogRead(vSense);
    temp = (temp * 505.0) / 1023.0;
    return temp * 0.707;
  }

  void init() {
    pinMode(AUX, INPUT);
    pinMode(M0, OUTPUT);
    pinMode(M1, OUTPUT);

    pinMode(buzzer, OUTPUT);
    pinMode(iSense, INPUT);
    pinMode(vSense, INPUT);

    pinMode(ledActive, 1);
    pinMode(ledWorking, 1);

    sendMode(NORMAL_MODE);

    Resp[0] = '\0';
    Memo[0] = '\0';
    Usage[0] = '\0';
    Temp[0] = '\0';
    Bufr[0] = '\0';

    Serial.begin(9600);
    Serial1.begin(9600);

    Serial.setTimeout(5000);

    initializeIsolator();
    wdt_disable();

    digitalWrite(buzzer, HIGH);
    for (i = 0; i < 3; i++) {
      digitalWrite(ledWorking, 1);
      delay(300);
      digitalWrite(ledWorking, 0);
      delay(300);
    }
    digitalWrite(buzzer, LOW);
    ct.current(iSense, 111.1);
    delay(1000);
    wdt_enable(WDTO_8S);
  }

  double measureCurrentAC() {
    double i = ct.calcIrms(1480);
    if (isnan(i) || isinf(i)) {
      return 0.0;
    } else {
      return i;
    }
  }
  int elementsof(const char* strlist)
  {
    int count = 0;
    for (i = 0; i < strlen(strlist); i++)
    {
      if (strlist[i] == ',') {
        count++;
      }
    }
    return count+1;
  }

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
    wdt_reset();
    return memory;
  }

  bool isListData(const char* data) {
    if (data[0] == '[' && data[strlen(data) - 1] == ']') {
      return true;
    } else {
      return false;
    }
    wdt_reset();
  }

  void deserializeAndExtractList(const char* list, const int required) {
    if(elementsof(list) != required)
    {
      wdt_reset();
      return;
    }
    meterParams.is_active = atoi(readStrList(Memo, list, 1));
    meterParams.is_on = atoi(readStrList(Memo, list, 2));
    meterParams.interval = atoi(readStrList(Memo, list, 3));
    meterParams.interval = meterParams.interval * 1000UL;
    if(meterParams.status == 204)
    {
      meterParams.balance = atof(readStrList(Memo, list, 4));
      meterParams.status = 200;
    }
    meterParams.last_energy = 0.0;
    if (chargecount > 1) {
      meterParams.energy = meterParams.last_energy;
    } else {
      meterParams.last_energy = 0.0;
    }
    wdt_reset();
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
    wdt_reset();
  }

  void substr(const char* input, char* output, int start, int stop) {
    if (start < 0 || start >= strlen(input) || stop < 0 || stop >= strlen(input) || start > stop) {
      output[0] = '\0';
    } else {
      strncpy(output, input + start, stop - start + 1);
      output[stop - start + 1] = '\0';
    }
    wdt_reset();
  }

  void checkSerial(char* res) {
    if (Serial.available()) {
      res[0] = '\0';
      while (Serial.available() > 0) {
        delay(3);
        char c = Serial.read();
        strncat(res, &c, 1);
        wdt_reset();
      }
      wdt_reset();
    }
    if (strlen(res) > 0) {
      trimWhiteSpace(res);
      if (isListData(res)) {
        if (strncmp(res, "[ACK]", 5) == 0) {
          res[0] = '\0';
          dumpcount = 0;
        } else {
          Memo[0] = '\0';
          substr(res, Memo, 1, strlen(res) - 1);
          res[0] = '\0';
          strcpy(res, Memo);
          Memo[0] = '\0';
          deserializeAndExtractList(res, 4);
        }
      } else {
        res[0] = '\0';
      }
      res[0] = '\0';
    }
    wdt_reset();
  }

  void dumpBuff(char* req) {
    sendMode(NORMAL_MODE);
    Serial.write(addr_buffer, 3);
    Serial.println(req);
    req[0] = '\0';
    wdt_reset();
  }

  void computeUsage(char* usage) {
    usage[0] = '\0';
    dtostrf(meterParams.avg_voltage, 4, 2, vStr);
    dtostrf(meterParams.avg_current, 4, 2, iStr);
    dtostrf(meterParams.energy, 4, 6, eStr);
    sprintf(usage, "{\"vol\":%s,\"amp\":%s,\"kwh\":%s}", vStr, iStr, eStr);
    eStr[0] = '\0';
    wdt_reset();
  }

  void serializeJSON(char* Buff) {
    computeUsage(Usage);
    Buff[0] = '\0';
    sprintf(Buff, "{\"adrh\":%d,\"adrl\":%d,\"data\":", addr_h, addr_l);
    sprintf(Temp, "{\"mid\":\"%s\",\"usage\":%s", meterId, Usage);
    strncat(Buff, Temp, strlen(Temp));
    strncat(Buff, "}}", 3);
    Temp[0] = '\0';
    wdt_reset();
  }

  void actions() {
    if (dumpcount >= 15) {
      meterParams.is_active = 0;
    }
    if (!meterParams.is_active) {
      if (beep_active) {
        beep(3, 500);
        beep_active = false;
        digitalWrite(ledActive, 0);
      }
      deactivateIsolator();
      if (meterParams.is_on) {
        digitalWrite(ledWorking, 1);
      }
      else {
        blynk();
      }
    } else if (meterParams.is_active) {
      if (!beep_active) {
        beep(1, 500);
        beep_active = true;
      }
      digitalWrite(ledActive, 1);
      if (meterParams.is_on) {
        if (!beep_on) {
          beep(2, 100);
          beep_on = true;
        }
        digitalWrite(ledWorking, 1);
        activateIsolator();
      } else {
        if (beep_on) {
          beep(2, 100);
          beep_on = false;
          digitalWrite(ledWorking, 0);
        }
        deactivateIsolator();
      }
    }
    wdt_reset();
  }

  void resetVars()
  {
    meterParams.avg_voltage = 0.00;
    meterParams.avg_current = 0.00;
  }

  void routineCalc() {
    meterParams.avg_voltage /= count;
    meterParams.avg_current /= count;
    count = 0;
    meterParams.energy = ((meterParams.avg_voltage * meterParams.avg_current) / 1000) * (float(meterParams.interval) / 3600000.0);  // unit is KWh
    meterParams.balance -= meterParams.energy;
    if(meterParams.energy < 0.0)
    {
      meterParams.energy = 0.0;
      meterParams.is_on = false;
    }
    wdt_reset();
  }

  void routine() {
    if (millis() - lastTime >= meterParams.interval) {
      routineCalc();
      serializeJSON(Bufr);
      dumpBuff(Bufr);
      meterParams.last_energy += meterParams.energy;
      meterParams.energy = 0.0;
      chargecount++;
      if (meterParams.is_active) {
        dumpcount++;
      }
      lastTime = millis();
      wdt_reset();
    }
  }

  void takeReadings() {
    float volt = measureVoltageAC();
    float Amp = fabs(measureCurrentAC());
    Serial1.print("V: ");
    Serial1.println(volt);
    Serial1.print("A: ");
    Serial1.println(Amp);
    meterParams.avg_voltage = meterParams.avg_voltage + volt;
    meterParams.avg_current = meterParams.avg_current + Amp;
    count++;
    wdt_reset();
  }

  void beep(int times, int interval) {
    for (int j = 0; j < times; j++) {
      digitalWrite(buzzer, HIGH);
      delay(interval);
      digitalWrite(buzzer, LOW);
      delay(interval);
    }
    wdt_reset();
  }

  void run(void) {
    checkSerial(Resp);
    actions();
    takeReadings();
    routine();
    wdt_reset();
  }
} meter;

void setup() {
  meter.init();
}

void loop() {
  meter.run();
}
