#include "../../inc/CookieManager.hpp"
#include <ctime>
#include <cstdlib>

std::string CookieManager::generateSimpleId()
{
    std::srand(std::time(0));
    std::ostringstream oss;
    oss << (std::rand() % 900000 + 100000);
    return oss.str();
}

std::string CookieManager::generateSetCookieHeader(const std::string& name, const std::string& value)
{
    return "Set-Cookie: " + name + "=" + value + "; Path=/\r\n";
}
