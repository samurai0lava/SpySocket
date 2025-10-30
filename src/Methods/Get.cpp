#include "../../inc/Get.hpp"
#include <cstdlib> // for std::strtoul
#include <errno.h> // for errno
#include <cstring> // for strerror


Get::Get(CClient& c) : client(c) {}


Get::~Get()
{
}

void printLocationStruct(const LocationStruct& loc) {
    std::cout << "LocationStruct {" << std::endl;

    if (loc.autoIndex == true)
        std::cout << "  autoIndex: true" << std::endl;
    else
        std::cout << "  autoIndex: false" << std::endl;

    std::cout << "  allowedMethods: ";
    for (std::set<std::string>::const_iterator it = loc.allowedMethods.begin();
        it != loc.allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << "  root: " << loc.root << std::endl;
    std::cout << "  indexPage: " << loc.indexPage << std::endl;

    std::cout << "  _return: ";
    for (size_t i = 0; i < loc._return.size(); ++i) {
        std::cout << "[" << loc._return[i].first << " => " << loc._return[i].second << "] ";
    }
    std::cout << std::endl;

    std::cout << "  cgi_path: ";
    for (size_t i = 0; i < loc.cgi_path.size(); ++i) {
        std::cout << loc.cgi_path[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  cgi_ext: ";
    for (size_t i = 0; i < loc.cgi_ext.size(); ++i) {
        std::cout << loc.cgi_ext[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  upload_enabled: " << (loc.upload_enabled ? "true" : "false") << std::endl;
    std::cout << "  upload_path: " << loc.upload_path << std::endl;

    std::cout << "}" << std::endl;
}
std::string Get::getMimeType(const std::string& path)
{
    size_t dot = path.find_last_of(".");
    if (dot == std::string::npos) return "text/html";

    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".mp4") return "video/mp4";
    if (ext == ".js") return "application/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    return "application/octet-stream";
}
std::string Get::matchLocation(const std::string& requestPath, const ConfigStruct& server)
{
    // Strip query string and fragment from the path before processing
    // Only the path component should be used for location matching
    std::string path = requestPath;
    size_t query_pos = path.find('?');
    size_t fragment_pos = path.find('#');
    size_t separator_pos = std::string::npos;

    if (query_pos != std::string::npos && fragment_pos != std::string::npos)
        separator_pos = (query_pos < fragment_pos) ? query_pos : fragment_pos;
    else if (query_pos != std::string::npos)
        separator_pos = query_pos;
    else if (fragment_pos != std::string::npos)
        separator_pos = fragment_pos;

    if (separator_pos != std::string::npos)
        path = path.substr(0, separator_pos);

    std::string removedSegment;
    std::string removedPath;
    while (true) {
        for (size_t i = 0; i < server.location.size(); ++i) {
            if (path == server.location[i].first) {

                if (server.location[i].second.allowedMethods.find("GET") == server.location[i].second.allowedMethods.end())
                {
                    throw std::runtime_error("Error 405 Method Not Allowed");
                }
                this->client._name_location = server.location[i].first;

                // Security check: validate the constructed path
                std::string fullPath = server.location[i].second.root + removedPath;
                // Extract the relative path part after root for security validation
                if (!is_path_secure(removedPath.empty() ? "" : removedPath.substr(1)))
                {
                    throw std::runtime_error("Error 403 Forbidden: Path traversal detected");
                }
                return fullPath;
            }
        }
        if (path == "/")
            break;
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash == 0)
        {
            removedSegment = path.substr(lastSlash);
            removedPath = removedSegment + removedPath;
            path = "/";
        }
        else
        {
            removedSegment = path.substr(lastSlash);
            removedPath = removedSegment + removedPath;
            path = path.substr(0, lastSlash);
        }
    }

    return "";
}
std::string Get::generateAutoIndex(const std::string& directoryPath)
{
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir)
        return "<html><body><h1>403 Forbidden</h1></body></html>";

    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html>\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>Index of " << directoryPath << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
    html << "ul { list-style: none; padding: 0; }\n";
    html << "li { margin: 5px 0; }\n";
    html << "a { text-decoration: none; color: #0066cc; padding: 5px; display: block; }\n";
    html << "a:hover { background-color: #f0f0f0; }\n";
    html << ".directory { font-weight: bold; }\n";
    html << "</style>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << directoryPath << "</h1>\n";
    html << "<ul>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        if (name == ".") continue; // skip current dir

        std::string fullPath = directoryPath + "/" + name;
        struct stat s;
        stat(fullPath.c_str(), &s);

        if (S_ISDIR(s.st_mode)) {
            html << "<li><a href=\"" << name << "/\">" << name << "/</a></li>\n";
        }
        else {
            html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
        }
    }

    closedir(dir);

    html << "</ul><hr></body></html>\n";
    return html.str();
}
bool Get::isDirectory(const std::string& path)
{
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}

bool Get::isFile(const std::string& path)
{
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
}

bool Get::pathExists(const std::string& path)
{
    struct stat s;
    return stat(path.c_str(), &s) == 0;
}

std::string Get::buildHttpHeaders(const std::string& path, size_t fileSize)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: " << this->getMimeType(path) << "\r\n";
    oss << "Content-Length: " << fileSize << "\r\n\r\n";
    return oss.str();
}
std::string Get::pathIsFile(std::string matchLocation)
{
    this->client.filePath = matchLocation;

    // Get file size using stat
    struct stat fileStat;
    if (stat(matchLocation.c_str(), &fileStat) == -1) {
        std::cerr << "Error getting file stats!" << std::endl;
         return getErrorPageFromConfig(500);
    }

    // Check for Range header for partial content requests
    std::map<std::string, std::string> headers = client.parser->getHeaders();
    bool hasRangeHeader = headers.find("range") != headers.end();
    size_t rangeStart = 0;
    size_t rangeEnd = fileStat.st_size - 1;
    bool isPartialContent = false;

    if (hasRangeHeader) {
        std::string rangeHeader = headers["range"];
        if (rangeHeader.find("bytes=") == 0) {
            std::string rangeSpec = rangeHeader.substr(6); // Remove "bytes="
            size_t dashPos = rangeSpec.find('-');
            if (dashPos != std::string::npos) {
                std::string startStr = rangeSpec.substr(0, dashPos);
                std::string endStr = rangeSpec.substr(dashPos + 1);

                if (!startStr.empty()) {
                    rangeStart = std::strtoul(startStr.c_str(), NULL, 10);
                }
                if (!endStr.empty()) {
                    rangeEnd = std::strtoul(endStr.c_str(), NULL, 10);
                }

                // Validate range
                if (rangeStart < (size_t)fileStat.st_size && rangeEnd < (size_t)fileStat.st_size && rangeStart <= rangeEnd) {
                    isPartialContent = true;
                }
            }
        }
    }

    // For large files or range requests, use different handling
    if (fileStat.st_size > 1024 * 1024 || isPartialContent)
    {

        std::ifstream file(matchLocation.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return getErrorPageFromConfig(500);
        }

        // Calculate content length for range
        size_t contentLength = rangeEnd - rangeStart + 1;
        // Seek to start position
        file.seekg(rangeStart);

        // Read the requested range
        std::vector<char> buffer(contentLength);
        file.read(&buffer[0], contentLength);
        size_t bytesRead = file.gcount();
        file.close();

        std::ostringstream response;
        if (isPartialContent) {
            response << "HTTP/1.1 206 Partial Content\r\n";
            response << "Content-Range: bytes " << rangeStart << "-" << (rangeStart + bytesRead - 1) << "/" << fileStat.st_size << "\r\n";
        }
        else {
            response << "HTTP/1.1 200 OK\r\n";
        }

        response << "Date: " << ft_time_format() << "\r\n";
        response << "Server: SpySocket/1.0\r\n";

        //  add id cookie
        std::string new_id = CookieManager::generateSimpleId();
        response << CookieManager::generateSetCookieHeader("id", new_id);
        response << "Content-Type: " << this->getMimeType(matchLocation) << "\r\n";
        response << "Content-Length: " << bytesRead << "\r\n";
        response << "Accept-Ranges: bytes\r\n";
        response << "Connection: close\r\n\r\n";

        // Add binary data
        std::string responseStr = response.str();
        responseStr.append(&buffer[0], bytesRead);

        this->client.chunkedSending = true;
        return responseStr;
    }

    std::ifstream file(matchLocation.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::string finalResponse = getErrorPageFromConfig(500);
        return (finalResponse);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::ostringstream response;
    response << "HTTP/1.1 200 OK \r\n";
    response << "Date: ";
    response << ft_time_format();
    response << "\r\n";
    response << "Server: SpySocket/1.0\r\n";
    // std::string current_id = this->client.parser->getId();
    // if (current_id.empty()) {
    std::string new_id = CookieManager::generateSimpleId();
    response << CookieManager::generateSetCookieHeader("id", new_id);
    // }
    response << "Content-type: " << this->getMimeType(matchLocation) << "\r\n";
    response << "Content-length: " << buffer.str().size() << "\r\n\r\n";
    response << buffer.str();
    this->client.chunkedSending = true;
    return (response.str());
}

std::string Get::handleDirectoryWithIndex(std::string indexPath)
{
    std::ifstream file(indexPath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return (getErrorPageFromConfig(500));
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    std::string new_id = CookieManager::generateSimpleId();
    response << CookieManager::generateSetCookieHeader("id", new_id);
    response << "Content-Type: " << this->getMimeType(indexPath) << "\r\n";
    response << "Content-Length: " << buffer.str().size() << "\r\n\r\n";
    response << buffer.str();
    return (response.str());
}
std::string Get::handleDirectoryWithAutoIndex(std::string matchLocation)
{
    std::string listing = this->generateAutoIndex(matchLocation);
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    std::string new_id = CookieManager::generateSimpleId();
    response << CookieManager::generateSetCookieHeader("id", new_id);
    response << "Content-Type: " << "text/html" << "\r\n";
    response << "Content-Length: " << listing.size() << "\r\n\r\n";
    response << listing;
    return (response.str());
}
std::string Get::MethodGet()
{
    if (this->client.uri.empty()) {
        std::cerr << "Empty URI in GET method" << std::endl;
        return (getErrorPageFromConfig(400));
    }
    std::string matchedLocation = matchLocation(this->client.uri, this->client.mutableConfig);

    bool found = false;
    LocationStruct locationMatched;
    for (size_t i = 0; i < this->client.mutableConfig.location.size(); i++)
    {
        if (this->client.mutableConfig.location[i].first == this->client._name_location)
        {
            locationMatched = this->client.mutableConfig.location[i].second;
            found = true;
            break;
        }
    }
    if (!found)
    {
        std::cerr << "No exact match for location : " << matchedLocation << std::endl;
        throw std::runtime_error("");
    }
    if (!locationMatched._return.empty()) {
		std::cout<< "Redirecting to " << locationMatched._return[0].second << std::endl;
        int statusCode = atoi(locationMatched._return[0].first.c_str());
        std::string target = locationMatched._return[0].second;
        this->client.chunkedSending = true;
        return buildRedirectResponse(statusCode, target);
    }
    if (!this->pathExists(matchedLocation))
    {
        std::string finalResponse = getErrorPageFromConfig(404);
        return (finalResponse);
    }
    if (this->isFile(matchedLocation)) {
        return(pathIsFile(matchedLocation));
    }
    else if (this->isDirectory(matchedLocation))
    {
        std::string indexPath = matchedLocation + "/" + locationMatched.indexPage;
        if (this->pathExists(indexPath) && this->isFile(indexPath))
            return(pathIsFile(indexPath));
        else if (locationMatched.autoIndex == true)
            return (this->handleDirectoryWithAutoIndex(matchedLocation));
    }
    return (getErrorPageFromConfig(403));
}


std::string Get::setupChunkedSending(const std::string& filePath)
{
    if (this->client.SendHeader == false)
    {
        struct stat s;
        if (stat(filePath.c_str(), &s) == -1) {
            return getErrorPageFromConfig(500);
        }
        this->client.fileSize = s.st_size;
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << "Content-Type: " << getMimeType(filePath) << "\r\n";
        oss << "Transfer-Encoding: chunked\r\n";
        oss << "\r\n";
        this->client.response = oss.str();
        this->client.SendHeader = true;
    }
    else
    {
        char buffer[this->client.chunkSize + 1];
        ft_memset(buffer, 0, this->client.chunkSize + 1);
        ssize_t bytesRead = read(this->client.fileFd, buffer, this->client.chunkSize);
        if (bytesRead == -1) {
            close(this->client.fileFd);
            return getErrorPageFromConfig(500);
        }
        else if (bytesRead == 0) {
            this->client.response = "0\r\n\r\n";
            close(this->client.fileFd);
            this->client.chunkedSending = true;
        }
        else {
            std::ostringstream oss;
            oss << std::hex << bytesRead << "\r\n";
            oss.write(buffer, bytesRead);
            oss << "\r\n";
            this->client.response = oss.str();
        }
    }
    return this->client.response;
}

void Get::printLocationStruct(const LocationStruct& loc)
{
    std::cout << "  LocationStruct {" << std::endl;
    std::cout << "    autoIndex: " << (loc.autoIndex ? "true" : "false") << std::endl;

    std::cout << "    allowedMethods: ";
    for (std::set<std::string>::const_iterator it = loc.allowedMethods.begin();
        it != loc.allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << "    root: " << loc.root << std::endl;
    std::cout << "    indexPage: " << loc.indexPage << std::endl;
    if (!loc.cgi_ext.empty())
        std::cout << "    cgi_ext: " << loc.cgi_ext[0] << std::endl;
    else
        std::cout << "    cgi_ext: (empty)" << std::endl;
    std::cout << "    upload_enabled: " << (loc.upload_enabled ? "true" : "false") << std::endl;
    std::cout << "    upload_path: " << loc.upload_path << std::endl;
    std::cout << "  }" << std::endl;
}

std::string Get::buildRedirectResponse(int statusCode, const std::string& target)
{
    std::ostringstream oss;
    std::string statusMessage = (statusCode == 301) ? "Moved Permanently" :
        (statusCode == 302) ? "Found" :
        (statusCode == 307) ? "Temporary Redirect" :
        "Redirect";
    oss << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    oss << "Location: " << target << "\r\n";
    oss << "Content-Length: 0\r\n";
    oss << "Connection: close\r\n\r\n";

    return oss.str();
}

std::string Get::getErrorPageFromConfig(int statusCode)
{
    for (size_t i = 0; i < this->client.mutableConfig.errorPage.size(); ++i)
    {
        if (std::atoi(this->client.mutableConfig.errorPage[i].first.c_str()) == statusCode)
        {
            std::string root = this->client.mutableConfig.root;
            std::string errorPage = this->client.mutableConfig.errorPage[i].second;

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
                this->client.chunkedSending = true;
                return response.str();
            }
        }
    }
    this->client.chunkedSending = true;
    return GenerateResErr(statusCode);
}

std::string Get::getStatusMessage(int statusCode)
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
