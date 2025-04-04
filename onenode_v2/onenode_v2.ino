#include <WiFi.h>
#include <HTTPClient.h>
#include <TM1637Display.h>

// WiFi credentials
const char* ssid = "jne.wton";
const char* password = "ihxj7664@";


const char* NodeID = "esp32";

unsigned long last_connect_millis = 0;

#define LED_PIN 2
#define CLK_PIN 18  // TM1637 CLK
#define DIO_PIN 19  // TM1637 DIO

#define BTN_PIN 20

// TM1637 Display
TM1637Display display(CLK_PIN, DIO_PIN);

// State variables
bool countdownActive = false;
bool countupActive = false;
bool showing = false;
unsigned long currentMillis = 0, previousMillis = 0, diff = 0;
unsigned long lastButtonPress = 0;
unsigned long countupStartMillis = 0;

unsigned long countdownDuration = 88200000; // 24 hours 30 minutes

// unsigned long countdownDuration = 180000;  // 3 minutes


long countdownRemaining = countdownDuration;
unsigned long countupValue = 0;
unsigned long lastBlinkTime = 0;

char resp[25];

unsigned long last_plug_millis = 0;

bool activated = false;



void processResponse(const char* res) {
  // Serial.print("Got: ");
  // Serial.println(res);
  if (strcmp(res, "[\"atv\"]") == 0 && !activated) {
    countdownActive = true;
    countupActive = false;
    countdownRemaining = countdownDuration;
    digitalWrite(LED_PIN, HIGH);
    activated = true;
    previousMillis = millis();
  } else if (strcmp(res, "[\"dtv\"]") == 0 && activated) {
    countdownActive = false;
    countupActive = false;
    digitalWrite(LED_PIN, LOW);
    activated = false;
  }
}

void connect() {
  int w_stat = WiFi.status();
  if (w_stat == WL_CONNECTED && millis() - last_connect_millis >= 30000L) {
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
  pinMode(BTN_PIN, INPUT_PULLUP);
  display.setBrightness(7);
  display.showNumberDecEx(0, 0, true);  // Clear display
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

void runTimer() {
  if (countdownActive) {

    if (millis() - previousMillis >= 1000) {
      diff = millis() - previousMillis;
      countdownRemaining -= diff;
      if (countdownRemaining <= 0) {
        countdownActive = false;
        countupActive = true;
        countdownRemaining = countdownDuration;
      } else {
        // Serial.print("CountDown Remaining: ");
        // Serial.println(countdownRemaining);
        displayTime(countdownRemaining);
      }
      previousMillis = millis();
    }
  }
  if (countupActive) {
    if (millis() - previousMillis >= 1000) {
      diff = millis() - previousMillis;
      countupValue += diff;
      // Serial.print("CountUp Value: ");
      // Serial.println(countupValue);
      displayTime(countupValue);
      previousMillis = millis();
    }
  }
}

void loop() {
  connect();
  runTimer();
  checkButton();
  actions();
}

void checkButton()
{
  if(!digitalRead(BTN_PIN)) {
    lastButtonPress = millis();
  }
}

void actions()
{
  if(millis() - lastButtonPress < 10000)
  {
    display.setBrightness(7, true);
    if(countdownRemaining <= 0)
    {
      blinkValue(countupValue);
    }
    else {
      displayTime(countdownRemaining);
    }
  }
  else {
    display.setBrightness(7, false);
  }
}

void displayTime(unsigned long timeInMillis) {
  unsigned long seconds = (timeInMillis / 1000) % 60;
  unsigned long minutes = (timeInMillis / (1000 * 60)) % 60;
  unsigned long hours = (timeInMillis / (1000 * 60 * 60));
  int displayValue = (hours * 100) + minutes;         // Display as HHMM
  display.showNumberDecEx(displayValue, 0x40, true);  // Display colon
}

void blinkValue(long value)
{
  if (!showing && millis() - lastBlinkTime >= 500) {
    displayTime(value);
    showing = true;
    lastBlinkTime = millis();
  } else if (showing && millis() - lastBlinkTime >= 500) {
    display.clear();
    showing = false;
    lastBlinkTime = millis();
  }
}

// void trimWhiteSpace(char* str) {
//   if (str == NULL) {
//     return;
//   }
//   int len = strlen(str);
//   int start = 0;
//   int end = len - 1;
//   while (isspace(str[start]) && start < len) {
//     start++;
//   }
//   while (end >= start && isspace(str[end])) {
//     end--;
//   }
//   int shift = 0;
//   for (int i = start; i <= end; i++) {
//     str[shift] = str[i];
//     shift++;
//   }
//   str[shift] = '\0';
// }
