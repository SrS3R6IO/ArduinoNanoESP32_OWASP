#include <WiFi.h>
#include <WebServer.h>

#include <Update.h> // Added in Version1.5 for outdated library risk 

#include <MD5Builder.h> // Added on Version 1.6 to create md5 hashes of passwords

/*
Version 1.8 included:
- The default credentials "admin" and "anonymous"
- We wont made the users change the password after a default login
- We can display the default users



Version 1.9 will include:

*/

// Structure for creating the default users
struct User {
  const char* username;
  const char* password;
};

// Common default credentials
User defaultUsers[] = {
  {"admin", "admin"},
  {"anonymous", ""},
  {"pepe", "12345"},       // v1.1 legacy weak password
  {"root", "root"},         // classic one
  {"user", "user"}
};

const int numUsers = sizeof(defaultUsers) / sizeof(defaultUsers[0]);


// Structure to store the users information on memory
struct SensitiveData {
  String lastUser = "";
  String lastPassword = "";
  String lastTCPCommand = "";
};

SensitiveData sensitiveLog;



// Wifi and password hardcoded
const char* ssid = "Telia-2G-C63F61";
const char* password = "NotMyWifIPASS";


WiFiServer insecureService(1337);  // Insecure backdoor service
WiFiClient client;


// Posible uso para la version 1.5
//WiFiClientSecure secureClient; // Secure client (made insecure manually)


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

    bool loginSuccess = false;

    // Check against defaultUsers array
    for (int i = 0; i < numUsers; i++) {
      if (user == defaultUsers[i].username && pass == defaultUsers[i].password) {
        loginSuccess = true;
        break;
      }
    }


    sensitiveLog.lastUser = user;
    sensitiveLog.lastPassword = pass;
    if (loginSuccess) {
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

// Handler for firmware update Added on Version1.4 for "Lack of secure update mechanism"
// Modified in V1.5, included the use of the library "Update.h"
// Idea from the recent attack on 4chan
// Otra implementación sería usar una version antigua y previamente explotada
void handleFirmwareUpdate() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update Start: %s\n", upload.filename.c_str());
    /*
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }*/
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.printf("[SIMULATION] Receiving %u bytes...\n", upload.currentSize);
    /*
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }*/
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("[SIMULATION] Update End: %u total bytes received.\n", upload.totalSize);
    /*
    if (Update.end(true)) { // true = force reboot
      Serial.printf("Update Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    */
  }
}

// Added on version 1.6, new route to dump the information stored in memory as plain text
void handleDump() {
  String page = "<html><body><h2>Dump of Sensitive Data (INSECURE)</h2>";
  page += "<p><b>Last User:</b> " + sensitiveLog.lastUser + "</p>";
  page += "<p><b>Last Password (Plaintext):</b> " + sensitiveLog.lastPassword + "</p>";
  page += "<p><b>Last TCP Command:</b> " + sensitiveLog.lastTCPCommand + "</p>";

  // (Opcional) Mostrar la contraseña como MD5 inseguro
  MD5Builder md5;
  md5.begin();
  md5.add(sensitiveLog.lastPassword);
  md5.calculate();
  page += "<p><b>Last Password (MD5 hash):</b> " + md5.toString() + "</p>";

  page += "</body></html>";

  server.send(200, "text/html", page);
}

void handleApiCommand() {
  if (!server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing 'cmd' parameter");
    return;
  }

  String cmd = server.arg("cmd"); // 💀 No validation
  Serial.print("Received insecure command: ");
  Serial.println(cmd);

  // Example: simulate command execution
  if (cmd == "LEDON") {
    digitalWrite(2, HIGH);
  } else if (cmd == "LEDOFF") {
    digitalWrite(2, LOW);
  }

  server.send(200, "text/plain", "Command received: " + cmd);
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

  // Force WiFiClientSecure to accept any certificate (for V1.5)
  //secureClient.setInsecure();
  //Serial.println("WiFiClientSecure configured as insecure.");

  // HTTP Endpoints
  server.on("/login", handleLogin);
  server.on("/panel", handlePanel);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/api/command", handleApiCommand);
  server.on("/dump", HTTP_GET, handleDump);


  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(1000);
    //ESP.restart();
  }, handleFirmwareUpdate);

  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Server HTTP on at port: 80");

  insecureService.begin();  // Start the insecure TCP service
  Serial.println("Insecure TCP service running on port 1337");


  Serial.println("Accepted default credentials:");
  for (int i = 0; i < numUsers; i++) {
    Serial.printf("User: '%s' Password: '%s'\n", defaultUsers[i].username, defaultUsers[i].password);
  }

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

    // Store last command 
    sensitiveLog.lastTCPCommand = command;

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
