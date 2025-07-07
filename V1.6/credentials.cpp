#include "credentials.h"

bool isAuthenticated = false;

SensitiveData sensitiveLog;


// Default users
User defaultUsers[] = {
  {"pepe", "12345"},        // v1.1 weak password
  {"admin", "admin"},       // Default credentials
  {"anonymous", ""},
  {"root", "root"},
  {"user", "user"}
};

const int numUsers = sizeof(defaultUsers) / sizeof(defaultUsers[0]);


// Credentials
const char ssid[] = "Telia-2G-C63F61";
const char password[] = "NotMyWifIPASS";
const char login_user[] = "pepe";
const char login_pass[] = "12345";
