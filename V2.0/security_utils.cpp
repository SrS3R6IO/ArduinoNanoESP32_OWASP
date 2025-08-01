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
  preferences.begin("users", true);
  String salt = preferences.getString(SALT_KEY, "");
  String storedHash = preferences.getString(HASH_KEY, "");
  preferences.end();

  String inputHash = sha256(inputPassword, salt);
  return inputHash == storedHash;
}


#define TCP_TLS_PORT 1337

void storeTCPAuthToken(const String& token) {
  preferences.begin("security", false);
  preferences.putString("tcp_token", token);
  preferences.end();
}

String loadTCPAuthToken() {
  preferences.begin("security", true);
  String token = preferences.getString("tcp_token", "");
  preferences.end();
  return token;
}

void secureTCPServiceTask(void *parameter) {
  // Initialize mbedTLS components
  mbedtls_net_context listen_fd, client_fd;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_x509_crt srvcert;
  mbedtls_pk_context pkey;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  const char *pers = "tls_server";

  mbedtls_net_init(&listen_fd);
  mbedtls_net_init(&client_fd);
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_x509_crt_init(&srvcert);
  mbedtls_pk_init(&pkey);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  // Seed random generator
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                        (const unsigned char *)pers, strlen(pers));

  // Load server certificate
  mbedtls_x509_crt_parse(&srvcert, (const unsigned char *)server_cert,
                         strlen(server_cert) + 1);
  mbedtls_pk_parse_key(&pkey, (const unsigned char *)server_key,
                       strlen(server_key) + 1, NULL, 0);

  // Bind TCP socket
  mbedtls_net_bind(&listen_fd, NULL, std::to_string(TCP_TLS_PORT).c_str(), MBEDTLS_NET_PROTO_TCP);

  // TLS config
  mbedtls_ssl_config_defaults(&conf,
                              MBEDTLS_SSL_IS_SERVER,
                              MBEDTLS_SSL_TRANSPORT_STREAM,
                              MBEDTLS_SSL_PRESET_DEFAULT);

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
  mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
  mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey);

  mbedtls_ssl_setup(&ssl, &conf);

  while (1) {
    // Accept client
    mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL);
    mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    if (mbedtls_ssl_handshake(&ssl) != 0) {
      mbedtls_net_free(&client_fd);
      mbedtls_ssl_session_reset(&ssl);
      continue;
    }

    // Read encrypted line
    char buf[512];
    memset(buf, 0, sizeof(buf));
    int len = mbedtls_ssl_read(&ssl, (unsigned char *)buf, sizeof(buf) - 1);
    if (len <= 0) {
      mbedtls_ssl_close_notify(&ssl);
      mbedtls_net_free(&client_fd);
      mbedtls_ssl_session_reset(&ssl);
      continue;
    }

    String line = String(buf);
    line.trim();

    String command, args, rest, token;
    int cmdSpace = -1;
    
    // Validate
    int firstSpace = line.indexOf(' ');
    if (firstSpace == -1 || line.length() > 512) {
      mbedtls_ssl_write(&ssl, (const unsigned char *)"400 Bad Request\n", 17);
      goto cleanup;
    }

    token = line.substring(0, firstSpace);
    rest = line.substring(firstSpace + 1);
    if (token != loadTCPAuthToken()) {
      mbedtls_ssl_write(&ssl, (const unsigned char *)"403 Forbidden\n", 15);
      goto cleanup;
    }

    cmdSpace = rest.indexOf(' ');
    command = cmdSpace == -1 ? rest : rest.substring(0, cmdSpace);
    args = cmdSpace == -1 ? "" : rest.substring(cmdSpace + 1);

    if (command == "LEDON") {
      isLampOn = true;
      mbedtls_ssl_write(&ssl, (const unsigned char *)"OK: LED ON\n", 11);
    } else if (command == "LEDOFF") {
      isLampOn = false;
      mbedtls_ssl_write(&ssl, (const unsigned char *)"OK: LED OFF\n", 12);
    } else if (command == "ECHO") {
      String reply = "ECHO: " + args + "\n";
      mbedtls_ssl_write(&ssl, (const unsigned char *)reply.c_str(), reply.length());
    } else {
      mbedtls_ssl_write(&ssl, (const unsigned char *)"400 Unknown Command\n", 22);
    }

  cleanup:
    mbedtls_ssl_close_notify(&ssl);
    mbedtls_net_free(&client_fd);
    mbedtls_ssl_session_reset(&ssl);
  }

  vTaskDelete(NULL);
}


