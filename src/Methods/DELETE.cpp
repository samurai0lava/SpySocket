#include "../../inc/webserv.hpp"
#include <errno.h> // for errno
#include <cstring> // for strerror

bool DeleteMethode::CheckFile(const std::string& uri)
{
    std::ifstream file(uri.c_str());
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

        if (request.getBody().length() > 0) {
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
    if (stat(uri.c_str(), &path_stat) != 0) {
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
        return false;
    }
    return true;
}


std::string DeleteMethode::PerformDelete(const std::string& uri, const ConfigStruct& config)
{
    std::pair<std::string, LocationStruct> location = get_location(uri,
            config);
    if (location.first.empty()) {
        std::string errorResponse = getErrorPageFromConfig(404, config);
        return errorResponse;
    }
    if(!location.second._return.empty())
    {
        return handle_redirect(location);
    }
    if (!checkIfAllowed("DELETE", config, uri)) {
        std::string errorResponse = getErrorPageFromConfig(405, config);
        return errorResponse;
    }
    std::string actualPath = mapUriToPath(uri, config);
    if (actualPath.empty()) {
        std::string errorResponse = getErrorPageFromConfig(404, config);
        return errorResponse;
    }
    if (!CheckFile(actualPath)) {
        std::string errorResponse = getErrorPageFromConfig(404, config);
        return errorResponse;
    }
    if (!CheckisDir(actualPath))
    {
        if (!CheckAccess(actualPath)) {
            std::string errorResponse = getErrorPageFromConfig(403, config);
            return errorResponse;
        }

        if (std::remove(actualPath.c_str()) != 0) {
            std::string errorResponse = getErrorPageFromConfig(500, config);
            return errorResponse;
        }
        std::string successResponse = generate_success_resp();
        return successResponse;
    }
    else
    {
        if (uri[uri.length() - 1] != '/') {
            std::string errorResponse = getErrorPageFromConfig(409, config);
            return errorResponse;
        }

        if (!CheckAccess(actualPath)) {
            std::string errorResponse = getErrorPageFromConfig(403, config);
            return errorResponse;
        }

        if (std::remove(actualPath.c_str()) != 0) {
            std::string errorResponse = getErrorPageFromConfig(500, config);
            return errorResponse;

        }
        std::string successResponse = generate_success_resp();
        return successResponse;
    }
}

std::string DeleteMethode::generate_success_resp()
{
    std::string response = "HTTP/1.1 204 No Content\r\n";
    response += "Date: ";
    response += ft_time_format();
    response += "\r\n";
    response += "Server: SpySocket/1.0\r\n";
    response += "Connection: close\r\n";
    std::string new_id = CookieManager::generateSimpleId();
    response += CookieManager::generateSetCookieHeader("id", new_id);
    response += "\r\n";
    return response;
}

bool DeleteMethode::checkIfAllowed(const std::string& method, const ConfigStruct& config, const std::string& uri) const
{
    std::string path = uri;

    while (true) {
        for (size_t i = 0; i < config.location.size(); ++i) {
            if (path == config.location[i].first) {
                const std::set<std::string>& allowedMethods = config.location[i].second.allowedMethods;
                return (allowedMethods.find(method) != allowedMethods.end()); // means DELETE is allowed
            }
        }
        if (path == "/")
            break;

        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash == 0) {
            path = "/";
        }
        else {
            path = path.substr(0, lastSlash);
        }
    }
    return false;
}

std::string DeleteMethode::mapUriToPath(const std::string& uri, const ConfigStruct& config) const
{
    std::string path = uri;
    std::string removedPath;

    while (true) {
        for (size_t i = 0; i < config.location.size(); ++i) {
            if (path == config.location[i].first) {
                return config.location[i].second.root + removedPath;
            }
        }

        if (path == "/")
            break;

        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash == 0) {
            std::string removedSegment = path.substr(lastSlash);
            removedPath = removedSegment + removedPath;
            path = "/";
        }
        else
        {
            std::string removedSegment = path.substr(lastSlash);
            removedPath = removedSegment + removedPath;
            path = path.substr(0, lastSlash);
        }
    }

    return "";
}

std::string DeleteMethode::getErrorPageFromConfig(int statusCode, const ConfigStruct& config)
{
    for (size_t i = 0; i < config.errorPage.size(); ++i)
    {
        if (std::atoi(config.errorPage[i].first.c_str()) == statusCode)
        {
            std::string root = config.root;
            std::string errorPage = config.errorPage[i].second;
            if (!root.empty() && root[root.length() - 1] != '/') {
                root += "/";
            }
            if (!errorPage.empty() && errorPage[0] == '/') {
                errorPage = errorPage.substr(1); 
            }
            std::string errorPagePath = root + errorPage;
            struct stat fileStat;
            if (stat(errorPagePath.c_str(), &fileStat) != 0) {
                continue; 
            }
            if (!S_ISREG(fileStat.st_mode)) {
                continue;
            }
            
            std::ifstream file(errorPagePath.c_str(), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();
                std::ostringstream response;
                response << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << buffer.str().size() << "\r\n\r\n";
                response << buffer.str();
                return response.str();
            }
        }
    }
    return GenerateResErr(statusCode);
}

std::string DeleteMethode::getStatusMessage(int statusCode)
{
    switch(statusCode)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Content Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        default: return "Unknown Error";
    }
}
