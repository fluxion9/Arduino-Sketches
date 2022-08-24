#include <EEPROM.h>     
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define redLed A4
#define greenLed A5
#define buzzer A3
bool programMode = false;
String uid;
int lastPos;

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

String cards[10] = {}; // array that stores uid cards


void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, 1);

  for(int y = 0; y < 3; ++y)
  {
    digitalWrite(greenLed, 1);
    delay(300);
    digitalWrite(greenLed, 0);
    delay(300);
  }
  digitalWrite(buzzer, 0);

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  ShowReaderDetails();


  if (EEPROM.read(1) != 143) {
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    do {
      successRead = getID();  
      for(int y = 0; y < 2; ++y)
      {
        lcd.setCursor(14 + y, 1);
        lcd.print('.');
        delay(250);      
      }
      for(int y = 0; y < 2; ++y)
      {
        lcd.setCursor(14 + y, 1);
        lcd.print(' ');    
      }
    }while (!successRead);
    for ( uint8_t j = 0; j < 4; j++ ) {       
      EEPROM.write( 2 + j, readCard[j] );
    }
    EEPROM.write(1, 143);
    Serial.println(F("Master Card Set"));
  }
  String id;
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for ( uint8_t i = 0; i < 4; i++ ) {
    masterCard[i] = EEPROM.read(2 + i);
    Serial.print(masterCard[i], HEX);
    id += String(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
}

void loop() {
    do {
    successRead = getID();
    if (programMode) {
      cycleLeds();
    }
    else {
      normalModeOn();
    }
  }while (!successRead);
  if (programMode) {
    if (isMaster(readCard)) {
      Serial.println(F("Master Card Scanned"));
      Serial.println(F("Exiting Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
    }
    else {
      if (findID(readCard)) {
        Serial.println(F("PICC recognised, removing..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Scan a PICC to ADD or REMOVE from memory"));
      }
      else {                   
        Serial.println(F("New PICC, adding..."));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Scan a PICC to ADD or REMOVE from memory"));
      }
    }
  }
  else {
    if (isMaster(readCard)) {
      programMode = true;
      Serial.println(F("Master Card Detected - Entered Program Mode"));
      uint8_t count = EEPROM.read(0);
      Serial.print(F("Found "));
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      Serial.println(F("Scan Master Card again to Exit Program Mode"));
      Serial.println(F("-----------------------------"));
    }
    else {
      if (findID(readCard)) {
        int index = lookUp(cards, sizeof(cards), compileId(readCard, sizeof(readCard)));
        Serial.println("Index is " + String(index));
        if(!index)
        {
          int pos = lastPos;
          cards[pos] = compileId(readCard, sizeof(readCard));
          status[pos] = "signedIn";
          Serial.print(F("Welcome! "));
          Serial.println(compileId(readCard, sizeof(readCard)));
          granted(300);
          lastPos++;
          delay(2000);
        }
        else if(index > 0 && status[index - 1] == "signedIn") {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("#ACCESS GRANTED#");
          lcd.setCursor(0, 1);
          delay(500);
          lcd.print("ID: " + compileId(readCard, sizeof(readCard)));
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("Bye " + users[index-1]);
          granted(300);
          status[index - 1] = "signedOut";
          delay(2000);
          lcd.setCursor(0, 0);
          lcd.print("Place a Card:   ");
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
        else if(index > 0 && status[index - 1] == "signedOut")
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("#ACCESS GRANTED#");
          lcd.setCursor(0, 1);
          delay(500);
          lcd.print("ID: " + compileId(readCard, sizeof(readCard)));
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("Welcome" + users[index-1]);
          granted(300);
          status[index - 1] = "signedIn";
          delay(2000);
          lcd.setCursor(0, 0);
          lcd.print("Place a Card:   ");
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
      }
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("#ACCESS DENIED#");
        lcd.setCursor(0, 1);
        delay(500);
        lcd.print("I don't know you");
        Serial.println(F("I don't know you"));
        denied();
        delay(2000);
        lcd.setCursor(0, 0);
        lcd.print("Place a Card:   ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
      }
    }
  }
}
