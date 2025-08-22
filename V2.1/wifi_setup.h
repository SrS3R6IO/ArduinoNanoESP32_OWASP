#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFi.h>
#include "server.h"
#include "security_utils.h"

void connectToWiFi();
void setupWiFiRoutes();
bool isWiFiConfigured();

#endif
