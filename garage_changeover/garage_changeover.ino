#define chargePin A3
#define isolator A2
#define buzzer 10
#define amberLED 9
#define pushSwitch 13
#define vSense A5

bool beepActive = false, pushState = false, ledstate = false;
unsigned long lastblinktime = 0;

const int onblinkinterval = 200;
const int offblinkinterval = 1500;

void blynk() {
  if (ledstate && (millis() - lastblinktime >= onblinkinterval)) {
    digitalWrite(amberLED, 0);
    ledstate = false;
    lastblinktime = millis();
  } else if (!ledstate && (millis() - lastblinktime >= offblinkinterval)) {
    digitalWrite(amberLED, 1);
    ledstate = true;
    lastblinktime = millis();
  }
}

float measureVoltageAC() {
  float temp = analogRead(vSense);
  temp = (temp * 505.0) / 1023.0;
  return temp * 0.707;
}

void beep(int times, int interval) {
  for (int j = 0; j < times; j++) {
    digitalWrite(buzzer, HIGH);
    delay(interval);
    digitalWrite(buzzer, LOW);
    delay(interval);
  }
}

void setup() {
  pinMode(chargePin, 1);
  pinMode(isolator, 1);
  pinMode(buzzer, 1);
  pinMode(amberLED, 1);
  pinMode(pushSwitch, 2);
  pinMode(vSense, 0);
}

void loop() {
  float voltage = measureVoltageAC();
  if (voltage >= 100.0) {
    if (!beepActive) {
      beep(2, 100);
      beepActive = true;
    }
    digitalWrite(isolator, 1);
  } else {
    digitalWrite(isolator, 0);
    beepActive = false;
  }
  if (!digitalRead(pushSwitch) && voltage >= 100.0) {
    if (!pushState) {
      beep(1, 200);
      delay(100);
      digitalWrite(chargePin, 1);
      pushState = true;
    } else if (pushState) {
      beep(1, 200);
      delay(100);
      digitalWrite(chargePin, 0);
      pushState = false;
    }
  }
  if (pushState) {
    blynk();
  }
  else {
    digitalWrite(amberLED, 0);
  }
}
