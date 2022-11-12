#include <rdm6300.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
//#include <Servo.h>
#include <Stepper.h>



//Servo servo;
Rdm6300 rdm6300;

#define redLed A2
#define greenLed A0
#define buzzer 6
//#define servoPin 9
#define RDM6300_RX_PIN 8

#define STEPS 100
#define buzz_freq 2000
Stepper stepper(STEPS, 9, 10, 11, 12);

bool programMode = false;

#define openPos 0
#define closePos 90

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
byte byteArray[4]; //stores converted integer to byteArray

struct cReader
{
  void Init( void )
  {
    pinMode(redLed, OUTPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(buzzer, OUTPUT);
    tone(buzzer, buzz_freq);

    for (int y = 0; y < 3; ++y)
    {
      digitalWrite(greenLed, 1);
      delay(300);
      digitalWrite(greenLed, 0);
      delay(300);
    }
    noTone( buzzer );

    //servo.attach(servoPin);
    stepper.setSpeed(60);
    rdm6300.begin(RDM6300_RX_PIN);
    Serial.begin(9600);
    if (EEPROM.read(1) != 143) {
      Serial.println(F("No Master Card Defined"));
      Serial.println(F("Scan A PICC to Define as Master Card"));
      do {
        successRead = getID();
      } while (!successRead);
      for ( uint8_t j = 0; j < 4; j++ ) {
        EEPROM.write( 2 + j, readCard[j] );
      }
      EEPROM.write(1, 143);
      Serial.println(F("Master Card Set"));
    }
    Serial.println(F("-------------------"));
    Serial.println(F("Master Card's UID"));
    for ( uint8_t i = 0; i < 4; i++ ) {
      masterCard[i] = EEPROM.read(2 + i);
      Serial.print(masterCard[i], HEX);
    }
    Serial.println("");
    Serial.println(F("-------------------"));
    Serial.println(F("Everything is ready"));
    Serial.println(F("Waiting PICCs to be scanned"));
  }

  void granted() {
    for (int x = 0; x < 3; ++x)
    {
      //digitalWrite( buzzer, 1 )
      tone(buzzer, buzz_freq);
      delay( 50 );
      noTone( buzzer );
      delay( 50 );
    }
    digitalWrite( redLed, 0 );
    digitalWrite( greenLed, 1 );
    Serial.println("ACCESS GRANTED!");
  }
  void denied() {
    for (int x = 0; x < 2; ++x)
    {
      tone(buzzer, buzz_freq);
      delay( 200 );
      noTone( buzzer );
      delay( 100 );
    }
    digitalWrite( greenLed, 0 );  // Make sure green LED is off
    digitalWrite( redLed, 1 );   // Turn on red LED
    Serial.println("ACCESS DENIED!");
  }
  uint8_t getID() {
    if (!rdm6300.get_new_tag_id()) {
      return 0;
    }
    intToByteArray(rdm6300.get_tag_id());
    Serial.println( F( "Scanned PICC's UID:" ) );
    for ( uint8_t i = 0; i < 4; i++ ) {
      readCard[i] = byteArray[i];
      Serial.print( readCard[i], HEX );
    }
    Serial.println( "" );
    return 1;
  }

  void intToByteArray(uint32_t num)
  {
    byteArray[3] = (num << (8 * 3)) >> (8 * 3);
    byteArray[2] = (num << (8 * 2)) >> (8 * 3);
    byteArray[1] = (num << (8 * 1)) >> (8 * 3);
    byteArray[0] = num >> (8 * 3);
  }

  void cycleLeds() {
    digitalWrite( redLed, 0 );
    digitalWrite( greenLed, 1 );
    delay( 200 );
    digitalWrite( redLed, 1 );
    digitalWrite( greenLed, 0 );
    delay( 200 );
  }

  void normalModeOn () {
    digitalWrite( redLed, 1 );
    digitalWrite( greenLed, 0 );
  }

  void readID( uint8_t number ) {
    uint8_t start = ( number * 4 ) + 2;    // Figure out starting position
    for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
      storedCard[i] = EEPROM.read( start + i );   // Assign values read from EEPROM to array
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
      Serial.println( F( "Succesfully added ID record to memory" ) );
    }
    else {
      failedWrite();
      Serial.println( F( "Failed! There is something wrong with ID or memory" ) );
    }
  }
  void deleteID( byte a[] ) {
    if ( !findID( a ) ) {
      failedWrite();
      Serial.println( F( "Failed! There is something wrong with ID or bad EEPROM" ) );
    }
    else {
      uint8_t num = EEPROM.read( 0 );
      uint8_t slot;
      uint8_t start;
      uint8_t looping;
      uint8_t j;
      uint8_t count = EEPROM.read( 0 );
      slot = findIDSLOT( a );
      start = ( slot * 4 ) + 2;
      looping = ( ( num - slot ) * 4 );
      num--;
      EEPROM.write( 0, num );
      for ( j = 0; j < looping; j++ ) {
        EEPROM.write( start + j, EEPROM.read( start + 4 + j ) );
      }
      for ( uint8_t k = 0; k < 4; k++ ) {
        EEPROM.write( start + j + k, 0  );
      }
      successDelete();
      Serial.println( F( "Succesfully removed ID record from memory" ) );
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
    uint8_t count = EEPROM.read( 0 );
    for ( uint8_t i = 1; i <= count; i++ ) {
      readID(i);
      if ( checkTwo( find, storedCard ) ) {
        return i;
      }
    }
  }
  bool findID( byte find[] ) {
    uint8_t count = EEPROM.read( 0 );
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
    digitalWrite( redLed, 0 );
    digitalWrite( greenLed, 1 );
    for (int x = 0; x < 3; ++x)
    {
      tone(buzzer, buzz_freq);
      delay(50);
      noTone(buzzer);
      delay(50);
    }
    delay(1000);
    digitalWrite( redLed, 0 );
    digitalWrite( greenLed, 0 );
  }
  void failedWrite() {
    digitalWrite( redLed, 1 );
    digitalWrite( greenLed, 0 );
    for ( int x = 0; x < 2; ++x )
    {
      tone(buzzer, buzz_freq);
      delay( 200 );
      noTone(buzzer);
      delay( 100 );
    }
    for ( int x = 0; x < 2; ++x )
    {
      digitalWrite( redLed, 1 );
      delay( 100 );
      digitalWrite( redLed, 0 );
      delay( 100 );
    }
  }
  void successDelete() {
    digitalWrite( redLed, 0 );
    digitalWrite( greenLed, 1 );
    for ( int x = 0; x < 3; ++x )
    {
      tone(buzzer, buzz_freq);
      delay( 50 );
      noTone(buzzer);
      delay( 50 );
    }
    delay( 1000 );
    digitalWrite( redLed, 0 );
    digitalWrite( greenLed, 0 );
  }
  bool isMaster( byte test[] ) {
    return checkTwo( test, masterCard );
  }

  void openDoor( float Delay )
  {
    //servo.write(openPos);
    stepper.step(50);
    delay( int( Delay * 1000 ) );
    //servo.write( closePos );
    stepper.step(-50);
  }






  void Run( void )
  {
    do {
      successRead = getID();
      if (programMode) {
        cycleLeds();
      }
      else {
        normalModeOn();
      }
    } while (!successRead);

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
          openDoor(5);
        }
        else {
          //denied
          denied();
        }
      }
    }
  }

} cR;

void setup() {
  cR.Init();
}

void loop() {
  cR.Run();
}
