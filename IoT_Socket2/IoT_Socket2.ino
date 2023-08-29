#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ACS712.h>

#define passwordLength 4
#define relayPin 5
#define voltagePin A2
#define currentPin A0

LiquidCrystal_I2C lcd(0x27, 16, 2);

ACS712  ACS(currentPin, 5.0, 1023, 66);

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {8, 7, 4, 3};
byte colPins[COLS] = {2, 12, 13};


Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


String password = "0000", input = "";

String Buffer = "", data = "", mem = "";


bool changingPassword = false, changingCurrentLimit = false;
bool isOutputOn = false;

float voltage, current, power, energy;

unsigned long lastDisplayTime = 0, lastReadTime = 0, lastSendTime;

float currentLimit = 10.0;

int displayMode = 0;

struct IoT_Socket2
{
  void init()
  {
    pinMode(relayPin, 1);
    switchOFF();
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);

    password.reserve(6);
    input.reserve(10);
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

  float measureVoltageAC()
  {
    float vol = analogRead(voltagePin);
    vol = (vol * 5.0) / 1023.0;
    vol = vol * 101.0;
    voltage = vol / 1.414;
    return voltage;
  }

  void takeReadings()
  {
    measureVoltageAC();
    measureCurrentAC();
    if (current > currentLimit)
    {
      switchOFF();
    }
    power = voltage * current;
    if (millis() - lastReadTime >= 2000)
    {
      if (isOutputOn)
      {
        energy += (power * 0.00056) / 1000.0;
      }
      lastReadTime = millis();
    }
  }

  void switchON()
  {
    digitalWrite(relayPin, 1);
    isOutputOn = true;
  }

  void switchOFF()
  {
    digitalWrite(relayPin, 0);
    isOutputOn = false;
  }

  void toggleOutput()
  {
    if (isOutputOn)
    {
      digitalWrite(relayPin, 0);
      isOutputOn = false;
    }
    else {
      digitalWrite(relayPin, 1);
      isOutputOn = true;
    }
  }

  void hideInput(byte from)
  {
    for (int i = 0; i < input.length(); i++)
    {
      if (input[i] == '\0')
      {
        lcd.setCursor(from + i, 1);
        lcd.print(" ");
      }
      else {
        lcd.setCursor(from + i, 1);
        lcd.print("*");
      }
    }
  }

  void display(int mode)
  {
    if (millis() - lastDisplayTime >= 1000)
    {
      switch (mode)
      {
        case 0:
          if (input.length() > 0)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Input:");
            hideInput(0);
          }
          else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("volt: " + String(voltage) + " V");
            lcd.setCursor(0, 1);
            lcd.print("curr: " + String(current) + " A");
          }
          break;
        case 1:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set Limit: ");
          lcd.setCursor(0, 1);
          lcd.print(input.length() > 0 ? input + " A" : input);
          break;
        case 2:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Change Key: ");
          lcd.setCursor(0, 1);
          hideInput(6);
          break;
        default:
          break;
      }
      lastDisplayTime = millis();
    }
  }

  void handlePasswordChange(char key) {
    if (key == '#') {
      if (input.length() < passwordLength)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Too short!");
        delay(1500);
        input = "";
      }
      else {
        password = input;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Success!");
        delay(1500);
        input = "";
        changingPassword = false;
      }
    }
    else if (isDigit(key))
    {
      if (input.length() < 4)
      {
        input += key;
      }
    }
  }

  void handleCurrentLimitSetting(char key) {
    if (key == '#') {
      currentLimit = input.toFloat();
      input = "";
      changingCurrentLimit = false;
    }
    else if (isdigit(key)) {
      input += key;
    }
    else if (key == '*')
    {
      input += '.';
    }
  }

  void handleNormalKeypadInput(char key) {
    if (key == '*') {
      changingPassword = true;
    }
    else if (isDigit(key)) {
      input += key;
      if (input.length() >= passwordLength && input == password)
      {
        toggleOutput();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Key Valid!");
        delay(1500);
        input = "";
      }
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Key Invalid");
        delay(1500);
        input = "";
      }
    }
    else if (key == '#') {
      changingCurrentLimit = true;
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
    Buffer.concat(currentLimit);
    Buffer.concat(",\"ps\":");
    Buffer.concat(isOutputOn);
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
        if (command == "iLim")
        {
          currentLimit = readStrList(&mem, data, 2).toFloat();
        }
      }
      else if (data == "+on-off")
      {
        toggleOutput();
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

  void run()
  {
    takeReadings();
    char key = keypad.getKey();
    if (key != NO_KEY) {
      if (changingPassword) {
        displayMode = 2;
        handlePasswordChange(key);
      }
      else if (changingCurrentLimit) {
        displayMode = 1;
        handleCurrentLimitSetting(key);
      } 
      else {
        displayMode = 0;
        handleNormalKeypadInput(key);
      }
    }
    display(displayMode);
  }
} iot_sock;


void setup() {
  iot_sock.init();
}

void loop() {
  iot_sock.run();
}
