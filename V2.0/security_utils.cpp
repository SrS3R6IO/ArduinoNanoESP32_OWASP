#include "security_utils.h"


#define SALT_KEY "salt"
#define HASH_KEY "hash"

Preferences preferences;

String generateRandomSalt(size_t length) {
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  String salt = "";
  for (size_t i = 0; i < length; i++) {
    salt += charset[random(0, sizeof(charset) - 2)];
  }
  return salt;
}

String sha256(const String& password, const String& salt) {
  String combined = salt + password;
  byte hash[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts_ret(&ctx, 0);
  mbedtls_sha256_update_ret(&ctx, (const unsigned char*)combined.c_str(), combined.length());
  mbedtls_sha256_finish_ret(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  String hex = "";
  for (int i = 0; i < 32; i++) {
    if (hash[i] < 16) hex += "0";
    hex += String(hash[i], HEX);
  }
  return hex;
}

bool storeHashedPassword(const String& password) {
  String salt = generateRandomSalt();
  String hash = sha256(password, salt);

  preferences.begin("users", false);
  preferences.putString("admin/salt", salt);
  preferences.putString("admin/hash", hash);
  preferences.putString("admin/role", "admin");
  preferences.end();
  return true;
}



bool userExists(const String& username) {
  preferences.begin("users", true);
  bool exists = preferences.isKey((username + "/hash").c_str());
  preferences.end();
  return exists;
}

bool createUser(const String& username, const String& password, const String& role) {
  if (userExists(username)) return false; // avoid overwriting users

  String salt = generateRandomSalt();
  String hash = sha256(password, salt);

  preferences.begin("users", false);
  preferences.putString((username + "/hash").c_str(), hash);
  preferences.putString((username + "/salt").c_str(), salt);
  preferences.putString((username + "/role").c_str(), role);
  preferences.end();
  return true;
}

bool verifyUserPassword(const String& username, const String& password) {
  preferences.begin("users", true);
  if (!preferences.isKey((username + "/hash").c_str())) {
    preferences.end();
    return false;
  }
  String salt = preferences.getString((username + "/salt").c_str());
  String storedHash = preferences.getString((username + "/hash").c_str());
  preferences.end();

  String computedHash = sha256(password, salt);
  return storedHash == computedHash;
}



bool isPasswordSet() {
  preferences.begin("users", true);
  String userList = preferences.getString("user_list", "");
  preferences.end();
  return userList.length() > 0;
}



bool checkPassword(const String& inputPassword) {
  preferences.begin("auth", true);
  String salt = preferences.getString(SALT_KEY, "");
  String storedHash = preferences.getString(HASH_KEY, "");
  preferences.end();

  String inputHash = sha256(inputPassword, salt);
  return inputHash == storedHash;
}

void clearStoredPassword() {
  preferences.begin("auth", false);
  preferences.remove(HASH_KEY);
  preferences.remove(SALT_KEY);
  preferences.end();
}
