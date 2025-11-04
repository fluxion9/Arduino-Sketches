/*
  Smart Energy Meter - ESP32
  - SoftAP WiFi config when specific key is held at boot
  - Async Web config page to store WiFi credentials in Preferences
  - Connect to WiFi (station) and poll meter status API
  - Read measurements from ATmega328 via I2C
  - LCD display of V, I, P (16x2)
  - Keypad: '#' for one-plastic device, '*' for other device; Enter = 'D'
  - Log device via API
  - Upload readings every 30s when metering
  - Non-blocking, modular structure

  USER: change items in // --- USER CONFIG --- below
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

// --- USER CONFIG ---
#define RELAY_PIN           16            // GPIO controlling isolator relay (change as needed)
#define LCD_I2C_ADDR        0x27          // typical I2C address for 16x2 I2C LCD
#define LCD_COLS            16
#define LCD_ROWS            2
#define ATMEGA_I2C_ADDR     0x08          // change if ATmega uses different I2C address


const char* SOFTAP_SSID    = "BoookmieMeterSetup";
const char* SOFTAP_PASS    = "12345678";

const char* PREFS_NAMESPACE = "metercfg";

// API endpoints (placeholders) - replace with your actual URLs
const char* STATUS_API      = "http://yourserver.com/api/meter/status";      // GET -> [is_active,is_on,balance,is_topped]
const char* LOG_DEVICE_API  = "http://yourserver.com/api/meter/log-device";  // POST { type: "#"/"*", code: "..." }
const char* UPLOAD_API      = "http://yourserver.com/api/meter/upload";      // POST { readings... }
const char* SUMMARY_API     = "http://yourserver.com/api/meter/summary";     // GET -> {devices: N}

// Key that triggers WiFi config mode when held at boot (choose any keypad key).
// We'll check if 'A' is pressed at boot. Change if you want another key.
const char SETUP_TRIGGER_KEY = 'A';

// Keypad wiring (assumes 5 rows x 4 columns keypad). Update pins to your wiring:
const byte ROWS = 5;
const byte COLS = 4;
byte rowPins[ROWS] = { 32, 33, 25, 26, 27 };   // Example row pins - change to match your wiring
byte colPins[COLS] = { 14, 12, 13, 15 };       // Example col pins - change to match your wiring

// Keymap for a 4x5 keypad. Adjust label layout if your keypad differs.
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
  {'F','G','H','E'}   // 5th row label choices (modify to match your physical keypad)
};
// Note: 'D' is used as Enter in this sketch. You can change mapping above or this behavior.


// --- END USER CONFIG ---

// Globals & Objects
Preferences prefs;
AsyncWebServer server(80);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// WiFi state stored in Preferences keys
String savedSSID = "";
String savedPASS = "";

bool inConfigMode = false;
bool wifiConnected = false;

// Meter state from remote API
bool meter_is_active = false;
bool meter_is_on = false;
double meter_balance = 0.0;
bool meter_is_topped = false;

// Meter reading cache (from ATmega via I2C)
float reading_voltage = 0.0;
float reading_current = 0.0;
float reading_power   = 0.0;
float reading_energy  = 0.0;
float reading_batt    = 0.0;

// Timing control (non-blocking)
unsigned long lastStatusMillis = 0;
const unsigned long STATUS_INTERVAL_MS = 10000UL; // 10s status check

unsigned long lastUploadMillis = 0;
const unsigned long UPLOAD_INTERVAL_MS = 30000UL; // 30s upload while metering

unsigned long lastLCDMillis = 0;
const unsigned long LCD_REFRESH_MS = 1000UL; // 1s lcd refresh

// Device-entry state machine for keypad
bool waitingDeviceEntry = false;
char currentDeviceType = 0; // '#' or '*'
String deviceCodeBuffer = "";
unsigned long deviceEntryStart = 0;
const unsigned long DEVICE_ENTRY_TIMEOUT_MS = 15000UL; // 15s to enter code

// Helper: show brief message on LCD line 2 for some seconds
unsigned long messageExpire = 0;
String transientMessage = "";

// Forward declarations
void startSoftAPConfig();
void setupServerRoutes();
void saveWiFiCreds(const char* ssid, const char* pass);
void loadWiFiCreds();
void connectToWiFi();
void fetchMeterStatus();
void uploadReadings();
void readFromAtmega();
void updateLCD();
void handleKeypad();
void logDeviceToServer(const String &type, const String &code);
void fetchSummaryAndDisplay();
String httpGET(const char* url);
bool httpPOSTJSON(const char* url, const String &jsonPayload, String &responseOut);

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // default isolator OFF initially

  Wire.begin(); // use default SDA/SCL pins on ESP32; change if using custom pins

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Booting...");

  prefs.begin(PREFS_NAMESPACE, false);
  loadWiFiCreds();

  // Check if setup trigger key is held during boot
  // We'll poll the keypad for a short time to see if SETUP_TRIGGER_KEY is pressed.
  // (If keypad library requires scanning in loop, we attempt a quick immediate read.)
  bool triggerHeld = false;
  unsigned long t0 = millis();
  while (millis() - t0 < 700) { // 700ms window at boot to press setup key
    char k = keypad.getKey();
    if (k == SETUP_TRIGGER_KEY) {
      triggerHeld = true;
      break;
    }
  }

  if (triggerHeld) {
    startSoftAPConfig();
  } else {
    // Normal operation: connect to saved WiFi if available
    connectToWiFi();
  }

  setupServerRoutes();
  lcd.clear();
}

void loop() {
  unsigned long now = millis();

  // Keep keypad handling responsive
  handleKeypad();

  // LCD refresh
  if (now - lastLCDMillis >= LCD_REFRESH_MS) {
    updateLCD();
    lastLCDMillis = now;
  }

  // Periodically fetch meter status when connected to internet and meter is "on"
  if (wifiConnected && (now - lastStatusMillis >= STATUS_INTERVAL_MS)) {
    lastStatusMillis = now;
    fetchMeterStatus();
  }

  // If meter is active & on -> metering mode: read from atmega frequently and upload every 30s
  if (meter_is_active && meter_is_on) {
    readFromAtmega();
    if (now - lastUploadMillis >= UPLOAD_INTERVAL_MS) {
      lastUploadMillis = now;
      uploadReadings();
    }
    // Ensure relay is enabled
    digitalWrite(RELAY_PIN, HIGH);
  } else if (meter_is_active && !meter_is_on) {
    // Meter active but off: disable relay and zero readings
    digitalWrite(RELAY_PIN, LOW);
    reading_voltage = reading_current = reading_power = reading_energy = 0.0;
  } else if (!meter_is_active) {
    // Meter inactive: stop metering and display summary of devices
    digitalWrite(RELAY_PIN, LOW);
    reading_voltage = reading_current = reading_power = reading_energy = 0.0;
    // If connected, fetch summary once (ensure not repeatedly hammering)
    static bool fetchedSummaryOnce = false;
    if (wifiConnected && !fetchedSummaryOnce) {
      fetchedSummaryOnce = true;
      fetchSummaryAndDisplay();
    }
  }

  // Clear transient message when expired
  if (transientMessage.length() && millis() > messageExpire) {
    transientMessage = "";
  }
}

// ---------- WiFi / Web config --------------

void startSoftAPConfig() {
  inConfigMode = true;
  WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP: "); Serial.println(myIP);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Config Mode AP:");
  lcd.setCursor(0,1);
  lcd.print(SOFTAP_SSID);
  // Start Async Server (server routes already configured in setup())
  // server.begin(); // server was created globally and routes configured in setupServerRoutes()
  // No blocking; user can connect to SoftAP and POST new credentials
}

void setupServerRoutes() {
  // Serve a simple config page at "/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String page = R"rawliteral(
      <!DOCTYPE html>
      <html>
        <head>
          <meta name="viewport" content="width=device-width,initial-scale=1">
          <title>Meter WiFi Setup</title>
          <style>
            body{font-family:Arial,Helvetica,sans-serif;margin:20px}
            .card{max-width:420px;margin:auto;padding:10px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,.15)}
            input{width:100%;padding:10px;margin:8px 0;border-radius:4px;border:1px solid #ccc}
            button{padding:10px 16px;border:0;background:#2196F3;color:white;border-radius:4px}
            label{font-weight:600}
          </style>
        </head>
        <body>
          <div class="card">
            <h3>Meter WiFi Setup</h3>
            <p>Enter WiFi credentials for the meter to connect to your network.</p>
            <label>SSID</label>
            <input id="ssid" placeholder="MyWiFiNetwork">
            <label>Password</label>
            <input id="pass" placeholder="wifi-password">
            <p><button onclick="save()">Save & Restart</button></p>
            <div id="msg"></div>
          </div>
          <script>
            function save(){
              var ssid = document.getElementById('ssid').value;
              var pass = document.getElementById('pass').value;
              var xhr = new XMLHttpRequest();
              xhr.open("POST","/save",true);
              xhr.setRequestHeader('Content-Type','application/json');
              xhr.onreadystatechange = function(){
                if(xhr.readyState==4){
                  document.getElementById('msg').innerText = xhr.responseText;
                }
              };
              xhr.send(JSON.stringify({ssid:ssid,pass:pass}));
            }
          </script>
        </body>
      </html>
    )rawliteral";
    request->send(200, "text/html", page);
  });

  // Save endpoint to receive JSON with ssid & pass
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    // this won't be called because body data is sent as chunk; use onRequestBody below
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    // parse simple JSON manually (no ArduinoJson to avoid dependency)
    String body;
    body.reserve(total+1);
    for (size_t i = 0; i < len; i++) body += (char)data[i];
    // crude parse
    String ssid = "";
    String pass = "";
    int p1 = body.indexOf("ssid");
    if (p1 >= 0) {
      int colon = body.indexOf(':', p1);
      int q1 = body.indexOf('"', colon);
      int q2 = body.indexOf('"', q1 + 1);
      if (q1 >= 0 && q2 >= 0) ssid = body.substring(q1 + 1, q2);
    }
    int p2 = body.indexOf("pass");
    if (p2 >= 0) {
      int colon = body.indexOf(':', p2);
      int q1 = body.indexOf('"', colon);
      int q2 = body.indexOf('"', q1 + 1);
      if (q1 >= 0 && q2 >= 0) pass = body.substring(q1 + 1, q2);
    }

    if (ssid.length() > 0) {
      saveWiFiCreds(ssid.c_str(), pass.c_str());
      String msg = "Saved. Restart device to apply.";
      request->send(200, "text/plain", msg);
    } else {
      request->send(400, "text/plain", "Missing ssid");
    }
  });

  // Optional: serve simple health route
  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

// Save WiFi credentials into Preferences
void saveWiFiCreds(const char* ssid, const char* pass) {
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  Serial.printf("Saved SSID: %s\n", ssid);
  // Provide feedback on LCD
  transientMessage = "Saved WiFi. Restart";
  messageExpire = millis() + 6000;
}

// Load WiFi credentials from Preferences
void loadWiFiCreds() {
  savedSSID = prefs.getString("ssid", "");
  savedPASS = prefs.getString("pass", "");
  Serial.printf("Loaded SSID: %s\n", savedSSID.c_str());
}

// Connect to saved WiFi in station mode (if credentials exist)
void connectToWiFi() {
  if (savedSSID.length() == 0) {
    lcd.clear();
    lcd.print("No WiFi creds");
    lcd.setCursor(0,1);
    lcd.print("Hold A to config");
    Serial.println("No saved WiFi credentials");
    wifiConnected = false;
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPASS.c_str());
  lcd.clear();
  lcd.print("Connecting to");
  lcd.setCursor(0,1);
  lcd.print(savedSSID.substring(0,16));
  Serial.printf("Connecting to %s\n", savedSSID.c_str());
  unsigned long start = millis();
  // Non-blocking connection loop with timeout
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
    lcd.clear();
    lcd.print("WiFi Connected");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP().toString().c_str());
    transientMessage = "WiFi OK";
    messageExpire = millis() + 3000;
    fetchMeterStatus();
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi connect failed");
    lcd.clear();
    lcd.print("WiFi failed");
    lcd.setCursor(0,1);
    lcd.print("Hold A to config");
  }
}

// ---------- I2C (ATmega) reading --------------

/*
  Protocol assumption:
  - ESP sends a request by calling Wire.requestFrom(ATMEGA_I2C_ADDR, N)
  - ATmega replies with an ASCII CSV line like:
      V,I,P,E,B\n
    Example: "230.1,1.234,284.6,123.45,7.89\n"
  - The sketch parses float values in that order: voltage,current,power,energy,batt
  - If your ATmega uses different protocol, adjust readFromAtmega() accordingly.
*/

