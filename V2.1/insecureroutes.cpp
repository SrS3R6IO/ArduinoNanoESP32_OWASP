#include "insecureroutes.h"

// Handler for firmware update added on Version1.4 for "Lack of secure update mechanism"
/* 
At first it would be modified in V1.5, included the use of the library "Update.h" to
set up an insecure method of a secure library,  but it was decided to implement 
an actual vulnerable library for educational purposes.
*/

#include <Update.h>
#include <mbedtls/md.h>
// Base64 encode to fully neutralize special chars
#include <Base64.h>

#define SECURE_UPDATE_TOKEN "s3cret-updat3-t0ken"
#define MAX_FIRMWARE_SIZE (1024 * 1024)  // 1MB

mbedtls_md_context_t shaCtx;

void handleFirmwareUpdate(HTTPRequest *req, HTTPResponse *res) {
  String token = getCookie(req, "session");
  if (!isCurrentUserAdmin(token)) {
    res->setStatusCode(403);
    res->setHeader("Content-Type", "text/html");
    res->print("<h3>Access denied</h3>");
    return;
  }

  size_t uploadedSize = 0;

  // Authorization check
  auto authHeader = req->getHeader("Authorization");
  if (authHeader.empty() || authHeader.rfind("Bearer ", 0) != 0) {
    Serial.println("[ERROR] No valid Authorization header");
    res->setStatusCode(401);
    res->setHeader("Content-Type", "text/html");
    res->print("No valid Bearer in authorization");
    return;
  }
  
  token = String(authHeader.c_str()).substring(7);
  if (token != SECURE_UPDATE_TOKEN) {
    Serial.println("[ERROR] Invalid token");
    res->setStatusCode(403);
    res->setHeader("Content-Type", "text/html");
    res->print("Invalid token");
    return;
  }

  // Initialize SHA256
  const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_init(&shaCtx);
  mbedtls_md_setup(&shaCtx, info, 0);
  mbedtls_md_starts(&shaCtx);

  Serial.println("[INFO] Starting firmware update simulation");


  // Read body
  if (req->getContentLength() > MAX_FIRMWARE_SIZE) {
    mbedtls_md_free(&shaCtx);
    Serial.println("[ERROR] Firmware too large");
    res->setStatusCode(400);
    res->setHeader("Content-Type", "text/html");
    res->print("Firmware too large");
    return;
  }

  std::vector<uint8_t> body(req->getContentLength());
  req->readBytes(body.data(), body.size());
  std::string bodyStr(body.begin(), body.end());

  uploadedSize += body.size();

  // Update SHA256 hash
  mbedtls_md_update(&shaCtx, body.data(), body.size());

  // Simulate writing to flash (commented out)
  /*
  if (Update.write(data, len) != len) {
    updateRejected = true;
    Serial.println("[ERROR] Update.write failed");
    return;
  }
  */


  // Finalize SHA256
  byte resultHash[32];
  mbedtls_md_finish(&shaCtx, resultHash);
  mbedtls_md_free(&shaCtx);

  String actualHashHex;
  for (int i = 0; i < 32; ++i) {
    if (resultHash[i] < 0x10) actualHashHex += "0";
    actualHashHex += String(resultHash[i], HEX);
  }

  std::string clientHash = req->getHeader("X-Firmware-Hash");
  if (clientHash != actualHashHex.c_str()) {
    Serial.println("[ERROR] Hash mismatch");
    res->setStatusCode(400);
    res->setHeader("Content-Type", "text/html");
    res->print("Hash mismatch. Update rejected.");
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
  res->setStatusCode(200);
  res->setHeader("Content-Type", "text/html");
  res->print("Firmware validated and accepted. Rebooting...");
  delay(1000);
  //ESP.restart();
}


// V1.4 adds a vulnerable route to demonstrate CVE-2025-53094 (CRLF Injection)
// Function that mimics a remote event logger for the dashboard

// How its userd: GET /vuln?log=Fan%20turned%20on HTTP/1.1
// Host: {Host IP addr}

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

String base64Encode(const String &input) {
  size_t inputLen = input.length();
  size_t encodedLen = 4 * ((inputLen + 2) / 3); // Safe size calculation
  char encodedLog[encodedLen + 1];  // +1 for null-terminator

  Base64.encode(encodedLog, (char *)input.c_str(), inputLen);
  encodedLog[encodedLen] = '\0';

  return String(encodedLog);
}

void handleVulnServer(HTTPRequest *req, HTTPResponse *res) {
  ResourceParameters *params = req->getParams();
  std::string logValue;

  if (params != nullptr && params->getQueryParameter("log", logValue)) {
    // Reuse your sanitizer/encoder that expect Arduino String
    String logEntry = String(logValue.c_str());

    // Sanitize to remove CR/LF etc. (your function)
    String sanitizedLogEntry = sanitizeHeaderValue(logEntry);

    // Base64 encode to prevent header injection
    String headerValue = base64Encode(sanitizedLogEntry);

    res->setStatusCode(200);
    res->setHeader("X-Log-Entry", std::string(headerValue.c_str()));
    res->print("Log received OK.");
  } else {
    res->setStatusCode(400);
    res->setHeader("Content-Type", "text/html");
    res->print("Missing 'log' parameter.");
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

void handleStatus(HTTPRequest *req, HTTPResponse *res){
  String token = getCookie(req, "session");
  if (!isCurrentUserAdmin(token)) {
    res->setStatusCode(403);
    res->setHeader("Content-Type", "text/html");
    res->print("<h3>Access denied</h3>");
    return;
  }
  
  String status = "DeviceID: ESP32-001\nFirmware: v2.0\n";
  res->setStatusCode(200);
  res->setHeader("Content-Type", "text/html");
  res->print(status);
}


// Register the update route BEFORE auth is set up
void setupUnAuthenticatedRoutes() {
  // Included in Version 1.3, modified in V2.0
  secureServer.registerNode(new ResourceNode("/update", "POST", [](HTTPRequest *req, HTTPResponse *res) {
    handleFirmwareUpdate(req, res);
  }));

  secureServer.registerNode(new ResourceNode("/vuln", "GET", [](HTTPRequest *req, HTTPResponse *res){
    handleVulnServer(req, res);
  }));

  secureServer.registerNode(new ResourceNode("/status", "GET", [](HTTPRequest *req, HTTPResponse *res){
    handleStatus(req, res);
  }));
}
