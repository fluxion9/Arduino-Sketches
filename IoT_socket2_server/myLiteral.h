const char index_html[] PROGMEM = R"=====(
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

            background: rgb(253, 243, 255);
        }
        header {
            text-align: center;
            padding: 0.5rem 1rem;
            height: 56px;
            margin-bottom: 1rem;
            width: 100%;
            display: flex;
            justify-content: space-between;
            align-items: center;
            /* border: 1px solid black; */
            margin-bottom: 0.5rem;
        }
        main {
            padding: 0 1rem;
            margin:auto;
            width: 100%;
            max-width: 540px;
            height: calc(100vh - 56px);
            /* border-radius: 1rem 1rem 0 0; */
        }
        @media screen and (min-width: 540px) {

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
        .app-view {
            display: none;
        }

        .app-view-visible {
            display: block;
        }

        .password-view {
            display: none;
        }
        .password-view-visible, .password-set {
            display: flex;
            border: 2px solid black;
             display: flex;
        flex-direction: column;
        border-radius: 1rem;
        padding: 1rem;
        }

       .password-input, .password-setter, .password-set-input, .password-enter {
        margin-bottom: 0.5rem;
        padding: 0.5rem 1rem;
       }
        .on-off-btn {
            border-radius: 1rem;
            padding: 0.5rem 1rem;
            display: inline-block;
            margin: 0;
            background: black;
            outline: 1px solid black;
            outline-offset: 3px;
            border: none;
            color: white;
        }
        .data-feed {
            width: 100%;
            /* border: 1px solid red; */
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
        }
        .power {
            text-align: center;
            border: 2px solid black;
            padding: 0.75rem 1.5rem;
            border-radius: 0.75rem;
            margin-bottom: 0.5rem;

        }
        .current, .voltage {
            border: 2px solid black;
            border-radius: 12px;
            margin-bottom: 1rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 0.5rem 1rem;
            width: 100%;
            min-height: 80px;
        }

        .current-view, .voltage-view {
            font-size: 32px;

        }

        .current-control, .voltage-control {
            width: 33.33%;
        }

        .current-control input, .voltage-control input {
            padding: 0.5rem 1rem;
            width: 100%;
            margin-bottom: 0.5rem;
            border-radius: 0.65rem;
            outline: none;
            border: none;
            background: rgb(225, 225, 225);
        }
        .current-control button, .voltage-control button {
            padding: 0.5rem 1rem;
            width: 100%;
            border-radius: 0.65rem;
            outline: none;
            border: none;
        }
    </style>
</head>
<body>
    <main>
        <div class="password-view password-view-visible">
                <h2>Enter password</h2>
                <input class="password-input" type="password" pattern="\d{4}" title="Please enter a 4-digit PIN" maxlength="4" required >
                <button class="password-enter" onClick="enterPassword()">Enter</button>
        </div>
        <div class="app-view">
            <header>
                <h2>Switch Control</h2>
                <button class="on-off-btn" onClick="ONOFF()">ON/OFF</button>
        </header>
            <div class="power">
                Total Power 0W
            </div>
            <div class="data-feed">
                <div class="current">
                    <div class="current-title">Current</div>
                    <div class="current-view">0A</div>
                    <div class="current-control">
                        <input id="current-input" step="0.1" type="number" max="30" min="0" onchange="changeCurrent()"  />
                        <button class="setter" onClick="setCurrent()">Set</button>
                    </div>
                </div>
                <div class="voltage">
                    <div class="voltage-title">Voltage</div>
                    <div class="voltage-view">0V</div>
                </div>
            </div>
            <div class="password-set">
                <h2>Change password</h2>
                <input id="pass-key" class="password-set-input" type="password" pattern="\d{4}" title="Please enter a 4-digit PIN" maxlength="4" required>
                <button class="password-setter" onClick="setPassword()">Set</button>
            </div>
        </div>
    </main>
    <script>
        var appView = document.querySelector(".app-view")
        var passwordView = document.querySelector(".password-view")
        var currentView = document.querySelector(".current-view");
        var voltageView = document.querySelector(".voltage-view");
        var currentInput = document.querySelector("#current-input");
        var powerView = document.querySelector(".power");
        var passwordSetInput = document.querySelector(".password-set-input");
        var passordInput = document.querySelector(".password-input")
        var power = 0
        var current = 0.0
        var voltage = 0.0
        var password = 0000
        getAll()
        setInterval(getAll, 2000 );
        function getAll() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                voltage = payLoad.curr;
                current = payLoad.volt;
                password = payLoad.key;
                currentView.innerHTML = `${current}A`
                voltageView.innerHTML = `${voltage}V`
                powerView.innerHTML = `Total Power ${current * voltage}W`
            }
            };
            xhttp.open("GET", "/get-data", true);
            xhttp.send();
        }
        function enterPassword() {
            if (passordInput.value == password) {
                appView.classList.add("app-view-visible")
                passwordView.classList.remove("password-view-visible")
            }
            else {
                console.log('wrong password')
            }
        }
        function ONOFF() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
            };
        xhttp.open("GET", "/on-off", true);
        xhttp.send();
        }
        function setCurrent() {
            var ilimit = document.getElementById("current-input").value;
            var url = "http://192.168.4.1/set-limit/?iLimit=" + ilimit;
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
            };
            xhttp.open("GET", url, true);
            xhttp.send();
            }

        function setPassword() {
            var key = document.getElementById("pass-key").value;
            var url = "http://192.168.4.1/set-key/?=" + key;
            key.value = "";
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
            };
            xhttp.open("GET", url, true);
            xhttp.send();
        }
        </script>
</body>
</html>
)=====";