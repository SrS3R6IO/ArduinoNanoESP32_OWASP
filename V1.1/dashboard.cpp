#include "dashboard.h"
#include "credentials.h"
#include "webpages.h"

bool isLampOn = false;
bool isMotorOn = false;
int temperature = 22;

String generateDashboardHTML() {
  String html = R"rawliteral(
    <html>
    <head>
      <style>
        body { font-family: Arial; text-align: center; margin-top: 40px; background: #fff; }
        button { padding: 10px 20px; margin: 10px; cursor: pointer; }
        .led {
          display: inline-block;
          width: 30px;
          height: 30px;
          border-radius: 50%;
          margin: 5px 15px;
          vertical-align: middle;
          border: 2px solid #333;
          transition: background-color 0.3s ease;
        }
        .green { background: #0f0 !important; border-color: #0a0 !important; }
        .red { background: #f00 !important; border-color: #a00 !important; }
        .yellow { background: #ff0; }
        .led.off { background: #444 ; border-color: #222;}
        #greenLed { background: #004400; border-color: #002200;}
        #redLed { background: #6c0000; border-color: #330000;}

        .microwave {
          width: 120px;
          height: 100px;
          border: 4px solid #666;
          border-radius: 10px;
          margin: 20px auto;
          position: relative;
          background: #ddd;
          box-shadow: inset 0 0 5px #aaa;
        }
        .microwave-door {
          position: absolute;
          width: 100px;
          height: 70px;
          top: 15px;
          left: 10px;
          background: #999;
          border-radius: 8px;
          box-shadow: inset 0 0 8px #666;
          transition: transform 0.5s ease;
          transform-origin: left center;
        }
        .microwave-door.open {
          transform: rotateY(-75deg);
          background: #bbb;
          box-shadow: inset 0 0 12px #999;
        }

        /* Spinner container to hold spinner and static timer */
        .microwave-spin-container {
          position: absolute;
          top: 35px;
          right: 10px;
          width: 60px;
          height: 30px;
          display: flex;
          align-items: center;
          justify-content: center;
        }
        .microwave-spin {
          width: 50px;
          height: 50px;
          border: 4px solid #ccc;
          border-top: 4px solid #2196f3;
          border-radius: 50%;
          animation: spin 1s linear infinite;
          display: none;
          z-index: 1;
          position: absolute;
        }
        .microwave-spin.running {
          display: block;
        }
        .motor-timer-text {
          position: absolute;
          z-index: 2;
          font-weight: bold;
          font-size: 1.2em;
          user-select: none;
          min-width: 50px;
          color: #000;
          display: none;
        }

        @keyframes spin {
          0% { transform: rotate(0deg); }
          100% { transform: rotate(360deg); }
        }

        .thermometer {
          width: 40px;
          height: 150px;
          border: 3px solid #333;
          border-radius: 20px;
          margin: 20px auto;
          background: #eee;
          position: relative;
          box-shadow: inset 0 0 10px #ccc;
        }
        .mercury {
          position: absolute;
          bottom: 10px;
          left: 50%;
          transform: translateX(-50%);
          width: 20px;
          border-radius: 10px;
          background: linear-gradient(to top, #f00, #f77);
          transition: height 1s ease;
          height: 15%;
        }
        #tempValue {
          font-weight: bold;
          font-size: 2em;
          margin-top: 10px;
        }
      </style>
    </head>
    <body>
      <h2>Control Panel</h2>
      <p><b>Operation LEDs</b></p>
      <div>
        <div id="greenLed" class="led off" title="Operation Success LED"></div>
        <div id="redLed" class="led off" title="Operation Failure LED"></div>
      </div>
      
      <div id="yellowLed" class="led )rawliteral";
    html += isLampOn ? "yellow" : "off";
    html += R"rawliteral(" title="Lamp LED">
      </div>
      <p></p>
      <button id="toggleYellow">)rawliteral";
  html += isLampOn ? "Turn Led Off" : "Turn Led On";
  html += R"rawliteral(</button>
     

      <p><b>Simulated Motor (Microwave)</b></p>
      <div class="microwave">
        <div id="microwaveDoor" class="microwave-door"></div>
        <div class="microwave-spin-container">
          <div id="microwaveSpin" class="microwave-spin"></div>
          <div id="motorTimer" class="motor-timer-text">00:00</div>
          
        </div>
      </div>
      <button id="motorToggleBtn">)rawliteral";
  html += isMotorOn ? "Stop Motor" : "Start Motor";
  html += R"rawliteral(</button>

      <p><b>Temperature Sensor</b></p>
      <div class="thermometer">
        <div id="mercury" class="mercury"></div>
      </div>
      <div id="tempValue">22&#176C</div>

      <script>
        let temperature = 22;
        let motorOn = )rawliteral" + String(isMotorOn ? "true" : "false") + R"rawliteral(;
        let lampOn = )rawliteral" + String(isLampOn ? "true" : "false") + R"rawliteral(;

        const motorBtn = document.getElementById('motorToggleBtn');
        let motorTimer = 0;
        let motorInterval = null;


        // Temperature Animation
        function updateTemperature(temp) {
          const mercury = document.getElementById('mercury');
          const tempValue = document.getElementById('tempValue');
          const maxTemp = 40;
          let heightPercent = 15 + (temp / maxTemp) * 75;
          heightPercent = Math.min(90, Math.max(15, heightPercent));
          mercury.style.height = heightPercent + '%';
          // The &#176; is º formated, because if not it will be displayed as: Âº
          tempValue.innetHTML = temp + '&#176;C';
        }

        setInterval(() => {
          let change = Math.random() < 0.5 ? -1 : 1;
          temperature = Math.max(15, Math.min(30, temperature + change));
          updateTemperature(temperature);
        }, 10000);

        updateTemperature(temperature);

        // Motor Animation
        function updateMotorVisual(isRunning) {
          const door = document.getElementById('microwaveDoor');
          const spinner = document.getElementById('microwaveSpin');
          const timerText = document.getElementById('motorTimer');
          if (isRunning) {
            door.classList.add('open');
            spinner.classList.add('running');
            timerText.style.display = 'flex';
          } else {
            door.classList.remove('open');
            spinner.classList.remove('running');
            console.log(timerText)
            timerText.style.display = 'none';
          }
        }

        function formatTime(seconds) {
          let mins = Math.floor(seconds / 60);
          let secs = seconds % 60;
          return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
        }

        function updateTimerDisplay() {
          document.getElementById('motorTimer').textContent = formatTime(motorTimer);
        }

        function startMotorCountdown() {
          if (motorInterval) clearInterval(motorInterval);

          updateMotorVisual(true);
          motorInterval = setInterval(() => {
            motorTimer--;
            updateTimerDisplay();

            if (motorTimer <= 0) {
              clearInterval(motorInterval);
              motorInterval = null;
              motorOn = false;
              updateMotorVisual(false);
              motorBtn.textContent = "Start Motor";
            }
          }, 1000);
        }


        // LED Control
        function showStatusLed(success) {
          const greenLed = document.getElementById('greenLed');
          const redLed = document.getElementById('redLed');

          // Remove green/red classes and add 'off' for both initially
          greenLed.classList.remove("green");
          redLed.classList.remove("red");
          greenLed.classList.add("off");
          redLed.classList.add("off");

          if (success) {
            greenLed.classList.add("green");
            greenLed.classList.remove("off");
          } else {
            redLed.classList.add("red");
            redLed.classList.remove("off");
          }

          setTimeout(() => {
            greenLed.classList.remove("green");
            redLed.classList.remove("red");
            greenLed.classList.add("off");
            redLed.classList.add("off");
          }, 3000);
        }

        // Toggle Motor
        document.getElementById('motorToggleBtn').addEventListener('click', () => {
          fetch('/motor/toggle', { method: 'POST' })
            .then(res => {
              if (res.ok) {
                if (!motorOn){
                  motorTimer = 30;
                  motorOn = true;
                  startMotorCountdown();
                  motorBtn.textContent = "Add 30s";
                }
                else{
                  motorTimer += 30; 
                  updateTimerDisplay();
                }
                showStatusLed(true);
              } else {
                showStatusLed(false);
              }
            }).catch(() => showStatusLed(false));
        });


        // Toggle Lamp
        document.getElementById('toggleYellow').addEventListener('click', () => {
          fetch('/lamp/toggle', { method: 'POST' })
            .then(res => {
              if (res.ok) {
                lampOn = !lampOn;
                document.getElementById('yellowLed').classList.toggle('yellow', lampOn);
                document.getElementById('yellowLed').classList.toggle('off', !lampOn);
                showStatusLed(true);
              } else {
                showStatusLed(false);
              }
            }).catch(() => showStatusLed(false));
        });

        updateMotorVisual(motorOn);
      </script>

    </body>
    </html>
  )rawliteral";

  return html;
}



void handlePanel(WebServer &server) {
  if (!isAuthenticated) {
    server.send(302, "text/html", "<meta http-equiv='refresh' content='0; url=/login'>");
    return;
  }
  server.send(200, "text/html", generateDashboardHTML());
}

void handleLampToggle(WebServer &server) {
  if (!isAuthenticated) {
    server.send(403, "application/json", "{\"success\":false, \"message\":\"Unauthorized\"}");
    return;
  }
  // Toggle lamp state
  isLampOn = !isLampOn;

  // Simulate sending an HTTP request to a real device
  Serial.println(isLampOn ? "HTTP: Turning lamp ON" : "HTTP: Turning lamp OFF");

  server.send(200, "application/json", "{\"success\":true}");
}

void handleMotorToggle(WebServer &server) {
  if (!isAuthenticated) {
    server.send(403, "application/json", "{\"success\":false, \"message\":\"Unauthorized\"}");
    return;
  }
  // Toggle motor state
  isMotorOn = !isMotorOn;
  // Simulate sending an HTTP request to a real device
  Serial.println(isMotorOn ? "HTTP: Starting motor (microwave)" : "HTTP: Stopping motor");

  server.send(200, "application/json", "{\"success\":true}");
}

void setupDashboardRoutes(WebServer &server) {
  server.on("/panel", HTTP_GET, [&server]() { handlePanel(server); });
  server.on("/lamp/toggle", HTTP_POST, [&server]() { handleLampToggle(server); });
  server.on("/motor/toggle", HTTP_POST, [&server]() { handleMotorToggle(server); });

}
