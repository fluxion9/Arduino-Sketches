#define buzzer 13
#define btn_1 6
#define btn_2 7
#define btn_3 8
#define btn_4 9

void setup() {
  pinMode(buzzer, 1);
  pinMode(btn_1, INPUT_PULLUP);
  pinMode(btn_2, INPUT_PULLUP);
  pinMode(btn_3, INPUT_PULLUP);
  pinMode(btn_4, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte val = read_keys();
//  switch (val)
//  {
//    case 1:
//      break;
//    case 2:
//      break;
//    case 3:
//      break;
//    case 4:
//      break;
//    default:
//      break;
//  }
}

byte read_keys()
{
  if (!digitalRead(btn_1))
  {
    beep();
    return 1;
  }
  else if (!digitalRead(btn_2))
  {
    beep();
    return 2;
  }
  else if (!digitalRead(btn_3))
  {
    beep();
    return 3;
  }
  else if (!digitalRead(btn_4))
  {
    beep();
    return 4;
  }
  else {
    return 0;
  }
}

void beep()
{
  digitalWrite(buzzer, 1);
  delay(100);
  digitalWrite(buzzer, 0);
}
