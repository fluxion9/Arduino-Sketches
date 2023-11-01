#include <Arduino.h>
#include <EmonLib.h>
#include <EEPROM.h>
#include <Ed25519.h>

#define switch0 2
#define AUX 3
#define M0 4
#define M1 5
#define switch1 6
#define buzzer 7
#define ledActive 8
#define ledWorking 9
#define wipePin 10
#define reset A3
#define iSense A0
#define vSense A1
#define SRx 11
#define STx 12
#define noiseSource A2

#define key_addr 0
#define addr_h 0xFF
#define addr_l 0x01
#define waitTime 10000
#define sampleTime 1500
#define NORMAL_MODE 0
#define POWER_SAVING_MODE 1
#define WAKEUP_MODE 2
#define SLEEP_MODE 3

int i;

const char* device_name = "maxwell";

char Resp[17], Bufr[400], Memo[65], Buf[65], Usage[70], Payload[300], Signature[130], pbkStr[65];

byte pbk[32], pvk[32], sig[64];

bool Registered = false, DEBUG = false;

byte gateway_channel = 0x1D;

uint8_t addr_buffer[3] = {0xFF, 0xFF, gateway_channel};

EnergyMonitor CTsense;

void byteArrayToHexStr(byte *Array, int arraySize, char *memory) {
  memory[0] = '\0';
  for (i = 0; i < arraySize; i++) {
    if (Array[i] < 16) {
      strcat(memory, "0");
    }
    char temp[3];
    sprintf(temp, "%02X", Array[i]);
    strcat(memory, temp);
  }
}

void readPrivateKey(int address, byte* keyArray, int keyArraySize) {
  int keySize = EEPROM.read(address);
  for (int i = 0; i < keySize; i++) {
    keyArray[i] = EEPROM.read(address + 1 + i);
  }
}

struct MeterParams {
  float avg_current = 0.00,
    avg_voltage = 0.00,
    energy = 0.00;

  unsigned long interval = 3000,
    timestamp_ = 0;

  int is_active = 0,
    is_on = 0;
} meterParams;

struct Meter {
  bool beep_active = false,
    beep_on = false;

  float volt,
    amp,
    temp,
    count = 0;

