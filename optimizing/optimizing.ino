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
#define iSense A0
#define vSense A1

#define key_addr 0
#define addr_h 0xFF
#define addr_l 0x01

#define NORMAL_MODE 0
#define POWER_SAVING_MODE 1
#define WAKEUP_MODE 2
#define SLEEP_MODE 3

int i;

const char* device_name = "maxwell";

char Resp[11], Bufr[320], Memo[33], Usage[42], Temp[129];

char strForm[17];

byte pbk[32], pvk[32], sig[64];

bool Registered = false, DEBUG = false;

byte gateway_channel = 0x1D;

uint8_t addr_buffer[3] = {0xFF, 0xFF, gateway_channel};

EnergyMonitor CTsense;

class Blinker
{
    byte ledPin;
    int onTime;
    int offTime;
    bool ledState;
    unsigned long previousMillis;
  public:
    Blinker(byte pin, int on, int off)
    {
      ledPin = pin;
      pinMode(ledPin, OUTPUT);
      onTime = on;
      offTime = off;
      ledState = LOW;
      previousMillis = 0;
    }
    void Update()
    {
      if ((ledState == HIGH) && (millis() - previousMillis >= onTime))
      {
        ledState = LOW;
        previousMillis = millis();
        digitalWrite(ledPin, ledState);
      }
      else if ((ledState == LOW) && (millis() - previousMillis >= offTime))
      {
        ledState = HIGH;
        previousMillis = millis();
        digitalWrite(ledPin, ledState);
      }
    }
};
Blinker blink(ledActive, 200, 1000);
Blinker blink0(ledWorking, 200, 3000);

struct MeterParams {
  float avg_current = 0.00,
        avg_voltage = 0.00;

  double energy = 0;

  unsigned long interval = 3000,
                timestamp_ = 0;

  int is_active = 0,
      is_on = 0;
} meterParams;

struct Meter {
  bool beep_active = false,
       beep_on = false;

  float count = 0;

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
    float temp = (float)analogRead(vSense);
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
    Serial.println("Boot..");
    if (EEPROM.read(key_addr) == 0) {
      Registered = false;
    } else {
      Registered = true;
      readPrivateKey(key_addr, pvk, sizeof(pvk));
      Ed25519::derivePublicKey(pbk, pvk);
      byteArrayToHexStr(pvk, sizeof(pvk), Temp);
      Temp[0] = '\0';
    }

    while (!Registered) {
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
    }

    Serial.setTimeout(5000);

    initializeIsolator();

