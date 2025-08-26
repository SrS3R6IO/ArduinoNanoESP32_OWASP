#include "wifi_setup.h"


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
    Serial.println("[WiFi] Connect to AP and go to https://192.168.4.1/wifi-setup");
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
    Serial.println("[WiFi] Connect to AP and go to https://192.168.4.1/wifi-setup");
  }
}

void setupWiFiRoutes() {
  secureServer.registerNode(new ResourceNode("/wifi-setup", "GET", [](HTTPRequest *req, HTTPResponse *res){
    res->setHeader("Content-Type", "text/html");
    res->println(R"rawliteral(
      <html><body>
      <h2>Wi-Fi Setup</h2>
      <form method="POST" action="/wifi-setup">
        SSID: <input name="ssid"><br>
        Password: <input name="password" type="password"><br>
        <input type="submit" value="Save & Reboot">
      </form>
      </body></html>
    )rawliteral");
  }));

  secureServer.registerNode(new ResourceNode("/wifi-setup", "POST", [](HTTPRequest *req, HTTPResponse *res){
    std::vector<uint8_t> body(req->getContentLength());
    req->readBytes(body.data(), body.size());
    std::string bodyStr(body.begin(), body.end());
    
    String ssid = getParam(bodyStr ,"ssid");
    String password = getParam(bodyStr ,"password");
    if (ssid.length() > 0 && password.length() > 0) {
      Preferences prefs;
      prefs.begin("wifi", false);
      prefs.putString("ssid", ssid);
      prefs.putString("password", password);
      prefs.end();
      res->setHeader("Content-Type", "text/html");
      res->println("<h2>Saved. Rebooting...</h2>");
      delay(2000);
      ESP.restart();
    } else {
      res->setStatusCode(400);
      res->println("<h2>Missing parameters</h2>");
    }
  }));


  secureServer.registerNode(new ResourceNode("/wifi-clear", "GET", [](HTTPRequest *req, HTTPResponse *res){
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    res->setHeader("Content-Type", "text/html");
    res->println("<h2>WiFi credentials cleared.</h2>");
  }));
}
