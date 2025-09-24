#include "../../inc/webserv.hpp"

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
    if (!checkIfAllowed("DELETE", config, uri)) {
        std::string errorResponse = GenerateResErr(405);
        return errorResponse;
    }

    std::string actualPath = mapUriToPath(uri, config);
    if (actualPath.empty()) {
        std::string errorResponse = GenerateResErr(404);
        return errorResponse;
    }

    if (!CheckFile(actualPath)) {
        std::string errorResponse = GenerateResErr(404);
        return errorResponse;
    }

    if (!CheckisDir(actualPath))
    {
        if (!CheckAccess(actualPath)) {
            std::string errorResponse = GenerateResErr(403);
            return errorResponse;
        }

        if (std::remove(actualPath.c_str()) != 0) {
            std::string errorResponse = GenerateResErr(500);
            return errorResponse;
        }
        std::string successResponse = generate_success_resp();
        return successResponse;
    }
    else
    {
        if (uri[uri.length() - 1] != '/') {
            std::string errorResponse = GenerateResErr(409);
            return errorResponse;
        }

        if (!CheckAccess(actualPath)) {
            std::string errorResponse = GenerateResErr(403);
            return errorResponse;
        }

        if (std::remove(actualPath.c_str()) != 0) {
            std::string errorResponse = GenerateResErr(500);
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
