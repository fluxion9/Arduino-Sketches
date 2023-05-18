#define cell1 A2 //voltage divider for cell1 is connected here
#define cell2 A0 //voltage divider for cell2 is connected here
#define cell3 A1 //voltage divider for cell3 is connected here

#define min_voltage 15.0
#define max_voltage 17.0

#define redLed 12
#define blueLed 7

#define inv_switch 4
#define charge 10

#define balance 8

#define fullyCharged 2
#define notFull 4
#define charging 6

byte cells[3] = {cell1, cell2, cell3}; //cell voltage divider array

unsigned long lastMillis = 0; //initializing last time with 0

String Buffer = "", data = "";

class Blinker
{
  int ledPin;
  long onTime;
  long offTime;

  int ledState;
  unsigned long previousMillis;
  unsigned long currentMillis;
  public:
  Blinker(int pin, long on, long off)
  {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);

    onTime = on;
    offTime = off;
    ledState = LOW;
    previousMillis = 0;
  }
  void Update()
  {
    currentMillis = millis();
    if((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
    {
      ledState = LOW;
      previousMillis = currentMillis;
      digitalWrite(ledPin, ledState);
    }
    else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime))
    {
      ledState = HIGH;
      previousMillis = currentMillis;
      digitalWrite(ledPin, ledState);
    }
  }
};

Blinker blinkBlue(blueLed, 300, 5000);

struct BMS
{
  bool b_state = false;
  bool bal_state = false;
  bool pow_state = false;
  
  float cell_voltage[3] = {0, 0, 0};
  float batteryVoltage = 0.0;

  byte state;
  
  int speed = 500;

  unsigned long currentStamp = 0, lastStamp = 0, cTime = 30000, wTime = 3000;

  void init(void)
  {
    Serial.begin(9600);
    for (int i = 0; i < 3; i++)
    {
      pinMode(cells[i], 0);
    }
    pinMode(redLed, 1);
    pinMode(blueLed, 1);
    pinMode(inv_switch, 1);
    pinMode(charge, 1);
    pinMode(balance, 1);

    Buffer.reserve(32);
    data.reserve(15);
    
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(blueLed, 1);
      delay(300);
      digitalWrite(blueLed, 0);
      delay(300);
    }
  }
  void power()
  {
    if(pow_state)
    {
      digitalWrite(inv_switch, 1);
      digitalWrite(redLed, 1);
    }
    else {
      digitalWrite(inv_switch, 0);
      digitalWrite(redLed, 0);
    }
  }
  void readCellVoltages() //method for reading cell voltagesr
  {
    batteryVoltage = 0;
    for (byte i = 0; i < 3; ++i)
    {
      float voltage = analogRead(cells[i]);
      voltage = (voltage * 55.0) / 1023.0;
      cell_voltage[i] = voltage;
    }
    cell_voltage[1] -= cell_voltage[0];
    cell_voltage[2] -= (cell_voltage[1] + cell_voltage[0]);
  }
  void balanceCells()
  {
    if (bal_state)
    {
      if (cells[0] != cells[1] || cells[0] != cells[2] || cells[1] != cells[2])
      {
        if (millis() - lastMillis >= speed)
        {
          if (b_state)
          {
            digitalWrite(balance, 0);
            b_state = !b_state;
          }
          else {
            digitalWrite(balance, 1);
            b_state = !b_state;
          }
          lastMillis = millis();
        }
      }
    }
  }

  void run(void)
  {
    readCellVoltages();
    power();
    balanceCells();
    blinkBlue.Update();
    while (Serial.available() > 0)
    {
      delay(3);
      char c = Serial.read();
      data += c;
    }
    if (data.length() > 0)
    {
      data.trim();
      if (data == "+balanceBat")
      {
        bal_state = !bal_state;
      }
      else if (data == "+power")
      {
        pow_state = !pow_state;
      }
      else if (data == "+read;")
      {
        load_buffer();
        Serial.println(Buffer);
      }
      data = "";
    }
  }

  void Charge(void)
  {
  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"v1\":");
    Buffer.concat(cell_voltage[0]);
    Buffer.concat(",\"v2\":");
    Buffer.concat(cell_voltage[1]);
    Buffer.concat(",\"v3\":");
    Buffer.concat(cell_voltage[2]);
    Buffer.concat(",\"ps\":");
    Buffer.concat(pow_state);
    Buffer.concat(",\"bs\":");
    Buffer.concat(bal_state);
    Buffer.concat("}");
  }

}bms;

void setup() {
  bms.init();
}

void loop() {
  bms.run();
}
