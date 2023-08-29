class Blinker
{
    int ledPin;
    int gndPin;
    long onTime;
    long offTime;

    int ledState;
    unsigned long previousMillis;
    unsigned long currentMillis;
  public:
    Blinker(int pin, int gnd, long on, long off)
    {
      ledPin = pin;
      gndPin = gnd;
      pinMode(ledPin, OUTPUT);
      pinMode(gndPin, 1);
      digitalWrite(gndPin, 0);

      onTime = on;
      offTime = off;
      ledState = LOW;
      previousMillis = 0;
    }
    void Update()
    {
      currentMillis = millis();
      if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
      {
        ledState = LOW;
        previousMillis = currentMillis;
        digitalWrite(ledPin, ledState);
      }
      else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime))
      {
        ledState = HIGH;
        previousMillis = currentMillis;
        digitalWrite(ledPin, ledState);
      }
    }
};

Blinker led(6, 8, 300, 3000);

void setup() {
  TCCR2B = TCCR2B & B11111000 | B00000010;
  pinMode(3, 1);
  analogWrite(3, 128);
}

void loop() {
  led.Update();
}
