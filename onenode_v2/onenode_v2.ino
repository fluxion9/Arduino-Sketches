#include <WiFi.h>
#include <HTTPClient.h>

#define LED_PIN 3

// WiFi credentials

const char* ssid = "jne.wton";
const char* password = "ihxj7664@";

const char* NodeID = "esp32";

unsigned long last_connect_millis = 0;

char resp[25];

unsigned long last_plug_millis = 0;

bool activated = false;



void processResponse(const char* res) {
  // Serial.print("Got: ");
  // Serial.println(res);
  if (strcmp(res, "[\"atv\"]") == 0 && !activated) {
    digitalWrite(LED_PIN, HIGH);
    activated = true;
  } else if (strcmp(res, "[\"dtv\"]") == 0 && activated) {
    digitalWrite(LED_PIN, LOW);
    activated = false;
  }
}

void connect() {
  int w_stat = WiFi.status();
  if (w_stat == WL_CONNECTED && millis() - last_connect_millis >= 1000) {
    HTTPClient http;
    char url[120];
    sprintf(url, "https://onebox.onegridenergies.com/device/%s", NodeID);
    // Serial.println("Connecting..");
    http.begin(url);
    int responseCode = http.GET();
    if (responseCode == HTTP_CODE_OK) {
      String payload = http.getString();
      processResponse(payload.c_str());
    }
    last_connect_millis = millis();
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-FI");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  connect();
}
