#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Wire.h>

// Pin and hardware definitions
#define RELAY_PIN 23      // GPIO for relay control
#define BUZZER_PIN 33     // GPIO for buzzer control
#define I2C_SLAVE_ADDR 8  // I2C address of ATmega328 slave
#define LCD_ADDR 0x27     // I2C address of 16x2 LCD

// #define KEYPAD_ROWS 5
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

String apiKey = "wuiee76";

String mid = "OG2501";  //change this to Meter ID

// (keypad1 = 5 x 4)
//4, 16, 17, 5, 18, 19, 25, 26, 27 ---- soldering
// byte rowPins[KEYPAD_ROWS] = { 27, 26, 25, 19, 18 };  // GPIO pins for keypad rows
// byte colPins[KEYPAD_COLS] = { 4, 16, 17, 5 };        // GPIO pins for keypad columns


// (keypad2 = 4 x 4)
//19, 18, 5, 17, 16, 4, 27, 26, 25 ---- soldering
byte rowPins[KEYPAD_ROWS] = {19, 18, 5, 17};  // GPIO pins for keypad rows
byte colPins[KEYPAD_COLS] = {16, 4, 27, 26};  // GPIO pins for keypad columns

// (keypad3 = 5 x 4)
// 19, 18, 5, 17, 16, 4, 27, 26, 25 ---- soldering
// byte rowPins[KEYPAD_ROWS] = {25, 26, 27, 4, 16};  // GPIO pins for keypad rows
// byte colPins[KEYPAD_COLS] = {19, 18, 5, 17};  // GPIO pins for keypad columns


// char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
//   { '!', '@', '#', '*' },
//   { '1', '2', '3', 'U' },
//   { '4', '5', '6', 'D' },
//   { '7', '8', '9', 'B' },
//   { 'L', '0', 'R', 'E' }
// };

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','*'},
  {'4','5','6','#'},
  {'7','8','9','!'},
  {'B','0','B','E'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
AsyncWebServer server(80);
Preferences prefs;
HTTPClient http;

bool is_active = false;
bool is_on = false;
float balance = 0.0;
bool is_topped = false;

bool is_deactivated = false;

int dev0 = 0;
int dev1 = 0;

bool summary_fetched = false;

float voltage = 0.0;
float current = 0.0;
float power = 0.0;
float energy = 0.0;
float last_energy = 0.0;
float battery_voltage = 0.0;

unsigned long last_status_check = 0;
unsigned long last_send = 0;
unsigned long last_lcd_update = 0;
unsigned long last_fetch = 0;
unsigned long last_reconnect = 0;
unsigned long last_input_time = 0;

bool input_mode = false;

String input_code = "";
String device_type = "";

bool config_mode = false;

String api_base_url = "https://meters.onegridenergies.com";

const String config_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WiFi Setup</title>
  <style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }
    h2 { color: #333; }
    form { max-width: 300px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
    input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
    button { width: 100%; padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; }
    button:hover { background-color: #45a049; }
    #status { margin-top: 15px; color: #333; font-weight: bold; }
  </style>
</head>
<body>
  <h2>Smart Energy Meter WiFi Setup</h2>
  <form id="wifiForm">
    <input type="text" id="ssid" name="ssid" placeholder="WiFi SSID" required>
    <input type="password" id="pw" name="pw" placeholder="WiFi Password" required>
    <button type="submit">Save and Restart</button>
  </form>
  <div id="status"></div>
  <script>
    const form = document.getElementById('wifiForm');
    const statusDiv = document.getElementById('status');

    form.addEventListener('submit', async (e) => {
      e.preventDefault(); // Prevent normal form submission

      const ssid = document.getElementById('ssid').value.trim();
      const pw = document.getElementById('pw').value.trim();

      if (!ssid || !pw) {
        statusDiv.textContent = "Please fill in all fields.";
        statusDiv.style.color = "red";
        return;
      }
      statusDiv.textContent = "Saving...";
      statusDiv.style.color = "#333";
      try {
        const response = await fetch('/save', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: `ssid=${encodeURIComponent(ssid)}&pw=${encodeURIComponent(pw)}`
        });
        if (response.ok) {
          statusDiv.textContent = "WiFi credentials saved! Restarting...";
          statusDiv.style.color = "green";
          form.reset();
          setTimeout(() => {
            statusDiv.textContent = "";
          }, 3000);
        } else {
          statusDiv.textContent = "Error saving WiFi details.";
          statusDiv.style.color = "red";
          setTimeout(() => {
            statusDiv.textContent = "";
          }, 3000);
        }
      } catch (err) {
        console.error(err);
        statusDiv.textContent = "Network error. Try again.";
        statusDiv.style.color = "red";
        setTimeout(() => {
          statusDiv.textContent = "";
        }, 3000);
      }
    });
  </script>
</body>
</html>
)rawliteral";

// Function prototypes
void setupWiFiConfigMode();
void connectToWiFi();
void fetchMeterStatus();
void handleMetering();
void fetchReadings();
void sendReadings();
void handleKeypadInput();
bool sendDeviceLog(String type, String code);
void displayLCD();
void handleWiFiDisconnect();
void handleSave(AsyncWebServerRequest *request);

void setup() {
  Serial.begin(115200);
  Serial.print("\n\n\n");
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("ONEGRID ENERGIES");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(500);
  for (int i = 0; i < 10; i++) {
    char key = keypad.getKey();
    if (key == 'B') {
      config_mode = true;
      break;
    }
    delay(100);
  }
  if (config_mode) {
    setupWiFiConfigMode();
  } else {
    connectToWiFi();
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    fetchMeterStatus();
  }
}

void loop() {
  if (config_mode) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    handleWiFiDisconnect();
  } else {
    if (millis() - last_status_check > 10000) {
      fetchMeterStatus();
      last_status_check = millis();
    }
    if (is_active && is_on) {
      handleKeypadInput();
    }
    handleMetering();
    if (is_active && is_on && millis() - last_send > 60000) {
      sendReadings();
      energy = 0.0;
      last_send = millis();
    }
  }
  if (millis() - last_lcd_update > 1000) {
    displayLCD();
    last_lcd_update = millis();
  }
}

