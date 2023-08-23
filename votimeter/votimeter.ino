void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, 0);
}

void loop() {
  float voltage = measureVoltageDC(A0, 1.0);
  Serial.println(fmap(voltage, 4.06, 0.15, 15.0, 42.0));
//  Serial.println(voltage);
  delay(100);
}

float measureVoltageDC(byte pin, float vdr)
{
  float Value = analogRead(pin);
  Value = (Value * 5.0) / 1023.0;
  Value = Value * vdr;
  return Value;
}

float fmap(float val, float fromLow, float fromHigh, float toLow, float toHigh)
{
  float norm = (val - fromLow) / (fromHigh - fromLow);
  float lerp = norm * (toHigh - toLow) + toLow;
  return lerp;
}
