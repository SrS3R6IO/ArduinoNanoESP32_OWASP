#ifndef AUTH_H
#define AUTH_H

#include <Arduino.h>
#include <map>

#include "security_utils.h"
#include "webpages.h"

extern bool isAuthenticated;

void setupAuthRoutes();
bool isSessionValid(const String &token);
#endif
