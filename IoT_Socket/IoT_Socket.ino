#include <ACS712.h>

#define voltagePin A4
#define voltageDividerRatio 101.0

#define currentPin A3
#define relayPin 13

#define off 0
#define on 1

ACS712  ACS(currentPin, 5.0, 1023, 66);

unsigned long lastReadTime = 0, lastSendTime = 0;

float energyLimit = 10.0;

String Buffer = "", data = "", mem = "";

bool state = off;

struct IoT_Socket
{
  float current = 0.0, voltage = 0.0, power = 0.0, energy = 0.0;

  void init()
  {
    pinMode(relayPin, 1);
    switchOFF();
    Serial.begin(9600);
    Buffer.reserve(64);
    mem.reserve(20);
    data.reserve(20);
    ACS.autoMidPoint();
  }
  float measureCurrentAC()
  {
    current = ACS.mA_AC();
    current /= 1000.0;
    return current;
  }
  float measureVoltage()
  {
    voltage = analogRead(voltagePin);
    voltage = (voltage * 5.0) / 1023.0;
    voltage = voltage * voltageDividerRatio;
    voltage /= 1.414;
    return voltage;
  }

  float measureVoltageAC()
  {
    float val = 0;
    float maxpk = 0, RMS = 0;
    unsigned long Time = millis(), sampleTime = 1000;
    while (millis() - Time <= sampleTime)
    {
      for (int i = 0; i < 300; ++i)
      {
        val += analogRead(voltagePin);
        val /= 2;
      }
      if (val <= 0)
      {
        maxpk = 0;
      }
      else
      {
        if (val > maxpk)
        {
          maxpk = val;
        }
      }
    }
    maxpk = (maxpk * 505.0) / 1024.0;
    RMS = maxpk * 0.707;
    voltage = RMS;
    return voltage;
  }

  void takeReadings()
  {
    measureVoltageAC();
    measureCurrentAC();
    power = voltage * current;
    if (millis() - lastReadTime >= 2000)
    {
      if (state == on)
      {
        energy += (power * 0.00056) / 1000.0;
        if (energy >= energyLimit)
        {
          switchOFF();
        }
      }
      lastReadTime = millis();
    }
  }

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"volt\":");
    Buffer.concat(voltage);
    Buffer.concat(",\"curr\":");
    Buffer.concat(current);
    Buffer.concat(",\"powr\":");
    Buffer.concat(power);
    Buffer.concat(",\"enrg\":");
    Buffer.concat(String(energy, 5));
    Buffer.concat(",\"limt\":");
    Buffer.concat(String(energyLimit, 5));
    Buffer.concat(",\"ps\":");
    Buffer.concat(state);
    Buffer.concat("}");
  }

  bool isListData(String *data)
  {
    if (data->startsWith("[") && data->endsWith("]"))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  String readStrList(String *memory, String strList, byte position)
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


  void sendData()
  {
    if (millis() - lastSendTime >= 2000)
    {
      load_buffer();
      Serial.println(Buffer);
      lastSendTime = millis();
    }
  }

  void checkSerial()
  {
    if (Serial.available())
    {
      while (Serial.available() > 0)
      {
        delay(3);
        char c = Serial.read();
        data += c;
      }
    }
    if (data.length() > 0)
    {
      data.trim();
      if (isListData(&data))
      {
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        String command = readStrList(&mem, data, 1);
        if (command == "eLim")
        {
          energy = 0.0;
          energyLimit = readStrList(&mem, data, 2).toFloat();
        }
      }
      else if (data == "+on-off")
      {
        switchONOFF();
      }
      else if (data == "+on")
      {
        switchON();
      }
      else if (data == "+off")
      {
        switchOFF();
      }
      data = "";
    }
  }

  void switchONOFF()
  {
    if (state == off)
    {
      digitalWrite(relayPin, 1);
      state = on;
    }
    else if (state == on)
    {
      digitalWrite(relayPin, 0);
      state = off;
    }
  }

  void switchON()
  {
    digitalWrite(relayPin, 1);
    state = on;
  }

  void switchOFF()
  {
    digitalWrite(relayPin, 0);
    state = off;
  }

  void run()
  {
    takeReadings();
    sendData();
    checkSerial();
  }
} iotSock;


void setup() {
  iotSock.init();
}

void loop() {
  iotSock.run();
}
