#include <WiFi.h>
#include <HTTPClient.h>
#include <TM1637Display.h>

// Pin definitions
#define LED_PIN 2
#define CLK 16   // TM1637 CLK pin
#define DIO 17   // TM1637 DIO pin

// WiFi credentials
const char* ssid = "jne.wton";
const char* password = "nzsm2327$";

// Device credentials
const char* deviceName = "ESP32-Test-dev";
const char* apiKey = "hgbxydtwue43";

String host = "https://onebox.onegridenergies.com";

// Timing variables
unsigned long last_connect_millis = 0;
unsigned long updateInterval = 10000;

// Device state
bool activated = false;

// TM1637 Display
TM1637Display display(CLK, DIO);

void processStatusResponse(const String &res) {
  // Example res: {"success":true,"data":[1,5]}

  int start = res.indexOf("[");
  int end   = res.indexOf("]");

  if (start == -1 || end == -1) {
    // Serial.println("Invalid response: missing array");
    return;
  }

  String dataStr = res.substring(start + 1, end); // e.g. "1,5"
  int comma = dataStr.indexOf(",");
  if (comma == -1) {
    // Serial.println("Invalid response: missing comma");
    return;
  }

  int active = dataStr.substring(0, comma).toInt();
  int activeDays = dataStr.substring(comma + 1).toInt();

  // Serial.print("Active: ");
  // Serial.println(active);
  // Serial.print("Active Days: ");
  // Serial.println(activeDays);

  if (active == 1 && !activated) {
    digitalWrite(LED_PIN, HIGH);
    activated = true;
  } else if (active == 0 && activated) {
    digitalWrite(LED_PIN, LOW);
    activated = false;
  }
  display.showNumberDec(activeDays, false);
}

void connectToAPI() {
  if (WiFi.status() == WL_CONNECTED && millis() - last_connect_millis >= updateInterval) {
    HTTPClient http;

    String url = host + "/devices/status?device=" + deviceName + "&apiKey=" + apiKey;
    // Serial.print("Requesting: ");
    // Serial.println(url);
    http.begin(url);
    int responseCode = http.GET();
    if (responseCode == HTTP_CODE_OK) {
      String payload = http.getString();
      // Serial.print("API Response: ");
      // Serial.println(payload);
      processStatusResponse(payload);
    } else if (responseCode == 404) {
      // Serial.println("Device disabled (404).");
      // digitalWrite(LED_PIN, LOW);
      display.showNumberDec(0, false);
      activated = false;
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(responseCode);
    }
    http.end();
    last_connect_millis = millis();
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  // Initialize TM1637 display
  display.setBrightness(15);
  display.showNumberDec(0, false);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
  }
  // Serial.println("\nWiFi connected");
}

void loop() {
  connectToAPI();
}
