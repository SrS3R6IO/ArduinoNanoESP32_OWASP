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


// Hardcoded credentials
extern const char ssid[];
extern const char password[];
extern const char login_user[];
extern const char login_pass[];

#endif
