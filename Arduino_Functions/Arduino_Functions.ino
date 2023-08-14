struct Debugger
{
  bool debug = true;
  Stream *ser;

  Debugger(Stream* serialPort)
  {
    ser = serialPort;
  }

  void log_str(String str, bool endline)
  {
    if (debug)
    {
      if (endline)
      {
        ser->println(str);
      }
      else {
        ser->print(str);
      }
    }
  }

  void log_int(long num, bool endline)
  {
    if (debug)
    {
      if (endline)
      {
        ser->println(num);
      }
      else {
        ser->print(num);
      }
    }
  }

  void log_uint(unsigned long num, bool endline)
  {
    if (debug)
    {
      if (endline)
      {
        ser->println(num);
      }
      else {
        ser->print(num);
      }
    }
  }

  void log_float(double num, bool endline)
  {
    if (debug)
    {
      if (endline)
      {
        ser->println(num);
      }
      else {
        ser->print(num);
      }
    }
  }
} debugger(&Serial);



float measureVoltage(byte pin)
{
  float val = 0, maxpk = 0, RMS = 0;
  unsigned long Time = millis(), sampleTime = 1000;
  while (millis() - Time <= sampleTime)
  {
    for (int i = 0; i < 300; ++i)
    {
      val += analogRead(pin);
    }
    val /= 300;
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
  return RMS;
}

String readStrList(String strList, byte position)
{
  byte index = 0;
  String mem = "";

  for (int i = 0; i < strList.length(); i++)
  {
    if (strList[i] == ',')
    {
      index++;
    }
    if (index == position - 1)
    {
      mem += strList[i];
    }
  }

  if (mem.startsWith(","))
  {
    mem = mem.substring(mem.indexOf(',') + 1);
  }

  return mem;
}

void readStrList(String* mem, String strList, byte position)
{
  byte index = 0;
  *mem = "";

  for (int i = 0; i < strList.length(); i++)
  {
    if (strList[i] == ',')
    {
      index++;
    }
    if (index == position - 1)
    {
      mem->concat(strList[i]);
    }
  }

  if (mem->startsWith(","))
  {
    *mem = mem->substring(mem->indexOf(',') + 1);
  }
}

void blockingEvent()
{
  unsigned long Time = millis(), timeout = 10000;
  while (true) //condition for wait while awaiting timeout
  {
    //place any statement to execute while awaiting timeot
    if (millis() - Time >= timeout)
    {
      return;
    }
  }
}

float fmap(float val, float fromLow, float fromHigh, float toLow, float toHigh)
  {
    float norm = (val - fromLow) / (fromHigh - fromLow);
    float lerp = norm * (toHigh - toLow) + toLow;
    return lerp;
  }

//void serializeJSON(String* buf)
//{
//  *buf = "";
//  buf->concat("{\"key_0\":");
//  buf->concat(val_0);
//  buf->concat(", \"key_1\":");
//  buf->concat(val_1);
//  buf->concat(", \"key_2\":\"");
//  buf->concat("str_0");
//  buf->concat("}");
//}

float measureVoltageDC(byte pin, int vdr)
{
  float value = analogRead(pin);
  value = (value * 5.0) / 1023.0;
  value = value * vdr
}


void setup() {}
void loop() {}
