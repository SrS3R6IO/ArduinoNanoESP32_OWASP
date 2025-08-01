#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include <Arduino.h>
#include <Preferences.h>
#include "mbedtls/sha256.h"

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


extern Preferences preferences;

#endif
