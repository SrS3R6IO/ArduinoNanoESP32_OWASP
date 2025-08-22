#include "insecureroutes.h"

// Handler for firmware update added on Version1.4 for "Lack of secure update mechanism"
/* 
At first it would be modified in V1.5, included the use of the library "Update.h" to
set up an insecure method of a secure library,  but it was decided to implement 
an actual vulnerable library for educational purposes.
*/

#include <Update.h>
#include <mbedtls/md.h>

#define SECURE_UPDATE_TOKEN "s3cret-updat3-t0ken"
#define MAX_FIRMWARE_SIZE (1024 * 1024)  // 1MB

mbedtls_md_context_t shaCtx;
bool updateRejected = false;
size_t uploadedSize = 0;

// Optional: hold Update state
bool updateInProgress = false;

void handleFirmwareUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (index == 0) {
    // === START OF UPLOAD ===

    updateRejected = false;
    uploadedSize = 0;
    updateInProgress = true;

    // Authorization check
    if (!request->hasHeader("Authorization") || !request->getHeader("Authorization")->value().startsWith("Bearer ")) {
      updateRejected = true;
      Serial.println("[ERROR] No valid Authorization header");
      return;
    }

    String token = request->getHeader("Authorization")->value().substring(7);
    if (token != SECURE_UPDATE_TOKEN) {
      updateRejected = true;
      Serial.println("[ERROR] Invalid token");
      return;
    }

    // Initialize SHA256
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_init(&shaCtx);
    mbedtls_md_setup(&shaCtx, info, 0);
    mbedtls_md_starts(&shaCtx);

    Serial.println("[INFO] Starting firmware update simulation");
  }

  if (updateRejected) return;

  uploadedSize += len;
  if (uploadedSize > MAX_FIRMWARE_SIZE) {
    updateRejected = true;
    Serial.println("[ERROR] Firmware too large");
    return;
  }

  // Update SHA256 hash
  mbedtls_md_update(&shaCtx, data, len);

  // Simulate writing to flash (commented out)
  /*
  if (Update.write(data, len) != len) {
    updateRejected = true;
    Serial.println("[ERROR] Update.write failed");
    return;
  }
  */

  if (final) {
    Serial.println("[INFO] Firmware upload complete");

    // Finalize SHA256
    byte resultHash[32];
    mbedtls_md_finish(&shaCtx, resultHash);
    mbedtls_md_free(&shaCtx);

    String actualHashHex;
    for (int i = 0; i < 32; ++i) {
      if (resultHash[i] < 0x10) actualHashHex += "0";
      actualHashHex += String(resultHash[i], HEX);
    }

    String clientHash = request->getHeader("X-Firmware-Hash") ? request->getHeader("X-Firmware-Hash")->value() : "";

    if (clientHash != actualHashHex) {
      Serial.println("[ERROR] Hash mismatch");
      updateRejected = true;
      request->send(400, "text/plain", "Hash mismatch. Update rejected.");
      return;
    }

    // Simulate applying the update (commented out)
    /*
    if (!Update.end()) {
      Serial.println("[ERROR] Update.end() failed");
      request->send(500, "text/plain", "Update end failed");
      return;
    }
    if (!Update.isFinished()) {
      Serial.println("[ERROR] Update not finished properly");
      request->send(500, "text/plain", "Update not finished properly");
      return;
    }
    */

    Serial.println("[OK] Simulated firmware update successful.");
    request->send(200, "text/plain", "Firmware validated and accepted. Rebooting...");
    delay(1000);
    ESP.restart();
  }
}


// V1.4 adds a vulnerable route to demonstrate CVE-2025-53094 (CRLF Injection)
// Function that mimics a remote event logger for the dashboard

// How its userd: GET /vuln?log=Fan%20turned%20on HTTP/1.1
// Host: {Host IP addr}
#include <Arduino.h>

// Helper: Remove CR and LF characters
String sanitizeHeaderValue(const String &input) {
  String output;
  for (size_t i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c != '\r' && c != '\n') {
      output += c;
    }
  }
  return output;
}

// Base64 encode to fully neutralize special chars
#include <Base64.h>

String base64Encode(const String &input) {
  size_t inputLen = input.length();
  size_t encodedLen = 4 * ((inputLen + 2) / 3); // Safe size calculation
  char encodedLog[encodedLen + 1];  // +1 for null-terminator

  Base64.encode(encodedLog, (char *)input.c_str(), inputLen);
  encodedLog[encodedLen] = '\0';

  return String(encodedLog);
}

void handleVulnServer(AsyncWebServerRequest *request) {
  if (request->hasParam("log")) {
    String logEntry = request->getParam("log")->value();

    // Sanitize to remove CR/LF characters
    String sanitizedLogEntry = sanitizeHeaderValue(logEntry);

    // Base64 encode to fully prevent header injection (maybe overkill)
    String headerValue = base64Encode(sanitizedLogEntry);

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Log received OK.");
    Serial.println("New log received: " + headerValue);
    response->addHeader("X-Log-Entry", headerValue);
    request->send(response);
  } else {
    request->send(400, "text/plain", "Missing 'log' parameter.");
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
  // Included in Version 1.3, modified in V2.0
  server.on("/update", HTTP_POST, 
    [](AsyncWebServerRequest *request) {
      // Esto solo se ejecuta cuando se completa la subida
      request->send(200, "text/plain", "Firmware uploaded.");
    },
    [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
      // Esto se ejecuta durante la subida de datos
      handleFirmwareUpdate(request, filename, index, data, len, final);
    }
  );



  // Included in Version 1.4
  server.on("/vuln", HTTP_GET, handleVulnServer);

  // Included in Version 1.5
  // Dump deleted on secure version
  //server.on("/dump", HTTP_GET, handleDump);
  server.on("/status", HTTP_GET, handleStatus);
}