void setupWiFiConfigMode() {
  lcd.clear();
  lcd.print("##Config Mode##");

  WiFi.softAP("OneGrid Mters", "12345678");

  // Set up web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", config_html);
  });

  server.on("/save", HTTP_POST, handleSave);

  server.begin();
}

void connectToWiFi() {
  prefs.begin("wifi", false);
  String ssid = prefs.getString("ssid", "");
  String pw = prefs.getString("pw", "");
  prefs.end();

  Serial.printf("WiFi Details -> ssid: %s ---- pswd: %s\n", ssid.c_str(), pw.c_str());

  if (ssid != "") {
    WiFi.begin(ssid.c_str(), pw.c_str());
    lcd.clear();
    lcd.print("Connecting WiFi");
  } else {
    lcd.clear();
    lcd.print("No WiFi Config");
  }
}

void fetchMeterStatus() {
  http.begin(api_base_url + "/meters/status?mid=" + mid);

  http.addHeader("x-api-key", apiKey);

  int http_code = http.GET();
  if (http_code == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.printf("Device Status: %s\n", payload.c_str());
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      is_active = doc[0].as<bool>();
      is_on = doc[1].as<bool>();
      balance = doc[2].as<float>();
      is_topped = doc[3].as<bool>();
      if (is_active) {
        summary_fetched = false;
        if (is_deactivated) {
          long_beep_long(1);
        }
        is_deactivated = false;
      } else {
        if(!is_deactivated) {
          long_beep_long(1);
        }
        is_deactivated = true;
      }
    } else {
      Serial.println("JSON parse failed");
    }
  } else {
    Serial.printf("Status GET failed: %d\n", http_code);
  }
  http.end();
}

void handleMetering() {
  if (is_active && is_on) {
    digitalWrite(RELAY_PIN, HIGH);
    if (millis() - last_fetch >= 1000) {
      fetchReadings();
      last_fetch = millis();
    }
  } else if (is_active && !is_on) {
    digitalWrite(RELAY_PIN, LOW);
    voltage = current = power = energy = battery_voltage = 0.0;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    voltage = current = power = energy = battery_voltage = 0.0;
    if (!summary_fetched) {
      http.begin(api_base_url + "/devices/summary?mid=" + mid);
      http.addHeader("x-api-key", apiKey);
      int http_code = http.GET();
      if (http_code == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("Device Summary: %s\n", payload.c_str());
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          dev0 = doc[0].as<int>();
          dev1 = doc[1].as<int>();
          summary_fetched = true;
        }
      }
      http.end();
    }
  }
}

void fetchReadings() {
  Wire.requestFrom(I2C_SLAVE_ADDR, 16);  // Request 16 bytes (4 floats x 4 bytes)
  if (Wire.available() == 16) {
    byte buf[16];
    for (int i = 0; i < 16; i++) {
      buf[i] = Wire.read();
    }
    memcpy(&voltage, buf, 4);
    memcpy(&current, buf + 4, 4);
    memcpy(&power, buf + 8, 4);
    memcpy(&battery_voltage, buf + 12, 4);
    energy += power * 0.00028;
  } else {
    Serial.println("I2C read failed");
  }
}

