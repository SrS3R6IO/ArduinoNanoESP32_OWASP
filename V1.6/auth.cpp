#include "auth.h"
#include "credentials.h"
#include "webpages.h"
#include "server.h"


// This functions verifies if the credentials are correct
void handleLogin(AsyncWebServerRequest *request) {
  // We make sure we receive a POST request
  if (request->method() == HTTP_POST) {
    // And that it has user/password arguments.
    if (request->hasParam("user", true) && request->hasParam("pass", true)){
      String user = request->getParam("user", true)->value();
      String pass = request->getParam("pass", true)->value();
      bool loginSuccess = false;

      for (int i = 0; i < numUsers; i++) {
        if (user == defaultUsers[i].username && pass == defaultUsers[i].password) {
          loginSuccess = true;
          break;
        }
      }
      if (loginSuccess) {
        isAuthenticated = true;
        // Store the information for the dump
        sensitiveLog.lastUser = user;
        sensitiveLog.lastPassword = pass;
        request->send(200, "text/html", success_login_redirect_html);
      } else {
        request->send(401, "text/html", failed_login_html);
      }
    } else {
      // Missing form data (either user or password)
      request->send(400, "text/html", failed_login_html);
    }
  } else {
    request->send(200, "text/html", login_form_html);
  }
}


void setupAuthRoutes() {
  server.on("/login", HTTP_ANY, handleLogin);
}
