#include <WiFi.h>
#include <WebServer.h>

// Wifi and password hardcoded
const char* ssid = "Telia-2G-C63F61";
const char* password = "NotMyWifIPASS";

const char* login_user = "pepe";
const char* login_pass = "pepe12345"; // Insecure password

WiFiServer insecureService(1337);  // Insecure backdoor service
WiFiClient client;

WebServer server(80);

bool isAuthenticated = false;

// Login form
const char* login_page = R"rawliteral(
<html><head><style>
body { font-family: Arial; background: #f2f2f2; text-align:center; margin-top:50px; }
input { padding: 10px; margin: 5px; width: 200px; }
button { padding: 10px 20px; margin-top: 10px; }
</style></head><body>
<h2>Login</h2>
<form action="/login" method="POST">
<input type="text" name="user" placeholder="Username"><br>
<input type="password" name="pass" placeholder="Password"><br>
<button type="submit">Login</button>
</form>
</body></html>
)rawliteral";


// User control panel
const char* control_panel = R"rawliteral(
<html><head><style>
body { font-family: Arial; background: #fff; text-align:center; margin-top:50px; }
button { padding: 10px 20px; margin: 10px; }
</style></head><body>
<h2>Control Panel</h2>
<a href="/led/on"><button>LED ON</button></a>
<a href="/led/off"><button>LED OFF</button></a>
</body></html>
)rawliteral";


// Login handler
void handleLogin() {
  if (server.method() == HTTP_POST) {
    String user = server.arg("user");
    String pass = server.arg("pass");

    if (user == login_user && pass == login_pass) {
      isAuthenticated = true;
      server.send(200, "text/html", R"rawliteral(
        <html><body><h2>Login correcto</h2>
        <p>Redirecting to the control panel...</p>
        <meta http-equiv='refresh' content='1; url=/panel'>
        </body></html>
      )rawliteral");
    } else {
      server.send(401, "text/plain", "Incorrect credentials.");
    }
  } else {
    server.send(200, "text/html", login_page);
  }
}

// Panel Handler
void handlePanel() {
  if (!isAuthenticated) {
    server.send(302, "text/html", "<meta http-equiv='refresh' content='0; url=/login'>");
    return;
  }
  server.send(200, "text/html", control_panel);
}


void handleNotFound() {
  if (!isAuthenticated) {
    server.send(302, "text/html", "<meta http-equiv='refresh' content='0; url=/login'>");
  } else {
    server.send(404, "text/plain", "Route not found.");
  }
}


// LED ON 
void handleLedOn() {
  if (!isAuthenticated) {
    server.send(403, "text/plain", "Access denied. Log in with /login");
    return;
  }
  digitalWrite(2, HIGH);
  server.send(200, "text/plain", "LED ON");
}

// LED OFF
void handleLedOff() {
  if (!isAuthenticated) {
    server.send(403, "text/plain", "Access denied. Log in with /login");
    return;
  }
  digitalWrite(2, LOW);
  server.send(200, "text/plain", "LED OFF");
}

void setup() {
  Serial.begin(115200);

  delay(1000);
  Serial.println("Booting...");
  
  // Wifi Setting
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected. IP: ");
  Serial.println(WiFi.localIP());

  // HTTP Endpoints
  server.on("/login", handleLogin);
  server.on("/panel", handlePanel);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Server HTTP on at port: 80");

  insecureService.begin();  // Start the insecure TCP service
  Serial.println("Insecure TCP service running on port 1337");

}

void loop() {
  server.handleClient();

  client = insecureService.available();
  if (client) {
    Serial.println("Incoming connection to insecure service");
    String command = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') break;
        command += c;
      }
    }
    client.println(command);
    /*
    command.trim();

    if (command == "LEDON") {
      digitalWrite(2, HIGH);
      client.println("LED turned ON (via insecure service)");
    } else if (command == "LEDOFF") {
      digitalWrite(2, LOW);
      client.println("LED turned OFF (via insecure service)");
    } else {
      client.println("Unknown command");
    }
    */

    client.stop();
  }

}
