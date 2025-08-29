#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include <Arduino.h>
#include <Preferences.h>
#include <string>
#include <vector>
#include <map>

#include "server.h"
#include "dashboard.h"

// encrypting TCP libraries
#include <WiFi.h>
#include "mbedtls/sha256.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "esp_wifi.h"
#include "cert.h"
#include "key.h"


struct Session {
  String username;
  unsigned long lastActivity;
};
struct LoginAttempt {
  int failedAttempts = 0;
  unsigned long lastAttempt = 0;
};


String hashPassword(const String& password);
bool isPasswordSet();
bool isCurrentUserAdmin(const String &token);
bool isSessionValid(const String &token);
String generateSession(const String &username);

bool storeHashedPassword(const String& password);
void clearStoredPassword();
String generateRandomSalt(size_t length = 16);
String sha256(const String& password, const String& salt);


bool userExists(const String& username);
bool createUser(const String& username, const String& password, const String& role);
bool verifyUserPassword(const String& username, const String& password);
std::vector<String> parseCSV(const String &list);
std::vector<String> getAllUsers();
String getUserRole(const String& username);
std::string readRequestBody(HTTPRequest *req);
String getParam(const std::string &body, const char *key);
String getCookie(HTTPRequest *req, const char *cookieName);


// TCP
void storeTCPAuthToken(const String& token);
String loadTCPAuthToken();
void secureTCPServiceTask(void *parameter);



void clearNVSStorage();

extern Preferences preferences;
extern std::map<String, Session> sessions;
extern std::map<String, LoginAttempt> loginAttempts;
#endif