void sendReadings() {
  http.begin(api_base_url + "/meters/update");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", apiKey);


  DynamicJsonDocument doc(256);
  // doc["apiKey"] = apiKey;
  doc["mid"] = mid;
  doc["vol"] = voltage;
  doc["pow"] = power;
  doc["eng"] = energy + last_energy;
  doc["bat"] = battery_voltage;

  String json;
  serializeJson(doc, json);

  Serial.printf("Sending: %s\n", json.c_str());

  int http_code = http.POST(json);

  if (http_code == HTTP_CODE_OK) {
    last_energy = 0.0;
    String payload = http.getString();
    Serial.printf("Device Status (Dump): %s\n", payload.c_str());
    DynamicJsonDocument rDoc(256);
    DeserializationError error = deserializeJson(rDoc, payload);
    if (!error) {
      is_active = rDoc[0].as<bool>();
      is_on = rDoc[1].as<bool>();
      balance = rDoc[2].as<float>();
      is_topped = rDoc[3].as<bool>();
      if (is_active) {
        summary_fetched = false;
      }
    } else {
      Serial.println("JSON parse failed");
    }
  } else {
    Serial.printf("Readings POST failed: %d\n", http_code);
    last_energy = energy;
  }
  http.end();
}

void handleKeypadInput() {
  char key = keypad.getKey();
  if (key) {
    last_input_time = millis();
    if (!input_mode) {
      if (key == '#') {
        beep_long(1);
        input_mode = true;
        device_type = "one-plastic";
        input_code = "";
        lcd.clear();
        lcd.print("Enter Code:");
      } else if (key == '*') {
        beep_long(1);
        input_mode = true;
        device_type = "other";
        input_code = "";
        lcd.clear();
        lcd.print("Enter Code:");
      }
    } else {
      if (key == 'E') {
        bool success = sendDeviceLog(device_type, input_code);
        if (success) {
          input_mode = false;
          lcd.clear();
          lcd.print("Device Logged");
          beep_short(2);
          delay(1000);
        } else {
          input_mode = false;
          lcd.clear();
          lcd.print("Failed to log");
          beep_long(3);
          delay(1000);
        }
      } else if (key == 'B') {
        input_mode = false;
        input_code = "";
        lcd.clear();
      } else if (isdigit(key)) {
        beep_short(1);
        input_code += key;
        lcd.setCursor(0, 1);
        lcd.print(input_code);
      }
    }
    if (input_mode && millis() - last_input_time > 10000) {
      input_mode = false;
      lcd.clear();
      lcd.print("Input Timeout");
      delay(1000);
    }
  }
}

bool sendDeviceLog(String type, String code) {
  http.begin(api_base_url + "/devices/record");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", apiKey);

  String devId = "";

  if (type == "one-plastic") {
    devId = "OGP" + code;
  } else if (type == "other") {
    devId = "OGD" + code;
  }

  DynamicJsonDocument doc(256);
  doc["mid"] = mid;
  doc["devId"] = devId;

  String json;
  serializeJson(doc, json);

  int http_code = http.POST(json);
  if (http_code != HTTP_CODE_OK) {
    Serial.printf("Log POST failed: %d\n", http_code);
    http.end();
    return false;
  } else {
    http.end();
    return true;
  }
}

void displayLCD() {
  if (input_mode) {
    return;
  }
  lcd.clear();
  if (!is_active) {
    lcd.print("OGP: ");
    lcd.print(dev0);
    lcd.setCursor(0, 1);
    lcd.print("OGD: ");
    lcd.print(dev1);
  } else if (is_active && !is_on) {
    lcd.print("V: 0.0  C: 0.0");
    lcd.setCursor(0, 1);
    lcd.print("P: 0.0");
  } else {
    lcd.print("V:");
    lcd.print(voltage, 1);
    lcd.print(" C:");
    lcd.print(current, 1);
    lcd.setCursor(0, 1);
    lcd.print("P:");
    lcd.print(power, 1);
  }
}

void handleWiFiDisconnect() {
  if (millis() - last_reconnect > 5000) {
    lcd.clear();
    lcd.print("Reconnecting...");
    WiFi.reconnect();
    delay(1000);
    last_reconnect = millis();
  }
}

void handleSave(AsyncWebServerRequest *request) {
  if (request->hasParam("ssid", true) && request->hasParam("pw", true)) {
    String ssid = request->getParam("ssid", true)->value();
    String pw = request->getParam("pw", true)->value();

    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pw", pw);
    prefs.end();

    Serial.printf("Saving ---> ssid: %s ----- pswd: %s\n", ssid.c_str(), pw.c_str());

    request->send(200);
    delay(3000);
    ESP.restart();
  } else {
    request->send(400);
  }
}

void beep_short(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(20);
  }
}

void beep_long(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(600);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
  }
}

void long_beep_long(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(2000);
    digitalWrite(BUZZER_PIN, LOW);
    delay(20);
  }
}