
float measureVoltage(int pin, float Max)
{
  float voltage = analogRead(pin);
  voltage = (voltage * Max) / 1024.0;
  return voltage;
}

float fmap(float val, float fromLow, float fromHigh, float toLow, float toHigh)
{
  float norm = (val - fromLow) / (fromHigh - fromLow);
  float lerp = norm * (toHigh - toLow) + toLow;
  return lerp;
}
int dutyCycle = 128;
float setVoltage = 0;
float outputVoltage = 0;

void setup() {
  TCCR2B = TCCR2B & B11111000 | B00000001;
  pinMode(A5, 0);
  pinMode(A2, 0);
  pinMode(11, 1);
}

void loop() {
  setVoltage = fmap((float)analogRead(A5), 0.0, 1023.0, 0.0, 20.0);
  outputVoltage = measureVoltage(A2, 55.0);
  if(outputVoltage < setVoltage)
  {
    dutyCycle -= 1;
    dutyCycle = constrain(dutyCycle, 1, 254);
  }
  if(outputVoltage > setVoltage)
  {
    dutyCycle += 1;
    dutyCycle = constrain(dutyCycle, 1, 254);
  }
  analogWrite(11, dutyCycle);
}
