#include "../../inc/Delete.hpp"
#include "../../inc/webserv.hpp"




bool DeleteMethode::CheckFile(const std::string& uri)
{
    std::ifstream file(uri);
    if (!file) {
        logError(404, "Not Found: File does not exist - " + uri);
        return false;
    }
    file.close();
    return true;
}

bool DeleteMethode::PerformDelete(const std::string& uri)
{
    if (!CheckFile(uri)) {
        return false;
    }

    if (std::remove(uri.c_str()) != 0) {
        logError(500, "Internal Server Error: Failed to delete file - " + uri);
        return false;
    }

    return true;
}


