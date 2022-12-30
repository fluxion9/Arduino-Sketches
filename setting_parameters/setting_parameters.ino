#include <SoftwareSerial.h>

#define M0 8
#define M1 7
#define Tx 5
#define Rx 6
#define Aux 4

#define ROM 0xC0
#define RAM 0xC2
#define Rx_ADDH 0x00
#define Rx_ADDL 0x00
#define Tx_ADDH 0xFF
#define Tx_ADDL 0x01
//#define SPEED 0x24
#define SPEED 0x1D
#define Tx_CHAN 0x1C
#define Rx_CHAN 0x1D
#define OPTION 0xC0

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
  if(Serial.available())
  {
    char data = Serial.read();
    if(data == 'A')
    {
      set_param(0);
    }
    else if(data == 'B')
    {
      set_param(1);
    }
    else if(data == 'C')
    {
      set_param(2);
    }
    else if(data == 'D')
    {
      set_param(3);
    }
    else if(data == 'E')
    {
      set_param(4);
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

void set_param(byte id)
{
    if(id == 0)
    {
      select_mode(3);
      delay(100);
      uint8_t buf[6] = {ROM, Rx_ADDH, Rx_ADDL, SPEED, Rx_CHAN, OPTION};
      lora.write(buf, 6);
    }
    else if(id == 1)
    {
      select_mode(3);
      delay(100);
      uint8_t buf[6] = {RAM, Rx_ADDH, Rx_ADDL, SPEED, Rx_CHAN, OPTION};
      lora.write(buf, 6);
    }
    else if(id == 2)
    {
      select_mode(3);
      delay(100);
      uint8_t buf[6] = {ROM, Tx_ADDH, Tx_ADDL, SPEED, Tx_CHAN, OPTION};
      lora.write(buf, 6);
    }
    else if(id == 3)
    {
      select_mode(3);
      delay(100);
      uint8_t buf[6] = {RAM, Tx_ADDH, Tx_ADDL, SPEED, Tx_CHAN, OPTION};
      lora.write(buf, 6);
    }
    else if(id == 4)
    {
      select_mode(0);
      delay(100);
    }
}
