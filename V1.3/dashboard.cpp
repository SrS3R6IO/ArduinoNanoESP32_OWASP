#include "dashboard.h"
#include "credentials.h"
#include "webpages.h"


// Local variables to define the state of the other hardware elements
// realistically speaking, unless they are wired, this variables should be 
// checked by asking the device wirelessly, but since they are all simulated,
// that wont be implemented.
bool isLampOn = false;
bool isMotorOn = false;
int temperature = 22;

// Hardcoded HTML, CSS and JS code to display the main dashboard
// This is also a major security flaw for V1.6, but we will get into that later.
// There is no separate html, css and js files involved due to the caracteristics of Arduino. 
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
        /* Define the colors of the status lights when on */
        .green { background: #0f0 !important; border-color: #0a0 !important; }
        .red { background: #f00 !important; border-color: #a00 !important; }
        /* Status light colors when off */
        #greenLed { background: #004400; border-color: #002200;}
        #redLed { background: #6c0000; border-color: #330000;}

        /* Colors for the main lamp */
        .yellow { background: #ff0; }
        .led.off { background: #444 ; border-color: #222;}

        /* 
          Box to containt the "microwave", since this is a siulation, this could
          also be a fridge, or other IoT device connected
        */
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
        /* Style for the microwave door */
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
        /* Small animation for when its working */
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
        /* Circle that spins simulating a work in progress */
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
        /* Countdown */
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
        /* Temperature sensor display */
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
        /* Ammount of "mercury" displayed on the sensor */
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
        /* 
          We need some local variables for the js since the previously
          defined ones (the global ones in arduino) wont work here
        */
        let temperature = 22;
        /* But we can still reference them */
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


// Load the pannel if the user is autheticated
void handlePanel(WebServer &server) {
  if (!isAuthenticated) {
    server.send(302, "text/html", "<meta http-equiv='refresh' content='0; url=/login'>");
    return;
  }
  server.send(200, "text/html", generateDashboardHTML());
}

// Function to send a message to the lamp to turn on (simulation of wireless lamp)
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

// Function to send a message to the microwave to turn on (simulation of wireless motor)
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

// Main function that loads all sever subroutes used on the panel
void setupDashboardRoutes(WebServer &server) {
  server.on("/panel", HTTP_GET, [&server]() { handlePanel(server); });
  server.on("/lamp/toggle", HTTP_POST, [&server]() { handleLampToggle(server); });
  server.on("/motor/toggle", HTTP_POST, [&server]() { handleMotorToggle(server); });

}
