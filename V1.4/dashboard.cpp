#include "dashboard.h"
#include "credentials.h"
#include "webpages.h"
#include "server.h"


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
          Box to containt the "fan", since this is a siulation, this could
          also be a fridge, or other IoT device connected
        */
        @keyframes spin{
          0%{
            transform: rotate(0deg);
          }
          100%{
            transform: rotate(360deg);
          }
        }
        #fan {
          display: flex;
          flex-direction: column; 
          align-items: center;     
          justify-content: center; 
          gap: 10px;              
          margin: 20px auto;
        }

        .ceiling-container{
          width: 150px;
          height: 150px;
          border-radius: 50%;
          position: relative;
          display: inline-block;
          &:after{
            content:"";
            position: absolute;
            left: 50%;
            top: 50%;
            border-radius: 50%;
            width: 35px;
            height: 35px;
            margin-left: -17.5px;
            margin-top: -17.5px;
            // border: 8px solid #555;
            background: #444;
            // background: radial-gradient(#333, #555 100%);
                box-shadow: inset 0 0px 0px 4px #444444, inset 0 1px 2px 11px #383838;
          }
        }
        .ceiling-container.running {
          animation: spin 6ms linear infinite;
        }
        .ceiling-fan{
          display: block;
          background: #ccc;
          border-radius: 2.5px;
          position: absolute;
          box-shadow: inset 1px 1px 20px #555;
          &:after{
            content: "";
            position: absolute;
            background: #666;
            display: block;
          }
          &.horizontal{
            width: auto;
            height: 25px;
            top: 50%;
            margin-top: -12.5px;
            transform: skewX(20deg);
            &:after{
              top: 25%;
              width: 8px;
              height: 50%;
            }
          }
          &.vertical{
            width: 25px;
            height: auto;
            left: 50%;
            margin-left: -12.5px;
            transform: skewY(20deg);
            &:after{
              height: 8px;
              width: 50%;
              margin-left: 25%;
            }
          }
          &.left{
            left: 0;
            right: 50%;
            margin-right: 22px;
            // border-radius: 14px 5px 5px 40px;
            border-radius: 50% 15px 15px 50%;
            &:after{
              left: 100%;
            }
          }
          &.right{
            right: 0;
            left: 50%;
            margin-left: 22px;
            // border-radius: 5px 40px 14px 5px;
            border-radius: 15px 50% 50% 15px;
            &:after{
              right: 100%;
            }
          }
          &.top{
            top: 0;
            bottom: 50%;
            margin-bottom: 22px;
            // border-radius: 40px 14px 5px 5px;
            border-radius: 50% 50% 15px 15px;
            &:after{
              top: 100%;
            }
          }
          &.bottom{
            top: 50%;
            bottom: 0;
            margin-top: 22px;
            // border-radius: 5px 5px 40px 14px;
            border-radius: 15px 15px 50% 50%;
            &:after{
              bottom: 100%;
            }
          }
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
  html += isLampOn ? "Turn Off" : "Turn On";
  html += R"rawliteral(</button>
     

      <p><b>Simulated Motor (fan)</b></p>
      <div id="fan">
        <div class="ceiling-container">
          <div class="ceiling-fan horizontal left"></div>
          <div class="ceiling-fan horizontal right"></div>  
          <div class="ceiling-fan vertical rotated top"></div>
          <div class="ceiling-fan vertical rotated bottom"></div>
        </div>

        <button id="motorToggleBtn">)rawliteral";
          html += isMotorOn ? "Stop Fan" : "Start Fan";
          html += R"rawliteral(</button>
      </div>

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
        const ledBtn = document.getElementById('toggleYellow');
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
        function updateMotorVisual(isOn) {
          const container = document.querySelector('.ceiling-container');
          if (isOn) {
            container.classList.add('running');
          } else {
            container.classList.remove('running');
          }
        }




        function formatTime(seconds) {
          let mins = Math.floor(seconds / 60);
          let secs = seconds % 60;
          return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
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
                motorOn = !motorOn;
                if (motorOn){
                  motorBtn.textContent = "Stop Fan"
                }
                else{motorBtn.textContent = "Start Fan"}
                updateMotorVisual(motorOn);
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
                if (lampOn){
                  ledBtn.textContent = "Turn Off"
                }
                else{ledBtn.textContent = "Turn On"}
                document.getElementById('yellowLed').classList.toggle('yellow', lampOn);
                document.getElementById('yellowLed').classList.toggle('off', !lampOn);
                showStatusLed(true);
              } else {
                showStatusLed(false);
              }
            }).catch(() => showStatusLed(false));
        });
        updateMotorVisual(motorOn)
      </script>

    </body>
    </html>
  )rawliteral";

  return html;
}



// Load the pannel if the user is autheticated
void handlePanel(AsyncWebServerRequest *request) {
  if (!isAuthenticated) {
    request->redirect("/login");
    return;
  }
  request->send(200, "text/html", generateDashboardHTML());
}

// Function to send a message to the lamp to turn on (simulation of wireless lamp)
void handleLampToggle(AsyncWebServerRequest *request) {
  if (!isAuthenticated) {
    request->send(403, "application/json", "{\"success\":false, \"message\":\"Unauthorized\"}");
    return;
  }
  // Toggle lamp state
  isLampOn = !isLampOn;

  // Simulate sending an HTTP request to a real device
  Serial.println(isLampOn ? "HTTP: Turning lamp ON" : "HTTP: Turning lamp OFF");

  request->send(200, "application/json", "{\"success\":true}");
}

// Function to send a message to the microwave to turn on (simulation of wireless motor)
void handleMotorToggle(AsyncWebServerRequest *request) {
  if (!isAuthenticated) {
    request->send(403, "application/json", "{\"success\":false, \"message\":\"Unauthorized\"}");
    return;
  }
  // Toggle motor state
  isMotorOn = !isMotorOn;
  // Simulate sending an HTTP request to a real device
  Serial.println(isMotorOn ? "HTTP: Starting motor (microwave)" : "HTTP: Stopping motor");

  request->send(200, "application/json", "{\"success\":true}");
}

// Main function that loads all sever subroutes used on the panel
void setupDashboardRoutes() {
  server.on("/panel", HTTP_GET, handlePanel);
  server.on("/lamp/toggle", HTTP_POST, handleLampToggle);
  server.on("/motor/toggle", HTTP_POST, handleMotorToggle);

}
