
#include <FileManager.h>
#include <SPI.h>
#include <SD.h>
#include <EmonLib.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>



#define Switch 9
#define buzzer 8
#define ledActive 2
#define ledWorking 3
#define iSense A1
#define vSense A5
#define chipSelect 10

float volt, amp, temp;
byte i;

float maxpk = 0, RMS = 0, cnt = 0;
unsigned long Time, sampleTime = 2000;

EnergyMonitor CTsense;

SoftwareSerial debugger(5, 6);

FileManager fm;

float count = 0;


float avg_current,
      min_voltage = 700.00,
      max_voltage = 0.00,
      max_current = 0.00,
      min_current = 700.00,
      avg_voltage = 0.00,
      account_balance = 0.00,
      account_charge = 0.00,
      tariff = 0.00,
      energy = 0.00;

int status = 204;
unsigned long unixEpoch,
         account_topup = 0,
         interval = 10000;

bool is_active = false,
     is_on = false;

//"b26d14ca-f747-4540-a293-5d297371b1c5"


struct Meter
{
  String resp, buf;

  unsigned long lastTime = 0,
                currentTime = 0;


  float measureVoltage()
  {
    float val = 0;
    maxpk = 0;
    RMS = 0;
    cnt = 0;
    Time = millis();
    while (millis() - Time <= sampleTime)
    {
      for (int i = 0; i < 300; ++i)
      {
        val += analogRead(vSense);
        cnt += 1;
      }
      val /= cnt;
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
    maxpk = (maxpk * 505.0) / 1023.0;
    RMS = maxpk * 0.707;
    return RMS;
  }


  float measureCurrent()
  {
    temp = CTsense.calcIrms(1480);
    return temp;
  }


  bool isJSONData()
  {
    if (resp.startsWith("{\"actv\"") && resp.endsWith("}"))
    {
      return true;
    }
    else {
      return false;
    }
  }


  void init()
  {
    Serial.begin(9600);
    debugger.begin(9600);
    Serial.setTimeout(10000);
    CTsense.current(iSense, 111.1);
    pinMode(buzzer, 1);
    pinMode(ledActive, 1);
    pinMode(ledWorking, 1);
    pinMode(iSense, 0);
    pinMode(vSense, 0);
    pinMode(Switch, 1);
    digitalWrite(buzzer, 1);

    for (i = 0; i < 3; ++i)
    {
      digitalWrite(ledWorking, 1);
      delay(300);
      digitalWrite(ledWorking, 0);
      delay(300);
    }

    //    if (!SD.begin(chipSelect)) {
    //      debugger.println(F("Card failed, or not present"));
    //      while (1);
    //    }
    //    debugger.println("Card Initialized! ");
    digitalWrite(buzzer, 0);
    //    buf.reserve(128);
    //    resp.reserve(64);
  }
  void deserializeAndExtractJSON(byte mode)
  {
    if (resp == "")
    {
      debugger.println(F("No data to deserialize..."));
    }
    else
    {
      debugger.print(F("deserializing: "));
      debugger.println(resp);
      DynamicJsonDocument doc(150);
      debugger.print(F("deserialization code: "));
      DeserializationError err = deserializeJson(doc, resp);
      debugger.println(err.f_str());
      JsonObject obj = doc.as<JsonObject>();
      debugger.println(obj);
      is_active = obj["actv"];
      is_on = obj["on"];
      account_topup = obj["tup"];
      if (mode == 0)
      {
        //account_balance = obj["bal"];
      }
      tariff = obj["tar"];
      interval = obj["intv"];
      interval = interval * 60000UL;
      status = obj["stat"];
      debugger.print(F("stat got: "));
      debugger.println(status);
    }
  }
  void dumpBuf()
  {
    Serial.println(buf);
    debugger.print(F("dumping: "));
    debugger.println(buf);

    unsigned long Time = millis(),
                  waitTime = 10000;

    while (!Serial.available())
    {
      if (millis() - Time >= waitTime)
      {
        return;
      }
    }

    if (Serial.available())
    {
      resp = Serial.readStringUntil(';');
      resp.trim();
      if (resp.length() > 0)
      {
        if (isJSONData())
        {
          debugger.print(F("Response: "));
          debugger.println(resp);
        }
        else {
          resp = "";
          debugger.println(F("!JSON"));
        }
      }
    }
  }


  void serializeJSON()
  {
    buf = "";
    buf = " {";
    buf += "\"uid\":\"";
    //    buf += fm.readLine("config.txt", 2);
    buf += "b26d14ca-f747-4540-a293-5d297371b1c5";
    buf += "\",";
    buf += "\"vMin\":";
    buf += min_voltage;
    buf += ",";
    buf += "\"vMax\":";
    buf += max_voltage;
    buf += ",";
    buf += "\"vAvg\":";
    buf += avg_voltage;
    buf += ",";
    buf += "\"iMin\":";
    buf += min_current;
    buf += ",";
    buf += "\"iMax\":";
    buf += max_current;
    buf += ",";
    buf += "\"iAvg\":";
    buf += avg_current;
    buf += ",";
    buf += "\"enrg\":";
    buf += String(energy, 4);
    buf += ",";
    buf += "\"chrg\":";
    buf += String(account_charge, 4);
    buf += ",";
    buf += "\"tup\":";
    buf += String(account_topup);
    buf += ",";
    buf += "\"bal\":";
    buf += account_balance;
    buf += ",";
    buf += "\"tar\":";
    buf += tariff;
    buf += ",";
    buf += "\"intv\":";
    buf += (interval / 60000UL);
    buf += ",";
    buf += "\"stat\":";
    buf += status;
    buf += ",";
    buf += "\"epoch\":";
    buf += unixEpoch;
    buf += "}";
    debugger.print(F("buflen: "));
    debugger.println(buf.length());
    debugger.print(F("bufsize: "));
    debugger.println(sizeof(buf));
  }


  void actions()
  {
    if (account_topup > 0.0)
    {
      account_balance += account_topup;
      account_topup = 0.0;
    }
    if (account_balance <= 0.0)
    {
      digitalWrite(Switch, 0);
      for (i = 0; i < 10; ++i)
      {
        digitalWrite(ledActive, 1);
        digitalWrite(ledWorking, 1);
        delay(150);
        digitalWrite(ledActive, 0);
        digitalWrite(ledWorking, 0);
        delay(150);
      }
      delay(1000);
    }
    else
    {
      if (!is_active && is_on)
      {
        digitalWrite(ledActive, 0);
        digitalWrite(Switch, 0);
        digitalWrite(ledWorking, 1);
      }
      else if (!is_active && !is_on)
      {
        digitalWrite(ledActive, 0);
        digitalWrite(Switch, 0);
        digitalWrite(ledWorking, 0);
      }
      else if (is_active && is_on)
      {
        digitalWrite(ledActive, 1);
        digitalWrite(Switch, 1);
        digitalWrite(ledWorking, 1);
      }
      else if (is_active && !is_on)
      {
        digitalWrite(ledActive, 1);
        digitalWrite(Switch, 0);
        digitalWrite(ledWorking, 0);
      }
    }
  }


  void routineCalc()
  {
    avg_voltage /= count;
    avg_current /= count;
    count = 0;
    energy = ((avg_voltage * avg_current) / 1000) * (float(interval) / 3600000.0); // unit is KWh
    account_charge = (energy * tariff);
    account_balance -= account_charge;
  }


  void routine()
  {
    currentTime = millis();
    if (currentTime - lastTime >= interval)
    {
      routineCalc();
      if (account_balance < 0)
      {
        account_balance = 0.0;
      }
      serializeJSON();
      dumpBuf();
      buf = "";
      if (status == 204)
      {
        deserializeAndExtractJSON(0);
      }
      else if (status == 200)
      {
        deserializeAndExtractJSON(1);
      }
      else {
        debugger.print(F("status code is: "));
        debugger.println(status);
      }
      avg_voltage = 0;
      avg_current = 0;
      lastTime = currentTime;
    }
  }


  void takeReadings()
  {
    volt = measureVoltage();
    amp = measureCurrent();
    avg_voltage = avg_voltage + volt;
    avg_current = avg_current + amp;
    count += 1;
    if (volt > max_voltage)
    {
      max_voltage = volt;
    }
    else if (volt < min_voltage && volt > 0.0)
    {
      min_voltage = volt;
    }
    if (amp > max_current)
    {
      max_current = amp;
    }
    else if (amp < min_current && amp > 0.0)
    {
      min_current = amp;
    }
    debugger.print(F("Voltage: "));
    debugger.println(volt);
    debugger.print(F("Current: "));
    debugger.println(amp);
    unixEpoch = millis() / 60000;
  }
};


Meter meter;


void setup() {
  meter.init();
}

void loop() {
  meter.actions();
  meter.takeReadings();
  meter.routine();
}
