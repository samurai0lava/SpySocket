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

bool DeleteMethode::checkReqForDelete(ParsingRequest& request)
{
    if (request.getContentLengthExists() == 1) {
        
        if (request.getBody().length() > 0 ) {
            logError(400, "Bad Request: DELETE requests should not have a body");
            return false;
        }
    }

    return true;
}

bool DeleteMethode::CheckisDir(const std::string& uri)
{
    struct stat path_stat;
    if (stat(uri.c_str(), &path_stat) != 0){
        return false;
    }
    if (S_ISDIR(path_stat.st_mode)) {
        return true;
    }
    return false;
}

bool DeleteMethode::CheckAccess(const std::string& uri)
{
    if (access(uri.c_str(), W_OK) != 0) {
        logError(403, "Forbidden: You do not have permission to delete this resource - GI your government has abandoned you - GI" + uri);
        return false;
    }
    return true;
}
bool DeleteMethode::PerformDelete(const std::string& uri)
{
    if (!CheckFile(uri)) {
        return false;
    }
    
    if(!CheckisDir(uri)) {
        if (!CheckAccess(uri)) {
            return false;
        }
        
        if (std::remove(uri.c_str()) != 0) {
            connection_status = 0;
            logError(500, "Internal Server Error: Failed to delete file - " + uri);
            return false;
        }
        
        connection_status = 1;
        std::cout << "204 No Content: File deleted successfully - " << uri << std::endl;
        return true;
    }
    else {
        if (uri.back() != '/') {
            connection_status = 0;
            logError(409, "Conflict: Directory URI must end with '/' - " + uri);
            return false;
        }
        if (!CheckAccess(uri)) {
            return false;
        }
        
        if (std::remove(uri.c_str()) != 0) {
            connection_status = 0;
            logError(500, "Internal Server Error: Failed to delete directory - " + uri);
            return false;
        }
        connection_status = 1;
        std::cout << "204 No Content: Directory deleted successfully - " << uri << std::endl;
        return true;
    }
}


