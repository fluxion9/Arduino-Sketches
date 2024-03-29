#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define ALL 4

const char* ssid = "PDB @ 192.168.4.1";
const char* password = "";

AsyncWebServer server(80);

String str_buffer = "", ser_buf = "";

void select(int index)
{

}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Mini Project</title>
    <style>
       @import url('https://fonts.googleapis.com/css2?family=Cabin+Sketch:wght@400;700&family=Comfortaa:wght@300;400;500;600;700&family=Darker+Grotesque:wght@300;400;500;600;700;800;900&family=Hanalei+Fill&family=Lekton:ital,wght@0,400;0,700;1,400&family=Poiret+One&family=Poppins:ital,wght@0,300;0,400;0,500;0,600;0,700;1,300;1,400;1,500;1,600;1,700&family=Press+Start+2P&family=Roboto:wght@300;400;500&family=Sulphur+Point:wght@300;400;700&display=swap');

:root {
    --primary: #172c0c;
    --secondary: #aae03e;
    --primary-dark: #0d1b05;
    --secondary-dark: #95c33a;
    --light: #f8fef5;
    --dark: #060606;
    --shadow-9: rgba(0,0,0,0.44);
    --shadow-7: rgba(0, 0, 0, 0.15);
    --shadow-4: rgba(0, 0, 0, 0.05);
    --outline: #00000028;
}

html {
    font-size: 100%;
    box-sizing: border-box;
}

*, *::before, *::after {
    box-sizing: inherit;
}

body {
    padding: 0;
    margin: 0;
    background: var(--light);
    font-family: 'Poppins', sans-serif;
}

header {
    /* border-bottom: 1px solid var(--dark); */
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    /* box-shadow: ; */
    text-align: center;
    background: var(--primary);
    overflow: hidden;
    border-radius: 0 0 2rem 2rem;   
    padding: 1rem;
    /* border-bottom: 1px solid var(--secondary); */
}

header h1 {
    color: var(--secondary);
    padding: 0.5rem 1rem;
    margin: 0;
    font-size: 2rem;
}

header div {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0.5rem 1rem;
    border: none;
    /* border-top: 1px solid var(--secondary); */
    /* border-bottom: 1px solid var(--secondary); */
}

header div h2 {
    margin: 0;
    font-size: 1.25rem;
    color: var(--secondary)
}

header div button {
    outline: none;
    border: none;
    padding: 0.5rem 1.5rem;
    border-radius: 1rem;
    color: var(--dark);
    background: var(--light);
    cursor: pointer;
}

header div button:hover {
    color: var(--secondary);
}


main {
    margin-top: 8.5rem;
    min-height: calc(100vh - 8.5rem);
    background: var(--light);
    /* padding-bottom: 3rem; */
    /* border: 1px solid red; */
}

main .phases {
    /* border: 1px solid red; */
    padding: 1rem;
    width: 100%;
    text-align: center;
}

.ac-phase {
    border-radius: 1rem;
    display: inline-block;
    margin: 0 0.25rem;
    background: var(--light);
    width: calc(50% - 1rem);
    margin-bottom: 1rem;
    padding: 1rem;
    box-shadow: 0px 2px 4px -1px rgba(0,0,0,0.2), 0px 4px 5px 0px rgba(0,0,0,0.14), 0px 1px 10px 0px rgba(0,0,0,0.12);

}




.ac-phase h4 {
    text-transform: uppercase;
    font-size: 1.125rem;
    font-weight: 500;
    margin-bottom: 0;
}

.ac-phase p {
    margin: 0;
}

.ac-phase .voltage {
    color: var(--secondary);
    font-size: 2rem;
    font-weight: 700;
    margin-bottom: 0.5rem;
}

.ac-phase .suggestion {
    margin-bottom: 0.5rem;
    color: var(--shadow-9);
}

.ac-phase button {
    padding: 0.5rem 1.5rem;
    /* outline: 2px solid var(--secondary); */
    border: none;
    background: var(--secondary);
    border-radius: 1rem;
    /* text-transform: uppercase; */
    cursor: pointer;
    color: var(--light);
    transition: 0.3;
    margin: 1rem 0;
}

.ac-phase button:hover {
    background: var(--secondary-dark);
    color: var(--light);
}

.ac-phase.selected {
    background: var(--secondary);
}

.ac-phase.selected h4 {
    color: var(--primary)
}

.ac-phase.selected .voltage {
    color: var(--light);
}

.ac-phase.selected button {
    background: var(--primary);

}

.ac-phase.selected button:hover {
    background: var(--primary-dark);

}


footer {
    position: fixed;
    bottom: 0;
    left: 0;
    width: 100%;
    color: var(--secondary);
    padding: 1rem;
    background: var(--primary);
    text-align: center;
    border-radius: 2rem 2rem 0 0;
    font-size: 0.65rem;
}


@media screen and (min-width: 760px) {
    header {
        display: flex;
        flex-direction: column;
        align-items: center;
    }
    header div {
        width: 70%;
        border: none;
        border-radius: 1.57rem;
        background: var(--light);
        padding: 0.75rem 1.5rem;
    }

    header div button {
        background: var(--primary);
        color: var(--secondary);
    }
    header div button:hover {
        background: var(--primary-dark);
    }
    

    main {
        /* border: 1px solid red; */
        margin-top: 9.5rem;
        
    min-height: calc(100vh - 8.5rem);
    }

    .phases {
        /* display: flex;
        width: 100%;
        border: 1px solid red; */
        text-align: center;
    }

    .ac-phase {
        width: calc(33.33% - 1rem);
        margin: 0.5rem;
    }


}
    </style>
