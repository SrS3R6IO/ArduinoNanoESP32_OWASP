#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

// File for credentials (should be classified information) 
// and the simulated hardware variables 

extern bool isAuthenticated;
extern bool isLampOn;
extern bool isMotorOn;
extern int temperature;

// Hardcoded credentials
extern const char ssid[];
extern const char password[];
extern const char login_user[];
extern const char login_pass[];

#endif
