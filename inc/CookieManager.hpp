#ifndef COOKIE_MANAGER_HPP
#define COOKIE_MANAGER_HPP

#include <string>
#include <sstream>

class CookieManager {
public:
    static std::string generateSetCookieHeader(const std::string& name, const std::string& value);
    static std::string generateSimpleId();
};

#endif
