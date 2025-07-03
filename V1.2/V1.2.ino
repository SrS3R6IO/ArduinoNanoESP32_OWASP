#include <WiFi.h>
#include <WebServer.h>

#include "auth.h"
#include "dashboard.h"
#include "credentials.h"

// Run our HTTP server
WebServer server(80);

// Addition of V1.2, insecure TCP service
WiFiServer insecureService(1337); 
WiFiClient client;

void setup() {
  // We will use the bandwith 115200 for local debugging
  Serial.begin(115200);

  // Necessary delay for things to work
  delay(1000);
  Serial.println("Booting...");
  
  // Wifi Setting
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Debugging messages
  Serial.println("\nConnected. IP: ");
  Serial.println(WiFi.localIP());

  // Start the server with the login page
  setupAuthRoutes(server);
  // And prepare the other endpoints (non accesible if ot authenticated)
  setupDashboardRoutes(server);

  // Add a not found in case of illegal endpoints
  server.onNotFound([]() {
    if (!isAuthenticated) {
      server.send(302, "text/html", "<meta http-equiv='refresh' content='0; url=/login'>");
    } else {
      server.send(404, "text/plain", "Route not found.");
    }
  });

  server.begin();
  Serial.println("Server running on port 80");

  // For V1.2, we start the insecure TCP backdoor
  insecureService.begin();
  Serial.println("Insecure TCP service running on port 1337");
}

void loop() {
  server.handleClient();

  // Code for handling the TCP backdoor
  client = insecureService.available();
  if (client) {
    Serial.println("Incoming connection to insecure service");
    String command = "";

    // We just check if everything is correct, and send the message back
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') break;
        command += c;
      }
    }
    client.println(command);
    // We also print it in our local serial monitor
    Serial.println(command);
    /*
     Altough this usage is not as risky as it should, if the TCP backdoor is 
     really implemented to receive commands, if not properly parsed it could lead 
     to a fatal outcome. Even more so on devices that have an operating system
    */
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