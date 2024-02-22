#define ledWorking PIN_PD7

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
Blinker blink0(ledWorking, 200, 3000);

void setup() {
  pinMode(ledWorking, 1);
}

void loop() {
  blink0.Update();
}
