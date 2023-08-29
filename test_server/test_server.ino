#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

#define FILE_PHOTO "/photo.jpg"
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>

    <style>
        html {
            box-sizing: border-box;
        }

        *, *::after, *::before {
            box-sizing: border-box;
        }

        body {
            padding: 0;
            margin: 0;
        }
        header {
            text-align: center;
            padding: 0.5rem 0;
            height: 56px;
        }

        main {
            padding: 0 1rem;
            margin:auto;
            width: 100%;
            max-width: 540px;
            background: rgb(253, 243, 255);
            height: calc(100vh - 56px);
            /* border-radius: 1rem 1rem 0 0; */
        }

        footer {
            padding-top: 1.5rem;
            padding-bottom: 0.75rem;
            text-align: center;
            font-size: 12px;
        }

        @media screen and (min-width: 540px) {
            body {
                background: black;
            }
            header {

            color: white;
            }
            main {
                height: auto;
                border-radius: 1rem;
                padding: 1rem;
                outline: 1px solid white;
                outline-offset: 6px;
            }
        }

        h1 {
            font-size: 1.25rem;
            font-weight: bold;
            margin: 0.5rem 0;
        }

        h2 {
            font-size: 1.125rem;
        }
        button {
            cursor: pointer;
        }

        .cam {
            width: 100%;
            /* border: 1px solid black; */
        }

        .cam-control {
            width: 100%;
            display: flex;
            justify-content: space-between;
            align-items: center;
            /* border: 1px solid black; */
            margin-bottom: 0.5rem;
        }

        .cam-feed {
            border: 2px solid black;
            width: 100%;
            border-radius: 1rem;
            min-height: 30vh;
            margin-bottom: 1rem;
            display: flex;
            align-items: center;
            justify-content: center;
            text-align: center;
            background: black;
            outline: 1px solid black;
            outline-offset: 4px;
            color: white;
        }


        .feed-btn {
            border-radius: 1rem;
            padding: 0.5rem 1rem;
            display: inline-block;
            margin: 0;
            background: rgb(62, 22, 181);
            outline: 1px solid rgb(62, 22, 181);
            outline-offset: 3px;
            border: none;
            color: white;
        }

        .data-title {
                margin-top: 1.5rem;
        }

        .data-feed {
            width: 100%;
            /* border: 1px solid red; */
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
        }

        .temp1 {
            border-radius: 12px;
            margin-bottom: 1rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 0.5rem 1rem;
            width: 100%;
        }

        .temp1-control {
            width: 33.33%;
        }

        .temp1-control input {
            padding: 0.5rem 1rem;
            width: 100%;
            margin-bottom: 0.5rem;
            border-radius: 0.65rem;
            outline: none;
            border: none;
        }
        .temp1-control button {
            padding: 0.5rem 1rem;
            width: 100%;
            border-radius: 0.65rem;
            outline: none;
            border: none;
        }
        .temp2 {
            margin-right: 1rem;
        }

        .temp2, .humid {
            /* border: 2px solid black; */
            border-radius: 12px;
            margin-bottom: 1rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 0.5rem 1rem;
            width: calc(50% - 0.5rem);
            min-height: 80px;
        }

        .temp2 button, .humid button {
            outline: none;
            border: none;

        }

        .temp1{
            background: rgb(62, 22, 181);
            color: rgb(239, 197, 61);

        }
        .temp2 button {
            background: rgb(85, 208, 242);
            color: white;
        }

        .temp2 button {
            background: rgb(62, 22, 181);
            color: white;
        }
        .humid button {
            background: rgb(239, 197, 61);
            color: black;
        }
        .temp2 {
            background: rgb(239, 197, 61);
            color: black;
        }
        .humid {
            background: rgb(85, 208, 242);
            color: black;
        }

        @media screen and (min-width: 540px) {
            .data-title {
                text-align: center;
            }

        }

        .temp1-view, .temp2-view, .humid-view {
            font-size: 24px;
            font-weight: bold;
        }


        .changer {
            margin-bottom: 6px;
            width: 30px;
            height: 30px;
            border-radius: 6px;
        }
        .setter {
            width: 100%;
            padding: 4px 12px;
            border-radius: 6px;
        }

    </style>
