#include "auth.h"
#include "security_utils.h"
#include "webpages.h"
#include "server.h"

#include <string>
#include <vector>


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



void handleLogin(HTTPRequest *req, HTTPResponse *res) {
  std::string method = req->getMethod();
  Serial.printf("[Login] Method=%s, BodyLen=%d\n", method.c_str(), req->getContentLength());

  if (!isPasswordSet()) {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/setup");
    res->println("");
    return;
  }


  // std::string method = req->getMethod();
  if (method == "POST") {
    
    // Read body (more complicated way for HTTPS)
    std::vector<uint8_t> body(req->getContentLength());
    req->readBytes(body.data(), body.size());
    std::string bodyStr(body.begin(), body.end());

    String user = "";
    String pass = "";

    size_t userPos = bodyStr.find("user=");
    if (userPos != std::string::npos) {
        size_t amp = bodyStr.find("&", userPos);
        std::string u = bodyStr.substr(userPos + 5,
            (amp == std::string::npos ? bodyStr.length() : amp) - (userPos + 5));
        user = String(u.c_str()); // convert std::string → Arduino String
    }

    size_t passPos = bodyStr.find("pass=");
    if (passPos != std::string::npos) {
        size_t amp = bodyStr.find("&", passPos);
        std::string u = bodyStr.substr(passPos + 5,
            (amp == std::string::npos ? bodyStr.length() : amp) - (passPos + 5));
        pass = String(u.c_str()); // convert std::string → Arduino String
    }

    // Validate
    if (user.length() == 0 || pass.length() == 0) {
        res->setStatusCode(400);
        res->println("Missing username or password");
        return;
    }


    if (millis() - lastFailedAttempt < BLOCK_DURATION && failedAttempts >= MAX_ATTEMPTS) {
      res->setStatusCode(429);
      res->setHeader("Content-Type", "text/plain");
      res->println("Too many attempts. Try again later.");
      return;
    }
    if (verifyUserPassword(user, pass)) {
      isAuthenticated = true;
      currentUser = user;
      failedAttempts = 0;
      lastActivity = millis();

      res->setStatusCode(200);
      res->setHeader("Content-Type", "text/html");
      res->println(success_login_redirect_html);
    } else{
      failedAttempts++;
      lastFailedAttempt = millis();
      res->setStatusCode(401);
      res->setHeader("Content-Type", "text/html");
      res->println(failed_login_html);
    }
  } else {
    req->discardRequestBody();
    res->setStatusCode(200);
    res->setHeader("Content-Type", "text/html");
    res->println(login_form_html);
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


void handlePasswordSetup(HTTPRequest *req, HTTPResponse *res) {
  if (isPasswordSet()) {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/setup");
    return;
  }

  std::string method = req->getMethod();
  if (method == "POST"){
    // Read body (more complicated way for HTTPS)
    std::string body;
    char buf[256];
    size_t readLen;
    while ((readLen = req->readChars(buf, sizeof(buf))) > 0) {
        body.append(buf, readLen);
    }

    // Convert to Arduino String for parsing
    String bodyStr = String(body.c_str());
    String username = "";
    String password = "";

    int userPos = bodyStr.indexOf("user=");
    if (userPos != -1) {
        int amp = bodyStr.indexOf("&", userPos);
        username = bodyStr.substring(userPos + 5, amp == -1 ? bodyStr.length() : amp);
    }
    int passPos = bodyStr.indexOf("pass=");
    if (passPos != -1) {
        int amp = bodyStr.indexOf("&", passPos);
        password = bodyStr.substring(passPos + 5, amp == -1 ? bodyStr.length() : amp);
    }

    if (username.length() == 0 || password.length() == 0) {
      res->setStatusCode(400);
      res->setHeader("Content-Type", "text/html");
      res->print("Username and password cannot be empty.");
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
      res->setStatusCode(400);
      res->setHeader("Content-Type", "text/html");
      res->print(
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

    res->setStatusCode(302);
    res->setHeader("Location", "/login");
    return;
  } else {
    // Setup form (GET)
    res->setStatusCode(200);
    res->setHeader("Content-Type", "text/html");
    res->print(setup_form_html);
  }
}


void handleAdminPage(HTTPRequest *req, HTTPResponse *res) {
  if (!isPasswordSet()) {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/setup");
    return;
  }
  
  if (!isCurrentUserAdmin()) {
    res->setStatusCode(200);
    res->setHeader("Content-Type", "text/html");
    res->print("<h3>Access denied</h3>");
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


  res->setStatusCode(200);
  res->setHeader("Content-Type", "text/html");
  res->print(html);
}


void handleUserRegistration(HTTPRequest *req, HTTPResponse *res) {
  if (!isPasswordSet()) {
    res->setStatusCode(302);
    res->setHeader("Location", "/setup");
    return;
  }

  if (!isCurrentUserAdmin()) {
    res->setStatusCode(403);
    res->setHeader("Content-Type", "text/html");
    res->print("Unauthorized");
    return;
  }
  std::string method = req->getMethod();
  if (method == "POST") {
    std::vector<uint8_t> body(req->getContentLength());
    req->readBytes(body.data(), body.size());
    std::string bodyStr(body.begin(), body.end());

    // Get the user and password from the request
    std::string userParam = getParamFromBody(bodyStr ,"user");
    std::string passParam = getParamFromBody(bodyStr ,"pass");
    
    if (userParam.empty() || passParam.empty()) {
      res->setStatusCode(400);
      res->setHeader("Content-Type", "text/html");
      res->print("Missing username or password");
      return;
    }

    std::string newUserParam = getParamFromBody(bodyStr ,"newuser");
    std::string newPassParam = getParamFromBody(bodyStr ,"pass");

    String newUser = String(newUserParam.c_str());
    String newPass = String(newPassParam.c_str());
      
    // Verify secure credentials
    if (newUser.length() == 0 || newPass.length() == 0) {
      res->setStatusCode(400);
      res->setHeader("Content-Type", "text/html");
      res->print("Username and password cannot be empty.");
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
      res->setStatusCode(400);
      res->setHeader("Content-Type", "text/html");
      res->print(
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

    res->setHeader("Content-Type", "text/html");
    if (created) {
      res->setStatusCode(200);
      res->print("<h3>User registered!</h3>");
    } else {
      res->setStatusCode(409);
      res->print("<h3>User already exists</h3>");
    }
  } else {
    res->setStatusCode(200);
    res->setHeader("Location", "/login");
  }
}


void handleLogout(HTTPRequest *req, HTTPResponse *res) {
  isAuthenticated = false;
  currentUser = "";
  res->setStatusCode(302); // HTTP redirect
  res->setHeader("Location", "/login");
}



void redirectIfNotSetup(HTTPRequest *req, HTTPResponse *res) {
  if (!isPasswordSet()) {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/setup");
  } else {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/login");
  }
}

void setupAuthRoutes() {

  // Root redirect
  secureServer.registerNode(new ResourceNode("/", "GET", [](HTTPRequest *req, HTTPResponse *res){
    redirectIfNotSetup(req, res);
  }));

  if (!isPasswordSet()) {
    secureServer.registerNode(new ResourceNode("/setup", "GET", [](HTTPRequest *req, HTTPResponse *res){
      handlePasswordSetup(req, res);
    }));

    secureServer.registerNode(new ResourceNode("/setup", "POST", [](HTTPRequest *req, HTTPResponse *res){
      handlePasswordSetup(req, res);
    }));
  }
  
  secureServer.registerNode(new ResourceNode("/login", "GET", [](HTTPRequest *req, HTTPResponse *res){
    handleLogin(req, res);
  }));

  secureServer.registerNode(new ResourceNode("/login", "POST", [](HTTPRequest *req, HTTPResponse *res){
      handleLogin(req, res);
  }));

  

  secureServer.registerNode(new ResourceNode("/admin", "GET", [](HTTPRequest *req, HTTPResponse *res){
    handleAdminPage(req, res);
  }));

  secureServer.registerNode(new ResourceNode("/register", "POST", [](HTTPRequest *req, HTTPResponse *res){
    handleUserRegistration(req, res);
  }));

  secureServer.registerNode(new ResourceNode("/logout", "GET", [](HTTPRequest *req, HTTPResponse *res){
    handleLogout(req, res);
  }));

  secureServer.registerNode(new ResourceNode("/clear-nvs", "GET", [](HTTPRequest *req, HTTPResponse *res){
    if (isCurrentUserAdmin()) {
      clearNVSStorage();
      res->setStatusCode(200);
      res->print("NVS cleared. Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      res->setStatusCode(302);
      res->setHeader("Location", "/login");
    }
  }));


  // Add a not found in case of not found endpoints
  secureServer.setDefaultNode(new ResourceNode("", "", [](HTTPRequest *req, HTTPResponse *res){
    req->discardRequestBody();
    res->setStatusCode(302);
    if (!isPasswordSet()) {
      res->setHeader("Location", "/setup");
    } else {
      res->setHeader("Location", "/login");
    }
    res->println("");  // <-- flush response
  }));

}

