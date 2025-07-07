#ifndef INSECUREROUTES_H
#define INSECUREROUTES_H

#include <Arduino.h>
#include <MD5Builder.h>
#include "server.h"
#include "credentials.h"


// Functions what are going to be insecure
// For V1.3 the /update, in V1.4 the /vuln, in V1.5 the /dump and /status
void setupUnAuthenticatedRoutes();

#endif
