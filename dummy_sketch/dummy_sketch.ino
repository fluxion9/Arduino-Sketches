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

String data = "", mem = "";
String Speed;
void setup() {
  Serial.begin(9600);
  data.reserve(32);
  mem.reserve(32);
}

void loop() {
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
        data = data.substring(data.indexOf('[')+1, data.indexOf(']'));
        Speed = readStrList(&mem, data, 1);
        Serial.println(Speed);
      }
      data = "";
    }
}
