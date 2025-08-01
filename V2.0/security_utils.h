#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include <Arduino.h>
#include <Preferences.h>
#include "mbedtls/sha256.h"

#include "dashboard.h"


// encrypting TCP libraries
#include <WiFi.h>
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "esp_wifi.h"
#include "cert.h"
#include "key.h"


String hashPassword(const String& password);
bool isCurrentUserAdmin();
bool isPasswordSet();
bool storeHashedPassword(const String& password);
bool checkPassword(const String& inputPassword);
void clearStoredPassword();
String generateRandomSalt(size_t length = 16);
String sha256(const String& password, const String& salt);


bool userExists(const String& username);
bool createUser(const String& username, const String& password, const String& role);
bool verifyUserPassword(const String& username, const String& password);
String getUserRole(const String& username);


// TCP
void storeTCPAuthToken(const String& token);
String loadTCPAuthToken();
void secureTCPServiceTask(void *parameter);

extern Preferences preferences;

#endif
