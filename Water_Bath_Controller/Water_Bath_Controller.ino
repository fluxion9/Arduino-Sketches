#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define heater 8
#define t_bus A1

LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(t_bus);
DallasTemperature t_sense(&oneWire);


float set_temp = 0.0;
float temperature = 0.0;

String enteredValue = "";
String Buffer = "", data = "", mem = "";

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {5, 7, 9, 10};
byte colPins[COLS] = {11, 12, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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
} sWriter;


struct WaterBath
{
  unsigned long lastDisplay = 0, lastSendTime = 0;
  bool settingTemp = false;

  byte paramNum = 0;
  byte screenPos = 0;
  void init(void)
  {
    Serial.begin(9600);
    pinMode(heater, 1);
    sWriter.init();
    Buffer.reserve(20);
    data.reserve(32);
    mem.reserve(20);
  }

  void handleKeypadInput(char key) {
    if (key == '#') {
      if (enteredValue.length() > 0) {
        float value = enteredValue.toFloat();
        if (value > 150.0  || value < 0.0) {
          lcd.clear();
          lcd.print("Out of Range!");
          delay(2000);
          enteredValue = "";
        }
        else {
          set_temp = value;
          enteredValue = "";
          settingTemp = false;
        }
      }
    }
    else if (key == '*') {
      enteredValue += ".";
    }
    else if (isdigit(key)) {
      settingTemp = true;
      enteredValue += key;
    }
  }

  void measureTemperature( void )
  {
    t_sense.requestTemperatures();
    temperature = t_sense.getTempCByIndex(0);
    temperature = fabs(temperature);
  }

  void startHeating()
  {
    digitalWrite(heater, 1);
  }

  void stopHeating()
  {
    digitalWrite(heater, 0);
  }

  void updateDisplay(int mode) {
    if (millis() - lastDisplay >= 1000) {
      if (mode == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set Temp: " + String(set_temp));
        lcd.setCursor(0, 1);
        lcd.print("Temp: " + String(temperature));
      }
      else if (mode == 1)
      {
        lcd.clear();
        lcd.print("INPUT:");
        lcd.setCursor(0, 1);
        lcd.print(enteredValue);
      }
      lastDisplay = millis();
    }
  }

  // void display()
  // {
  //   if(millis() - lastDisplay >= 1000)
  //   {
  //     paramNum++;
  //     screenPos++;

  //     if(screenPos > 1)
  //     {
  //       screenPos = 0;
  //     }
  //     else if(screenPos < 0)
  //     {
  //       screenPos = 0;
  //     }

  //     if(paramNum > 2)
  //     {
  //       paramNum = 0;
  //     }
  //     else if(paramNum < 0)
  //     {
  //       paramNum = 0;
  //     }

  //     if(paramNum == 0)
  //     {
  //       sWriter.write(screenPos, 0, "Set Temp: " + String(set_temp), 50);
  //     }
  //     else if(paramNum == 1)
  //     {
  //       sWriter.write(screenPos, 0, "Temp: " + String(temperature), 50);
  //     }
  //     else if(paramNum == 2)
  //     {
  //       sWriter.write(screenPos, 0, "IP: " + IP, 50);
  //     }
  //     lastDisplay = millis();
  //   }
  // }

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
    Buffer.concat(",\"stemp\":");
    Buffer.concat(set_temp);
    Buffer.concat("}");
  }

  void sendData()
  {
    if (millis() - lastSendTime > 1500)
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
        set_temp = readStrList(&mem, data, 1).toFloat();
      }
      data = "";
    }
  }

  void run(void)
  {
    measureTemperature();
    char key = keypad.getKey();

    if (key != NO_KEY)
    {
      handleKeypadInput(key);
    }

    if (settingTemp)
    {
      updateDisplay(1);
    } else {
      updateDisplay(0);
    }

    if (temperature <= set_temp - 0.5)
    {
      startHeating();
    }else if(temperature > set_temp)
    {
      stopHeating();
    }

    sendData();
    checkSerial();
  }
} wBath;


void setup() {
  wBath.init();
}

void loop() {
  wBath.run();
}
