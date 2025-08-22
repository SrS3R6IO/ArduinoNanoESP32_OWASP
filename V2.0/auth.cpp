
#include "auth.h"
#include "security_utils.h"
#include "webpages.h"
#include "server.h"


bool isAuthenticated = false;
String currentUser = "";


// Session TimeOut
unsigned long authTimestamp = 0;
const unsigned long SESSION_TIMEOUT = 300000; // 5 minutes
unsigned long lastActivity = 0;


bool isSessionValid() {
  return isAuthenticated && (millis() - lastActivity < SESSION_TIMEOUT);
}


bool isCurrentUserAdmin() {
  return isSessionValid() && getUserRole(currentUser) == "admin";
}




// Rate limiting login attempts
unsigned long lastFailedAttempt = 0;
int failedAttempts = 0;
const unsigned long BLOCK_DURATION = 30000; // 30 seconds
const int MAX_ATTEMPTS = 5;



void handleLogin(AsyncWebServerRequest *request) {
  if (!isPasswordSet()) {
    request->redirect("/setup");
    return;
  }


  if (request->method() == HTTP_POST &&
      request->hasParam("user", true) &&
      request->hasParam("pass", true)) {
      
      String user = request->getParam("user", true)->value();
      String pass = request->getParam("pass", true)->value();


      if (millis() - lastFailedAttempt < BLOCK_DURATION && failedAttempts >= MAX_ATTEMPTS) {
        request->send(429, "text/plain", "Too many attempts. Try again later.");
        return;
      }
      if (verifyUserPassword(user, pass)) {
        isAuthenticated = true;
        currentUser = user;
        failedAttempts = 0;
        lastActivity = millis();

        request->send(200, "text/html", success_login_redirect_html);
      } else{
        failedAttempts++;
        lastFailedAttempt = millis();
        request->send(401, "text/html", failed_login_html);
      }
  } else {
    request->send(200, "text/html", login_form_html);
  }
}

// Function for handling the user list (for the admin management of users)
// Adds a username to the persistent user list
void addToUserList(const String& username) {
  preferences.begin("users", false);
  String list = preferences.getString("user_list", "");
  if (list.indexOf(username) < 0) {
    if (list.length() > 0) {
      list += "," + username;
    } else {
      list = username;
    }
    preferences.putString("user_list", list);
  }
  preferences.end();
}

// Removes a user from the persistent user list
void removeFromUserList(const String& username) {
  preferences.begin("users", false);
  String list = preferences.getString("user_list", "");
  String updatedList = "";

  int start = 0;
  while (start >= 0) {
    int comma = list.indexOf(',', start);
    String user = comma == -1 ? list.substring(start) : list.substring(start, comma);
    if (user != username && user.length() > 0) {
      if (updatedList.length() > 0) {
        updatedList += ",";
      }
      updatedList += user;
    }
    if (comma == -1) break;
    start = comma + 1;
  }

  preferences.putString("user_list", updatedList);
  preferences.end();
}

// Returns all users as a vector of strings
std::vector<String> getAllUsers() {
  std::vector<String> users;
  preferences.begin("users", true);
  String list = preferences.getString("user_list", "");
  preferences.end();

  int start = 0;
  while (start >= 0) {
    int comma = list.indexOf(',', start);
    String user = comma == -1 ? list.substring(start) : list.substring(start, comma);
    if (user.length() > 0) {
      users.push_back(user);
    }
    if (comma == -1) break;
    start = comma + 1;
  }
  return users;
}

String getUserRole(const String& username) {
  preferences.begin("users", true);
  String role = preferences.getString((username + "/role").c_str(), "user");
  preferences.end();
  return role;
}



// Maybe for later
void deleteUser(const String& username) {
  preferences.begin("users", false);
  preferences.remove((username + "/hash").c_str());
  preferences.remove((username + "/salt").c_str());
  preferences.remove((username + "/role").c_str());
  preferences.end();

  removeFromUserList(username);
}


void handlePasswordSetup(AsyncWebServerRequest *request) {
  if (isPasswordSet()) {
    request->redirect("/login");
    return;
  }

  if (request->method() == HTTP_POST &&
      request->hasParam("username", true) &&
      request->hasParam("password", true)) {

    String username = request->getParam("username", true)->value();
    String password = request->getParam("password", true)->value();

    if (username.length() == 0 || password.length() == 0) {
      request->send(400, "text/html", "Username and password cannot be empty.");
      return;
    }

    // Password policy enforcement
    const char* pw = password.c_str();
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    const String specialChars = "!@#$%^&*()_+-=";

    for (int i = 0; i < password.length(); i++) {
      if (isUpperCase(pw[i])) hasUpper = true;
      else if (isLowerCase(pw[i])) hasLower = true;
      else if (isDigit(pw[i])) hasDigit = true;
      else if (specialChars.indexOf(pw[i]) >= 0) hasSpecial = true;
    }

    if (password.length() < 8 || !hasUpper || !hasLower || !hasDigit || !hasSpecial) {
      request->send(400, "text/html",
        "Password must be at least 8 characters long and include:<br>"
        "- One uppercase letter<br>"
        "- One lowercase letter<br>"
        "- One digit<br>"
        "- One special character (!@#$%^&*()_+-=)<br><br>"
        "<a href='/'>Go back</a>"
      );
      return;
    }

    // Generate salt
    String salt = generateRandomSalt();
    String hashedPassword = sha256(password, salt);

    preferences.begin("users", false);
    preferences.putString((username + "/hash").c_str(), hashedPassword);
    preferences.putString((username + "/salt").c_str(), salt);
    preferences.putString((username + "/role").c_str(), "admin");
    addToUserList(username);
    preferences.end();

    request->redirect("/login");

  } else {
    // Setup form (GET)
    request->send(200, "text/html", setup_form_html);
  }
}


