// C++ code
//

int ldrVal1;
int ldrVal2;

void setup()
{
  Serial.begin(9600); // Start Serial communication with the serial monitor at 9600 Baud Rate
}

void loop()
{
  ldrVal1 = analogRead(A5); // Read analog value at A2
  ldrVal2 = analogRead(A4); // Read analog value at A4

  Serial.print("ldr val1: ");
  Serial.println(ldrVal1); // Print analog value at A2
  Serial.print("ldr val2: ");
  Serial.println(ldrVal2); // Print analog value at a4

  delay(1000); //wait for 1 second
}
