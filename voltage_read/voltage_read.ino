#define vBatt A3
#define vChg A2
#define sChg A0
#define von A1

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("Battery Voltage" + String(measureVoltage(vBatt, 55.0)));
  Serial.println("vChg Voltage" + String(measureVoltage(vChg, 55.0)));
  Serial.println("sChg Voltage" + String(measureVoltage(sChg, 55.0)));
  Serial.println("von Voltage" + String(measureVoltage(von, 55.0)));
  delay(1000);
}

float measureVoltage(int pin, float Max)
  {
    float voltage = analogRead(pin);
    voltage = (voltage * Max) / 1023.0;
    return voltage + 0.16;
  }
