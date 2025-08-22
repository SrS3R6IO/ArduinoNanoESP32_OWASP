#include "insecureroutes.h"

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

// V1.4 adds a vulnerable route to demonstrate CVE-2025-53094 (CRLF Injection)
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


// V1.5 adds a vulnerable route that stores information
void handleDump(AsyncWebServerRequest *request){
  String page = "<html><body><h2>Dump of Sensitive Data (INSECURE)</h2>";
  page += "<p><b>Last User:</b> " + sensitiveLog.lastUser + "</p>";
  page += "<p><b>Last Password (Plaintext):</b> " + sensitiveLog.lastPassword + "</p>";
  page += "<p><b>Last TCP Command:</b> " + sensitiveLog.lastTCPCommand + "</p>";

  // Optional: Show MD5 hash
  MD5Builder md5;
  md5.begin();
  md5.add(sensitiveLog.lastPassword);
  md5.calculate();
  page += "<p><b>Last Password (MD5 hash):</b> " + md5.toString() + "</p>";

  page += "</body></html>";
  
  request->send(200, "text/html", page);
}

void handleStatus(AsyncWebServerRequest *request){
  String status = "DeviceID: ESP32-001\nFirmware: v1.5\nUser: " + sensitiveLog.lastUser + "\n";
  request->send(200, "text/plain", status);
}


// Register the update route BEFORE auth is set up
void setupUnAuthenticatedRoutes() {
  // Included in Version 1.3
  server.on("/update", HTTP_POST, handleFirmwareUpdate);
  
  // Included in Version 1.4
  server.on("/vuln", HTTP_GET, handleVulnServer);

  // Included in Version 1.5
  server.on("/dump", HTTP_GET, handleDump);
  server.on("/status", HTTP_GET, handleStatus);
}
