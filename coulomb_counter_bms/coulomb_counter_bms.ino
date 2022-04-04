#include "ACS712.h"
#define currentOut A5 //current sensor for meassuring discharge current
#define output 5  //pin for enabling output from the battery
#define cell1 A2 //voltage divider for cell1 is connected here
#define cell2 A3 //voltage divider for cell2 is connected here
#define cell3 A4 //voltage divider for cell3 is connected here
ACS712  cout(currentOut, 5.0, 1023, 100); //creating a current sensor object
double charge = 0.0; //variable for storing depleted charge
float batteryVoltage; // stores battery voltage
float cell_voltage[3] = {}; //array for storing individual cell voltage
byte cells[3] = {cell1, cell2, cell3}; //cell voltage divider array
unsigned long lastMillis = 0; //initializing last time with 0
struct BMS     //creating a BMS class
{
  void initializeBMS() //method for initializing the BMS
  {
    Serial.begin(115200);
    pinMode(currentOut, 0);
    pinMode(output, 1);
    cout.autoMidPoint();
    digitalWrite(output, 1);
    for (byte i = 0; i < 3; ++i)
    {
      pinMode(cells[i], 0);
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
      batteryVoltage += cell_voltage[i];
    }
    cell_voltage[1] -= cell_voltage[0];
    cell_voltage[2] -= (cell_voltage[1] + cell_voltage[0]);
  }
  void countQ() //method for counting charge expended
  {
    if (millis() - lastMillis >= 1000)
    {
      double c = cout.mA_DC();
      if (c > 0)
      {
        Serial.print(F("Current: "));
        Serial.print(c, 4);
        Serial.println(F(" mA"));
        charge = charge + (c * 0.0002777d);
        showValues();
      }
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
    Serial.print("\n\n");
  }
};
BMS bms; //creating a BMS object
void setup() {
  bms.initializeBMS(); //initializing the BMS
}

void loop() {
  bms.readCellVoltages(); //Reading cell voltages
  bms.countQ(); // counting the charge expended
}
