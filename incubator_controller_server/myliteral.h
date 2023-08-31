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
                <iframe id="camera-feed" src="http://192.168.4.1/i-content" 
                scrolling="no" 
                title="feed" style="border-radius: 1rem;" width="512px" 
                height="300px"></iframe>
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
        var iframe = document.getElementById('camera-feed');
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
          xhttp.open("GET", "http://192.168.4.1/get-data", true);
          xhttp.send();
        }
        function getImage(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              let response = JSON.parse(this.responseText);
              if (response.stat == "1")
              {
                iframe.src = iframe.src;
              }
            }
          };
          xhttp.open("GET", "http://192.168.4.1/check-for-image", true);
          xhttp.send();
        }
        function captureImage(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "http://192.168.4.1/capture", true);
          xhttp.send();
        }
        function setTemperature() {
            var val = setTemp.value;
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "http://192.168.4.1/set-temp/?temp=" + val, true);
          xhttp.send();
        }
        getPayLoad();
        setInterval(getPayLoad, 1500);
        </script>
</body>
</html>
)rawliteral";

const char i_content[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<body style="margin: 0;">
    <image src="http://192.168.4.1/saved-photo" height="300px" width="500px"/>
</body>
</html>
)rawliteral";