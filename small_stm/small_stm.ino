#include <Wire.h>
#define SLAVE_ADDRESS 30  // I2C address of the STM32 slave

uint8_t received_data[14];  // Buffer to store received data
int received_length = 0;

void setup() {
  Serial.begin(115200);  // Start serial communication for debugging
  Serial.println("Starting I2c...");
  Wire.begin();  // Join I2C bus as master
  Serial.println("Done!");
  pinMode(LED_BUILTIN, OUTPUT);  // Optional: Use the built-in LED for success indication
}

void loop() {
  Serial.println("Sending Data....");
  Wire.beginTransmission(40);
  Wire.write(200);
  Wire.endTransmission();
  delay(1000);
  // Request data from STM32 slave
  Serial.println("Requesting data...");
  Wire.requestFrom(SLAVE_ADDRESS, sizeof(received_data));
  Serial.println("Done!");

  received_length = 0;
  // Read received data byte by byte
  while (Wire.available()) {
    received_data[received_length++] = Wire.read();
  }

  // If data is received, print it over Serial and toggle LED
  if (received_length > 0) {
    Serial.print("Received from slave: ");
    for (int i = 0; i < received_length; i++) {
      Serial.print((char)received_data[i]);  // Cast to char to print as a string
    }
    Serial.println();

    digitalWrite(LED_BUILTIN, HIGH);  // Indicate success
    delay(200);  // Small delay for visual effect
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(1000);  // 1 second delay before the next request
  Serial.println("Retrying...");
}
