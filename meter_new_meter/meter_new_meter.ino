#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08

#define vSense A0  // AC voltage sense pin
#define iSense A3  // ACS712 current sense pin
#define bSense A2  // Battery voltage sense pin

// #define vSense A0  // AC voltage sense pin
// #define iSense A2  // ACS712 current sense pin
// #define bSense A1  // Battery voltage sense pin

const float VREF = 5.0;
const int ADC_MAX = 1023;
// const float ACS712_SENSITIVITY = 0.185; //20A version
const float ACS712_SENSITIVITY = 0.066; //30A version

const int SAMPLE_COUNT = 300;

float voltage = 0.0;
float current = 0.0;
float power = 0.0;
float battery_voltage = 0.0;

unsigned long last_print_time = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(onRequestEvent);
}

void loop() {
  voltage = measureVoltageAC();
  current = measureCurrentAC();
  power = voltage * current;
  battery_voltage = measureBatteryVoltage();

  if (millis() - last_print_time >= 2000) {
    Serial.print("ACV: ");
    Serial.print(voltage);
    Serial.print("V, ");
    Serial.print("ACI: ");
    Serial.print(current);
    Serial.print("A, ");
    Serial.print("ACP: ");
    Serial.print(power);
    Serial.print("W, ");
    Serial.print("BAT: ");
    Serial.print(battery_voltage);
    Serial.println("V");
    last_print_time = millis();
  }
}

float measureVoltageAC() {
  const uint16_t sampleCount = 1000;
  const float adcReference = 5.0;
  const float adcResolution = 1023.0;
  const float calibrationFactor = 312.0;

  float sumSquares = 0.0;
  float offset = 0.0;

  for (int i = 0; i < 200; i++) {
    offset += analogRead(vSense);
    delayMicroseconds(200);
  }
  offset /= 200.0;

  for (int i = 0; i < sampleCount; i++) {
    float sample = analogRead(vSense) - offset;
    sumSquares += sample * sample;
    delayMicroseconds(200); // ~5kHz sampling
  }

  float rmsAdc = sqrt(sumSquares / sampleCount);

  float rmsVoltage = (rmsAdc * adcReference) / adcResolution;

  return rmsVoltage * calibrationFactor;
}

float measureBatteryVoltage() {
  float raw = analogRead(bSense);
  float voltage = (raw * VREF) / ADC_MAX;
  voltage *= 11.0;
  return voltage;
}

float measureCurrentAC() {
  float offset = 512.0;
  float sumSq = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float reading = analogRead(iSense);
    float voltage = (reading * VREF) / ADC_MAX;
    float current = (voltage - (VREF / 2.0)) / ACS712_SENSITIVITY;
    sumSq += current * current;
    delayMicroseconds(200);
  }
  float Irms = sqrt(sumSq / SAMPLE_COUNT);
  return Irms;
}

void onRequestEvent() {
  byte buf[16];
  memcpy(buf, &voltage, 4);
  memcpy(buf + 4, &current, 4);
  memcpy(buf + 8, &power, 4);
  memcpy(buf + 12, &battery_voltage, 4);
  Wire.write(buf, 16);
}
