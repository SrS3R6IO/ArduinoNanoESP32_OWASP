#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <Arduino.h>

// File for credentials (should be classified information) 
extern bool isAuthenticated;

// New structure to store sensitive information (V1.5)
struct SensitiveData {
  String lastUser = "";
  String lastPassword = "";
  String lastTCPCommand = "";
};
extern SensitiveData sensitiveLog;

// Included in V1.8
// Structure for creating the default users
struct User {
  const char* username;
  const char* password;
};

extern User defaultUsers[];
// This will be used for printing all the users
extern const int numUsers; 



// Hardcoded credentials
extern const char ssid[];
extern const char password[];
extern const char login_user[];
extern const char login_pass[];

#endif
