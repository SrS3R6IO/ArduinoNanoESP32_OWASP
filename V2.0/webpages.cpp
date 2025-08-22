#include "webpages.h"

const char* login_form_html = R"rawliteral(
<html>
<head>
<style>
  body { font-family: Arial; background: #f2f2f2; text-align:center; margin-top:50px; }
  input { padding: 10px; margin: 5px; width: 200px; }
  button { padding: 10px 20px; margin-top: 10px; }
</style>
</head>
<body>
  <h2>Login</h2>
  <form action="/login" method="POST">
    <input type="text" name="user" placeholder="Username"><br>
    <input type="password" name="pass" placeholder="Password"><br>
    <button type="submit">Login</button>
  </form>
</body>
</html>
)rawliteral";

const char* admin_panel_html = R"rawliteral(
<html><body>
<h2>Admin Panel</h2>
<form method="POST" action="/register">
  New Password: <input name="pass" type="password"><br>
  <input type="submit" value="Register User">
</form>
</body></html>
)rawliteral";



const char* success_login_redirect_html = R"rawliteral(
<html>
<body>
  <h2>Login successful</h2>
    <p>Redirecting to panel...</p>
<meta http-equiv='refresh' content='1; url=/panel'>
</body>
</html>
)rawliteral";

const char* failed_login_html = R"rawliteral(
<html>
<body>
  <h2>Login failed</h2>
    <p>Incorrect username or password.</p>
<meta http-equiv='refresh' content='1; url=/login'>
</body>
</html>
)rawliteral";



const char* setup_form_html = R"rawliteral(
      <html><body>
      <h2>Initial Setup</h2>
      <form method='POST' action='/setup'>
        Username: <input type='text' name='username'><br>
        Password: <input type='password' name='password'><br>
        <input type='submit' value='Set Admin'>
      </form>
      </body></html>
    )rawliteral";


