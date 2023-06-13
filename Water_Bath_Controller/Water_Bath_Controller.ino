#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define heater 8
#define t_bus A1

LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(t_bus);
DallasTemperature t_sense(&oneWire);


float set_temp = 25.0;
float temperature = 0.0;

String IP = "192.168.4.1";
String Buffer = "", data = "", mem = "";


struct Scroll_W
{
  void init(void)
  {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
  }

  void clearRow(byte row)
  {
    lcd.setCursor(0, row);
    lcd.print("                ");
  }

  void write(int row, int col, String message, int Delay)
  {
    clearRow(row);
    lcd.setCursor(col, row);
    for (int i = 0; i < message.length(); i++)
    {
      lcd.setCursor(col + i, row);
      lcd.print(message[i]);
      delay(Delay);
    }
  }
}sWriter;


struct WaterBath
{
  unsigned long lastDisplay = 0;
  byte paramNum = 0;
  byte screenPos = 0;
  void init(void)
  {
    Serial.begin(9600);
    pinMode(heater, 1);
    sWriter.init();
    IP.reserve(16);
    Buffer.reserve(20);
    data.reserve(32);
    mem.reserve(20);
  }

  void measureTemperature( void )
  {
    t_sense.requestTemperatures();
    temperature = t_sense.getTempCByIndex(0);
  }
  
  void startHeating()
  {
    digitalWrite(heater, 1);
  }
  
  void stopHeating()
  {
    digitalWrite(heater, 0);
  }
  
  void display()
  {
    if(millis() - lastDisplay >= 1000)
    {
      paramNum++;
      screenPos++;
      
      if(screenPos > 1)
      {
        screenPos = 0;
      }
      else if(screenPos < 0)
      {
        screenPos = 0;
      }
      
      if(paramNum > 2)
      {
        paramNum = 0;
      }
      else if(paramNum < 0)
      {
        paramNum = 0;
      }

      if(paramNum == 0)
      {
        sWriter.write(screenPos, 0, "Set Temp: " + String(set_temp), 50);
      }
      else if(paramNum == 1)
      {
        sWriter.write(screenPos, 0, "Temp: " + String(temperature), 50);
      }
      else if(paramNum == 2)
      {
        sWriter.write(screenPos, 0, "IP: " + IP, 50);
      }
      lastDisplay = millis();
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

  void load_buffer(void)
  {
    Buffer = "";
    Buffer.concat("{\"wtemp\":");
    Buffer.concat(temperature);
    Buffer.concat("}");
  }
  
  void run(void)
  {
    measureTemperature();
    if(temperature < set_temp)
    {
      startHeating();
    }
    else {
      stopHeating();
    }
    display();
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
        set_temp = readStrList(&mem, data, 1).toFloat();
        IP = readStrList(&mem, data, 2);
      }
      else if (data == "+read;")
      {
        load_buffer();
        Serial.println(Buffer);
      }
      data = "";
    }
  }
}wBath;


void setup() {
  wBath.init();
}

void loop() {
  wBath.run();
}
