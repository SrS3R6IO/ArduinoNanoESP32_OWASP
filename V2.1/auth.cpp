#include "auth.h"



const unsigned long BLOCK_DURATION = 30000;
const int MAX_ATTEMPTS = 5;

// User Management

// Function for handling the user list (for the admin management of users)
// Adds a username to the persistent user list
void addToUserList(const String &username) {
  preferences.begin("users", false);
  String list = preferences.getString("user_list", "");
  if (list.indexOf(username) < 0) {
    list += (list.length() > 0 ? "," : "") + username;
    preferences.putString("user_list", list);
  }
  preferences.end();
}

// Removes a user from the persistent user list
void removeFromUserList(const String &username) {
  preferences.begin("users", false);
  String list = preferences.getString("user_list", "");
  String updatedList;
  for (auto &u : parseCSV(list)) {
    if (u != username) updatedList += (updatedList.length() ? "," : "") + u;
  }
  preferences.putString("user_list", updatedList);
  preferences.end();
}




void deleteUser(const String &username) {
  preferences.begin("users", false);
  preferences.remove((username + "/hash").c_str());
  preferences.remove((username + "/salt").c_str());
  preferences.remove((username + "/role").c_str());
  preferences.end();
  removeFromUserList(username);
}



bool isPasswordStrong(String &password) {
  if (password.length() < 8) return false;

  bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
  const String specialChars = "!@#$%^&*()_+-=";

  for (int i = 0; i < password.length(); i++) {
    char c = password[i];
    if (isUpperCase(c)) hasUpper = true;
    else if (isLowerCase(c)) hasLower = true;
    else if (isDigit(c)) hasDigit = true;
    else if (specialChars.indexOf(c) >= 0) hasSpecial = true;
  }
  return hasUpper && hasLower && hasDigit && hasSpecial;
}



void handleLogin(HTTPRequest *req, HTTPResponse *res) {
  std::string method = req->getMethod();
  Serial.printf("[Login] Method=%s, BodyLen=%d\n", method.c_str(), req->getContentLength());

  if (!isPasswordSet()) {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/setup");
    res->println("");
    return;
  }

  if (method == "POST") {
    std::string bodyStr = readRequestBody(req);
    String user = getParam(bodyStr, "user");
    String pass = getParam(bodyStr, "pass");

    if (user.length() == 0 || pass.length() == 0) {
      res->setStatusCode(400); 
      res->println("Missing username or password"); 
      return;
    }

    LoginAttempt &attempt = loginAttempts[user];
    if (millis() - attempt.lastAttempt < BLOCK_DURATION && attempt.failedAttempts >= MAX_ATTEMPTS) {
      res->setStatusCode(429); 
      res->println("Too many attempts. Try again later."); 
      return;
    }

    if (verifyUserPassword(user, pass)) {
      String token = generateSession(user);
      res->setStatusCode(200);

      std::string cookieHeader = ("session=" + token + "; HttpOnly; Secure").c_str();
      res->setHeader("Set-Cookie", cookieHeader);

      res->println(success_login_redirect_html);
      attempt.failedAttempts = 0;
    } else {
      attempt.failedAttempts++; 
      attempt.lastAttempt = millis();
      res->setStatusCode(401); 
      res->println(failed_login_html);
    }
  } else {
    req->discardRequestBody();
    res->setStatusCode(200); 
    res->println(login_form_html);
  }
}

void handleLogout(HTTPRequest *req, HTTPResponse *res) {
  String token = getCookie(req, "session");
  if (sessions.count(token)){
    sessions.erase(token);
  }
  res->setStatusCode(302); 
  res->setHeader("Location", "/login");
}





// Admin & Registration Management


void handleUserRegistration(HTTPRequest *req, HTTPResponse *res) {
  if (!isPasswordSet()) {
    res->setStatusCode(302);
    res->setHeader("Location", "/setup");
    return;
  }

  String token = getCookie(req, "session");
  if (!isCurrentUserAdmin(token)) { 
    res->setStatusCode(403); 
    res->setHeader("Content-Type", "text/html");
    res->println("Unauthorized"); 
    return; 
  }

  if (req->getMethod() != "POST") { 
    res->setStatusCode(405); 
    return; 
  }
  std::string bodyStr = readRequestBody(req);
  String newUser = getParam(bodyStr, "newuser");
  String newPass = getParam(bodyStr, "pass");
    
  // Verify secure credentials
  if (newUser.length() == 0 || newPass.length() == 0) {
    res->setStatusCode(400);
    res->setHeader("Content-Type", "text/html");
    res->print("Username and password cannot be empty."
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
    if (!isPasswordStrong(password)) {
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
  String token = getCookie(req, "session");
  if (!isPasswordSet()) { 
    res->setStatusCode(302); 
    res->setHeader("Location", "/setup"); 
    return;
  }

  if (!isCurrentUserAdmin(token)) {
    res->setStatusCode(403);
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
    html += "<a href='/delete?user=" + user + "'>Delete</a></li>";
  }
  html += "</ul>";


  res->setStatusCode(200);
  res->setHeader("Content-Type", "text/html");
  res->print(html);
}


void handleDeleteUser(HTTPRequest *req, HTTPResponse *res) {
  if (!isPasswordSet()) {
    res->setStatusCode(302); // HTTP redirect
    res->setHeader("Location", "/setup");
    return;
  }

  String token = getCookie(req, "session");
  if (!isCurrentUserAdmin(token)) {
    res->setStatusCode(403);
    res->println("Unauthorized");
    return;
  }

  std::string fullReq = req->getRequestString(); 
  String full = String(fullReq.c_str());
  int qPos = full.indexOf("?");
  int spPos = full.indexOf(" ", qPos); 

  String usernameToDelete = "";
  if (qPos != -1 && spPos != -1) {
    String query = full.substring(qPos + 1, spPos); 
    int eqPos = query.indexOf("=");
    if (eqPos != -1 && query.substring(0, eqPos) == "user") {
      usernameToDelete = query.substring(eqPos + 1);
    }
  }

  if (usernameToDelete.length() == 0) {
      res->setStatusCode(400);
      res->println("Missing user parameter");
      return;
  }

  // Prevent deleting self
  if (sessions[token].username == usernameToDelete) {
      res->setStatusCode(400);
      res->println("Cannot delete your own account");
      return;
  }

  deleteUser(usernameToDelete);
  res->setStatusCode(200);
  res->println("User deleted successfully. <a href='/admin'>Go back</a>");
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

  secureServer.registerNode(new ResourceNode("/delete", "GET", [](HTTPRequest *req, HTTPResponse *res){
    handleDeleteUser(req, res);
  }));


  secureServer.registerNode(new ResourceNode("/clear-nvs", "GET", [](HTTPRequest *req, HTTPResponse *res){
    String token = getCookie(req, "session");
    if (isCurrentUserAdmin(token)) {
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