void handleAdminPage(AsyncWebServerRequest *request) {
  if (!isPasswordSet()) {
    request->redirect("/setup");
    return;
  }
  
  if (!isCurrentUserAdmin()) {
    request->send(403, "text/html", "<h3>Access denied</h3>");
    return;
  }

  String html = R"rawliteral(
    <html><body>
    <h2>Admin Panel</h2>
    <form method="POST" action="/register">
      Username: <input name="newuser" type="text"><br>
      Password: <input name="pass" type="password"><br>
      <input type="submit" value="Register User">
    </form>

    <button onclick="clearNVS()">Factory Reset Device</button>

    <script>
    function clearNVS() {
      if (confirm("Are you sure you want to erase all saved data and reboot the device?")) {
        fetch("/clear-nvs")
          .then(response => response.text())
          .then(msg => alert(msg))
          .catch(err => alert("Failed: " + err));
      }
    }
    </script>

    </body></html>
  )rawliteral";

  std::vector<String> users = getAllUsers();
  html += "<h3>Registered Users</h3><ul>";
  for (const auto& user : users) {
    html += "<li>" + user + " (" + getUserRole(user) + ")</li>";
  }
  html += "</ul>";


  request->send(200, "text/html", html);
}


void handleUserRegistration(AsyncWebServerRequest *request) {
  if (!isPasswordSet()) {
    request->redirect("/setup");
    return;
  }

  if (!isCurrentUserAdmin()) {
    request->send(403, "text/html", "Unauthorized");
    return;
  }

  if (request->hasParam("newuser", true) && request->hasParam("pass", true)) {
    String newUser = request->getParam("newuser", true)->value();
    String newPass = request->getParam("pass", true)->value();
    
    // Verify secure credentials
    if (newUser.length() == 0 || newPass.length() == 0) {
      request->send(400, "text/html", "Username and password cannot be empty.");
      return;
    }

    // Password policy enforcement
    const char* pw = newPass.c_str();
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    const String specialChars = "!@#$%^&*()_+-=";

    for (int i = 0; i < newPass.length(); i++) {
      if (isUpperCase(pw[i])) hasUpper = true;
      else if (isLowerCase(pw[i])) hasLower = true;
      else if (isDigit(pw[i])) hasDigit = true;
      else if (specialChars.indexOf(pw[i]) >= 0) hasSpecial = true;
    }

    if (newPass.length() < 8 || !hasUpper || !hasLower || !hasDigit || !hasSpecial) {
      request->send(400, "text/html",
        "Password must be at least 8 characters long and include:<br>"
        "- One uppercase letter<br>"
        "- One lowercase letter<br>"
        "- One digit<br>"
        "- One special character (!@#$%^&*()_+-=)<br><br>"
        "<a href='/'>Go back</a>"
      );
      return;
    }

    bool created = createUser(newUser, newPass, "user");
    addToUserList(newUser);

    if (created) {
      request->send(200, "text/html", "<h3>User registered!</h3>");
    } else {
      request->send(409, "text/html", "<h3>User already exists</h3>");
    }
  } else {
    request->send(400, "text/html", "Missing data");
  }
}


void handleLogout(AsyncWebServerRequest *request) {
  isAuthenticated = false;
  currentUser = "";
  request->redirect("/login");
}



void redirectIfNotSetup(AsyncWebServerRequest *request) {
  if (!isPasswordSet()) {
    request->redirect("/setup");
  } else {
    request->redirect("/login");
  }
}

void setupAuthRoutes() {
  server.on("/", HTTP_GET, redirectIfNotSetup);

  if (!isPasswordSet()) {
    server.on("/setup", HTTP_ANY, handlePasswordSetup);
  }

  server.on("/login", HTTP_ANY, handleLogin);
  server.on("/admin", HTTP_GET, handleAdminPage);
  server.on("/register", HTTP_POST, handleUserRegistration);
  server.on("/logout", HTTP_GET, handleLogout);

  server.on("/clear-nvs", HTTP_GET, [](AsyncWebServerRequest *request){
    if (isCurrentUserAdmin()){
      clearNVSStorage();           
      request->send(200, "text/plain", "NVS cleared. Rebooting...");
      delay(1000);
      ESP.restart();
    }
    else{
      request->redirect("/login");
    }
  });


  // Add a not found in case of not found endpoints
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (!isPasswordSet()) {
      request->redirect("/setup");
    } else {
      request->redirect("/login");
    }
  });

}