  unsigned long lastTime = 0;

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
    temp = (float)analogRead(vSense);
    temp = (temp * 505.0) / 1024.0;
    return temp * 0.707;
  }

  float measureCurrentAC() {
    return CTsense.calcIrms(1480);
  }

  void init() {
    pinMode(AUX, INPUT);
    pinMode(reset, INPUT);
    pinMode(M0, OUTPUT);
    pinMode(M1, OUTPUT);
    sendMode(NORMAL_MODE);
    Resp[16] = '\0';
    Memo[64] = '\0';
    Usage[70] = '\0';
    pbkStr[70] = '\0';
    Signature[130] = '\0';
    Payload[300] = '\0';
    Bufr[400] = '\0';
    Serial.begin(9600);
    if (EEPROM.read(key_addr) == 0) {
      Registered = false;
    } else {
      Registered = true;
      readPrivateKey(key_addr, pvk, sizeof(pvk));
      Ed25519::derivePublicKey(pbk, pvk);
      byteArrayToHexStr(pbk, sizeof(pbk), pbkStr);
    }
    Serial.println("Run()");
    while (!Registered) {
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
    }
    Serial.setTimeout(10000);
    CTsense.current(iSense, 111.1);
    pinMode(buzzer, OUTPUT);
    pinMode(ledActive, OUTPUT);
    pinMode(ledWorking, OUTPUT);
    pinMode(iSense, INPUT);
    pinMode(vSense, INPUT);
    initializeIsolator();
    digitalWrite(buzzer, HIGH);
    for (i = 0; i < 3; ++i) {
      digitalWrite(ledWorking, HIGH);
      delay(300);
      digitalWrite(ledWorking, LOW);
      delay(300);
    }
    digitalWrite(buzzer, LOW);
  }

  char* readStrList(char* memory, const char* strList, byte position) {
    byte index = 0;
    memory[0] = '\0';
    for (i = 0; i < strlen(strList); i++) {
      if (strList[i] == ',') {
        index++;
      }
      if (index == position - 1) {
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

  void deserializeAndExtractList(char* Buff) {
    meterParams.is_active = atoi(readStrList(Memo, Buff, 1));
    meterParams.is_on = atoi(readStrList(Memo, Buff, 2));
    meterParams.interval = atoi(readStrList(Memo, Buff, 3));
    meterParams.interval = meterParams.interval * 1000UL;
  }

  void checkSerial(char* res) {
    if (Serial.available()) {
      while (Serial.available() > 0) {
        delay(3);
        char c = Serial.read();
        strncat(res, &c, 1);
      }
    }
    if (strlen(res) > 0) {
      String strRes(res);
      strRes.trim();
      if (isListData(res)) {
        if (strRes == "[net_err]" || strRes == "[stat_err]") {
          res[0] = '\0';
        } else {
          strcpy(res, strRes.c_str());
          deserializeAndExtractList(res);
        }
      } else {
        res[0] = '\0';
      }
      res[0] = '\0';
    }
  }

  void dumpBuff(char* req) {
    sendMode(NORMAL_MODE);
    Serial.write(addr_buffer, 3);
    Serial.println(req);
    req[0] = '\0';
  }

  void computeUsage(char* usage) {
    usage[0] = '\0';
    sprintf(usage, "{\"ts\":%d,\"kwh\":%.4f}", 0, meterParams.energy);
  }

  void signMessage() {
    int len = strlen(Usage);
    byte arr[len];
    for (int i = 0; i < len; i++) {
      arr[i] = Usage[i];
    }
    Ed25519::sign(sig, pvk, pbk, arr, sizeof(arr));
    byteArrayToHexStr(sig, sizeof(sig), Signature);
    Serial.println(Signature);
  }

  void computePayLoad(char* payload, const char* PBK, const char* signature) {
    payload[0] = '\0';
    sprintf(payload, "{\"usage\":%s,\"pbk\":\"%s\",\"sig\":\"%s\"}", Usage, PBK, signature);
    Serial.println(payload);
  }

  void serializeJSON(char* Buff, const char* payload) {
    computeUsage(Usage);
    signMessage();
    computePayLoad(Payload, pbkStr, Signature);
    Buff[0] = '\0';
    sprintf(Buff, "{\"adrh\":%d,\"adrl\":%d,\"dev\":\"%s\",\"data\":%s}", addr_h, addr_l, device_name, payload);
  }

  void actions() {
    if (!meterParams.is_active) {
      if (beep_active) {
        beep(3, 500);
        beep_active = false;
        digitalWrite(ledActive, LOW);
      }
      deactivateIsolator();
      if (meterParams.is_on) {
        digitalWrite(ledWorking, HIGH);
      } else {
      }
    } else if (meterParams.is_active) {
      if (!beep_active) {
        beep(1, 500);
        beep_active = true;
      }
      digitalWrite(ledActive, HIGH);
      if (meterParams.is_on) {
        if (!beep_on) {
          beep(2, 100);
          beep_on = true;
        }
        digitalWrite(ledWorking, HIGH);
        activateIsolator();
      } else {
        if (beep_on) {
          beep(2, 100);
          beep_on = false;
          digitalWrite(ledWorking, LOW);
        }
        deactivateIsolator();
      }
    }
  }

  void routineCalc() {
    meterParams.avg_voltage /= count;
    meterParams.avg_current /= count;
    count = 0;
    meterParams.energy = ((meterParams.avg_voltage * meterParams.avg_current) / 1000) * (float(meterParams.interval) / 3600000.0); // unit is KWh
  }

  void routine() {
    if (millis() - lastTime >= meterParams.interval) {
      routineCalc();
      serializeJSON(Bufr, Payload);
      dumpBuff(Bufr);
      lastTime = millis();
    }
  }

  void takeReadings() {
    volt = measureVoltageAC();
    amp = fabs(measureCurrentAC());
    meterParams.avg_voltage = meterParams.avg_voltage + volt;
    meterParams.avg_current = meterParams.avg_current + amp;
    count++;
  }

  void beep(int times, int interval) {
    for (int j = 0; j < times; j++) {
      digitalWrite(buzzer, HIGH);
      delay(interval);
      digitalWrite(buzzer, LOW);
      delay(interval);
    }
  }

  void run(void) {
    checkSerial(Resp);
    actions();
    takeReadings();
    routine();
  }
} meter;

void setup() {
  meter.init();
}

void loop() {
  meter.run();
}
