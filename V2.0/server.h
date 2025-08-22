// server_instance.h
#ifndef SERVER_INSTANCE_H
#define SERVER_INSTANCE_H

// ESPAsyncWebServer has version 3.7.8, vulnerable (CVE-2025-53094)
#include <ESPAsyncWebServer.h>

// Declare server instance from main .ino
extern AsyncWebServer server;

#endif