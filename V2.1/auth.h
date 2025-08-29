#ifndef AUTH_H
#define AUTH_H

#include <Arduino.h>

#include "security_utils.h"
#include "webpages.h"

extern bool isAuthenticated;

void setupAuthRoutes();

#endif
