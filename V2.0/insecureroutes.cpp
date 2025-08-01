#include "insecureroutes.h"

// Handler for firmware update added on Version1.4 for "Lack of secure update mechanism"
/* 
At first it would be modified in V1.5, included the use of the library "Update.h" to
set up an insecure method of a secure library,  but it was decided to implement 
an actual vulnerable library for educational purposes.
*/
void handleFirmwareUpdate(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "[SIMULATION] Firmware update endpoint reached.");

  /* // Implementation of how the update library would have been
  // Added in Version1.5 for outdated library risk
  #include <Update.h>
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update Start: %s\n", upload.filename.c_str());
    /*
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.printf("[SIMULATION] Receiving %u bytes...\n", upload.currentSize);
    
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("[SIMULATION] Update End: %u total bytes received.\n", upload.totalSize);
    
    if (Update.end(true)) { // true = force reboot
      Serial.printf("Update Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    */
}

// V1.4 adds a vulnerable route to demonstrate CVE-2025-53094 (CRLF Injection)
void handleVulnServer(AsyncWebServerRequest *request) {
  if (request->hasParam("header")) {
    String attackerInput = request->getParam("header")->value();

    // Make a manual response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Header injected (insecure)");
    // Vulnearble
    response->addHeader("X-Injected", attackerInput); 
    request->send(response);

  } else {
    request->send(400, "text/plain", "Missing 'header' param");
  }
}

/*
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

  // Include the default users
  page  += R"rawliteral(
              <li>Username: <strong>pepe</strong>, Password: <strong>12345</strong></li>
              <li>Username: <strong>admin</strong>, Password: <strong>admin</strong></li>
              <li>Username: <strong>anonymous</strong>, Password: <em>(empty)</em></li>
              <li>Username: <strong>root</strong>, Password: <strong>root</strong></li>
              <li>Username: <strong>user</strong>, Password: <strong>user</strong></li>
            </ul>
          )rawliteral";

  page += "</body></html>";
  
  request->send(200, "text/html", page);
}
*/

void handleStatus(AsyncWebServerRequest *request){
  String status = "DeviceID: ESP32-001\nFirmware: v2.0\n";
  request->send(200, "text/plain", status);
}


// Register the update route BEFORE auth is set up
void setupUnAuthenticatedRoutes() {
  // Included in Version 1.3
  server.on("/update", HTTP_POST, handleFirmwareUpdate);
  
  // Included in Version 1.4
  server.on("/vuln", HTTP_GET, handleVulnServer);

  // Included in Version 1.5
  // Dump deleted on secure version
  //server.on("/dump", HTTP_GET, handleDump);
  server.on("/status", HTTP_GET, handleStatus);
}
