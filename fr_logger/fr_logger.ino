#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>

#include "DHT.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define chipSelect 5
#define tBus 27
#define dhtpin 17
#define dhttype DHT22

#define refreshRate 500    // Refresh screen every {value} ms
#define dumpInterval 5000  // Dump Data to SD every {value} ms

LiquidCrystal_I2C lcd(0x27, 20, 4);
OneWire oneWire(tBus);
DallasTemperature t_sense(&oneWire);
DHT dht(dhtpin, dhttype);

const char* ssid = "Switch-Gateway-0x100";      // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "gate-0xc4-way-0x4c";  // The password of the Wi-Fi network

struct Logger {
  float airTemp = 0.0, humidity = 0.0;
  float sampleTemp[6] = { 0.0 };

  String localTime = "";
  String dataString = "";

  char Buff[155];

  unsigned long lastLogTime = 0.0, lastDispTime = 0, startMillis = 0, lastErrTime = 0;

  void init(void) {
    Serial.begin(9600);
    dht.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   Data - Logger!   ");
    delay(2000);
    lcd.setCursor(0, 2);
    lcd.print(" Initializing.      ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.status() != WL_CONNECTED) {
      while (WiFi.status() != WL_CONNECTED) {
        displayConnError();
        Serial.print(".");
        delay(500);
      }
      Serial.println();
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   Data - Logger!   ");
    lcd.print(" Initializing.      ");
    for (int i = 14; i < 19; i++) {
      lcd.setCursor(i, 2);
      lcd.print(".");
      delay(500);
    }
    delay(2000);
    localTime.reserve(20);
    dataString.reserve(64);
    while (!SD.begin(chipSelect)) {
      Serial.println("failed to mount sd. ");
      delay(3000);
    }
    dataString = "Air Temp (*),Humid (%),Temp1 (*C),Temp2 (*C),Temp3 (*C),Temp4 (*C),Temp5 (*C),Temp6 (*C),Time Stamp (DD:HH:MM:SS)";
    Serial.println(dataString);
    File file = SD.open("/data.csv", FILE_WRITE);
    if (file) {
      file.println(dataString);
      file.close();
    } else {
      Serial.println("Bad SD card. ");
      while (1)
        ;
    }
    Buff[0] = '\0';
    lastDispTime = millis();
    lastErrTime = millis();
    lastLogTime = millis();
    startMillis = millis();
  }

  void clearRow(byte row) {
    lcd.setCursor(0, row);
    lcd.print("                    ");
  }

  void displayConnError() {
    if (millis() - lastErrTime >= 1000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("       Error!       ");
      lcd.setCursor(0, 2);
      lcd.print("No WiFi Network..");
      lastErrTime = millis();
    }
  }

  void measureTemperatures(void) {
    airTemp = dht.readTemperature();
    humidity = dht.readHumidity();
    t_sense.requestTemperatures();
    for (int i = 0; i < 6; i++) {
      sampleTemp[i] = fabs(t_sense.getTempCByIndex(i));
    }
  }

  void displayTemp(int period) {
    if (millis() - lastDispTime >= period) {
      measureTemperatures();
      lcd.setCursor(0, 0);
      lcd.print("Air  Hum   T1   T2  ");
      clearRow(1);
      lcd.setCursor(0, 1);
      lcd.print(airTemp, 1);
      lcd.print(" ");
      lcd.print(humidity, 1);
      for (int i = 0; i < 2; i++) {
        lcd.print(" ");
        lcd.print(sampleTemp[i], 1);
      }
      lcd.setCursor(0, 2);
      lcd.print(" T3   T4   T5   T6  ");
      clearRow(3);
      lcd.setCursor(0, 3);
      for (int i = 2; i < 6; i++) {
        lcd.print(sampleTemp[i], 1);
        lcd.print(" ");
      }
      lastDispTime = millis();
    }
  }

  void epochToLocal(unsigned long unixEpoch) {
    long second = unixEpoch % 60;
    long minute = (unixEpoch / 60) % 60;
    long hour = (unixEpoch / 3600) % 24;
    long day = (unixEpoch / 86400) % 365;
    localTime = "";
    localTime += String(day) + ':';
    localTime += String(hour) + ':';
    localTime += String(minute) + ':';
    localTime += String(second);
  }

  void logData(int interval) {
    if (millis() - lastLogTime >= interval - 2000) {  //2000 is a measured offset
      measureTemperatures();
      epochToLocal((millis() - startMillis) / 1000);
      dataString = "";
      dataString += airTemp;
      dataString += ",";
      dataString += humidity;
      dataString += ",";
      for (int i = 0; i < 6; i++) {
        dataString += sampleTemp[i];
        dataString += ",";
      }
      dataString += localTime;
      Serial.println(dataString);
      File file = SD.open("/data.csv", FILE_WRITE);
      if (file) {
        file.println(dataString);
        file.close();
      } else {
        Serial.println("Bad SD card. ");
      }
      // dumpToThingSpeak();
      lastLogTime = millis();
    }
  }
  void run(void) {
    if (WiFi.status() != WL_CONNECTED) {
      while (WiFi.status() != WL_CONNECTED) {
        displayConnError();
        Serial.print(".");
        delay(500);
      }
      Serial.println();
    } else {
      measureTemperatures();
      displayTemp(refreshRate);
      logData(dumpInterval);
    }
  }

  char* floatToStr(float input) {
    char strForm[20];
    dtostrf(input, 4, 2, strForm);
    return strForm;
  }

  void dumpToThingSpeak() {
    HTTPClient http;
    WiFiClient client;
    sprintf(Buff, "https://api.thingspeak.com/update?api_key=H36MEHOH0X6SDVAD&field1=%s&field2=%s&field3=%s&field4=%s&field5=%s&field6=%s&field7=%s&field8=%s", floatToStr(airTemp), floatToStr(humidity), floatToStr(sampleTemp[0]), floatToStr(sampleTemp[1]), floatToStr(sampleTemp[2]), floatToStr(sampleTemp[3]), floatToStr(sampleTemp[4]), floatToStr(sampleTemp[5]));
    Serial.println(Buff);
    http.begin(client, Buff);
    int httpCode = http.GET();
    http.end();
  }
} logger;

void setup() {
  logger.init();
}

void loop() {
  logger.run();
}