void readFromAtmega() {
  // Request up to 64 bytes and read available string
  Wire.requestFrom(ATMEGA_I2C_ADDR, (uint8_t)64);
  String s;
  unsigned long t0 = millis();
  while (Wire.available()) {
    char c = Wire.read();
    if (c == '\n') break;
    s += c;
    // safety break
    if (millis() - t0 > 50) break;
  }
  s.trim();
  if (s.length() == 0) {
    // No data; leave previous readings unchanged
    return;
  }

  // parse CSV
  // Example: "230.1,1.234,284.6,123.45,7.89"
  int idx1 = s.indexOf(',');
  int idx2 = s.indexOf(',', idx1 + 1);
  int idx3 = s.indexOf(',', idx2 + 1);
  int idx4 = s.indexOf(',', idx3 + 1);
  if (idx1 < 0 || idx2 < 0 || idx3 < 0 || idx4 < 0) {
    // parse fail
    Serial.println("I2C parse fail: " + s);
    return;
  }
  reading_voltage = s.substring(0, idx1).toFloat();
  reading_current = s.substring(idx1 + 1, idx2).toFloat();
  reading_power   = s.substring(idx2 + 1, idx3).toFloat();
  reading_energy  = s.substring(idx3 + 1, idx4).toFloat();
  reading_batt    = s.substring(idx4 + 1).toFloat();

  // For debugging:
  Serial.printf("I2C: V=%.2f I=%.3f P=%.2f E=%.3f B=%.2f\n",
                reading_voltage, reading_current, reading_power, reading_energy, reading_batt);
}

