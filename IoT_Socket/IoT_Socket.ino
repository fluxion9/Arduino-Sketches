const int voltagePin = A0;  
const float voltageDividerRatio = 2.0;

const int currentPin = A1;
ACS712 currentSensor(ACS712_30A, currentPin);

const int relayPin = 8;
bool isOutputOn = false;

float voltage, current, power, energyConsumed;
unsigned long lastMillis = 0;

const float voltageReference = 5.0;
float currentLimit = 10.0;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(9600);

  Serial.println("Ready");

  currentSensor.begin();
}

void loop() {
  // Serial command handling
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("/off")) {
      digitalWrite(relayPin, LOW);
      isOutputOn = false;
    } else if (command.startsWith("/on")) {
      digitalWrite(relayPin, HIGH);
      isOutputOn = true;
    } else if (command.startsWith("/limit")) {
      command.remove(0, 7); 
      float newLimit = command.toFloat();
      currentLimit = newLimit;
    } else if (command.startsWith("/getdata")) {
      // Send data over serial in JSON format
      if (isOutputOn) {
        Serial.print("{");
        Serial.print("\"voltage\": ");
        Serial.print(voltage);
        Serial.print(", \"current\": ");
        Serial.print(current);
        Serial.print(", \"power\": ");
        Serial.print(power);
        Serial.print(", \"energy\": ");
        Serial.print(energyConsumed);
        Serial.println("}");
      }
    }
  }

  int rawVoltage = analogRead(voltagePin);
  voltage = ((float)rawVoltage / 1023.0) * voltageReference * voltageDividerRatio;

  current = currentSensor.getCurrentAC();

  power = voltage * current;

  unsigned long currentMillis = millis();
  float deltaTime = (currentMillis - lastMillis) / 1000.0;
  energyConsumed += (power * deltaTime) / 3600000.0;
  lastMillis = currentMillis;

  if (current > currentLimit) {
    digitalWrite(relayPin, LOW);
  }

  delay(1000);
}
