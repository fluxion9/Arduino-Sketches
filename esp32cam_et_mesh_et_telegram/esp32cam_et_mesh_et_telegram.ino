#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char *net_ssid = "proxy-cam.net";
const char *net_password = "prc-2024";

const char *ssid = "mnet.net";
const char *password = "mnet-2023";

AsyncWebServer server(80);

// Initialize Telegram BOT
// String BOTtoken = "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";  // your Bot Token (Get from Botfather)
String BOTtoken = "7024830282:AAGoHcVkOrwmUNFnzVGCChXbP5zZBUMiSt8";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "819434606";

String sta_num = "";

bool sendPhoto = false;
bool intruder = false;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4

bool flashState = LOW;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22


String users_phone_number = "+2347087109150";
String devs_phone_number = "+2347039913826";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }

        .container {
            text-align: center;
        }
    </style>
    <title>Mnet Server SPA</title>
</head>
<body>
    <div class="container">
        <h1>Mnet Server</h1>
        <p>No. of Connected Devices: <span id="connectedDevices">0</span></p>
    </div>

    <script>
        function updateConnectedDevices() {
            const connectedDevicesElement = document.getElementById('connectedDevices');
             var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    connectedDevicesElement.textContent = this.responseText;
                }
            };
            xhttp.open("GET", "/sta-count", true);
            xhttp.send();
        }
        updateConnectedDevices();
        setInterval(updateConnectedDevices, 1500);
    </script>
</body>
</html>
)rawliteral";


void configInitCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}

void flash(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(12, HIGH);
    delay(150);
    digitalWrite(12, LOW);
    delay(150);
  }
}

void handleNewMessages(int numNewMessages) {
  // Serial.print("Handle New Messages: ");
  // Serial.println(numNewMessages);
  flash(3);
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    CHAT_ID = chat_id;
    // if (chat_id != CHAT_ID) {
    //   bot.sendMessage(chat_id, "Unauthorized user", "");
    //   continue;
    // }
    // Print the received message
    String text = bot.messages[i].text;
    // Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome , " + from_name + "\n";
      welcome += "Use the following commands to interact with the Proxy Cam \n";
      welcome += "/photo : takes a new photo\n";
      welcome += "/flash : toggles flash LED \n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      flash(3);
      // Serial.println("Change flash LED state");
    }
    if (text == "/photo") {
      sendPhoto = true;
      flash(3);
      // Serial.println("New photo request");
    }
  }
}

String sendPhotoTelegram() {
  const char *myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  //Dispose first picture because of bad quality
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb);  // dispose the buffered image

  // Take a new photo
  fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    // Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }

  // Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    // Serial.println("Connection successful");
    flash(2);

    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    size_t imageLen = fb->len;
    size_t extraLen = head.length() + tail.length();
    size_t totalLen = imageLen + extraLen;

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n = n + 1024) {
      if (n + 1024 < fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      } else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        clientTCP.write(fbBuf, remainder);
      }
    }

    clientTCP.print(tail);

    esp_camera_fb_return(fb);

    int waitTime = 10000;  // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + waitTime) > millis()) {
      // Serial.print(".");
      flash(2);
      delay(100);
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state == true) getBody += String(c);
        if (c == '\n') {
          if (getAll.length() == 0) state = true;
          getAll = "";
        } else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length() > 0) break;
    }
    clientTCP.stop();
    // Serial.println(getBody);
  } else {
    getBody = "Connected to api.telegram.org failed.";
    flash(4);
    // Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  // Init Serial Monitor
  Serial.begin(9600);

  // Set LED Flash as output
  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
  digitalWrite(FLASH_LED_PIN, flashState);

  // Config and init the camera
  configInitCamera();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password, 1, 0, 8);
  IPAddress IP = WiFi.softAPIP();
  // Serial.println();
  // Serial.print("softAP IP: ");
  // Serial.println(IP);
  // Serial.print("Connecting to ");
  // Serial.println(net_ssid);
  WiFi.begin(net_ssid, net_password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print(".");
    flash(1);
    delay(500);
  }
  // Serial.println();
  // Serial.print("ESP32-CAM IP Address: ");
  // Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/sta-count", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", sta_num.c_str());
  });

  server.on("/ping!", HTTP_GET, [](AsyncWebServerRequest *request) {
    intruder = true;
    request->send(200);
  });

  server.begin();
}

void sendSMS(String phoneNumber) {
  Serial.println("AT");
  delay(1000);
  Serial.println("AT+CMGF=1");
  delay(1000);
  Serial.println("AT+CMGS=\"" + phoneNumber + "\"");
  delay(1000);
  Serial.print("Hi Boss!\nI'm sorry to say this but just so you know, its about to go down! :-)\n\n");
  Serial.print("On a more serious note, an intruder just crossed your territory.");
  delay(1000);
  Serial.write(26);
  delay(1000);
}

void loop() {
  sta_num = String(WiFi.softAPgetStationNum());
  if (sendPhoto) {
    // Serial.println("Preparing photo");
    flash(3);
    sendPhotoTelegram();
    sendPhoto = false;
  }
  if (intruder) {
    // Serial.println("Intruder Detected!");
    // Serial.println("Sending SMS...");
    flash(3);
    for (int i = 0; i < 3; ++i) {
      digitalWrite(2, 0);
      delay(500);
      digitalWrite(2, 1);
      delay(500);
    }
    digitalWrite(2, 1);
    sendSMS(users_phone_number);
    // Serial.println("Sending Photo...");
    sendPhotoTelegram();
    bot.sendMessage(CHAT_ID, "An Intruder was detected, please check this photo to confirm.", "");
    // Serial.println("Done.");
    sendSMS(devs_phone_number);
    intruder = false;
  }
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      // Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}