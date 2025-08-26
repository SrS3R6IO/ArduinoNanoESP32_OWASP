#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "webpages.h"
#include "server.h"
#include "auth.h"

void setupDashboardRoutes();
extern bool isLampOn;
extern bool isMotorOn;
#endif
