#include "../../inc/Delete.hpp"
#include "../../inc/webserv.hpp"


bool DeleteMethode::CheckFile(const std::string& uri)
{
    std::ifstream file(uri);
    if (!file) {
        error_code = 404;
        error_message = "Not Found: File does not exist - " + uri;
        return false;
    }
    file.close();
    return true;
}

bool DeleteMethode::checkReqForDelete(ParsingRequest& request)
{
    if (request.getContentLengthExists() == 1) {
        
        if (request.getBody().length() > 0 ) {
            error_code = 400;
            error_message = "Bad Request: DELETE requests should not have a body";
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
        error_code = 403;
        error_message = "Forbidden: You do not have permission to delete this resource - " + uri;
        // logError(403, "Forbidden: You do not have permission to delete this
        // logError(403, "Forbidden: You do not have permission to delete this resource - GI your government has abandoned you - GI" + uri);
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
            error_code = 500;
            error_message = "Internal Server Error: Failed to delete file - " + uri;
            return false;
        }
        
        connection_status = 1;
        std::cout << "204 No Content: File deleted successfully - " << uri << std::endl;
        return true;
    }
    else {
        if (uri.back() != '/') {
            error_code = 409;
            error_message = "Conflict: Directory URI must end with '/' - " + uri;
            return false;
        }
        if (!CheckAccess(uri)) {
            return false;
        }
        
        if (std::remove(uri.c_str()) != 0) {
            error_code = 500;
            error_message = "Internal Server Error: Failed to delete directory - " + uri;
            return false;
        }
        status_code = 204;
        status_phrase = "No Content";
        return true;
    }
}



// //generate response for DELETE request
// std::string DeleteMethode::generate_resp(void)
// {
//     std::string response = "HTTP/1.1 204 No Content\r\n";
//     response += "Connection: close\r\n";
//     response += "Content-Length: 0\r\n";
//     response += "\r\n"; // End of headers
//     return response;
// }

// //generate error response for DELETE request
// std::string DeleteMethode::generate_error_resp(int error_code, const std::string& message)
// {
//     std::string response = "HTTP/1.1 " + std::to_string(error_code) + " Error\r\n";
//     response += "Content-Type: text/plain\r\n";
//     response += "Connection: close\r\n";
//     response += "Content-Length: " + std::to_string(message.length()) + "\r\n";
//     response += "\r\n";
//     response += message;
//     return response;
// }
