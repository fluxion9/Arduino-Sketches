//#include <Wire.h>
//pin definations
#include <SoftwareSerial.h>
#define iSense A3
#define chargePin 8
#define output 3
#define cell1 A0
#define cell2 A1
#define cell3 A2
#define lilLed 9


//state definations
#define fullyCharged 2
#define notFull 4
#define charging 6

SoftwareSerial serial(A5, A4);

String b_data = "";

float battCapacity = 1500, fullChargeVoltage = 12.4, lowChargeVoltage = 10.0;

float charge = 0.0, current = 0.0;

float batteryVoltage, batteryPercentage;

float cell_voltage[3] = {};

byte cells[3] = {cell1, cell2, cell3};

unsigned long currentStamp = 0, lastMillis = 0, lastStamp = 0, cTime = 60000, wTime = 3000;

byte state;


struct BMS     //creating a BMS class
{


  void initializeBMS() //method for initializing the BMS
  {
    pinMode(output, 1);
    serial.begin(9600);
    pinMode(lilLed, 1);
    for (byte i = 0; i < 3; ++i)
    {
      pinMode(cells[i], 0);
    }
    float temp = (analogRead(cells[2]) * 55.0) / 1023.0;
    if (temp > lowChargeVoltage && temp < fullChargeVoltage)
    {
      while (1)
      {
        digitalWrite(lilLed, 1);
        delay(100);
        digitalWrite(lilLed, 0);
        delay(100);
      }
    }
    else if (temp <= lowChargeVoltage)
    {
      while (1);
    }
    charge = battCapacity;
    state = fullyCharged;
    pinMode(iSense, 0);
    readCellVoltages();
  }


  float measureCurrent(byte pin)
  {
    float voltage = analogRead(pin);
    voltage = (voltage * 5.0) / 1023.0;
    voltage -= 2.5;
    float current = voltage / 0.0133;
    return current * 1000;
  }


  void checkCondition(void)
  {
    if (batteryVoltage < lowChargeVoltage)
    {
      digitalWrite(output, 0);
    }
    else {
      digitalWrite(output, 0);
    }
  }


  void readCellVoltages(void) //method for reading cell voltages
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
    batteryVoltage = analogRead(cells[2]);
    batteryVoltage = (batteryVoltage * 55.0) / 1023.0;
  }


  void Charge(void)
  {
    currentStamp = millis();
    if (state == notFull && (currentStamp - lastStamp >= wTime))
    {
      readCellVoltages();
      if (batteryPercentage >= 100 && batteryVoltage >= fullChargeVoltage)
      {
        state = fullyCharged;
      }
      else {
        digitalWrite(chargePin, 1);
        state = charging;
      }
      lastStamp = millis();
    }
    else if (state == charging && (currentStamp - lastStamp >= cTime))
    {
      digitalWrite(chargePin, 0);
      readCellVoltages();
      if (batteryPercentage >= 100 && batteryVoltage >= fullChargeVoltage)
      {
        state = fullyCharged;
      }
      else {
        digitalWrite(chargePin, 1);
        state = notFull;
      }
      lastStamp = millis();
    }
    else if (state ==  fullyCharged)
    {
      digitalWrite(chargePin, 0);
      readCellVoltages();
      if (batteryPercentage < 100 || batteryVoltage < fullChargeVoltage)
      {
        state = notFull;
      }
      else {
        state = fullyCharged;
      }
      lastStamp = millis();
    }
  }


  void countQ(void)
  {
    if (millis() - lastMillis >= 1000)
    {
      current = measureCurrent(iSense);
      if (current > 0)
      {
        charge = charge - (current * 0.0002777d);
        if (charge < 0)
        {
          charge = 0;
        }
      }
      else if (current < 0)
      {
        charge = charge + (-1.0 * current * 0.0002777d);
      }
      lastMillis = millis();
    }
  }

  void seriallizeData(void)
  {
    b_data = "[";
    b_data += batteryVoltage;
    b_data += ",";
    b_data += batteryPercentage;
    b_data += ",";
    b_data += charge;
    b_data += ",";          //My Serializing Art ;)
    b_data += current;
    b_data += ",";
    b_data += cell_voltage[0];
    b_data += ",";
    b_data += cell_voltage[1];
    b_data += ",";
    b_data += cell_voltage[2];
    b_data += "]";
    serial.println(b_data);
  }


  void computePercentage(void)
  {
    batteryPercentage = (charge / battCapacity) * 100;
    if (batteryPercentage > 100)
    {
      batteryPercentage = 100;
    }
  }


  void doRoutine(void)
  {
    checkCondition();
    countQ();
    computePercentage();
    Charge();
    seriallizeData();
  }

};
BMS bms; //creating a BMS object


//void requestEvent() {
//  Wire.print(b_data);
//}


//void receiveEvent(int howMany) {
//
//}


void setup() {
  bms.initializeBMS();              //initializing the BMS
  //  Wire.begin(101);
  //  Wire.onRequest(requestEvent);
  //  Wire.onReceive(receiveEvent);
}


void loop() {
  bms.doRoutine();
}
