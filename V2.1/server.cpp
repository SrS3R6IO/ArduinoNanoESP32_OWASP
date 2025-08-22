#include "server.h"

std::string getParamFromBody(const std::string &body, const std::string &key) {
    size_t pos = body.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.length() + 1;
    size_t end = body.find("&", pos);
    return body.substr(pos, end - pos);
}