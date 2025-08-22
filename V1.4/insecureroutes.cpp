#include "insecureroutes.h"
#include "server.h"

// Handler for firmware update added on Version1.4 for "Lack of secure update mechanism"


/* 
At first it would be modified in V1.5, included the use of the library "Update.h" to
set up an insecure method of a secure library,  but it was decided to implement 
an actual vulnerable library for educational purposes.
*/
void handleFirmwareUpdate(AsyncWebServerRequest *request) {
  // We make sure it's a POST request
  if (request->method() == HTTP_POST) {
    // To make it easier to debbug, the information will be plain text
    if (request->hasArg("plain")) {
      String firmwareData = request->arg("plain");

      Serial.println("Received firmware update request:");
      Serial.println(firmwareData.substring(0, 100)); // Print first 100 characters

      request->send(200, "text/plain", "Firmware update received and applied. Rebooting...");

      // Simulate "firmware update"
      delay(1000);
      ESP.restart();  
    } else {
      request->send(400, "text/plain", "Bad Request: No firmware payload.");
    }
  } else {
    request->send(405, "text/plain", "Method Not Allowed");
  }
}

// Add a vulnerable route to demonstrate CVE-2025-53094 (CRLF Injection)
// Function that mimics a remote event logger for the dashboard

// How its userd: GET /vuln?log=Fan%20turned%20on HTTP/1.1
// Host: {Host IP addr}
void handleVulnServer(AsyncWebServerRequest *request) {
  if (request->hasParam("log")) {
    String logEntry = request->getParam("log")->value();

    // CRLF injection vulnerability in header
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Log received OK.");
    response->addHeader("X-Log-Entry", logEntry);  // ← Vulnerable spot
    request->send(response);
  } else {
    request->send(400, "text/plain", "Missing 'log' parameter.");
  }
}





// Register the update route BEFORE auth is set up
void setupUnAuthenticatedRoutes() {
  server.on("/update", HTTP_POST, handleFirmwareUpdate);
  
  server.on("/vuln", HTTP_GET, handleVulnServer);
}