// ---------- API interactions --------------

void fetchMeterStatus() {
  if (!wifiConnected) return;
  String resp = httpGET(STATUS_API);
  if (resp.length() == 0) {
    Serial.println("Status fetch empty");
    return;
  }
  // Expecting a JSON array like: [1,1,12.34,0] or CSV? We'll support simple bracketed CSV.
  // crude parse: remove whitespace
  resp.trim();
  Serial.println("Status resp: " + resp);
  // remove leading/trailing brackets if present
  if (resp.startsWith("[")) resp = resp.substring(1);
  if (resp.endsWith("]")) resp = resp.substring(0, resp.length()-1);
  // split by commas
  int splits[4];
  int pos = 0;
  int last = 0;
  int found = 0;
  for (int i = 0; i < resp.length() && found < 4; i++) {
    if (resp.charAt(i) == ',') {
      splits[found++] = i;
    }
  }
  // easiest: use tokenization
  String parts[4];
  int start = 0;
  int p = 0;
  for (int i = 0; i <= resp.length(); i++) {
    if (i == resp.length() || resp.charAt(i) == ',') {
      parts[p++] = resp.substring(start, i);
      start = i + 1;
      if (p >= 4) break;
    }
  }
  // fallback: if fewer tokens, parse as best-effort
  if (p >= 1) {
    meter_is_active = (parts[0].toInt() != 0);
  }
  if (p >= 2) {
    meter_is_on = (parts[1].toInt() != 0);
  } else {
    meter_is_on = true;
  }
  if (p >= 3) {
    meter_balance = parts[3-1].toFloat(); // bug safe adjust? wait...
    // Correction: parts[2] is third token
    meter_balance = parts[2].toFloat();
  }
  if (p >= 4) {
    meter_is_topped = (parts[3].toInt() != 0);
  }

  // If meter became active again, clear summary fetch flag so summary will later be fetched again when inactive
  static bool prevActive = false;
  if (meter_is_active && !prevActive) {
    // resumed
  }
  prevActive = meter_is_active;
  Serial.printf("Parsed status: active=%d on=%d bal=%.2f topped=%d\n",
                meter_is_active, meter_is_on, meter_balance, meter_is_topped);
}