</head>
<body>
    <header>
        <h1> Egg Incubator UI </h1>
    </header>
    <main>
        <div class="cam">
            <div class="cam-control">
                <h2>Camera Feed</h2>
                <button class="feed-btn" onClick="captureImage()">Capture</button>
            </div>

            <div class="cam-feed">
                <div> No cam feed</div>
            </div>
        </div>

        <div>
            <h2 class="data-title">Data Monitoring</h2>
            <div class="data-feed">
                <div class="temp1">

                        <div class="temp1-title">Incubator <br/> Temperature</div>
                        <div class="temp1-view" id="temp1">0&deg;C</div>
                    <div class="temp1-control">
                        <input id="temp1-input" step=0.1 type="number" value="0" max="100" min="0"/>
                        <button class="setter" onClick="setTemperature()">Set</button>
                    </div>
                </div>
                <div class="temp2">
                    <div class="temp2-title">Water tray <br/> Temperature</div>
                    <div class="temp2-view" id="temp2">0&deg;C</div>
                </div>
                <div class="humid">
                    <div class="temp2-title">Humidity <br/> Level</div>
                    <div class="humid-view" id="humid">0%</div>
                </div>
            </div>

        </div>

    </main>

    <footer>by precious nwaoha &copy; 2023</footer>

    <script>
        var hum = document.getElementById("humid");
        var atemp = document.getElementById("temp1");
        var ttemp = document.getElementById("temp2");
        var setTemp = document.getElementById("temp1-input");
        var camfeedView = document.querySelector(".cam-feed");

        function getPayLoad() {
          getImage();
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                hum.innerHTML = payLoad.humd + '%';
                atemp.innerHTML = payLoad.atemp + '&deg;C';
                ttemp.innerHTML = payLoad.ttemp + '&deg;C';
            }
          };
          xhttp.open("GET", "/get-data", true);
          xhttp.send();
        }

        function getImage(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              let response = JSON.parse(this.responseText);
              if (response.stat === "1")
              {
                camfeedView.innerHTML = `<img src="saved-photo" alt="camera feed showing eggs"/>`
              }
            }
          };
          xhttp.open("GET", "/check-for-image", true);
          xhttp.send();
        }

        function captureImage(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/capture", true);
          xhttp.send();
        }

        function setTemperature() {
            var val = setTemp.value;
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/set-temp/?temp=" + val, true);
          xhttp.send();
        }
        getPayLoad();
        setInterval(getPayLoad, 1500);
        </script>
</body>
</html>
)rawliteral";

const char* ssid = "Egg Incubator UI";
const char* password = "";

AsyncWebServer server(80);

bool takeNewPhoto = false, newPhotoAvailable = true;
String data_buffer = "", ser_buf = "", input = "";

struct Incubator
{
    void init(void)
    {
        Serial.begin(9600);
        WiFi.softAP(ssid);
        IPAddress IP = WiFi.softAPIP();
        data_buffer.reserve(64);
        ser_buf.reserve(32);
        input.reserve(20);
        if (!SPIFFS.begin(true)) {
          ESP.restart();
        }
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
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
        if (psramFound()) {
          config.frame_size = FRAMESIZE_UXGA;
          config.jpeg_quality = 10;
          config.fb_count = 2;
        } else {
          config.frame_size = FRAMESIZE_SVGA;
          config.jpeg_quality = 12;
          config.fb_count = 1;
        }
        esp_err_t err = esp_camera_init(&config);
        if (err != ESP_OK) {
          ESP.restart();
        }
        server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", index_html);
        });

        server.on("/set-temp", HTTP_GET, [](AsyncWebServerRequest * request) {
        input = "";
        input.concat("[");
        input.concat(request->getParam(0)->value());
        input.concat("]");
        Serial.println(input);
        request->send(200);
        });

        server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", data_buffer.c_str()); 
        });

        server.on("/check-for-image", HTTP_GET, [](AsyncWebServerRequest *request){
          if(newPhotoAvailable)
          {
            request->send_P(200, "text/plain", "{\"stat\":1}"); 
          }
          else {
            request->send_P(200, "text/plain", "{\"stat\":0}"); 
          }
        });

        server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
        newPhotoAvailable = false;
        });

        server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
        takeNewPhoto = true;
        request->send(200);
        });
        server.begin();
    }

    void run(void)
    {
      if (takeNewPhoto) {
        capturePhotoSaveSpiffs();
        takeNewPhoto = false;
      }
      if(Serial.available())
      {
        while(Serial.available() > 0)
        {
          delay(3);
          char c = Serial.read();
          ser_buf += c;
        }
      }
      if(ser_buf.length() > 0)
      {
        ser_buf.trim();
        data_buffer = ser_buf;
        ser_buf = "";
      }  
    }

    bool checkPhoto( fs::FS &fs ) {
      File f_pic = fs.open( FILE_PHOTO );
      unsigned int pic_sz = f_pic.size();
      return ( pic_sz > 100 );
    }

    void capturePhotoSaveSpiffs( void ) {
      camera_fb_t * fb = NULL;
      bool ok = 0;
      do {
        fb = esp_camera_fb_get();
        if (!fb) {
          return;
        }
        File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
        if (!file) {
        }
        else {
          file.write(fb->buf, fb->len);
        }
        file.close();
        esp_camera_fb_return(fb);
        ok = checkPhoto(SPIFFS);
      } while ( !ok );
      newPhotoAvailable = true;
    }
}incubator;

void setup()
{
  incubator.init();
}

void loop()
{
  incubator.run();
}

