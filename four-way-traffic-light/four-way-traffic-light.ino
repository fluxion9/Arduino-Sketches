// Traffic Light - 4 Way Intersection
// Arduino Nano, LEDs on pins 2-13
// 4 Red, 4 Yellow, 4 Green LEDs

// Assign pins
int redPins[4]    = {2, 5, 8, 11};   // Reds
int yellowPins[4] = {3, 6, 9, 12};   // Yellows
int greenPins[4]  = {4, 7, 10, 13};  // Greens

void setup() {
  // Set all pins as outputs
  for (int i = 0; i < 4; i++) {
    pinMode(redPins[i], OUTPUT);
    pinMode(yellowPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
  }

  // Run startup light show
  initSequence();
}

void loop() {
  // Main 4-way traffic light sequence
  for (int i = 0; i < 4; i++) {
    trafficCycle(i);
  }
}

// ========== Functions ==========

// Startup flashing sequence
void initSequence() {
  // Forward (Red → Yellow → Green)
  flashGroup(redPins, 4, 300);
  flashGroup(yellowPins, 4, 300);
  flashGroup(greenPins, 4, 300);

  // Backward (Green → Yellow → Red, faster)
  flashGroup(greenPins, 4, 200);
  flashGroup(yellowPins, 4, 200);
  flashGroup(redPins, 4, 200);

  delay(500); // short pause before starting
}

// Flash group of LEDs
void flashGroup(int pins[], int size, int duration) {
  for (int i = 0; i < size; i++) digitalWrite(pins[i], HIGH);
  delay(duration);
  for (int i = 0; i < size; i++) digitalWrite(pins[i], LOW);
  delay(200);
}

// Run a traffic cycle for one direction
void trafficCycle(int side) {
  // Step 1: Red ON for all
  setAll(redPins, 4, HIGH);
  setAll(yellowPins, 4, LOW);
  setAll(greenPins, 4, LOW);

  // Step 2: Green ON for current side, others stay red
  digitalWrite(redPins[side], LOW);
  digitalWrite(greenPins[side], HIGH);
  delay(5000); // green time (5s)

  // Step 3: Yellow ON for current side
  digitalWrite(greenPins[side], LOW);
  digitalWrite(yellowPins[side], HIGH);
  delay(2000); // yellow time (2s)

  // Step 4: Back to red
  digitalWrite(yellowPins[side], LOW);
  digitalWrite(redPins[side], HIGH);
  delay(500); // small buffer before next side
}

// Helper: set all pins in a group
void setAll(int pins[], int size, int state) {
  for (int i = 0; i < size; i++) {
    digitalWrite(pins[i], state);
  }
}