void uploadReadings() {
  if (!wifiConnected) return;
  // Build JSON payload
  String payload = "{";
  payload += "\"voltage\":" + String(reading_voltage, 3) + ",";
  payload += "\"current\":" + String(reading_current, 4) + ",";
  payload += "\"power\":"   + String(reading_power, 3) + ",";
  payload += "\"energy\":"  + String(reading_energy, 3) + ",";
  payload += "\"battery\":" + String(reading_batt, 3);
  payload += "}";
  String resp;
  bool ok = httpPOSTJSON(UPLOAD_API, payload, resp);
  if (ok) {
    Serial.println("Upload OK: " + resp);
  } else {
    Serial.println("Upload failed");
  }
}

void logDeviceToServer(const String &type, const String &code) {
  if (!wifiConnected) {
    transientMessage = "No Net: cannot log";
    messageExpire = millis() + 3000;
    return;
  }
  String payload = "{";
  payload += "\"type\":\"" + type + "\",";
  payload += "\"code\":\"" + code + "\"";
  payload += "}";
  String resp;
  bool ok = httpPOSTJSON(LOG_DEVICE_API, payload, resp);
  if (ok) {
    transientMessage = "Device logged";
    messageExpire = millis() + 4000;
    Serial.println("Log device resp: " + resp);
  } else {
    transientMessage = "Log failed";
    messageExpire = millis() + 4000;
  }
}

