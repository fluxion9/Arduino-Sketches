#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27 for a 16x2 LCD

const int buzzerPin = 9;  // Buzzer connected to pin 9

const int mq2Pin = A0;  // MQ2 sensor connected to analog pin A0

float threshold = 120.0;

void setup() {
  pinMode(buzzerPin, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas Detection");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("Warming up...   ");
  delay(20000L);
  lcd.setCursor(0, 1);
  lcd.print("Done!           ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas Detection");
  while(readConc() > threshold);
  for(int i = 0; i < 2; i++)
  {
    digitalWrite(buzzerPin, 1);
    delay(150);
    digitalWrite(buzzerPin, 0);
    delay(150);
  }
}

float readConc()
{
  float sensorValue = analogRead(mq2Pin);
  float voltage = sensorValue / 1024.0 * 5.0;  // Convert analog reading to voltage
  float ppm = voltage * 100;                   // Convert voltage to parts per million (ppm)
  return ppm;
}

void loop() {
  float ppm = readConc();
  // Display the concentration on LCD
  lcd.setCursor(0, 1);
  lcd.print("Conc: ");
  lcd.print(ppm);
  lcd.print(" ppm");
  // Detect gas and beep if detected
  if (ppm > threshold) {  // Change threshold as needed
    lcd.setCursor(0, 0);
    lcd.print("Gas Detected!  ");
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
    while(readConc() > threshold);
    delay(1000);                    // Beep for 1 second
    digitalWrite(buzzerPin, LOW);   // Turn off the buzzer
    delay(1000);                    // Wait for 1 second before detecting again
    lcd.setCursor(0, 0);
    lcd.print("Gas Detection");
  }
  delay(1000);  // Wait for 1 second before next reading
}
