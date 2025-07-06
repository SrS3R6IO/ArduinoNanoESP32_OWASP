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