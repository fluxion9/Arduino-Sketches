#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Water-Bath Controller";
const char* password = "";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<style>
    *, *::after, *::before {
        box-sizing: border-box;
        font-size: inherit;
    }
    body {
        padding: 0;
        margin: 0;
        font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
    }
    main {
        background: #E0E2EC;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: space-between;
        padding: 4rem 2rem;
        height: 100vh;
        position: relative;
    }

    .wrapper {
        display: flex;
        flex-direction: column;
        align-items: center;
    }

    .show {
        border-radius: 50%;
        width: 150px;
        height: 150px;
background: var(--material-theme-white, #FFF);
box-shadow: -1px 4px 4px 0px rgba(0, 0, 0, 0.25);
        font-size: 36px;
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 16px;
        margin-bottom: 4rem;
        font-weight: 700;
    }
    button {
        border: none;
        outline: none;
    }

    .controls {
        display: none;
    }

    .set-btn {
        border-radius: 24px;
        background: var(--material-theme-sys-light-primary-fixed-dim, #ABC7FF);
        box-shadow: -1px 4px 4px 0px rgba(0, 0, 0, 0.25);
        border-radius: 18px;
        padding: 10px 16px;
        font-weight: 700;
        cursor: pointer;
        margin: 0 8px;
        color: black;
    }

    .cont {
        display: flex;
        justify-content: center;
        align-items: center;
        background: white;
        border-radius: 8px;
        border-radius: 12px;
background: var(--material-theme-white, #FFF);
box-shadow: -1px 4px 4px 0px rgba(0, 0, 0, 0.25);
        font-weight: 500;
        cursor: pointer;
        width: 35px;
        height: 35px;
        line-height: 35px;
        text-align: center;
        font-size: 24px;
    }

.power-btn {
    width: 96px;
    border-radius: 18px;
    padding: 8px 24px;
    cursor: pointer;
    position: relative;
    height: 36px;
    border-radius: 24px;
background: var(--material-theme-sys-light-primary-fixed-dim, #ABC7FF);
box-shadow: -1px 4px 4px 0px rgba(0, 0, 0, 0.25);

}

.power-btn-inner {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    cursor: pointer;
    position: absolute;
    top: calc(50% - 15px);
    transition: left 0.3s;
    left: 2px;
    background: #fff;
}
.power-btn-text {
    font-weight: 700;
    position: absolute;
    top: calc(50% - 8px);
    right: 16px;
}

.on-btn {
    right: 2px;
    left: auto;
    background: #fff;
}

.on-text {
    left: 16px;
    right: auto;
}

.on-controls {
    display: flex;
        justify-content: center;
        align-items: center;
}

.set-temp {
    background: #ABC7FF;;
    color: #121212;
}

.menu-btn {
    position: absolute;
    top: 0;
    left: 0;
    /* border: 1px solid red; */
    padding: 1rem;
    font-weight: 900;
    color: #fff;
    cursor: pointer;
    letter-spacing: 6px;
    opacity: 0;
}

.menu-btn:hover {
    opacity: 1;
}

.menu {
    position: absolute;
    left: -250px;
    top: 0;
    width: 250px;
    height: 100vh;
    background: #fff;
    color: black;
    transition: left .5s;
}

.menu-open {
    left: 0;
}

.close-menu {
    width: 32px;
    height: 32px;
    display: flex;
    align-items: center;
    justify-content: center;
    text-align: center;
    font-size: 32px;
    cursor: pointer;
}


</style>


<body>
    <main >
        <div class="menu-btn" onclick="openMenu()">
            |||
        </div>
        <nav class="menu">
            <div class="close-menu" onclick="closeMenu()">&times;</div>
        </nav>
        <div class="wrapper">
            <div class="show">OFF</div>

            <div class="controls">
                <button class="cont" onclick="changeTemp('-')">-</button>
                <button class="set-btn" onclick="setTemp()">Set Temperature</button>
                <button class="cont" onclick="changeTemp('+')">+</button>
            </div>
        </div>


        <button class="power-btn" onclick="handleSwitch()">

            <div class="power-btn-inner off"></div>
            <div class="power-btn-text">OFF</div>
        </button>
    </main>
</body>
<script>
    let isOn = false
    let realTemp = 25
    let temp = realTemp
    const powerBtnInnerEl = document.querySelector(".power-btn-inner")
    const powerBtnTextEl = document.querySelector(".power-btn-text")
    const showTempEl = document.querySelector(".show")
    const controlsEl = document.querySelector(".controls")
    const menuBtnEl = document.querySelector(".menu-btn")
    const menuEl = document.querySelector(".menu")

    function openMenu() {
        console.log({isOn})
        if (isOn) {
            menuEl.classList.add('menu-open')
        }
    }
    function closeMenu() {
        menuEl.classList.remove('menu-open')
    }

    function handleSwitch() {

        isOn = !isOn
        console.log({isOn}, "changed")
        if (isOn){
            powerBtnInnerEl.classList.add("on-btn")
            powerBtnTextEl.classList.add("on-text")
            controlsEl.classList.add("on-controls")
            powerBtnTextEl.innerHTML = "ON"
            showTempEl.innerHTML = `${temp}&deg;C`
        } else {
            powerBtnTextEl.classList.remove("on-text")
            powerBtnInnerEl.classList.remove("on-btn")
            controlsEl.classList.remove("on-controls")
            powerBtnTextEl.innerHTML = "OFF"
            showTempEl.innerHTML = `OFF`
            showTempEl.classList.remove("set-temp")
        }
    }

    function changeTemp(type) {
        if (type === "-") {
            if (temp === 0) {
                return
            }

            temp--
        } else {
            if (temp === 100) {
                return
            }
            temp++
        }

        showTempEl.classList.remove("set-temp")
        showTempEl.innerHTML = `${temp}&deg;C`

    }

    function setTemp() {
        showTempEl.classList.add("set-temp")
        console.log("set Temp to", temp)
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/set-params/?temp=" + temp, true);
          xhttp.send();
    }
</script>
</html>
)rawliteral";

String data_buffer = "", ser_buf = "", input = "";

String ip = "192.168.4.1";

unsigned long last_millis = 0;

struct wBath
{
    void init(void)
    {
        Serial.begin(9600);
        WiFi.softAP(ssid);
        IPAddress IP = WiFi.softAPIP();
//        Serial.print("AP IP address: ");
//        Serial.println(IP);
        data_buffer.reserve(64);
        ser_buf.reserve(32);
        input.reserve(20);
        
        server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", index_html);
        });

        server.on("/set-params", HTTP_GET, [](AsyncWebServerRequest * request) {
        input = "";
        input.concat("[");
        input.concat(request->getParam(0)->value());
        input.concat(",");
        input.concat(ip);
        input.concat("]");
        Serial.println(input);
        request->send(200);
    });
        
        server.begin();
    }

    void run(void)
    {
      if((millis() - last_millis) >= 2000)
      {
        Serial.println("+read;");
        last_millis = millis();
      }
      while(Serial.available() > 0)
      {
        delay(3);
        char c = Serial.read();
        ser_buf += c;
      }
      if(ser_buf.length() > 0)
      {
        ser_buf.trim();
        data_buffer = ser_buf;
        ser_buf = "";
      }  
    }
    
}wbath;

void setup()
{
  wbath.init();
}

void loop()
{
  wbath.run();
}