void fetchSummaryAndDisplay() {
  if (!wifiConnected) {
    transientMessage = "No net for summary";
    messageExpire = millis() + 3000;
    return;
  }
  String resp = httpGET(SUMMARY_API);
  if (resp.length() == 0) {
    transientMessage = "Summary fail";
    messageExpire = millis() + 3000;
    return;
  }
  // Expecting JSON like {"devices": 12} or plain number
  resp.trim();
  int devicesCount = -1;
  int p1 = resp.indexOf("devices");
  if (p1 >= 0) {
    int colon = resp.indexOf(':', p1);
    if (colon >= 0) {
      int comma = resp.indexOf(',', colon);
      int end = comma >= 0 ? comma : resp.indexOf('}', colon);
      if (end >= 0) {
        String num = resp.substring(colon+1, end);
        devicesCount = num.toInt();
      }
    }
  } else {
    // try parse as plain number
    devicesCount = resp.toInt();
  }
  if (devicesCount >= 0) {
    lcd.clear();
    lcd.print("Inactive");
    lcd.setCursor(0,1);
    lcd.print("Devices: ");
    lcd.print(devicesCount);
    transientMessage = ""; // keep display until active
  } else {
    transientMessage = "Summary parse fail";
    messageExpire = millis() + 3000;
  }
}

// ---------- HTTP helpers (basic, no ArduinoJson) ----------

String httpGET(const char* url) {
  HTTPClient http;
  http.setTimeout(7000);
  http.begin(url);
  int code = http.GET();
  String payload = "";
  if (code > 0) {
    payload = http.getString();
  } else {
    Serial.printf("HTTP GET fail: %d\n", code);
  }
  http.end();
  return payload;
}

