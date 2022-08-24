#include <EEPROM.h>     
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <Servo.h>
Servo servo;

#define redLed A4
#define greenLed A5
#define buzzer A3
#define servoPin 11


bool programMode = false;

#define openPos 0
#define closePos 90

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);



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

  servo.attach(servoPin);
  
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
        //granted
        granted();
        openDoor(10);
      }
      else {
        //denied
        denied();
      }
    }
  }
}

void granted() {
  for(int x = 0; x < 3; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(50);
    digitalWrite(buzzer, 0);
    delay(50);
  }
  digitalWrite(redLed, 0);
  digitalWrite(greenLed, 1);
}
void denied() {
  for(int x = 0; x < 2; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(200);
    digitalWrite(buzzer, 0);
    delay(100);
  }
  digitalWrite(greenLed, 0);  // Make sure green LED is off
  digitalWrite(redLed, 1);   // Turn on red LED
}
uint8_t getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA();
  return 1;
}

void ShowReaderDetails() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    digitalWrite(greenLed, 0);  // Make sure green LED is off
    digitalWrite(redLed, 1);   // Turn on red LED
    while (true); // halt
  }
}
void cycleLeds() {
  digitalWrite(redLed, 0);  
  digitalWrite(greenLed, 1);  
  delay(200);
  digitalWrite(redLed, 1);  
  digitalWrite(greenLed, 0);    
}
void normalModeOn () {
  digitalWrite(redLed, 1);
  digitalWrite(greenLed, 0);
}

void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}
void writeID( byte a[] ) {
  if ( !findID( a ) ) {
    uint8_t num = EEPROM.read(0);    
    uint8_t start = ( num * 4 ) + 6;
    num++;               
    EEPROM.write( 0, num );
    for ( uint8_t j = 0; j < 4; j++ ) {
      EEPROM.write( start + j, a[j] );
    }
    successWrite();
    Serial.println(F("Succesfully added ID record to memory"));
  }
  else {
    failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or memory"));
  }
}
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {
    failedWrite();     
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);  
    uint8_t slot;      
    uint8_t start;     
    uint8_t looping;    
    uint8_t j;
    uint8_t count = EEPROM.read(0);
    slot = findIDSLOT( a );
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      
    EEPROM.write( 0, num );  
    for ( j = 0; j < looping; j++ ) {        
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));
    }
    for ( uint8_t k = 0; k < 4; k++ ) {
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("Succesfully removed ID record from memory"));
  }
}
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {
    if ( a[k] != b[k] ) {
       return false;
    }
  }
  return true;  
}
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);   
  for ( uint8_t i = 1; i <= count; i++ ) { 
    readID(i);
    if ( checkTwo( find, storedCard ) ) { 
      return i;
    }
  }
}
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);
  for ( uint8_t i = 1; i < count; i++ ) {
    readID(i);      
    if ( checkTwo( find, storedCard ) ) { 
      return true;
    }
    else { 
    }
  }
  return false;
}
void successWrite() {
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 1); 
  for(int x = 0; x < 3; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(50);
    digitalWrite(buzzer, 0);
    delay(50);
  }
  delay(1000);
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 0);
}
void failedWrite() {
  digitalWrite(redLed, 1); 
  digitalWrite(greenLed, 0); 
  for(int x = 0; x < 2; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(200);
    digitalWrite(buzzer, 0);
    delay(100);
  }
  for(int x = 0; x < 2; ++x)
  {
    digitalWrite(redLed, 1);
    delay(100);
    digitalWrite(redLed, 0);
    delay(100);
  }
}
void successDelete() {
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 1); 
  for(int x = 0; x < 3; ++x)
  {
    digitalWrite(buzzer, 1);
    delay(50);
    digitalWrite(buzzer, 0);
    delay(50);
  }
  delay(1000);
  digitalWrite(redLed, 0); 
  digitalWrite(greenLed, 0);
}
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}

void openDoor(float Delay)
{
  servo.write(openPos);
  delay(int(Delay * 1000));
  servo.write(closePos);
}
