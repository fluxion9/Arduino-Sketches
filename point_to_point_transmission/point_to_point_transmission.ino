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
#define SPEED 0x24
#define Tx_CHAN 0x1C
#define Rx_CHAN 0x1D
#define OPTION 0xC0

#define PREFIX 0xFF

String data = "";

SoftwareSerial lora(Tx, Rx);

uint8_t msg_buf[32];

void setup() {
  pinMode(M0, 1);
  pinMode(M1, 1);

  pinMode(Aux, 0);
  pinMode(13, 1);

  lora.begin(19200);
  Serial.begin(9600);

  select_mode(0);
  delay(2000);
}

void loop() {
  check_response();
  delay(5);
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
  if (Serial.available() > 0)
  {
    data = Serial.readString();
    send_BP2P(data, 0);
  }
}

void send_BP2P(String msg, byte channel)
{
  if (channel == 0)
  {
    select_mode(0);
    delay(100);
    msg.toCharArray(msg_buf, 32);
    uint8_t buf[3] = {Tx_ADDH, Tx_ADDL, Tx_CHAN};
    lora.write(buf, 3);
    lora.write(msg_buf, 32);
  }
  else if (channel == 1)
  {
    select_mode(0);
    delay(100);
    msg.toCharArray(msg_buf, 32);
    uint8_t buf[3] = {PREFIX, PREFIX, Rx_CHAN};
    lora.write(buf, 3);
    lora.write(msg_buf, 32);
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
