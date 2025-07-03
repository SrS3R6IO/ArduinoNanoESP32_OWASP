#include "auth.h"
#include "credentials.h"
#include "webpages.h"

// This functions verifies if the credentials are correct
void handleLogin(WebServer &server) {
  // We make sure we receive a POST request
  if (server.method() == HTTP_POST) {
    // And that it has user/password arguments.
    String user = server.arg("user");
    String pass = server.arg("pass");

    if (user == login_user && pass == login_pass) {
      isAuthenticated = true;
      server.send(200, "text/html", success_login_redirect_html);
    } else {
      server.send(401, "text/html", failed_login_html);
    }
  } else {
    server.send(200, "text/html", login_form_html);
  }
}


void setupAuthRoutes(WebServer &server) {
  server.on("/login", HTTP_ANY, [&server]() { handleLogin(server); });
}
