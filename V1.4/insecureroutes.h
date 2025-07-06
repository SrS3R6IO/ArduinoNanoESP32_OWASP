#ifndef INSECUREROUTES_H
#define INSECUREROUTES_H

#include <Arduino.h>

// Functions what are going to be insecure
// For V1.3 the /update, in V1.4 the /vuln
void setupUnAuthenticatedRoutes();

#endif
