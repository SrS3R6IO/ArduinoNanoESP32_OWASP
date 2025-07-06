#include "insecureroutes.h"
#include "server.h"

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

// Add a vulnerable route to demonstrate CVE-2025-53094 (CRLF Injection)
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



// Register the update route BEFORE auth is set up
void setupUnAuthenticatedRoutes() {
  server.on("/update", HTTP_POST, handleFirmwareUpdate);
  
  server.on("/vuln", HTTP_GET, handleVulnServer);
}
