
#include "wifi_setup.h"
#include "security_utils.h"


bool isWiFiConfigured() {
  preferences.begin("wifi", true);
  bool configured = preferences.isKey("ssid") && preferences.isKey("password");
  preferences.end();
  return configured;
}

void connectToWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();

  if (ssid.isEmpty() || password.isEmpty()) {
    Serial.println("[WiFi] No credentials stored. Starting AP mode...");
    WiFi.softAP("ESP32_Setup", "config123");
    Serial.println("[WiFi] Connect to AP and go to http://192.168.4.1/wifi-setup");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("[WiFi] Connecting");
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Connected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("[WiFi] Connection failed. Starting AP mode...");
    WiFi.softAP("ESP32_Setup", "config123");
    Serial.println("[WiFi] Connect to AP and go to http://192.168.4.1/wifi-setup");
  }
}

void setupWiFiRoutes() {
  server.on("/wifi-setup", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"rawliteral(
      <html><body>
      <h2>Wi-Fi Setup</h2>
      <form action="/wifi-setup" method="POST">
        SSID: <input name="ssid"><br>
        Password: <input name="password" type="password"><br>
        <input type="submit" value="Save & Reboot">
      </form>
      </body></html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/wifi-setup", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();

      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      preferences.end();

      request->send(200, "text/html", "<h2>Saved. Rebooting...</h2>");
      delay(2000);
      ESP.restart();
    } else {
      request->send(400, "text/html", "<h2>Missing parameters</h2>");
    }
  });

  server.on("/wifi-clear", HTTP_GET, [](AsyncWebServerRequest *request) {
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    request->send(200, "text/html", "<h2>WiFi credentials cleared. Rebooting...</h2>");
    delay(2000);
    ESP.restart();
  });
}



