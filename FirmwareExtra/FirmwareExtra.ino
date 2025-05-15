#include <WiFi.h>

const char* ssid = "Telia-2G-C63F61"; // Igual que el original
const char* password = "NotMyWifIPASS"; 

WiFiServer backdoorServer(4444);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  backdoorServer.begin();
  Serial.println("Backdoor service listening on port 4444");

  pinMode(2, OUTPUT);
}

void loop() {
  // LED parpadeando rápido
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  delay(100);

  WiFiClient client = backdoorServer.available();
  if (client) {
    Serial.println("Incoming connection to backdoor...");
    client.println("Hello from malicious firmware!");
    client.stop();
  }
}
