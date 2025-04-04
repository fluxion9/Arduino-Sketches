#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <TM1637Display.h>

// WiFi credentials
const char* ssid = "jne.wton";
const char* password = "gftk2678~";

// const char* serverAddress = "ws://onelantern.onegridenergies.com";
const char* ws_server = "wss://onelantern.onegridenergies.com";

String NodeID = "ESP32";

// int port = 3000;

// String ws_server = "";

// Pins
#define LED_PIN 2
#define CLK_PIN 18  // TM1637 CLK
#define DIO_PIN 19  // TM1637 DIO

// TM1637 Display
TM1637Display display(CLK_PIN, DIO_PIN);

// State variables
bool countdownActive = false;
bool countupActive = false;
bool showing = false;
unsigned long currentMillis = 0, previousMillis = 0;
unsigned long countdownStartMillis = 0;
unsigned long countupStartMillis = 0;
// unsigned long countdownDuration = 88200000; // 24 hours 30 minutes
unsigned long countdownDuration = 180000;  // 3 minutes
unsigned long countdownRemaining = countdownDuration;
unsigned long countupValue = 0;
unsigned long lastBlinkTime = 0;

char resp[25];

unsigned long last_plug_millis = 0;

bool client_connected = false;

using namespace websockets;

// WebSocket client
WebsocketsClient client;

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  Serial.println(message.data());
  resp[0] = '\0';
  strcpy(resp, message.data().c_str());
  trimWhiteSpace(resp);
  processResponse(resp);
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    // client_connected = true;
    Serial.println("Connnection Opened");
    socketEmit("identify", NodeID);
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    client_connected = false;
    Serial.println("Connnection Closed");
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

void connectWS() {
  int w_stat = WiFi.status();
  if (w_stat == WL_CONNECTED && !client_connected && millis() - last_plug_millis >= 2000) 
  {
    // ws_server = "ws://" + WiFi.localIP().toString() + ":" + String(port);
    Serial.print("ws_server: ");
    Serial.println(ws_server);
    Serial.println("Connecting Socket...");
    client_connected = client.connect(ws_server);
    Serial.print("Client connected: ");
    Serial.println(client_connected);
    last_plug_millis = millis();
  }
}

void processResponse(char* res) {
  Serial.print("Got: ");
  Serial.println(res);
  if (strcmp(res, "activate") == 0) {
    countdownActive = true;
    countupActive = false;
    countdownRemaining = countdownDuration;
    digitalWrite(LED_PIN, HIGH);
    countdownStartMillis = millis();
  } else if (strcmp(res, "deactivate") == 0) {
    countdownActive = false;
    countupActive = false;
    digitalWrite(LED_PIN, LOW);
    blinkCountupValue();
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  display.setBrightness(7);
  display.showNumberDecEx(0, 0, true);  // Clear display
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);

  Serial.println("WebSocket callbacks initialized");

  // while(1)
  // {
  //   displayTime(millis());
  //   delay(500);
  // }
}

void runTimer() {
  if (countdownActive) {
    if (millis() - previousMillis >= 1000) {
      countdownRemaining -= 1000;
      if (countdownRemaining <= 0) {
        countdownActive = false;
        countupActive = true;
        countdownRemaining = countdownDuration;
        digitalWrite(LED_PIN, LOW);
        countupStartMillis = millis();
      } else {
        displayTime(countdownRemaining);
      }
      previousMillis = millis();
    }
  }
  if (countupActive) {
    if (millis() - previousMillis >= 1000) {
      countupValue += 1000;
      displayTime(countupValue);
      previousMillis = millis();
    }
  }
}

void loop() {
  connectWS();
  runTimer();
  client.poll();
}

// void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
//   if (type == WStype_TEXT) {
//     String message = String((char*)payload);
//     Serial.println("Message: " + message);

//     if (message == "/activate") {
//       countdownActive = true;
//       countupActive = false;
//       countdownRemaining = countdownDuration;
//       digitalWrite(LED_PIN, HIGH);
//       countdownStartMillis = millis();
//     }

//     if (message == "/deactivate") {
//       countdownActive = false;
//       countupActive = false;
//       digitalWrite(LED_PIN, LOW);
//       blinkCountupValue();
//     }
//   }
// }

void displayTime(unsigned long timeInMillis) {
  unsigned long seconds = (timeInMillis / 1000) % 60;
  unsigned long minutes = (timeInMillis / (1000 * 60)) % 60;
  unsigned long hours = (timeInMillis / (1000 * 60 * 60));
  int displayValue = (hours * 100) + minutes;         // Display as HHMM
  display.showNumberDecEx(displayValue, 0x40, true);  // Display colon
}

void blinkCountupValue() {
  if(!showing && millis() - lastBlinkTime >= 500)
  {
    displayTime(countupValue);
    showing = true;
    lastBlinkTime = millis();
  }
  else if(showing && millis() - lastBlinkTime >= 500) 
  {
    display.clear();
    showing = false;
    lastBlinkTime = millis();
  }
}

void trimWhiteSpace(char* str) {
  if (str == NULL) {
    return;
  }
  int len = strlen(str);
  int start = 0;
  int end = len - 1;
  while (isspace(str[start]) && start < len) {
    start++;
  }
  while (end >= start && isspace(str[end])) {
    end--;
  }
  int shift = 0;
  for (int i = start; i <= end; i++) {
    str[shift] = str[i];
    shift++;
  }
  str[shift] = '\0';
}

void socketEmit(String eventName, String msg) {
  // Format the message as Socket.IO expects: ["event_name", data]
  String message = "42[\"" + eventName + "\"," + "{\"message\":" + msg + "}]";
  Serial.print("Sending: ");
  Serial.println(message);
  // client.send(message);
}