    digitalWrite(buzzer, HIGH);
    delay(1800);
    digitalWrite(buzzer, LOW);
    CTsense.current(iSense, 111.1);
  }

  float measureCurrentAC() {
    return CTsense.calcIrms(1480);
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
    return memory;
  }

  bool isListData(const char* data) {
    if (data[0] == '[' && data[strlen(data) - 1] == ']') {
      return true;
    }
    else {
      return false;
    }
  }

  void deserializeAndExtractList(const char* list) {
    meterParams.is_active = atoi(readStrList(Memo, list, 1));
    Serial.print(F("atv: "));
    Serial.println(meterParams.is_active);
    meterParams.is_on = atoi(readStrList(Memo, list, 2));
    Serial.print(F("on: "));
    Serial.println(meterParams.is_on);
    meterParams.interval = atoi(readStrList(Memo, list, 3));
    Serial.print(F("intv: "));
    Serial.println(meterParams.interval);
    meterParams.interval = meterParams.interval * 1000UL;
  }

  void trimWhiteSpace(char *str) {
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

  void substr(const char *input, char *output, int start, int stop) {
    if (start < 0 || start >= strlen(input) || stop < 0 || stop >= strlen(input) || start > stop) {
      output[0] = '\0';
    } else {
      strncpy(output, input + start, stop - start + 1);
      output[stop - start + 1] = '\0';
    }
  }

  void checkSerial(char* res) {
    if (Serial.available()) {
      res[0] = '\0';
      while (Serial.available() > 0) {
        delay(3);
        char c = Serial.read();
        strncat(res, &c, 1);
      }
    }
    if (strlen(res) > 0)
    {
      trimWhiteSpace(res);
      if (isListData(res))
      {
        if (strncmp(res, "[net_err]", 9) == 0 || strncmp(res, "[stat_err]", 10) == 0)
        {
          res[0] = '\0';
        }
        else {
          Memo[0] = '\0';
          substr(res, Memo, 1, strlen(res) - 1);
          res[0] = '\0';
          strcpy(res, Memo);
          Memo[0] = '\0';
          deserializeAndExtractList(res);
        }
      }
      else {
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
    dtostrf(meterParams.energy, 4, 6, strForm);
    sprintf(usage, "{\"ts\":%ld,\"kwh\":%s}", millis(), strForm);
    strForm[0] = '\0';
  }

  void serializeJSON(char* Buff) {
    computeUsage(Usage);
    Buff[0] = '\0';
    sprintf(Buff, "{\"adrh\":%d,\"adrl\":%d,\"dev\":\"%s\",\"data\":", addr_h, addr_l, device_name);
    sprintf(Temp, "{\"usage\":%s,", Usage);
    strncat(Buff, Temp, strlen(Temp));
    strncat(Buff, "\"pbk\":\"", 7);
    Temp[0] = '\0';
    byteArrayToHexStr(pbk, sizeof(pbk), Temp);
    strncat(Buff, Temp, strlen(Temp));
    strncat(Buff, "\",\"sig\":\"", 9);
    int len = strlen(Usage);
    byte arr[len];
    for (int i = 0; i < len; i++) {
      arr[i] = Usage[i];
    }
    Ed25519::sign(sig, pvk, pbk, arr, sizeof(arr));
    Temp[0] = '\0';
    byteArrayToHexStr(sig, sizeof(sig), Temp);
    strncat(Buff, Temp, strlen(Temp));
    strncat(Buff, "\"}}", 3);
    Temp[0] = '\0';
  }

  void byteArrayToHexStr(byte *Array, int arraySize, char *memory) {
    memory[0] = '\0';
    for (i = 0; i < arraySize; i++) {
      char temp[3];
      sprintf(temp, "%02x", Array[i]);
      strcat(memory, temp);
    }
  }

  void readPrivateKey(int address, byte* keyArray, int keyArraySize) {
    int keySize = EEPROM.read(address);
    for (int i = 0; i < keySize; i++) {
      keyArray[i] = EEPROM.read(address + 1 + i);
    }
  }

  void actions() {
    if (!meterParams.is_active)
    {
      if (beep_active)
      {
        beep(3, 500);
        beep_active = false;
        digitalWrite(ledActive, 0);
      }
      blink.Update();
      deactivateIsolator();
      if (meterParams.is_on)
      {
        digitalWrite(ledWorking, 1);
      }
      else {
        blink0.Update();
      }
    }
    else if (meterParams.is_active)
    {
      if (!beep_active)
      {
        beep(1, 500);
        beep_active = true;
      }
      digitalWrite(ledActive, 1);
      if (meterParams.is_on)
      {
        if (!beep_on)
        {
          beep(2, 100);
          beep_on = true;
        }
        digitalWrite(ledWorking, 1);
        activateIsolator();
      }
      else {
        if (beep_on)
        {
          beep(2, 100);
          beep_on = false;
          digitalWrite(ledWorking, 0);
        }
        blink0.Update();
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
      serializeJSON(Bufr);
      dumpBuff(Bufr);
      lastTime = millis();
    }
  }

  void takeReadings() {
    float volt = measureVoltageAC();
    float Amp = fabs(measureCurrentAC());
//    float Amp = 0.25;
    Serial.print("V: ");
    Serial.println(volt);
    Serial.print("I: ");
    Serial.println(Amp);
    meterParams.avg_voltage = meterParams.avg_voltage + volt;
    meterParams.avg_current = meterParams.avg_current + Amp;
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
