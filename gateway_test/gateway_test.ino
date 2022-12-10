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
  
  select_mode(3);
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
  if(Serial.available())
  {
    char data = Serial.read();
    if(data == 'A')
    {
      send_command(0);
    }
    else if(data == 'B')
    {
      send_command(2);
    }
    else if(data == 'C')
    {
      send_command(3);
    }
    else if(data == 'D')
    {
      send_command(4);
    }
//    switch(data)
//    {
//      case 'A':
//        send_command(0);
//        break;
//      case 'B':
//       send_command(2);
//       break;
//      case 'C':
//       send_command(3);
//       break;
//      case 'D':
//       send_command(4);
//       break;
//      default:
//        break;
//    }
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

void send_command(byte command)
{
    if(command == 0)
    {
      select_mode(3);
      Serial.println("Restore Factory settings: ");
      delay(1000);
      uint8_t CMD[3] = {0xC9, 0xC9, 0xC9};
      lora.write(CMD, 3);
    }
    else if(command == 1)
    {
      select_mode(0);
      Serial.println("Message: ");
      delay(1000);
      uint8_t MSG[5] = {0x68, 0x65, 0x6C, 0x6C, 0x6F};
      lora.write(MSG, 5);
    }
    else if(command == 2)
    {
      select_mode(3);
      Serial.println("Handshake: ");
      delay(1000);
      uint8_t MSG2[3] = {0xE1, 0xE1, 0xE1};
      lora.write(MSG2, 3);
    }
    else if(command == 3)
    {
      select_mode(3);
      Serial.println("Current Config: ");
      delay(1000);
      uint8_t MSG3[3] = {0xC1, 0xC1, 0xC1};
      lora.write(MSG3, 3);
    }
    else if(command == 4)
    {
      select_mode(3);
      Serial.println("Module Version: ");
      delay(1000);
      uint8_t MSG4[3] = {0xC3, 0xC3, 0xC3};
      lora.write(MSG4, 3);
    }
//  switch (command)
//  {
//    case 0:
//      select_mode(3);
//      delay(1000);
//      uint8_t CMD[3] = {0xC9, 0xC9, 0xC9};
//      lora.write(CMD, 3);
//      break;
//    case 1:
//      select_mode(0);
//      delay(1000);
//      uint8_t MSG[5] = {0x68, 0x65, 0x6C, 0x6C, 0x6F};
//      lora.write(MSG, 5);
//      break;
//    case 2:
//      select_mode(3);
//      delay(1000);
//      uint8_t MSG2[3] = {0xE1, 0xE1, 0xE1};
//      lora.write(MSG2, 3);
//      break;
//    case 3:
//      select_mode(3);
//      Serial.println("Current Config: ");
//      delay(1000);
//      uint8_t MSG3[3] = {0xC1, 0xC1, 0xC1};
//      lora.write(MSG3, 3);
//      break;
//    case 4:
//      select_mode(3);
//      Serial.println("Module Version: ");
//      delay(1000);
//      uint8_t MSG4[3] = {0xC3, 0xC3, 0xC3};
//      lora.write(MSG4, 3);
//      break;
//  }
}
