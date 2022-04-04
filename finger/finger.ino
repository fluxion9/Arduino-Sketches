#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
bool userstate[127];
#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(9, 10);
#define rst A5
#define Backlight 8
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id = 1;
int count = 0;
bool enrollment = false;
void setup() {
  lcd.begin(16, 2);
  pinMode(Backlight, 1);
  digitalWrite(Backlight, 1);
  lcd.setCursor(0, 0);
  lcd.print("Hello!");
  delay(1500);
  lcd.setCursor(0, 1);
  lcd.print("Initializing....   ");
  delay(1500);
  lcd.clear();
  pinMode(rst, 2);
  finger.begin(57600);
  if (finger.verifyPassword()) {
  } else {
    lcd.setCursor(0, 0);
    lcd.print("No finger sensor");
    while (1) {
      delay(1);
    }
  }
  if (!digitalRead(rst))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("#Enrollment#");
    enrollment = true;
    finger.emptyDatabase();
    delay(1500);
    while (enrollment)
    {
      while (!getFingerprintEnroll());
      delay(300);
      if (!digitalRead(rst))
      {
        enrollment = false;
        delay(100);
      }
    }
  }
  for (byte i = 0; i < 127; i++)
  {
    userstate[i] = false;
  }
  lcd.setCursor(0, 0);
  lcd.print("Place the finger....");
  lcd.setCursor(0, 1);
  lcd.print("Persons: " + String(count) + "        ");
}

void loop() {
  byte val = scanFingerprint();
  if (val > 0)
  {
    if (!userstate[val - 1])
    {
      count++;
      lcd.setCursor(0, 1);
      lcd.print("Persons: " + String(count) + "        ");
      userstate[val - 1] = true;
    }
  }
  delay(1000);
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        break;
      case FINGERPRINT_NOFINGER:
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        break;
      case FINGERPRINT_IMAGEFAIL:
        break;
      default:
        break;
    }
    if (!digitalRead(rst))
    {
      enrollment = false;
      delay(100);
    }
  }


  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0, 1);
      lcd.print("Finger Scanned.        ");
      delay(700);
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.setCursor(0, 1);
      lcd.print("Image messy.        ");
      delay(700);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0, 1);
      lcd.print("comm error.        ");
      delay(700);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.setCursor(0, 1);
      lcd.print("error.        ");
      delay(700);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.setCursor(0, 1);
      lcd.print("no feature.        ");
      delay(700);
      return p;
    default:
      return p;
  }
  lcd.setCursor(0, 1);
  lcd.print("remove finger.        ");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
    if (!digitalRead(rst))
    {
      enrollment = false;
      delay(100);
    }
    delay(1000);
  }
  p = -1;
  lcd.setCursor(0, 1);
  lcd.print("place the finger.        ");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        break;
      case FINGERPRINT_NOFINGER:
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        break;
      case FINGERPRINT_IMAGEFAIL:
        break;
      default:
        break;
    }
    if (!digitalRead(rst))
    {
      enrollment = false;
      delay(100);
    }
  }
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0, 1);
      lcd.print("Finger Scanned.        ");
      delay(700);
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.setCursor(0, 1);
      lcd.print("Image messy.        ");
      delay(700);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0, 1);
      lcd.print("comm error.        ");
      delay(700);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.setCursor(0, 1);
      lcd.print("error.        ");
      delay(700);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.setCursor(0, 1);
      lcd.print("no feature.        ");
      delay(700);
      return p;
    default:
      lcd.setCursor(0, 1);
      lcd.print("Unknown error");
      delay(700);
      return p;
  }


  p = finger.createModel();

  if (p == FINGERPRINT_OK) {
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    return p;
  } else {
    return p;
  }

  p = finger.storeModel(id);


  if (p == FINGERPRINT_OK) {
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    return p;
  } else {
    return p;
  }
  id++;
  lcd.setCursor(0, 1);
  lcd.print("finger added.        ");
  if (!digitalRead(rst))
  {
    enrollment = false;
    delay(100);
  }
  return true;
}


byte scanFingerprint() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      return false;
    case FINGERPRINT_IMAGEFAIL:
      return false;
    default:
      return false;
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      return false;
    case FINGERPRINT_FEATUREFAIL:
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      return false;
    default:
      return false;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return false;
  } else if (p == FINGERPRINT_NOTFOUND) {
    return false;
  } else {
    return false;
  }

  return finger.fingerID;
}
