#include "insecureroutes.h"
#include <Arduino.h>

// This is the insecure firmware update handler
void handleUpdate(WebServer &server) {
  // We make sure it's a POST request
  if (server.method() == HTTP_POST) {
    // To make it easier to debbug, the information will be plain text
    if (server.hasArg("plain")) {
      String firmwareData = server.arg("plain");

      Serial.println("Received firmware update request:");
      Serial.println(firmwareData.substring(0, 100)); // Print first 100 characters

      server.send(200, "text/plain", "Firmware update received and applied. Rebooting...");

      // Simulate "firmware update"
      delay(1000);
      ESP.restart();  
    } else {
      server.send(400, "text/plain", "Bad Request: No firmware payload.");
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

// Register the update route BEFORE auth is set up
void setupUnAuthenticatedRoutes(WebServer &server) {
  server.on("/update", HTTP_POST, [&]() {
    handleUpdate(server);
  });
}
