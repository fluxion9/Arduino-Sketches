#include "ACS712.h"
#define currentOut A5
#define currentIn A1
#define chargePin 9
#define output 5
#define cell1 A2
#define cell2 A3
#define cell3 A4
ACS712  cout(currentOut, 5.0, 1023, 100);
ACS712  cin(currentIn, 5.0, 1023, 100);
const float battCapacity = 1500, fullChargeVoltage = 12.4, lowChargeVoltage = 10.0;
double charge = 0.0; 
float batteryVoltage; 
float cell_voltage[3] = {}; 
float batteryPercentage;
byte cells[3] = {cell1, cell2, cell3}; 
unsigned long lastMillis = 0;
struct BMS     //creating a BMS class
{
  void initializeBMS() //method for initializing the BMS
  {
    Serial.begin(115200);
    pinMode(output, 1);
    for (byte i = 0; i < 3; ++i)
    {
      pinMode(cells[i], 0);
    }
    float temp = (analogRead(cells[2]) * 55.0) / 1023.0;
    if(temp > lowChargeVoltage && temp < fullChargeVoltage)
    {
      while(1)
      {
        digitalWrite(output, 1);
        delay(3000);
        digitalWrite(output, 0);
        delay(1500);
      }
    }
    else if(temp <= lowChargeVoltage)
    {
      while(1);
    }
    charge = battCapacity;
    pinMode(currentOut, 0);
    pinMode(currentIn, 0);
    cout.autoMidPoint();
    cin.autoMidPoint();
  }
  void readCellVoltages() //method for reading cell voltages
  {
    digitalWrite(chargePin, 0);
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
    if (batteryVoltage >= fullChargeVoltage)
    {
      digitalWrite(chargePin, 0);
      digitalWrite(output, 1);
    }
    else if (batteryVoltage < lowChargeVoltage)
    {
      digitalWrite(output, 0);
      digitalWrite(chargePin, 1);
    }
    else {
      digitalWrite(output, 1);
      digitalWrite(chargePin, 1);
    }
  }
  void countQ()
  {
    if (millis() - lastMillis >= 1000)
    {
      double d = cout.mA_DC();
      double c = cin.mA_DC();
      if (d > 0)
      {
        Serial.print(F("Discharge Current: "));
        Serial.print(d, 4);
        Serial.println(F(" mA"));
        charge = charge - (d * 0.0002777d);
        if(charge < 0)
        {
          charge = 0;
        }
      }
      if (c > 100)
      {
        Serial.print(F("Charging Current: "));
        Serial.print(c, 4);
        Serial.println(F(" mA"));
        charge = charge + (c * 0.0002777d);
      }
      showValues();
      lastMillis = millis();
    }
  }
  void showValues() //method for displaying values and parameters
  {
    Serial.print(F("Cell 1: "));
    Serial.println(cell_voltage[0], 4);
    Serial.print(F("Cell 2: "));
    Serial.println(cell_voltage[1], 4);
    Serial.print(F("Cell 3: "));
    Serial.println(cell_voltage[2], 4);
    Serial.print(F("Charge: "));
    Serial.print(charge, 4);
    Serial.println(F(" mAh"));
    Serial.print(F("Battery Voltage: "));
    Serial.print(batteryVoltage, 4);
    Serial.println(F(" V"));
    computePercentage();
    Serial.print(F("Battery Percentage: "));
    Serial.print(batteryPercentage, 0);
    Serial.println(F(" %"));
    Serial.print("\n\n");
  }
  void computePercentage()
  {
    batteryPercentage = (charge / battCapacity) * 100;
    if(batteryPercentage > 100)
    {
      batteryPercentage = 100;
    }
  }
};
BMS bms; //creating a BMS object
void setup() {
  bms.initializeBMS(); //initializing the BMS
}

void loop() {
  bms.readCellVoltages();
  bms.countQ();
}
