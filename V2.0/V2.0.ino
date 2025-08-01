// Substitue for WebServer in version 1.4
#include "server.h"

#include "auth.h"
#include "dashboard.h"

// Added in V1.4
#include "insecureroutes.h"


// Added in V2.0 secure version
#include "wifi_setup.h"
#include "security_utils.h"



// Run our HTTP server
AsyncWebServer server(80);

// Addition of V1.2, insecure TCP service
WiFiServer service(1337); 
WiFiClient client;

void setup() {
  // We will use the bandwith 115200 for local debugging
  Serial.begin(115200);

  // Necessary delay for things to work
  delay(1000);
  Serial.println("Booting...");
  
  // Wifi Setting
  connectToWiFi();

  // Start server
  server.begin();

  // Add routes
  setupWiFiRoutes();
  setupAuthRoutes();

  // Start the server with the login page
  setupAuthRoutes();
  // And prepare the other endpoints (non accesible if ot authenticated)
  setupDashboardRoutes();

  // Add the unauthenticated routes, included on V1.3 (new routes on V1.4)
  setupUnAuthenticatedRoutes();


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

}


