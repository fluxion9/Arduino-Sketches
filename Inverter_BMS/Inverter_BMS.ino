#define cell1 A0 //voltage divider for cell1 is connected here
#define cell2 A1 //voltage divider for cell2 is connected here
#define cell3 A2 //voltage divider for cell3 is connected here

#define min_voltage 15.0
#define max_voltage 17.0

#define redLed 3
#define blueLed 4

#define inv_switch 5
#define charge 6

#define balance 7

#define fullyCharged 2
#define notFull 4
#define charging 6

byte cells[3] = {cell1, cell2, cell3}; //cell voltage divider array

unsigned long lastMillis = 0; //initializing last time with 0


struct BMS
{
  bool b_state = false;
  float cell_voltage[3] = {};
  float batteryVoltage = 0.0;

  byte state;

  unsigned long currentStamp = 0, lastStamp = 0, cTime = 30000, wTime = 3000;

  void init(void)
  {
    for (int i = 0; i < 3; i++)
    {
      pinMode(cells[i], 0);
    }
    pinMode(redLed, 1);
    pinMode(blueLed, 1);
    pinMode(inv_switch, 1);
    pinMode(charge, 1);
    pinMode(balance, 1);

    for (int i = 0; i < 3; i++)
    {
      digitalWrite(blueLed, 1);
      delay(300);
      digitalWrite(blueLed, 0);
      delay(300);
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
  void balanceCells()
  {
    if (true)
    {
      if (cells[0] != cells[1] || cells[0] != cells[2] || cells[1] != cells[2])
      {
        if (millis() - lastMillis <= speed)
        {
          if (b_state)
          {
            digitalWrite(balance, 0);
            b_state = !b_state;
          }
          else {
            digitalWrite(balance, 1); l
            b_state = !b_state;
          }
          lastMillis = millis();
        }
      }
    }
  }

  void run(void)
  {
    if (cells[0] <= min_voltage || cells[1] <= min_voltage || cells[2] <= min_voltage)
    {
      digitalWrite(inv_switch, 0);
    }
  }

  void Charge(void)
  {

  }


}
void setup() {

}

void loop() {

}
