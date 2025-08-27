#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <TM1637Display.h>

// --- Pins ---
#define CLK 3         // TM1637 CLK
#define DIO 2         // TM1637 DIO
#define BUZZER 4      // Buzzer pin
#define BUTTON 5      // Push button pin (connected to GND when pressed)

// --- Devices ---
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
TM1637Display display(CLK, DIO);

// --- Timing control ---
unsigned long displayUntil = 0;   // when to clear display
bool showingTemp = false;

void setup() {
  Wire.begin();
  mlx.begin();

  display.setBrightness(0x0f);
  display.showNumberDec(0, true);  // show 0000 on boot

  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);   // button between pin and GND
}

void loop() {
  unsigned long now = millis();

  // --- Button handling ---
  if (digitalRead(BUTTON) == LOW) {   // pressed
    beep();

    float tempC = mlx.readObjectTempC();
    int displayTemp = (int)(tempC * 10); // keep 1 decimal place

    // show as 36:5 (colon used as fake decimal point)
    display.showNumberDecEx(displayTemp, 0b00100000, true, 4, 0);

    displayUntil = now + 5000;  // show for 5s
    showingTemp = true;

    delay(300); // basic debounce
  }

  // --- Timeout clearing ---
  if (showingTemp && now > displayUntil) {
    display.showNumberDec(0, true); // back to 0000
    showingTemp = false;
  }
}

// --- Buzzer helper ---
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}
