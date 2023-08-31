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
#include "myliteral.h"

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

        server.on("/i-content", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", i_content);
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