</head>
<body>
    <header>
        <h1>PDB Control Lab</h1>
        <div>
            <h2 id="on-phase">Disconnected</h2>
            <button onclick="resetPhases()">Reset All Phases</button>
        </div>
    </header>

    <main>
        <div class="phases">
            <span class="ac-phase phase-1" id="phase-1">
                <div>
                    <h4>Phase 1 Voltage</h4>
                    <p class="voltage" id="v1">0</p>
                    <p class="suggestion" id="sug1"></p>
                </div>
                <button onclick="selectPhase(1)">
                    Select Phase
                </button>
            </span>
            <span class="ac-phase phase-2"  id="phase-2">
                <div>
                    <h4>Phase 2 Voltage</h4>
                    <p class="voltage" id="v2">0</p>
                    <p class="suggestion" id="sug2"></p>
                </div>
                <button onclick="selectPhase(2)">
                    Select Phase
                </button>
            </span>
            <span class="ac-phase phase-3"  id="phase-3">
                <div>
                    <h4>Phase 3 Voltage</h4>
                    <p class="voltage" id="v3">0</p>
                    <p class="suggestion" id="sug3"></p>
                </div>
                <button onclick="selectPhase(3)">
                    Select Phase
                </button>
            </span>
        </div>
    </main>

    <footer>
        EEE Mini porject &copy; UNN 2023
    </footer>
    <script>
        var phase1 = document.querySelector(".phase-1");
        var phase2 = document.querySelector(".phase-2");
        var phase3 = document.querySelector(".phase-3");
        var onPhase = document.querySelector("#on-phase")

        getVoltages();
        setInterval(getVoltages, 3000 );
        function selectPhase(phase) {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          if(phase == 1)
          {
            console.log("select1 sent");
            xhttp.open("GET", "/select1", true);
            xhttp.send();
            phase1.classList.add("selected");
            phase2.classList.remove("selected");
            phase3.classList.remove("selected");
            onPhase.innerHTML = "On Phase 1"
          }
          else if(phase == 2)
          {
            xhttp.open("GET", "/select2", true);
            xhttp.send();
            console.log("select2 sent");
            phase2.classList.add("selected");
            phase3.classList.remove("selected");
            phase1.classList.remove("selected");
            onPhase.innerHTML = "On Phase 2"
          }
          else if(phase == 3)
          {
            console.log("select3 sent");
            xhttp.open("GET", "/select3", true);
            xhttp.send();
            phase3.classList.add("selected");
            phase2.classList.remove("selected");
            phase1.classList.remove("selected");
            onPhase.innerHTML = "On Phase 3"
          }
        }

        function resetPhases() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          console.log("resetting Phases");
          xhttp.open("GET", "/resetPhases", true);
          xhttp.send();
          phase1.classList.remove("selected");
            phase2.classList.remove("selected");
            phase3.classList.remove("selected");
        onPhase.innerHTML = "Disconnected"
        }

        function getVoltages() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById("v1").innerHTML = JSON.parse(this.responseText).v1 + "V";
              document.getElementById("v2").innerHTML = JSON.parse(this.responseText).v2 + "V";
              document.getElementById("v3").innerHTML = JSON.parse(this.responseText).v3 + "V";
            }
          };
          console.log("requesting Voltages");
          xhttp.open("GET", "/getVoltages", true);
          xhttp.send();
        }
        </script>
</body>
</html>
)rawliteral";


struct PDB
{
  unsigned long last_millis = 0;
    void init(void)
    {
        Serial.begin(9600);
        //Serial.println("Setting WiFi Access point...");
        WiFi.softAP(ssid); // no password
        IPAddress IP = WiFi.softAPIP();
        //Serial.print("AP IP address: ");
        //Serial.println(IP);

        str_buffer.reserve(32);

        server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
            request->send_P(200, "text/html", index_html);
        });

        server.on("/select1", HTTP_GET, [](AsyncWebServerRequest * request) {
            //Serial.println("selecting 1st phase...");
            Serial.println("+set1;");
            request->send(200);
        });

        server.on("/select2", HTTP_GET, [](AsyncWebServerRequest * request) {
            //Serial.println("selecting 2nd phase...");
            Serial.println("+set2;");
            request->send(200);
        });

        server.on("/select3", HTTP_GET, [](AsyncWebServerRequest * request) {
            //Serial.println("selecting 3rd phase...");
            Serial.println("+set3;");
            request->send(200);
        });

        server.on("/resetPhases", HTTP_GET, [](AsyncWebServerRequest * request) {
            //Serial.println("selecting 3rd phase...");
            Serial.println("+unset;");
            request->send(200);
        });

        server.on("/getVoltages", HTTP_GET, [](AsyncWebServerRequest * request) {
            //Serial.println("Reading AC phases...");
            request->send_P(200, "text/plain", str_buffer.c_str());
        });

        server.begin();
    }

    void run(void)
    {
        if(millis() - last_millis >= 5000)
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
          str_buffer = ser_buf;
          ser_buf = "";
        }  
    }
}pdb;


void setup()
{
    pdb.init();
}

void loop()
{
    pdb.run();
}
