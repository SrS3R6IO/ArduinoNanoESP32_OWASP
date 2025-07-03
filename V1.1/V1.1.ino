#include <WiFi.h>
#include <WebServer.h>

#include "auth.h"
#include "dashboard.h"
#include "credentials.h"

// Run our HTTP server
WebServer server(80);


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
}

void loop() {
  server.handleClient();
}