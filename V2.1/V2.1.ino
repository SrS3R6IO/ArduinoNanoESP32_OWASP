// Substitue for WebServer in version 1.4
#include "server.h"

#include "auth.h"
#include "dashboard.h"

// Added in V1.4
#include "insecureroutes.h"


// Added in V2.0 secure version
#include "wifi_setup.h"
#include "security_utils.h"


using namespace httpsserver;
// Create SSLCert using DER arrays
SSLCert cert(cert_der, cert_der_len, key_der, key_der_len);
// Run HTTPS Server
HTTPSServer secureServer(&cert, 443);

// Addition of V1.2, insecure TCP service
WiFiServer service(1337); 
WiFiClient client;

void setup() {
  // We will use the bandwith 115200 for local debugging
  Serial.begin(115200);

  // Necessary delay for things to work
  delay(1000);
  Serial.println("Booting...");

  if (preferences.begin("wifi", true)) {
    Serial.println("Stored keys in 'wifi' namespace:");

    // Example: if you know some keys
    if (preferences.isKey("ssid")) {
      Serial.print("SSID: ");
      Serial.println(preferences.getString("ssid", ""));
    }
    if (preferences.isKey("password")) {
      Serial.print("Password: ");
      Serial.println(preferences.getString("password", ""));
    }
  preferences.end();
  } else{
    Serial.println("NO WIFI PREFFERENCES");
  }
  
  // Wifi Setting
  connectToWiFi();

  // Add routes
  setupWiFiRoutes();

  // Start the server with the login page
  setupAuthRoutes();
  // And prepare the other endpoints (non accesible if ot authenticated)
  setupDashboardRoutes();

  // Add the unauthenticated routes, included on V1.3 (new routes on V1.4)
  setupUnAuthenticatedRoutes();

  Serial.printf("Cert len: %u, Key len: %u\n", cert_der_len, key_der_len);
  for (int i=0;i<4;i++) Serial.printf("%02X ", cert_der[i]); Serial.println();
  for (int i=0;i<4;i++) Serial.printf("%02X ", key_der[i]); Serial.println();

  delay(500);
  secureServer.start();

  // Load the TCP authenthication token
  // Modify later to an interface after login in for a more secure implementation
  if (loadTCPAuthToken() == "") {
    storeTCPAuthToken("s3cur3t0k3n");  // Default on first boot only
  }
  
  // TCP backdoor
  xTaskCreatePinnedToCore(
    secureTCPServiceTask,   // Function
    "SecureTCPTask",        // Name
    8192,                   // Stack size
    (void *)&service,       // Param (pass by pointer)
    1,                      // Priority
    NULL,                   // Optional task handle
    0                       // Run on Core 0
  );

  
}

void loop() {
  secureServer.loop();
}


