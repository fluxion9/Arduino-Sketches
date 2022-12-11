#include <SoftwareSerial.h>

#define M0 8
#define M1 7
#define Tx 5
#define Rx 6
#define Aux 4

SoftwareSerial lora(Tx, Rx);

void setup() {
  pinMode(M0, 1);
  pinMode(M1, 1);
 
  pinMode(Aux, 0);
  pinMode(13, 1);
  
  lora.begin(9600);
  Serial.begin(9600);
  
  select_mode(0);
  delay(2000);
}

void loop() {
  check_response();
}

void check_response()
{
  if (lora.available())
  {
    while (lora.available() > 1)
    {
      Serial.write(lora.read());
    }
  }
  if (Serial.available())
  {
    while (Serial.available() > 1)
    {
      lora.write(Serial.read());
    }
  }
}
void select_mode(byte mode)
{
  switch (mode)
  {
    case 0:
      digitalWrite(M0, 0);
      digitalWrite(M1, 0);
      break;
    case 1:
      digitalWrite(M0, 0);
      digitalWrite(M1, 1);
      break;
    case 2:
      digitalWrite(M0, 1);
      digitalWrite(M1, 0);
      break;
    case 3:
      digitalWrite(M0, 1);
      digitalWrite(M1, 1);
      break;
    default:
      digitalWrite(M0, 0);
      digitalWrite(M1, 0);
      break;
  }
}
