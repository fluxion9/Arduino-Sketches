const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Temperature Sensor Dashboard</title>
  <style>
    :root {
      --temperature-color: #ff6b6b;
      --humidity-color: #1dd1a1;
      --bg-color: #f1f2f6;
      --card-radius: 16px;
      --card-shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
    }

    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
      font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
    }

    body {
      background-color: var(--bg-color);
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: start;
      min-height: 100vh;
      padding: 20px;
    }

    h1 {
      margin-bottom: 24px;
      color: #2f3542;
    }

    .card-container {
      display: flex;
      flex-direction: column;
      gap: 20px;
      width: 100%;
      max-width: 400px;
    }

    .card {
      padding: 20px;
      border-radius: var(--card-radius);
      box-shadow: var(--card-shadow);
      color: white;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 1.2rem;
    }

    .temperature-card {
      background-color: var(--temperature-color);
    }

    .humidity-card {
      background-color: var(--humidity-color);
    }

    .value {
      font-size: 2rem;
      font-weight: bold;
    }

    @media (min-width: 600px) {
      .card-container {
        flex-direction: row;
        justify-content: center;
      }
    }
  </style>
</head>
<body>
  <h1>Temperature Sensor Dashboard</h1>
  <div class="card-container">
    <div class="card temperature-card">
      <div>Temperature</div>
      <div class="value" id="tempValue">-- °C</div>
    </div>
    <div class="card humidity-card">
      <div>Humidity</div>
      <div class="value" id="humidityValue">-- %</div>
    </div>
  </div>

  <script>
    async function fetchSensorData() {
      try {
        const response = await fetch("/get-data");
        const data = await response.json();

        document.getElementById("tempValue").textContent = `${data.temp} °C`;
        document.getElementById("humidityValue").textContent = `${data.humd} %`;
      } catch (error) {
        console.error("Error fetching sensor data:", error);
      }
    }

    setInterval(fetchSensorData, 1000);
    fetchSensorData();
  </script>
</body>
</html>
)rawliteral";