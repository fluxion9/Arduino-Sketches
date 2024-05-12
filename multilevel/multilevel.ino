#define DL 11
#define DR 10
#define UL 9
#define UR 8
#define UM 7

void setup() {
  pinMode(DL, 1);
  pinMode(DR, 1);
  pinMode(UL, 1);
  pinMode(UR, 1);
  pinMode(UM, 1);
}

void loop() {
  digitalWrite(DL, 1);

  digitalWrite(UL, 1);
  delay(5);
  digitalWrite(UL, 0);

  digitalWrite(UM, 1);
  delay(5);
  digitalWrite(UM, 0);

  digitalWrite(UR, 1);
  delay(5);
  digitalWrite(UR, 0);

  digitalWrite(UM, 1);
  delay(5);
  digitalWrite(UM, 0);

  digitalWrite(UL, 1);
  delay(5);
  digitalWrite(UL, 0);

  digitalWrite(DL, 0);
  digitalWrite(DR, 1);

  digitalWrite(UR, 1);
  delay(5);
  digitalWrite(UR, 0);

  digitalWrite(UM, 1);
  delay(5);
  digitalWrite(UM, 0);

  digitalWrite(UL, 1);
  delay(5);
  digitalWrite(UL, 0);

  digitalWrite(UM, 1);
  delay(5);
  digitalWrite(UM, 0);

  digitalWrite(UR, 1);
  delay(5);
  digitalWrite(UR, 0);

  digitalWrite(DR, 0);
}

