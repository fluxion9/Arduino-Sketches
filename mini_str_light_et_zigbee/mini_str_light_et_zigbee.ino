#define rcwlpin A2
#define ldrpin A3
#define vin A1
#define outpin 9

#define mid 128
#define full 255

// #define conversionFactor 0.05376
#define conversionFactor 0.0197

unsigned long lastMotionTime = 0, lastSendTime = 0;

float battery = 0.0;

char ser_data[32], payload[100];

bool status = 1;

struct STR {
  void init() {
    pinMode(rcwlpin, 0);
    pinMode(ldrpin, 0);
    pinMode(vin, 0);
    pinMode(outpin, 1);
    pinMode(LED_BUILTIN, 1);
    Serial.begin(9600);
  }

  bool batteryIsCharged() {
    int adcVal = analogRead(vin);
    float voltage = (float)adcVal * conversionFactor;
    battery = voltage;
    if (voltage >= 6.0)
      return true;
    else
      return false;
  }

  void loadJSON() {
    payload[0] = '\0';
    sprintf(payload, "{\"sta\":%d,\"batt\":%d,\"ldr\":%d}", status, analogRead(vin), analogRead(ldrpin));
  }

  bool motionDetected() {
    if (digitalRead(rcwlpin))
      return true;
    else
      return false;
  }

  void run() {
    if (Serial.available()) {
      ser_data[0] = '\0';
      while (Serial.available()) {
        delay(3);
        char m = Serial.read();
        byte len = strlen(ser_data);
        if (m != '\n' || m != '\r') {
          ser_data[len] = m;
          ser_data[len + 1] = '\0';
        }
      }
      if (strlen(ser_data) > 0) {
        status = atoi(ser_data);
      }
    }
    if (status && batteryIsCharged() && analogRead(ldrpin) >= 900) {
      if (millis() - lastMotionTime <= 10000) {
        analogWrite(outpin, full);
      } else {
        analogWrite(outpin, mid);
      }
      if (motionDetected()) {
        lastMotionTime = millis();
      }
    } else {
      analogWrite(outpin, 0);
      while (!batteryIsCharged()) {
        digitalWrite(LED_BUILTIN, 1);
        delay(100);
        digitalWrite(LED_BUILTIN, 0);
        delay(100);
      }
    }
    if (millis() - lastSendTime >= 1500) {
      loadJSON();
      Serial.println(payload);
      lastSendTime = millis();
    }
  }
} str;

void setup() {
  str.init();
}

void loop() {
  str.run();
}
