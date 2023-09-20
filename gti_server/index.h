const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Grid-Tied Inverter</title>
    <style>
        body {
            text-align: center;
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            margin: 0;
        }

        main {
            position: absolute;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            padding: 0 20%;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }

        h1 {
            color: #c46b18;
            margin-bottom: 1rem;
        }

        .params-container {
            display: flex;
            width: 100%;
            gap: 1rem;
            margin-block: 1rem;
            margin-bottom: 2rem;
            justify-content: center;
        }

        .params {
            border: 1px solid #ccc;
            padding: 1rem;
            border-radius: 6px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-between;
            gap: 1rem;
            background-color: #ffffff;
            box-shadow: 0px 2px 6px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s ease-in-out;
        }

        .params:hover {
            transform: translateY(-5px);
        }

        .lamp-container {
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            margin-top: 20px;
        }

        .lamp {
            display: flex;
            flex-direction: column;
            align-items: center;
            margin: 10px;
        }

        .lamp-btn {
            background-color: transparent;
            border: 2px solid goldenrod;
            color: goldenrod;
            padding: 10px 20px;
            cursor: pointer;
            transition: background-color 0.3s, color 0.3s;
            margin-top: 10px;
        }

        .lamp-btn:hover {
            background-color: goldenrod;
            color: black;
        }

        .param-value {
            width: 100%;
            padding: 0.5rem;
            font-weight: 600;
            text-align: center;
            border: none;
            background-color: #ffffff;
            outline: none;
            color: #333;
        }

        @media only screen and (max-width: 780px) {
            main {
                padding: 0;
                padding: 1rem;
            }
            .param {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <main>
        <h1>Grid-Tied Inverter and Smart Meter Dashboard</h1>
        <div class="params-container">
            <div class="params">
                Grid Voltage (V)
                <input
                    title="Grid Voltage"
                    disabled
                    id="volt"
                    class="param-value"
                    value="0.0"
                />
            </div>
            <div class="params">
                Inverter Voltage (V)
                <input
                    title="Inverter Voltage"
                    disabled
                    id="volt0"
                    class="param-value"
                    value="0.0"
                />
            </div>
            <div class="params">
                Current (A)
                <input
                    title="Current"
                    disabled
                    id="curr"
                    class="param-value"
                    value="0.0"
                />
            </div>
            <div class="params">
                Power (W)
                <input
                    title="Power"
                    disabled
                    id="powr"
                    class="param-value"
                    value="0.0"
                />
            </div>
            <div class="params">
                Energy (kWHr)
                <input
                    title="Energy"
                    disabled
                    id="enrg"
                    class="param-value"
                    value="0.0"
                />
            </div>
        </div>
        <div class="lamp-container">
            <div class="lamp">
                <button class="lamp-btn" onclick="activateLoad()">Activate Load</button>
            </div>
            <div class="lamp">
                <button class="lamp-btn" onclick="deactivateLoad()">Deactivate Load</button>
            </div>
            <div class="lamp">
                <button class="lamp-btn" onclick="clamp()">Clamp Grid</button>
            </div>
            <div class="lamp">
                <button class="lamp-btn" onclick="unclamp()">Unclamp Grid</button>
            </div>
        </div>
    </main>

    <script lang="text/javascript">
        let grid = document.getElementById('volt');
        let ivtr = document.getElementById('volt0');
        let curr = document.getElementById('curr');
        let powr = document.getElementById('powr');
        let enrg = document.getElementById('enrg');

        function activateLoad(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/on", true);
          xhttp.send();
        }

        function deactivateLoad(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/off", true);
          xhttp.send();
        }

        function clamp(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/clamp", true);
          xhttp.send();
        }

        function unclamp(){
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/uclamp", true);
          xhttp.send();
        }

        function getPayLoad() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    let payLoad = JSON.parse(this.responseText);
                    grid.setAttribute('value', payLoad.grid);
                    ivtr.setAttribute('value', payLoad.ivtr);
                    curr.setAttribute('value', payLoad.curr);
                    powr.setAttribute('value', payLoad.powr);
                    enrg.setAttribute('value', payLoad.enrg);
                }
            };
            xhttp.open("GET", "/get-data", true);
            xhttp.send();
        }
        getPayLoad();
        setInterval(getPayLoad, 1500);
    </script>
</body>
</html>
)=====";