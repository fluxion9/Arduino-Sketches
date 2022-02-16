

void setup() {
  pinMode(A0, 0);
  Serial.begin(9600);
}

void loop() {
  Serial.println(measureAC(A0));
}

float measureAC(int pin)
{
  float val = 0;
  float maxpk = 0, RMS = 0;
  unsigned long Time = millis(), sampleTime = 2000;
  while (millis() - Time <= sampleTime)
  {
    for (int i = 0; i < 300; ++i)
    {
      val += analogRead(pin);
      val /= 2;
    }
    if (val <= 0)
    {
      maxpk = 0;
    }
    else
    {
      if (val > maxpk)
      {
        maxpk = val;
      }
    }
  }
  maxpk = (maxpk * 505.0) / 1023.0;
  RMS = maxpk * 0.707;
  return RMS;
}
