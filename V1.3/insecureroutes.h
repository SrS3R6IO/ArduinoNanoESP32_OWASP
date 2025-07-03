#ifndef INSECUREROUTES_H
#define INSECUREROUTES_H

#include <WebServer.h>

// Functions what are going to be insecure
// For V1.3 the /update 
void setupUnAuthenticatedRoutes(WebServer &server);

#endif
