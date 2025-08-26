// server_instance.h
#ifndef SERVER_INSTANCE_H
#define SERVER_INSTANCE_H

// ESPAsyncWebServer has version 3.7.8, vulnerable (CVE-2025-53094)
// #include <ESPAsyncWebServer.h>

// Modified server for HTTPS
#include <WiFi.h>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <string.h>


using namespace httpsserver;
// Declare server instance from main .ino
extern HTTPSServer secureServer;

#endif