bool httpPOSTJSON(const char* url, const String &jsonPayload, String &responseOut) {
  HTTPClient http;
  http.setTimeout(7000);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST((uint8_t*)jsonPayload.c_str(), jsonPayload.length());
  if (code > 0) {
    responseOut = http.getString();
    http.end();
    return (code >= 200 && code < 300);
  } else {
    Serial.printf("HTTP POST fail: %d\n", code);
    http.end();
    return false;
  }
}

// ---------- Keypad handling ----------

void handleKeypad() {
  char k = keypad.getKey();
  if (!k) return;

  Serial.printf("Key: %c\n", k);

  // If waiting for device entry (after # or *)
  if (waitingDeviceEntry) {
    deviceEntryStart = millis(); // reset timeout on each key
    if (k == 'D') { // Enter key
      // submit device
      if (deviceCodeBuffer.length() > 0) {
        logDeviceToServer(String(currentDeviceType), deviceCodeBuffer);
      } else {
        transientMessage = "No code entered";
        messageExpire = millis() + 2000;
      }
      waitingDeviceEntry = false;
      deviceCodeBuffer = "";
      currentDeviceType = 0;
      return;
    } else if (k == '*') {
      // allow clearing buffer with '*' while entering code
      deviceCodeBuffer = "";
      transientMessage = "Cleared";
      messageExpire = millis() + 1500;
      return;
    } else {
      // append numeric or alpha key (limit length)
      if (deviceCodeBuffer.length() < 20) {
        deviceCodeBuffer += k;
        transientMessage = deviceCodeBuffer;
        messageExpire = millis() + 5000;
      }
      return;
    }
  }

  // Not currently in entry mode -> check special keys
  if (k == '#') {
    waitingDeviceEntry = true;
    currentDeviceType = '#';
    deviceCodeBuffer = "";
    deviceEntryStart = millis();
    transientMessage = "Enter code (#)... D=Enter";
    messageExpire = millis() + 10000;
    return;
  }
  if (k == '*') {
    waitingDeviceEntry = true;
    currentDeviceType = '*';
    deviceCodeBuffer = "";
    deviceEntryStart = millis();
    transientMessage = "Enter code (*)... D=Enter";
    messageExpire = millis() + 10000;
    return;
  }

  // other keys could be used for local functions; ignore for now.
}

// ---------- LCD update ----------

void updateLCD() {
  lcd.clear();
  if (!wifiConnected && inConfigMode) {
    lcd.setCursor(0,0);
    lcd.print("AP Mode:");
    lcd.setCursor(0,1);
    lcd.print(SOFTAP_SSID);
    return;
  }

  // If meter inactive -> show inactive summary message (unless a transient message exists)
  if (!meter_is_active) {
    lcd.setCursor(0,0);
    lcd.print("Meter: INACTIVE");
    // show transient or summary on line 2
    if (transientMessage.length()) {
      lcd.setCursor(0,1);
      lcd.print(transientMessage.substring(0,16));
    }
    return;
  }

  // meter active
  if (meter_is_on) {
    // show V and I on first line, P on second
    char buf1[17];
    snprintf(buf1, sizeof(buf1), "V:%4.1fV I:%4.3fA", reading_voltage, reading_current);
    lcd.setCursor(0,0);
    lcd.print(buf1);
    char buf2[17];
    snprintf(buf2, sizeof(buf2), "P:%6.1fW B:%3.1fV", reading_power, reading_batt);
    lcd.setCursor(0,1);
    lcd.print(buf2);
  } else {
    lcd.setCursor(0,0);
    lcd.print("Meter: OFF");
    lcd.setCursor(0,1);
    lcd.print("Relay open, 0's");
  }

  // show transient message briefly on second line if present
  if (transientMessage.length()) {
    lcd.setCursor(0,1);
    lcd.print(transientMessage.substring(0,16));
  }
}

/* END OF SKETCH */

