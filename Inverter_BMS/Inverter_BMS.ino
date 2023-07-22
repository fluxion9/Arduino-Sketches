#define cell1 A2 //voltage divider for cell1 is connected here
#define cell2 A0 //voltage divider for cell2 is connected here
#define cell3 A1 //voltage divider for cell3 is connected here


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

float min_voltage = 15.0, v_offset = 0.5;
float max_voltage = 17.0, max_v = max_voltage + v_offset;

String Buffer = "", data = "", mem = "";

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
      if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
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

long Speed = 0;

struct BMS
{
  bool b_state = false;
  bool bal_state = false;
  bool pow_state = false;
  bool lock_state = false;
  bool charge_state = false;
  bool chargeControlEnabled = true;

  unsigned long upTime = 0, lastInterval = 0;

  int rcount = 0, batUnChgCount = 0;

  float cell_voltage[3] = {0, 0, 0};
  float batteryVoltage = 0.0;

  byte state;

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

    Buffer.reserve(64);
    data.reserve(32);
    mem.reserve(20);

    Speed = 500;

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
    if (pow_state)
    {
      digitalWrite(inv_switch, 1);
      digitalWrite(redLed, 1);
    }
    else {
      digitalWrite(inv_switch, 0);
      digitalWrite(redLed, 0);
    }
  }
  void readCellVoltages() //method for reading cell voltages
  {
    batteryVoltage = 0;
    rcount = 5;

    cell_voltage[0] = 0.0;
    cell_voltage[1] = 0.0;
    cell_voltage[2] = 0.0;

    for (byte i = 0; i < 3; ++i)
    {
      for (byte j = 0; j < rcount; ++j)
      {
        float voltage = analogRead(cells[i]);
        voltage = (voltage * 55.0) / 1024.0;
        cell_voltage[i] += voltage;
      }
    }
    cell_voltage[0] /= rcount;
    cell_voltage[1] /= rcount;
    cell_voltage[2] /= rcount;

    batteryVoltage = cell_voltage[2];

    cell_voltage[1] -= cell_voltage[0];
    cell_voltage[2] -= (cell_voltage[1] + cell_voltage[0]);
  }
  void balanceCells()
  {
    if (bal_state)
    {
      if (cell_voltage[0] != cell_voltage[1] || cell_voltage[0] != cell_voltage[2] || cell_voltage[1] != cell_voltage[2])
      {
        if (millis() - lastMillis >= Speed)
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

  bool isListData(String* data)
  {
    if (data->startsWith("[") && data->endsWith("]"))
    {
      return true;
    }
    else {
      return false;
    }
  }

  String readStrList(String* memory, String strList, byte position)
  {
    byte index = 0;
    *memory = "";
    for (int i = 0; i < strList.length(); i++)
    {
      if (strList[i] == ',')
      {
        index++;
      }
      if (index == position - 1)
      {
        memory->concat(strList[i]);
      }
    }
    if (memory->startsWith(","))
    {
      *memory = memory->substring(memory->indexOf(',') + 1);
    }
    return *memory;
  }

  void runInterval(int interval)
  {
    if (millis() - lastInterval >= interval)
    {
      react();
      Charge();
      lastInterval = millis();
    }
  }

  void run(void)
  {
    upTime = millis() / 1000;
    readCellVoltages();
    power();
    balanceCells();
    blinkBlue.Update();
    runInterval(1000);
    while (Serial.available() > 0)
    {
      delay(3);
      char c = Serial.read();
      data += c;
    }
    if (data.length() > 0)
    {
      data.trim();
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        Speed = readStrList(&mem, data, 1).toInt();
        chargeControlEnabled = readStrList(&mem, data, 2).toInt();
        min_voltage = readStrList(&mem, data, 3).toFloat();
        max_voltage = readStrList(&mem, data, 4).toFloat();
        v_offset = readStrList(&mem, data, 5).toFloat();
        max_v = max_voltage + v_offset;
      }
      else if (data == "+balanceBat" && !lock_state)
      {
        bal_state = !bal_state;
      }
      else if (data == "+power" && !lock_state)
      {
        pow_state = !pow_state;
      }
      else if (data == "!lock")
      {
        lock_state = true;
      }
      else if (data == "!unlock")
      {
        lock_state = false;
      }
      else if (data == "!reset")
      {
        init();
      }
      else if (data == "+read;")
      {
        load_buffer();
        Serial.println(Buffer);
      }
      data = "";
    }
  }

  float getMaxCellVoltage()
  {
    if (cell_voltage[0] == cell_voltage[1] && cell_voltage[0] == cell_voltage[2])
    {
      return cell_voltage[0];
    }
    else if (cell_voltage[0] > cell_voltage[1] && cell_voltage[0] > cell_voltage[2])
    {
      return cell_voltage[0];
    }
    else if (cell_voltage[1] > cell_voltage[0] && cell_voltage[1] > cell_voltage[2])
    {
      return cell_voltage[1];
    }
    else {
      return cell_voltage[2];
    }
  }

  void Charge(void)
  {
    if (chargeControlEnabled)
    {
      if (getMaxCellVoltage() >= max_v)
      {
        digitalWrite(charge, 1);
        charge_state = false;
        max_v = max_voltage;
      }
      else {
        digitalWrite(charge, 0);
        charge_state = true;
        max_v = max_voltage + v_offset;
      }
    }
    else {
      digitalWrite(charge, 0);
      charge_state = true;
    }
  }

  void react()
  {
    if (cell_voltage[0] < min_voltage || cell_voltage[1] < min_voltage || cell_voltage[2] < min_voltage)
    {
      batUnChgCount++;
      if (batUnChgCount >= 5)
      {
        pow_state = false;
      }
    }
    else {
      batUnChgCount = 0;
    }
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
    Buffer.concat(",\"vt\":");
    Buffer.concat(batteryVoltage);
    Buffer.concat(",\"cs\":");
    Buffer.concat(charge_state);
    Buffer.concat(",\"cce\":");
    Buffer.concat(chargeControlEnabled);
    Buffer.concat(",\"ps\":");
    Buffer.concat(pow_state);
    Buffer.concat(",\"bs\":");
    Buffer.concat(bal_state);
    Buffer.concat(",\"ls\":");
    Buffer.concat(lock_state);
    Buffer.concat(",\"ut\":");
    Buffer.concat(upTime);
    Buffer.concat(",\"vmax\":");
    Buffer.concat(max_voltage);
    Buffer.concat(",\"vmin\":");
    Buffer.concat(min_voltage);
    Buffer.concat(",\"es\":");
    Buffer.concat(Speed);
    Buffer.concat(",\"vset\":");
    Buffer.concat(max_v);
    Buffer.concat(",\"vOft\":");
    Buffer.concat(v_offset);
    Buffer.concat("}");
  }

} bms;

void setup() {
  bms.init();
}

void loop() {
  bms.run();
}
